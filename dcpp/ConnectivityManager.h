/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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

#ifndef DCPLUSPLUS_DCPP_CONNECTIVITY_MANAGER_H
#define DCPLUSPLUS_DCPP_CONNECTIVITY_MANAGER_H

#include "Util.h"
#include "Speaker.h"
#include "Singleton.h"

namespace dcpp {

class ConnectivityManagerListener {
public:
   virtual ~ConnectivityManagerListener() { }
   template<int I> struct X { enum { TYPE = I }; };

   typedef X<0> Message;
   typedef X<1> Finished;

   virtual void on(Message, const string&) throw() { }
   virtual void on(Finished) throw() { }
};

class ConnectivityManager : public Singleton<ConnectivityManager>, public Speaker<ConnectivityManagerListener>
{
public:
   void detectConnection();
   void setup(bool settingsChanged, int lastConnectionMode);
        bool isRunning() const { return running; }

private:
   friend class Singleton<ConnectivityManager>;
        friend class UPnPManager;

   ConnectivityManager();
   virtual ~ConnectivityManager() throw() { }

        void mappingFinished(bool success);
        void log(const string& msg);

   void startSocket();
   void listen();
   void disconnect();

   bool autoDetected;
        bool running;
};

} // namespace dcpp

#endif // !defined(CONNECTIVITY_MANAGER_H)
