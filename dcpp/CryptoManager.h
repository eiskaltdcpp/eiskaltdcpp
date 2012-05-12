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

#include "SettingsManager.h"
#include "Exception.h"
#include "Singleton.h"
#include "SSLSocket.h"

namespace dcpp {

STANDARD_EXCEPTION(CryptoException);

class CryptoManager : public Singleton<CryptoManager>
{
public:
    string makeKey(const string& aLock);
    const string& getLock() { return lock; }
    const string& getPk() { return pk; }
    bool isExtended(const string& aLock) { return strncmp(aLock.c_str(), "EXTENDEDPROTOCOL", 16) == 0; }

    void decodeBZ2(const uint8_t* is, size_t sz, string& os);

    SSLSocket* getClientSocket(bool allowUntrusted);
    SSLSocket* getServerSocket(bool allowUntrusted);

    void loadCertificates() noexcept;
    void generateCertificate();
    bool checkCertificate() noexcept;
    const vector<uint8_t>& getKeyprint() const noexcept;

    bool TLSOk() const noexcept;
private:

    friend class Singleton<CryptoManager>;

    CryptoManager();
    virtual ~CryptoManager();

    ssl::SSL_CTX clientContext;
    ssl::SSL_CTX clientVerContext;
    ssl::SSL_CTX serverContext;
    ssl::SSL_CTX serverVerContext;

    ssl::DH dh;

    bool certsLoaded;

    vector<uint8_t> keyprint;
    const string lock;
    const string pk;

    string keySubst(const uint8_t* aKey, size_t len, size_t n);
    bool isExtra(uint8_t b) {
        return (b == 0 || b==5 || b==124 || b==96 || b==126 || b==36);
    }
    void loadKeyprint(const string& file) noexcept;
};

} // namespace dcpp
