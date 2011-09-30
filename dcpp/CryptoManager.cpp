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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdinc.h"
#include "CryptoManager.h"

#include <boost/scoped_array.hpp>

#include "File.h"
#include "LogManager.h"
#include "ClientManager.h"
#include "version.h"

#include <openssl/bn.h>

#include <bzlib.h>

namespace dcpp {



CryptoManager::CryptoManager()
:
    certsLoaded(false),
    lock("EXTENDEDPROTOCOLABCABCABCABCABCABC"),
    pk("DCPLUSPLUS" VERSIONSTRING)
{
    SSL_library_init();

    clientContext.reset(SSL_CTX_new(TLSv1_client_method()));
    clientVerContext.reset(SSL_CTX_new(TLSv1_client_method()));
    serverContext.reset(SSL_CTX_new(TLSv1_server_method()));
    serverVerContext.reset(SSL_CTX_new(TLSv1_server_method()));

    if(clientContext && clientVerContext && serverContext && serverVerContext) {
        dh.reset(DH_new());

        static unsigned char dh4096_p[]={
                0xCA,0x35,0xA8,0xBB,0x65,0x33,0x28,0xC6,0x3F,0xD7,0x21,0x55,
                0x95,0xDF,0xC0,0xDC,0x11,0x10,0x23,0x2D,0x1E,0xD6,0x52,0x23,
                0xA1,0x52,0xB8,0xDD,0x4A,0x25,0xEE,0xF4,0x78,0xB6,0x89,0x9E,
                0xB6,0x33,0xEB,0x01,0xA6,0x46,0x31,0xD8,0x3D,0x12,0xB4,0x7B,
                0x1F,0x64,0x0C,0x84,0x10,0x80,0xFB,0x4F,0x74,0x21,0xA3,0x9B,
                0xF5,0x97,0xD1,0x05,0x97,0x9D,0x52,0x4F,0x91,0x3C,0xE1,0xA8,
                0x97,0xE0,0x33,0x9D,0xCB,0x9D,0x9D,0x2A,0xB5,0x3E,0xF5,0x8D,
                0x7F,0xEE,0x91,0xEE,0x4E,0xC5,0xA6,0xAB,0x54,0xD9,0xC2,0xA5,
                0x0D,0x2E,0xEA,0x1A,0x39,0xFD,0x30,0x4A,0x1C,0xB7,0x34,0x2B,
                0x7D,0x51,0xF6,0xB1,0xD1,0x8D,0xCE,0x28,0xDC,0xF9,0xDE,0x34,
                0xAF,0x1E,0xD1,0x7C,0xC2,0xD3,0x38,0x8E,0xBD,0x35,0x01,0x53,
                0xDD,0x2E,0xB5,0x83,0xC8,0xEF,0x08,0x15,0x59,0x6E,0xA3,0xC4,
                0x71,0x57,0x8C,0x4D,0xFD,0xA7,0x19,0x40,0x88,0x68,0x4E,0xD6,
                0x8F,0x5C,0xE5,0xEC,0xCF,0x5F,0xEB,0x9A,0xA4,0x66,0x40,0x8B,
                0x02,0x87,0xA7,0x3A,0x58,0x81,0xF1,0x6A,0x14,0x16,0x75,0x7D,
                0x33,0x01,0xEB,0x3F,0xA8,0x02,0xDC,0x09,0x32,0x12,0x00,0x20,
                0x47,0xFF,0x01,0x14,0xFE,0x9E,0x3A,0x44,0x4D,0xED,0x85,0xD5,
                0xDA,0x2F,0xE3,0x99,0xCE,0xDA,0x84,0x64,0xCB,0x0C,0x8C,0x00,
                0x90,0x19,0x70,0x1C,0x00,0x1D,0x63,0x3C,0x77,0x16,0x8D,0x3D,
                0x86,0x97,0x22,0x23,0x2F,0x7B,0xAB,0xB8,0xEB,0x94,0xA4,0x01,
                0xAA,0x34,0xBA,0xEA,0x7D,0x7A,0x37,0xB7,0x0C,0x75,0xEB,0x00,
                0x8D,0x52,0x7A,0xE2,0xDF,0x78,0x7C,0x4F,0x54,0x9E,0xA4,0xDD,
                0xC9,0xFC,0x08,0x7C,0x45,0x70,0x43,0x0F,0x39,0xE3,0x7E,0x48,
                0xB8,0xDC,0x9D,0xEC,0xB9,0x51,0x29,0x86,0x29,0x60,0xF6,0x4F,
                0xF7,0xCA,0xDD,0x3B,0x7F,0xAE,0xE2,0x54,0x4C,0x53,0x42,0x55,
                0xC0,0x39,0x24,0xE1,0x1A,0xAD,0x9E,0xCC,0x75,0x5E,0xF1,0xE2,
                0xD6,0xAE,0xCD,0x9A,0x91,0xC3,0x7B,0xE5,0x29,0xAD,0xCA,0xC2,
                0x00,0xC1,0xF9,0xF4,0x6D,0xD2,0x4B,0xD4,0x5A,0x56,0x39,0xCD,
                0xAC,0xCA,0xE7,0xD1,0x8C,0x15,0x4D,0x2B,0x59,0x67,0x29,0x72,
                0xE7,0x40,0x14,0x81,0x9E,0x26,0x48,0xF8,0x6C,0x51,0xF5,0xBE,
                0x64,0xD1,0xF4,0x4D,0x98,0xE7,0xFD,0x5E,0x23,0x1E,0xDF,0xBA,
                0xBD,0x2E,0xB1,0x81,0x26,0x98,0x9C,0x2F,0xE8,0xD5,0x32,0x6B,
                0x94,0x91,0x8C,0x2E,0xB8,0xD9,0xC9,0x2F,0x22,0x9D,0xA6,0x52,
                0x02,0xDF,0x99,0x63,0x64,0x7E,0xB8,0x68,0xAB,0x17,0x54,0x7E,
                0xED,0x9E,0xD1,0x45,0x64,0x36,0x65,0xE8,0x09,0x50,0xAB,0xB0,
                0xD4,0x8C,0x79,0x9F,0x4C,0xB8,0x26,0x45,0xBE,0x0F,0xDE,0x14,
                0x6F,0xEC,0x70,0x21,0x1A,0xA0,0x1D,0xD0,0x7D,0xA2,0x0F,0x85,
                0xA5,0x7C,0xC1,0x0A,0x74,0xB1,0x7B,0x5A,0xD2,0xC4,0x0F,0xD5,
                0x90,0x24,0x3E,0xEC,0x89,0x7E,0xB8,0xED,0x6E,0x19,0x85,0xB9,
                0x58,0x36,0xA1,0x33,0x7D,0x14,0xFE,0x4F,0x55,0xA9,0xB6,0x42,
                0x7E,0x97,0x2A,0x96,0x50,0x14,0x0D,0xEA,0x02,0xB1,0xD2,0x22,
                0xEB,0xE7,0xF4,0xAC,0xB6,0x37,0xCA,0xAB,0x4A,0x1E,0x4D,0x4E,
                0xCF,0xFE,0x5D,0xEF,0x23,0x78,0xC6,0xBB,
                };

        static unsigned char dh4096_g[]={
                0x02,
                };

        if(dh) {
            dh->p = BN_bin2bn(dh4096_p, sizeof(dh4096_p), 0);
            dh->g = BN_bin2bn(dh4096_g, sizeof(dh4096_g), 0);

            if (!dh->p || !dh->g) {
                dh.reset();
            } else {
                SSL_CTX_set_options(serverContext, SSL_OP_SINGLE_DH_USE);
                SSL_CTX_set_options(serverVerContext, SSL_OP_SINGLE_DH_USE);
                SSL_CTX_set_tmp_dh(serverContext, (DH*)dh);
                SSL_CTX_set_tmp_dh(serverVerContext, (DH*)dh);
            }
        }

        SSL_CTX_set_verify(serverContext, SSL_VERIFY_NONE, 0);
        SSL_CTX_set_verify(clientContext, SSL_VERIFY_NONE, 0);
        SSL_CTX_set_verify(clientVerContext, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
        SSL_CTX_set_verify(serverVerContext, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
    }
}

CryptoManager::~CryptoManager() {
}

bool CryptoManager::TLSOk() const noexcept {
    return BOOLSETTING(USE_TLS) && certsLoaded && !keyprint.empty();
}

void CryptoManager::generateCertificate() {
    // Generate certificate using OpenSSL
    if(SETTING(TLS_PRIVATE_KEY_FILE).empty()) {
        throw CryptoException(_("No private key file chosen"));
    }
    if(SETTING(TLS_CERTIFICATE_FILE).empty()) {
        throw CryptoException(_("No certificate file chosen"));
    }

    ssl::BIGNUM bn(BN_new());
    ssl::RSA rsa(RSA_new());
    ssl::EVP_PKEY pkey(EVP_PKEY_new());
    ssl::X509_NAME nm(X509_NAME_new());
    const EVP_MD *digest = EVP_sha1();
    ssl::X509 x509ss(X509_new());
    ssl::ASN1_INTEGER serial(ASN1_INTEGER_new());

    if(!bn || !rsa || !pkey || !nm || !x509ss || !serial) {
        throw CryptoException(_("Error generating certificate"));
    }

    int days = 10;
    int keylength = 2048;

#define CHECK(n) if(!(n)) { throw CryptoException(#n); }

    // Generate key pair
    CHECK((BN_set_word(bn, RSA_F4)))
    CHECK((RSA_generate_key_ex(rsa, keylength, bn, NULL)))
    CHECK((EVP_PKEY_set1_RSA(pkey, rsa)))

    // Set CID
    CHECK((X509_NAME_add_entry_by_txt(nm, "CN", MBSTRING_ASC,
        (const unsigned char*)ClientManager::getInstance()->getMyCID().toBase32().c_str(), -1, -1, 0)))

    // Prepare self-signed cert
    ASN1_INTEGER_set(serial, (long)Util::rand());
    CHECK((X509_set_serialNumber(x509ss, serial)))
    CHECK((X509_set_issuer_name(x509ss, nm)))
    CHECK((X509_set_subject_name(x509ss, nm)))
    CHECK((X509_gmtime_adj(X509_get_notBefore(x509ss), 0)))
    CHECK((X509_gmtime_adj(X509_get_notAfter(x509ss), (long)60*60*24*days)))
    CHECK((X509_set_pubkey(x509ss, pkey)))

    // Sign using own private key
    CHECK((X509_sign(x509ss, pkey, digest)))

#undef CHECK
    // Write the key and cert
    {
        File::ensureDirectory(SETTING(TLS_PRIVATE_KEY_FILE));
        FILE* f = fopen(SETTING(TLS_PRIVATE_KEY_FILE).c_str(), "w");
        if(!f) {
            return;
        }
        PEM_write_RSAPrivateKey(f, rsa, NULL, NULL, 0, NULL, NULL);
        fclose(f);
    }
    {
        File::ensureDirectory(SETTING(TLS_CERTIFICATE_FILE));
        FILE* f = fopen(SETTING(TLS_CERTIFICATE_FILE).c_str(), "w");
        if(!f) {
            File::deleteFile(SETTING(TLS_PRIVATE_KEY_FILE));
            return;
        }
        PEM_write_X509(f, x509ss);
        fclose(f);
    }
}

void CryptoManager::loadCertificates() noexcept {
    if(!BOOLSETTING(USE_TLS) || !clientContext || !clientVerContext || !serverContext || !serverVerContext)
        return;

    keyprint.clear();
    certsLoaded = false;

    const string& cert = SETTING(TLS_CERTIFICATE_FILE);
    const string& key = SETTING(TLS_PRIVATE_KEY_FILE);

    if(cert.empty() || key.empty()) {
        LogManager::getInstance()->message(_("TLS disabled, no certificate file set"));
        return;
    }

    if(File::getSize(cert) == -1 || File::getSize(key) == -1 || !checkCertificate()) {
        // Try to generate them...
        try {
            generateCertificate();
            LogManager::getInstance()->message(_("Generated new TLS certificate"));
        } catch(const CryptoException& e) {
            LogManager::getInstance()->message(str(F_("TLS disabled, failed to generate certificate: %1%") % e.getError()));
        }
    }

    if(SSL_CTX_use_certificate_file(serverContext, cert.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }
    if(SSL_CTX_use_certificate_file(clientContext, cert.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }

    if(SSL_CTX_use_certificate_file(serverVerContext, cert.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }
    if(SSL_CTX_use_certificate_file(clientVerContext, cert.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }

    if(SSL_CTX_use_PrivateKey_file(serverContext, key.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }
    if(SSL_CTX_use_PrivateKey_file(clientContext, key.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }

    if(SSL_CTX_use_PrivateKey_file(serverVerContext, key.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }
    if(SSL_CTX_use_PrivateKey_file(clientVerContext, key.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }

    StringList certs = File::findFiles(SETTING(TLS_TRUSTED_CERTIFICATES_PATH), "*.pem");
    StringList certs2 = File::findFiles(SETTING(TLS_TRUSTED_CERTIFICATES_PATH), "*.crt");
    certs.insert(certs.end(), certs2.begin(), certs2.end());

    for(StringIter i = certs.begin(); i != certs.end(); ++i) {
        if(
            SSL_CTX_load_verify_locations(clientContext, i->c_str(), NULL) != SSL_SUCCESS ||
            SSL_CTX_load_verify_locations(clientVerContext, i->c_str(), NULL) != SSL_SUCCESS ||
            SSL_CTX_load_verify_locations(serverContext, i->c_str(), NULL) != SSL_SUCCESS ||
            SSL_CTX_load_verify_locations(serverVerContext, i->c_str(), NULL) != SSL_SUCCESS
        ) {
            LogManager::getInstance()->message("Failed to load trusted certificate from " + *i);
        }
    }

    loadKeyprint(cert.c_str());

    certsLoaded = true;
}

bool CryptoManager::checkCertificate() noexcept {
    FILE* f = fopen(SETTING(TLS_CERTIFICATE_FILE).c_str(), "r");
    if(!f) {
        return false;
    }

    X509* tmpx509 = NULL;
    PEM_read_X509(f, &tmpx509, NULL, NULL);
    fclose(f);

    if(!tmpx509) {
        return false;
    }
    ssl::X509 x509(tmpx509);

    ASN1_INTEGER* sn = X509_get_serialNumber(x509);
    if(!sn || !ASN1_INTEGER_get(sn)) {
        return false;
    }

    X509_NAME* name = X509_get_subject_name(x509);
    if(!name) {
        return false;
    }
    int i = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
    if(i == -1) {
        return false;
    }
    X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, i);
    ASN1_STRING* str = X509_NAME_ENTRY_get_data(entry);
    if(!str) {
        return false;
    }

    unsigned char* buf = 0;
    i = ASN1_STRING_to_UTF8(&buf, str);
    if(i < 0) {
        return false;
    }
    std::string cn((char*)buf, i);
    OPENSSL_free(buf);

    if(cn != ClientManager::getInstance()->getMyCID().toBase32()) {
        return false;
    }

    ASN1_TIME* t = X509_get_notAfter(x509);
    if(t) {
        if(X509_cmp_current_time(t) < 0) {
            return false;
        }
    }
    return true;
}

const vector<uint8_t>& CryptoManager::getKeyprint() const noexcept {
        return keyprint;
}

void CryptoManager::loadKeyprint(const string& file) noexcept {
        FILE* f = fopen(SETTING(TLS_CERTIFICATE_FILE).c_str(), "r");
        if(!f) {
                return;
        }

        X509* tmpx509 = NULL;
        PEM_read_X509(f, &tmpx509, NULL, NULL);
        fclose(f);

        if(!tmpx509) {
                return;
        }

        ssl::X509 x509(tmpx509);

        keyprint = ssl::X509_digest(x509, EVP_sha256());
}

SSLSocket* CryptoManager::getClientSocket(bool allowUntrusted) {
    return new SSLSocket(allowUntrusted ? clientContext : clientVerContext);
}
SSLSocket* CryptoManager::getServerSocket(bool allowUntrusted) {
    return new SSLSocket(allowUntrusted ? serverContext : serverVerContext);
}


void CryptoManager::decodeBZ2(const uint8_t* is, size_t sz, string& os) {
    bz_stream bs = { 0 };

    if(BZ2_bzDecompressInit(&bs, 0, 0) != BZ_OK)
        throw CryptoException(_("Error during decompression"));

    // We assume that the files aren't compressed more than 2:1...if they are it'll work anyway,
    // but we'll have to do multiple passes...
    size_t bufsize = 2*sz;
    boost::scoped_array<char> buf(new char[bufsize]);

    bs.avail_in = sz;
    bs.avail_out = bufsize;
    bs.next_in = reinterpret_cast<char*>(const_cast<uint8_t*>(is));
    bs.next_out = &buf[0];

    int err;

    os.clear();

    while((err = BZ2_bzDecompress(&bs)) == BZ_OK) {
        if (bs.avail_in == 0 && bs.avail_out > 0) { // error: BZ_UNEXPECTED_EOF
            BZ2_bzDecompressEnd(&bs);
            throw CryptoException(_("Error during decompression"));
        }
        os.append(&buf[0], bufsize-bs.avail_out);
        bs.avail_out = bufsize;
        bs.next_out = &buf[0];
    }

    if(err == BZ_STREAM_END)
        os.append(&buf[0], bufsize-bs.avail_out);

    BZ2_bzDecompressEnd(&bs);

    if(err < 0) {
        // This was a real error
        throw CryptoException(_("Error during decompression"));
    }
}

string CryptoManager::keySubst(const uint8_t* aKey, size_t len, size_t n) {
    boost::scoped_array<uint8_t> temp(new uint8_t[len + n * 10]);

    size_t j=0;

    for(size_t i = 0; i<len; i++) {
        if(isExtra(aKey[i])) {
            temp[j++] = '/'; temp[j++] = '%'; temp[j++] = 'D';
            temp[j++] = 'C'; temp[j++] = 'N';
            switch(aKey[i]) {
            case 0: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '0'; break;
            case 5: temp[j++] = '0'; temp[j++] = '0'; temp[j++] = '5'; break;
            case 36: temp[j++] = '0'; temp[j++] = '3'; temp[j++] = '6'; break;
            case 96: temp[j++] = '0'; temp[j++] = '9'; temp[j++] = '6'; break;
            case 124: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '4'; break;
            case 126: temp[j++] = '1'; temp[j++] = '2'; temp[j++] = '6'; break;
            }
            temp[j++] = '%'; temp[j++] = '/';
        } else {
            temp[j++] = aKey[i];
        }
    }
    return string((const char*)&temp[0], j);
}

string CryptoManager::makeKey(const string& aLock) {
    if(aLock.size() < 3)
        return Util::emptyString;

    boost::scoped_array<uint8_t> temp(new uint8_t[aLock.length()]);
    uint8_t v1;
    size_t extra=0;

    v1 = (uint8_t)(aLock[0]^5);
    v1 = (uint8_t)(((v1 >> 4) | (v1 << 4)) & 0xff);
    temp[0] = v1;

    string::size_type i;

    for(i = 1; i<aLock.length(); i++) {
        v1 = (uint8_t)(aLock[i]^aLock[i-1]);
        v1 = (uint8_t)(((v1 >> 4) | (v1 << 4))&0xff);
        temp[i] = v1;
        if(isExtra(temp[i]))
            extra++;
    }

    temp[0] = (uint8_t)(temp[0] ^ temp[aLock.length()-1]);

    if(isExtra(temp[0])) {
        extra++;
    }

    return keySubst(&temp[0], aLock.length(), extra);
}

} // namespace dcpp
