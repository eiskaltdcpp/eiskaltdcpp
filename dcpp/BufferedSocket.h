/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include <deque>
#include <memory>
#include "typedefs.h"
#include "BufferedSocketListener.h"
#include "Semaphore.h"
#include "Thread.h"
#include "Speaker.h"
#include "Util.h"
#include "Socket.h"
#include "Atomic.h"

namespace dcpp {
using std::deque;
using std::function;
using std::pair;
using std::unique_ptr;

class BufferedSocket : public Speaker<BufferedSocketListener>, private Thread {
public:
    enum Modes {
        MODE_LINE,
        MODE_ZPIPE,
        MODE_DATA
    };

    enum NatRoles {
        NAT_NONE,
        NAT_CLIENT,
        NAT_SERVER
    };

    /**
     * BufferedSocket factory, each BufferedSocket may only be used to create one connection
     * @param sep Line separator
     * @return An unconnected socket
     */
    static BufferedSocket* getSocket(char sep, bool v4only = false) {
        return new BufferedSocket(sep, v4only);
    }

    static void putSocket(BufferedSocket* aSock) {
        if(aSock) {
            aSock->removeListeners();
            aSock->shutdown();
        }
    }

    static void waitShutdown() {
        while(sockets > 0)
            Thread::sleep(100);
    }

    uint16_t accept(const Socket& srv, bool secure, bool allowUntrusted);
    void connect(const string& aAddress, const string& aPort, bool secure, bool allowUntrusted, bool proxy);
    void connect(const string& aAddress, const string& aPort, const string& localPort, NatRoles natRole, bool secure, bool allowUntrusted, bool proxy);

    /** Sets data mode for aBytes bytes. Must be called within onLine. */
    void setDataMode(int64_t aBytes = -1) { mode = MODE_DATA; dataBytes = aBytes; }
    /**
     * Rollback is an ugly hack to solve problems with compressed transfers where not all data received
     * should be treated as data.
     * Must be called from within onData.
     */
    void setLineMode(size_t aRollback) { setMode (MODE_LINE, aRollback);}
    void setMode(Modes mode, size_t aRollback = 0);
    Modes getMode() const { return mode; }
    const string& getIp() const { return sock->getIp(); }

    bool isSecure() const { return sock->isSecure(); }
    bool isTrusted() const { return sock->isTrusted(); }
    std::string getCipherName() const { return sock->getCipherName(); }
    vector<uint8_t> getKeyprint() const { return sock->getKeyprint(); }

    void write(const string& aData) { write(aData.data(), aData.length()); }
    void write(const char* aBuf, size_t aLen) noexcept;
    /** Send the file f over this socket. */
    void transmitFile(InputStream* f) { Lock l(cs); addTask(SEND_FILE, new SendFileInfo(f)); }

    /** Call a function from the socket's thread. */
    void callAsync(function<void ()> f) { Lock l(cs); addTask(ASYNC_CALL, new CallData(f)); }

    void disconnect(bool graceless = false) noexcept { Lock l(cs); if(graceless) disconnecting = true; addTask(DISCONNECT, 0); }

    string getLocalIp() const { return sock->getLocalIp(); }
    uint16_t getLocalPort() const { return sock->getLocalPort(); }


    GETSET(char, separator, Separator)
private:
    enum Tasks {
        CONNECT,
        DISCONNECT,
        SEND_DATA,
        SEND_FILE,
        SHUTDOWN,
        ACCEPTED,
        ASYNC_CALL
    };

    enum State {
        STARTING, // Waiting for CONNECT/ACCEPTED/SHUTDOWN
        RUNNING,
        FAILED
    };

    struct TaskData {
        virtual ~TaskData() { }
    };
    struct ConnectInfo : public TaskData {
        ConnectInfo(string addr_, string port_, string localPort_, NatRoles natRole_, bool proxy_) : addr(addr_), port(port_), localPort(localPort_), natRole(natRole_), proxy(proxy_) { }
        string addr;
        string port;
        string localPort;
        NatRoles natRole;
        bool proxy;
    };
    struct SendFileInfo : public TaskData {
        SendFileInfo(InputStream* stream_) : stream(stream_) { }
        InputStream* stream;
    };
    struct CallData : public TaskData {
        CallData(function<void ()> f) : f(f) { }
        function<void ()> f;
    };
    BufferedSocket(char aSeparator, bool v4only);

    virtual ~BufferedSocket();

    CriticalSection cs;

    Semaphore taskSem;
    deque<pair<Tasks, unique_ptr<TaskData> > > tasks;

    Modes mode;
    std::unique_ptr<UnZFilter> filterIn;
    int64_t dataBytes;
    size_t rollback;
    string line;
    ByteVector inbuf;
    ByteVector writeBuf;
    ByteVector sendBuf;

    std::unique_ptr<Socket> sock;
    State state;
    bool disconnecting;
    bool v4only;

    virtual int run();

    void threadConnect(const string& aAddr, const string& aPort, const string& localPort, NatRoles natRole, bool proxy);
    void threadAccept();
    void threadRead();
    void threadSendFile(InputStream* is);
    void threadSendData();

    void fail(const string& aError);
    static Atomic<long,memory_ordering_strong> sockets;

    bool checkEvents();
    void checkSocket();

    void setSocket(std::unique_ptr<Socket>&& s);
    void setOptions();
    void shutdown();
    void addTask(Tasks task, TaskData* data);
};

} // namespace dcpp
