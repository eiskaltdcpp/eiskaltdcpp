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

#include "stdinc.h"

#include "UserConnection.h"
#include "ClientManager.h"

#include "StringTokenizer.h"
#include "AdcCommand.h"
#include "Transfer.h"
#ifdef LUA_SCRIPT
#include "ScriptManager.h"
#endif
#include "DebugManager.h"
#include "ConnectionManager.h"

namespace dcpp {

const string UserConnection::FEATURE_MINISLOTS = "MiniSlots";
const string UserConnection::FEATURE_XML_BZLIST = "XmlBZList";
const string UserConnection::FEATURE_ADCGET = "ADCGet";
const string UserConnection::FEATURE_ZLIB_GET = "ZLIG";
const string UserConnection::FEATURE_TTHL = "TTHL";
const string UserConnection::FEATURE_TTHF = "TTHF";
const string UserConnection::FEATURE_ADC_BAS0 = "BAS0";
const string UserConnection::FEATURE_ADC_BASE = "BASE";
const string UserConnection::FEATURE_ADC_BZIP = "BZIP";
const string UserConnection::FEATURE_ADC_TIGR = "TIGR";

const string UserConnection::FILE_NOT_AVAILABLE = "File Not Available";

const string UserConnection::UPLOAD = "Upload";
const string UserConnection::DOWNLOAD = "Download";

void UserConnection::on(BufferedSocketListener::Line, const string& aLine) throw () {
    if(aLine.length() < 2) {
        fire(UserConnectionListener::ProtocolError(), this, _("Invalid data"));
        return;
    }

    if(aLine[0] == 'C' && !isSet(FLAG_NMDC)) {
        if(!Text::validateUtf8(aLine)) {
            fire(UserConnectionListener::ProtocolError(), this, _("Non-UTF-8 data in an ADC connection"));
            return;
        }
        COMMAND_DEBUG(aLine, DebugManager::CLIENT_IN, getRemoteIp());
        dispatch(aLine);
        return;
    } else if(aLine[0] == '$') {
        setFlag(FLAG_NMDC);
    } else {
        fire(UserConnectionListener::ProtocolError(), this, _("Invalid data"));
        return;
    }
    COMMAND_DEBUG(aLine, DebugManager::CLIENT_IN, getRemoteIp());
    string cmd;
    string param;

    string::size_type x;
#ifdef LUA_SCRIPT
    if(onUserConnectionMessageIn(this, aLine)) {
        disconnect(true);
        return;
    }
#endif

    if( (x = aLine.find(' ')) == string::npos) {
        cmd = aLine;
    } else {
        cmd = aLine.substr(0, x);
        param = aLine.substr(x+1);
    }

    if(cmd == "$MyNick") {
        if(!param.empty())
            fire(UserConnectionListener::MyNick(), this, param);
    } else if(cmd == "$Direction") {
        x = param.find(" ");
        if(x != string::npos) {
            fire(UserConnectionListener::Direction(), this, param.substr(0, x), param.substr(x+1));
        }
    } else if(cmd == "$Error") {
        if(Util::stricmp(param.c_str(), FILE_NOT_AVAILABLE) == 0 ||
            param.rfind(/*path/file*/" no more exists") != string::npos) {
            fire(UserConnectionListener::FileNotAvailable(), this);
        } else if (::strncmp(param.c_str(), "CTM2HUB", 7) == 0 ) {
            ConnectionManager::getInstance()->addCTM2HUB(socket->getIp(), Util::toString(getPort()));
            fire(UserConnectionListener::ProtocolError(), this, param);
        } else {
            fire(UserConnectionListener::ProtocolError(), this, param);
        }
    } else if(cmd == "$GetListLen") {
        fire(UserConnectionListener::GetListLength(), this);
    } else if(cmd == "$Get") {
        x = param.find('$');
        if(x != string::npos) {
            fire(UserConnectionListener::Get(), this, Text::toUtf8(param.substr(0, x), encoding), Util::toInt64(param.substr(x+1)) - (int64_t)1);
        }
    } else if(cmd == "$Key") {
        if(!param.empty())
            fire(UserConnectionListener::Key(), this, param);
    } else if(cmd == "$Lock") {
        if(!param.empty()) {
            x = param.find(" Pk=");
            if(x != string::npos) {
                fire(UserConnectionListener::CLock(), this, param.substr(0, x), param.substr(x + 4));
            } else {
                // Workaround for faulty linux clients...
                x = param.find(' ');
                if(x != string::npos) {
                    setFlag(FLAG_INVALIDKEY);
                    fire(UserConnectionListener::CLock(), this, param.substr(0, x), Util::emptyString);
                } else {
                    fire(UserConnectionListener::CLock(), this, param, Util::emptyString);
                }
            }
        }
    } else if(cmd == "$Send") {
        fire(UserConnectionListener::Send(), this);
    } else if(cmd == "$MaxedOut") {
        fire(UserConnectionListener::MaxedOut(), this);
    } else if(cmd == "$Supports") {
        if(!param.empty()) {
            fire(UserConnectionListener::Supports(), this, StringTokenizer<string>(param, ' ').getTokens());
        }
    } else if(cmd.compare(0, 4, "$ADC") == 0) {
        dispatch(aLine, true);
    } else {
        fire(UserConnectionListener::ProtocolError(), this, _("Invalid data"));
    }
}

#ifdef LUA_SCRIPT
bool UserConnectionScriptInstance::onUserConnectionMessageIn(UserConnection* aConn, const string& aLine) {
    Lock l(cs);
    MakeCall("dcpp", "UserDataIn", 1, aConn, aLine);
    return GetLuaBool();
}

bool UserConnectionScriptInstance::onUserConnectionMessageOut(UserConnection* aConn, const string& aLine) {
    Lock l(cs);
    MakeCall("dcpp", "UserDataOut", 1, aConn, aLine);
    return GetLuaBool();
}
#endif

void UserConnection::connect(const string& aServer, uint16_t aPort, uint16_t localPort, BufferedSocket::NatRoles natRole) throw(SocketException, ThreadException) {
    dcassert(!socket);

    setPort(aPort);
    socket = BufferedSocket::getSocket(0);
    socket->addListener(this);
    socket->connect(aServer, aPort, localPort, natRole, isSet(FLAG_SECURE), BOOLSETTING(ALLOW_UNTRUSTED_CLIENTS), true);
}

void UserConnection::accept(const Socket& aServer) throw(SocketException, ThreadException) {
    dcassert(!socket);
    socket = BufferedSocket::getSocket(0);
    socket->addListener(this);
    socket->accept(aServer, isSet(FLAG_SECURE), BOOLSETTING(ALLOW_UNTRUSTED_CLIENTS));
}

void UserConnection::inf(bool withToken) {
    AdcCommand c(AdcCommand::CMD_INF);
    c.addParam("ID", ClientManager::getInstance()->getMyCID().toBase32());
    if(withToken) {
        c.addParam("TO", getToken());
    }
    send(c);
}

void UserConnection::sup(const StringList& features) {
    AdcCommand c(AdcCommand::CMD_SUP);
    for(StringIterC i = features.begin(); i != features.end(); ++i)
        c.addParam(*i);
    send(c);
}

void UserConnection::supports(const StringList& feat) {
    const string x = Util::toString(" ",feat);
    send("$Supports " + x + '|');
}

void UserConnection::handle(AdcCommand::STA t, const AdcCommand& c) {
    if(c.getParameters().size() >= 2) {
        const string& code = c.getParam(0);
        if(!code.empty() && code[0] - '0' == AdcCommand::SEV_FATAL) {
            fire(UserConnectionListener::ProtocolError(), this, c.getParam(1));
            return;
        }
    }

    fire(t, this, c);
}

void UserConnection::on(Connected) noexcept {
    lastActivity = GET_TICK();
    fire(UserConnectionListener::Connected(), this);
}

void UserConnection::on(Data, uint8_t* data, size_t len) noexcept {
    lastActivity = GET_TICK();
    fire(UserConnectionListener::Data(), this, data, len);
}

void UserConnection::on(BytesSent, size_t bytes, size_t actual) noexcept {
    lastActivity = GET_TICK();
    fire(UserConnectionListener::BytesSent(), this, bytes, actual);
}

void UserConnection::on(ModeChange) noexcept {
    lastActivity = GET_TICK();
    fire(UserConnectionListener::ModeChange(), this);
}

void UserConnection::on(TransmitDone) noexcept {
    fire(UserConnectionListener::TransmitDone(), this);
}

void UserConnection::on(Updated) noexcept {
    fire(UserConnectionListener::Updated(), this);
}

void UserConnection::on(Failed, const string& aLine) noexcept {
    setState(STATE_UNCONNECTED);
    fire(UserConnectionListener::Failed(), this, aLine);

    delete this;
}

// # ms we should aim for per segment
static const int64_t SEGMENT_TIME = 120*1000;
static const int64_t MIN_CHUNK_SIZE = 64*1024;

void UserConnection::updateChunkSize(int64_t leafSize, int64_t lastChunk, uint64_t ticks) {

    if(chunkSize == 0) {
        chunkSize = std::max((int64_t)64*1024, std::min(lastChunk, (int64_t)1024*1024));
        return;
    }

    if(ticks <= 10) {
        // Can't rely on such fast transfers - double
        chunkSize *= 2;
        return;
    }

    double lastSpeed = (1000. * lastChunk) / ticks;

    int64_t targetSize = chunkSize;

    // How long current chunk size would take with the last speed...
    double msecs = 1000 * targetSize / lastSpeed;

    if(msecs < SEGMENT_TIME / 4) {
        targetSize *= 2;
    } else if(msecs < SEGMENT_TIME / 1.25) {
        targetSize += leafSize;
    } else if(msecs < SEGMENT_TIME * 1.25) {
        // We're close to our target size - don't change it
    } else if(msecs < SEGMENT_TIME * 4) {
        targetSize = std::max(MIN_CHUNK_SIZE, targetSize - chunkSize);
    } else {
        targetSize = std::max(MIN_CHUNK_SIZE, targetSize / 2);
    }

    chunkSize = targetSize;
}

} // namespace dcpp
