/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "BufferedSocket.h"

#include "TimerManager.h"
#include "SettingsManager.h"

#include "Streams.h"
#include "SSLSocket.h"
#include "CryptoManager.h"
#include "ZUtils.h"

#include "ThrottleManager.h"

namespace dcpp {

// Polling is used for tasks...should be fixed...
#define POLL_TIMEOUT 250

BufferedSocket::BufferedSocket(char aSeparator) throw(ThreadException) :
separator(aSeparator), mode(MODE_LINE), dataBytes(0), rollback(0), state(STARTING),
disconnecting(false)
{
	start();

	Thread::safeInc(sockets);
}

volatile long BufferedSocket::sockets = 0;

BufferedSocket::~BufferedSocket() throw() {
	Thread::safeDec(sockets);
}

void BufferedSocket::setMode (Modes aMode, size_t aRollback) {
	if (mode == aMode) {
		dcdebug ("WARNING: Re-entering mode %d\n", mode);
		return;
	}

	switch (aMode) {
		case MODE_LINE:
			rollback = aRollback;
			break;
		case MODE_ZPIPE:
			filterIn = std::auto_ptr<UnZFilter>(new UnZFilter);
			break;
		case MODE_DATA:
			break;
	}
	mode = aMode;
}

void BufferedSocket::setSocket(std::auto_ptr<Socket> s) {
	dcassert(!sock.get());
	if(SETTING(SOCKET_IN_BUFFER) > 0)
		s->setSocketOpt(SO_RCVBUF, SETTING(SOCKET_IN_BUFFER));
	if(SETTING(SOCKET_OUT_BUFFER) > 0)
		s->setSocketOpt(SO_SNDBUF, SETTING(SOCKET_OUT_BUFFER));
	s->setSocketOpt(SO_REUSEADDR, 1);	// NAT traversal

	inbuf.resize(s->getSocketOptInt(SO_RCVBUF));

	sock = s;
}

void BufferedSocket::accept(const Socket& srv, bool secure, bool allowUntrusted) throw(SocketException) {
	dcdebug("BufferedSocket::accept() %p\n", (void*)this);

	std::auto_ptr<Socket> s(secure ? CryptoManager::getInstance()->getServerSocket(allowUntrusted) : new Socket);

	s->accept(srv);

	setSocket(s);

	Lock l(cs);
	addTask(ACCEPTED, 0);
}

void BufferedSocket::connect(const string& aAddress, uint16_t aPort, bool secure, bool allowUntrusted, bool proxy) throw(SocketException) {
	connect(aAddress, aPort, 0, NAT_NONE, secure, allowUntrusted, proxy);
}

void BufferedSocket::connect(const string& aAddress, uint16_t aPort, uint16_t localPort, NatRoles natRole, bool secure, bool allowUntrusted, bool proxy) throw(SocketException) {
	dcdebug("BufferedSocket::connect() %p\n", (void*)this);
	std::auto_ptr<Socket> s(secure ? (natRole == NAT_SERVER ? CryptoManager::getInstance()->getServerSocket(allowUntrusted) : CryptoManager::getInstance()->getClientSocket(allowUntrusted)) : new Socket);

	s->create();
	setSocket(s);
	sock->bind(localPort, SETTING(BIND_ADDRESS));

	Lock l(cs);
	addTask(CONNECT, new ConnectInfo(aAddress, aPort, localPort, natRole, proxy && (SETTING(OUTGOING_CONNECTIONS) == SettingsManager::OUTGOING_SOCKS5)));
}

#define LONG_TIMEOUT 30000
#define SHORT_TIMEOUT 1000
void BufferedSocket::threadConnect(const string& aAddr, uint16_t aPort, uint16_t localPort, NatRoles natRole, bool proxy) throw(SocketException) {
	dcassert(state == STARTING);

	dcdebug("threadConnect %s:%d/%d\n", aAddr.c_str(), (int)localPort, (int)aPort);
	fire(BufferedSocketListener::Connecting());

	const uint64_t endTime = GET_TICK() + LONG_TIMEOUT;
	state = RUNNING;

	while (GET_TICK() < endTime) {
		dcdebug("threadConnect attempt to addr \"%s\"\n", aAddr.c_str());
		try {
			if(proxy) {
				sock->socksConnect(aAddr, aPort, LONG_TIMEOUT);
			} else {
				sock->connect(aAddr, aPort);
			}

			bool connSucceeded;
			while(!(connSucceeded = sock->waitConnected(POLL_TIMEOUT)) && endTime >= GET_TICK()) {
				if(disconnecting) return;
			}

			if (connSucceeded) {
				fire(BufferedSocketListener::Connected());
				return;
			}
		}
		catch (const SSLSocketException&) {
			throw;
		} catch (const SocketException&) {
			if (natRole == NAT_NONE)
				throw;
			Thread::sleep(SHORT_TIMEOUT);
		}
	}

	throw SocketException(_("Connection timeout"));
}

void BufferedSocket::threadAccept() throw(SocketException) {
	dcassert(state == STARTING);

	dcdebug("threadAccept\n");

	state = RUNNING;

	uint64_t startTime = GET_TICK();
	while(!sock->waitAccepted(POLL_TIMEOUT)) {
		if(disconnecting)
			return;

		if((startTime + 30000) < GET_TICK()) {
			throw SocketException(_("Connection timeout"));
		}
	}
}

void BufferedSocket::threadRead() throw(Exception) {
	if(state != RUNNING)
		return;

	int left = (mode == MODE_DATA) ? ThrottleManager::getInstance()->read(sock.get(), &inbuf[0], (int)inbuf.size()) : sock->read(&inbuf[0], (int)inbuf.size());
	if(left == -1) {
		// EWOULDBLOCK, no data received...
		return;
	} else if(left == 0) {
		// This socket has been closed...
		throw SocketException(_("Connection closed"));
	}
	string::size_type pos = 0;
	// always uncompressed data
	string l;
	int bufpos = 0, total = left;

	while (left > 0) {
		switch (mode) {
			case MODE_ZPIPE: {
					const int BUF_SIZE = 1024;
					// Special to autodetect nmdc connections...
					string::size_type pos = 0;
					boost::scoped_array<char> buffer(new char[BUF_SIZE]);
					l = line;
					// decompress all input data and store in l.
					while (left) {
						size_t in = BUF_SIZE;
						size_t used = left;
						bool ret = (*filterIn) (&inbuf[0] + total - left, used, &buffer[0], in);
						left -= used;
						l.append (&buffer[0], in);
						// if the stream ends before the data runs out, keep remainder of data in inbuf
						if (!ret) {
							bufpos = total-left;
							setMode (MODE_LINE, rollback);
							break;
						}
					}
					// process all lines
					while ((pos = l.find(separator)) != string::npos) {
                       	if(pos > 0) // check empty (only pipe) command and don't waste cpu with it ;o)
							fire(BufferedSocketListener::Line(), l.substr(0, pos));
						l.erase (0, pos + 1 /* separator char */);
					}
					// store remainder
					line = l;

					break;
				}
			case MODE_LINE:
				// Special to autodetect nmdc connections...
				if(separator == 0) {
					if(inbuf[0] == '$') {
						separator = '|';
					} else {
						separator = '\n';
					}
				}
				l = line + string ((char*)&inbuf[bufpos], left);
				while ((pos = l.find(separator)) != string::npos) {
	                if(pos > 0) // check empty (only pipe) command and don't waste cpu with it ;o)
						fire(BufferedSocketListener::Line(), l.substr(0, pos));
					l.erase (0, pos + 1 /* separator char */);
					if (l.length() < (size_t)left) left = l.length();
					if (mode != MODE_LINE) {
						// we changed mode; remainder of l is invalid.
						l.clear();
						bufpos = total - left;
						break;
					}
				}
				if (pos == string::npos) 
					left = 0;
				line = l;
				break;
			case MODE_DATA:
				while(left > 0) {
					if(dataBytes == -1) {
						fire(BufferedSocketListener::Data(), &inbuf[bufpos], left);
						bufpos += (left - rollback);
						left = rollback;
						rollback = 0;
					} else {
						int high = (int)min(dataBytes, (int64_t)left);
						fire(BufferedSocketListener::Data(), &inbuf[bufpos], high);
						bufpos += high;
						left -= high;

						dataBytes -= high;
						if(dataBytes == 0) {
							mode = MODE_LINE;
							fire(BufferedSocketListener::ModeChange());
						}
					}
				}
				break;
		}
	}

	if(mode == MODE_LINE && line.size() > static_cast<size_t>(SETTING(MAX_COMMAND_LENGTH))) {
		throw SocketException(_("Maximum command length exceeded"));
	}
}

void BufferedSocket::threadSendFile(InputStream* file) throw(Exception) {
	if(state != RUNNING)
		return;

	if(disconnecting)
		return;
	dcassert(file != NULL);
	size_t sockSize = (size_t)sock->getSocketOptInt(SO_SNDBUF);
	size_t bufSize = max(sockSize, (size_t)64*1024);

	ByteVector readBuf(bufSize);
	ByteVector writeBuf(bufSize);

	size_t readPos = 0;

	bool readDone = false;
	dcdebug("Starting threadSend\n");
	while(!disconnecting) {
		if(!readDone && readBuf.size() > readPos) {
			// Fill read buffer
			size_t bytesRead = readBuf.size() - readPos;
			size_t actual = file->read(&readBuf[readPos], bytesRead);

			if(bytesRead > 0) {
				fire(BufferedSocketListener::BytesSent(), bytesRead, 0);
			}

			if(actual == 0) {
				readDone = true;
			} else {
				readPos += actual;
			}
		}

		if(readDone && readPos == 0) {
			fire(BufferedSocketListener::TransmitDone());
			return;
		}

		readBuf.swap(writeBuf);
		readBuf.resize(bufSize);
		writeBuf.resize(readPos);
		readPos = 0;

		size_t writePos = 0, writeSize = 0;
		int written = 0;

		while(writePos < writeBuf.size()) {
			if(disconnecting)
				return;
			
			if(written == -1) {
				// workaround for OpenSSL (crashes when previous write failed and now retrying with different writeSize)
				written = sock->write(&writeBuf[writePos], writeSize);
			} else {
				writeSize = min(sockSize / 2, writeBuf.size() - writePos);	
				written = ThrottleManager::getInstance()->write(sock.get(), &writeBuf[writePos], writeSize);
			}
			
			if(written > 0) {
				writePos += written;

				fire(BufferedSocketListener::BytesSent(), 0, written);

			} else if(written == -1) {
				if(!readDone && readPos < readBuf.size()) {
					// Read a little since we're blocking anyway...
					size_t bytesRead = min(readBuf.size() - readPos, readBuf.size() / 2);
					size_t actual = file->read(&readBuf[readPos], bytesRead);

					if(bytesRead > 0) {
						fire(BufferedSocketListener::BytesSent(), bytesRead, 0);
					}

					if(actual == 0) {
						readDone = true;
					} else {
						readPos += actual;
					}
				} else {
					while(!disconnecting) {
						int w = sock->wait(POLL_TIMEOUT, Socket::WAIT_WRITE | Socket::WAIT_READ);
						if(w & Socket::WAIT_READ) {
							threadRead();
						}
						if(w & Socket::WAIT_WRITE) {
							break;
						}
					}
				}
			}
		}
	}
}

void BufferedSocket::write(const char* aBuf, size_t aLen) throw() {
	if(!sock.get())
		return;
	Lock l(cs);
	if(writeBuf.empty())
		addTask(SEND_DATA, 0);

	writeBuf.insert(writeBuf.end(), aBuf, aBuf+aLen);
}

void BufferedSocket::threadSendData() throw(Exception) {
	if(state != RUNNING)
		return;

	{
		Lock l(cs);
		if(writeBuf.empty())
			return;

		writeBuf.swap(sendBuf);
	}

	size_t left = sendBuf.size();
	size_t done = 0;
	while(left > 0) {
		if(disconnecting) {
			return;
		}

		int w = sock->wait(POLL_TIMEOUT, Socket::WAIT_READ | Socket::WAIT_WRITE);

		if(w & Socket::WAIT_READ) {
			threadRead();
		}

		if(w & Socket::WAIT_WRITE) {
			int n = sock->write(&sendBuf[done], left);
			if(n > 0) {
				left -= n;
				done += n;
			}
		}
	}
	sendBuf.clear();
}

bool BufferedSocket::checkEvents() throw(Exception) {
	while(state == RUNNING ? taskSem.wait(0) : taskSem.wait()) {
		pair<Tasks, boost::shared_ptr<TaskData> > p;
		{
			Lock l(cs);
			dcassert(tasks.size() > 0);
			p = tasks.front();
			tasks.erase(tasks.begin());
		}

		if(p.first == SHUTDOWN) {
			return false;
		} else if(p.first == UPDATED) {
			fire(BufferedSocketListener::Updated());
			continue;
		}

		if(state == STARTING) {
			if(p.first == CONNECT) {
				ConnectInfo* ci = static_cast<ConnectInfo*>(p.second.get());
				threadConnect(ci->addr, ci->port, ci->localPort, ci->natRole, ci->proxy);
			} else if(p.first == ACCEPTED) {
				threadAccept();
			} else {
				dcdebug("%d unexpected in STARTING state\n", p.first);
			}
		} else if(state == RUNNING) {
			if(p.first == SEND_DATA) {
				threadSendData();
			} else if(p.first == SEND_FILE) {
				threadSendFile(static_cast<SendFileInfo*>(p.second.get())->stream); break;
			} else if(p.first == DISCONNECT) {
				fail(_("Disconnected"));
			} else {
				dcdebug("%d unexpected in RUNNING state\n", p.first);
			}
		}
	}
	return true;
}

void BufferedSocket::checkSocket() throw(Exception) {
	int waitFor = sock->wait(POLL_TIMEOUT, Socket::WAIT_READ);

	if(waitFor & Socket::WAIT_READ) {
		threadRead();
	}
}

/**
 * Main task dispatcher for the buffered socket abstraction.
 * @todo Fix the polling...
 */
int BufferedSocket::run() {
	dcdebug("BufferedSocket::run() start %p\n", (void*)this);
	while(true) {
		try {
			if(!checkEvents()) {
				break;
			}
			if(state == RUNNING) {
				checkSocket();
			}
		} catch(const Exception& e) {
			fail(e.getError());
		}
	}
	dcdebug("BufferedSocket::run() end %p\n", (void*)this);
	delete this;
	return 0;
}

void BufferedSocket::fail(const string& aError) {
	if(sock.get()) {
		sock->disconnect();
	}

	if(state == RUNNING) {
		state = FAILED;
		fire(BufferedSocketListener::Failed(), aError);
	}
}

void BufferedSocket::shutdown() {
	Lock l(cs);
	disconnecting = true;
	addTask(SHUTDOWN, 0);
}

void BufferedSocket::addTask(Tasks task, TaskData* data) {
	dcassert(task == DISCONNECT || task == SHUTDOWN || task == UPDATED || sock.get());
	tasks.push_back(make_pair(task, data)); taskSem.signal();
}

} // namespace dcpp
