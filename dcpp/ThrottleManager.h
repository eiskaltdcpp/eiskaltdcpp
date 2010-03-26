/*
 * Copyright (C) 2009 Big Muscle
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _THROTTLEMANAGER_H
#define _THROTTLEMANAGER_H

#include "DownloadManager.h"
#include "Singleton.h"
#include "Socket.h"
#include "Thread.h"
#include "TimerManager.h"
#include "UploadManager.h"

/**
 * Manager for limiting traffic flow.
 * Inspired by Token Bucket algorithm: http://en.wikipedia.org/wiki/Token_bucket
 */
namespace dcpp{
#define CONDWAIT_TIMEOUT 250
    class ThrottleManager : public Singleton<ThrottleManager>, private TimerManagerListener
    {
    private:

        // download limiter
        int                         downLimit;
        int64_t                     downTokens;
        CriticalSection downCS;
        // upload limiter
        int                         upLimit;
        int64_t                     upTokens;
        CriticalSection upCS;
        // stack up throttled read & write threads
        CriticalSection stateCS;
        CriticalSection waitCS[2];
        long activeWaiter;

        void shutdown() {
            Lock l(stateCS);
            if (activeWaiter != -1) {
                waitCS[activeWaiter].leave();
                activeWaiter = -1;
            }
        }
        void waitToken() {
            // no tokens, wait for them, so long as throttling still active
            // avoid keeping stateCS lock on whole function
            CriticalSection *curCS = 0;
            {
                Lock l(stateCS);
                if (activeWaiter != -1)
                    curCS = &waitCS[activeWaiter];
            }
            // possible post-CS aW shifts: 0->1/1->0: lock lands in wrong place, will
            // either fall through immediately or wait depending on whether in
            // stateCS-protected transition elsewhere; 0/1-> -1: falls through. Both harmless.
            if (curCS)
                Lock l(*curCS);
        }

        friend class Singleton<ThrottleManager>;

        // constructor
        ThrottleManager(void) : downTokens(0), upTokens(0), downLimit(0), upLimit(0)
        {
            TimerManager::getInstance()->addListener(this);
        }

        // destructor
        ~ThrottleManager(void)
        {
            shutdown();
            TimerManager::getInstance()->removeListener(this);
        }

        // TimerManagerListener
        void on(TimerManagerListener::Minute, uint32_t aTick) throw()//[+] IRainman, merge
        {
            if (!BOOLSETTING(THROTTLE_ENABLE))
                return;

            // alternative limiter
            if (Util::checkLimiterTime()) {
                SettingsManager::getInstance()->set(SettingsManager::MAX_UPLOAD_SPEED_LIMIT, SETTING(MAX_UPLOAD_SPEED_LIMIT_TIME));
                SettingsManager::getInstance()->set(SettingsManager::MAX_DOWNLOAD_SPEED_LIMIT, SETTING(MAX_DOWNLOAD_SPEED_LIMIT_TIME));
            } else {
                SettingsManager::getInstance()->set(SettingsManager::MAX_UPLOAD_SPEED_LIMIT, SETTING(MAX_UPLOAD_SPEED_LIMIT_NORMAL));
                SettingsManager::getInstance()->set(SettingsManager::MAX_DOWNLOAD_SPEED_LIMIT, SETTING(MAX_DOWNLOAD_SPEED_LIMIT_NORMAL));
            }
        }

        void on(TimerManagerListener::Second, uint32_t aTick) throw() {
            if (!BOOLSETTING(THROTTLE_ENABLE))
                return;

            downLimit   = SETTING(MAX_DOWNLOAD_SPEED_LIMIT);
            upLimit     = SETTING(MAX_UPLOAD_SPEED_LIMIT);

            {
                Lock l(downCS);
                if(downLimit > 0)
                    downTokens = downLimit * 1024;
                else
                    downTokens = -1;
            }

            {
                Lock l(upCS);
                if(upLimit > 0)
                    upTokens = upLimit * 1024;
                else
                    upTokens = -1;
            }

            // let existing events drain out (fairness).
            // www.cse.wustl.edu/~schmidt/win32-cv-1.html documents various
            // fairer strategies, but when only broadcasting, irrelevant
            {
                Lock l(stateCS);

                dcassert(activeWaiter == 0 || activeWaiter == 1);
                waitCS[1-activeWaiter].enter();
                Thread::safeExchange(activeWaiter, 1-activeWaiter);
                waitCS[1-activeWaiter].leave();
            }
        }
    public:

        /*
 * Throttles traffic and reads a packet from the network
 */
        int read(Socket* sock, void* buffer, size_t len) {
            int64_t readSize = -1;
            size_t downs = DownloadManager::getInstance()->getDownloadCount();
            if(!SETTING(THROTTLE_ENABLE) || downTokens == -1 || downs == 0)
                return sock->read(buffer, len);

            {
                Lock l(downCS);

                if(downTokens > 0)
                {
                    int64_t slice = (SETTING(MAX_DOWNLOAD_SPEED_LIMIT) * 1024) / downs;
                    readSize = min(slice, min(static_cast<int64_t>(len), downTokens));

                    // read from socket
                    readSize = sock->read(buffer, static_cast<size_t>(readSize));

                    if(readSize > 0)
                        downTokens -= readSize;
                }
            }

            if(readSize != -1)
            {
                Thread::yield(); // give a chance to other transfers to get a token
                return readSize;
            }

            waitToken();
            return -1;  // from BufferedSocket: -1 = retry, 0 = connection close
        }

        /*
     * Limits a traffic and writes a packet to the network
     * We must handle this a little bit differently than downloads, because of that stupidity in OpenSSL
     */
        int write(Socket* sock, void* buffer, size_t& len) {
            bool gotToken = false;
            size_t ups = UploadManager::getInstance()->getUploadCount();
            if(!SETTING(THROTTLE_ENABLE) || upTokens == -1 || ups == 0)
                return sock->write(buffer, len);

            {
                Lock l(upCS);

                if(upTokens > 0)
                {
                    size_t slice = (SETTING(MAX_DOWNLOAD_SPEED_LIMIT) * 1024) / ups;
                    len = min(slice, min(len, static_cast<size_t>(upTokens)));
                    upTokens -= len;

                    gotToken = true; // token successfuly assigned
                }
            }

            if(gotToken)
            {
                // write to socket
                int sent = sock->write(buffer, len);

                Thread::yield(); // give a chance to other transfers get a token
                return sent;
            }

            waitToken();
            return 0;   // from BufferedSocket: -1 = failed, 0 = retry
        }

    };
}
#endif  // _THROTTLEMANAGER_H
