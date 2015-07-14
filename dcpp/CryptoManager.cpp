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
#include "CryptoManager.h"

#include "File.h"
#include "LogManager.h"
#include "ClientManager.h"
#include "version.h"

#include <openssl/bn.h>

#include <bzlib.h>

namespace dcpp {

static const char ciphersuites[] =
"ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-GCM-SHA256:"
"ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA256:"
"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256:"
"ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA256:"
"ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:"
"!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";

CryptoManager::CryptoManager()
:
    certsLoaded(false),
    lock("EXTENDEDPROTOCOLABCABCABCABCABCABC"),
    pk("DCPLUSPLUS" VERSIONSTRING)
{
    SSL_library_init();

    clientContext.reset(SSL_CTX_new(SSLv23_client_method()));
    clientVerContext.reset(SSL_CTX_new(SSLv23_client_method()));
    serverContext.reset(SSL_CTX_new(SSLv23_server_method()));
    serverVerContext.reset(SSL_CTX_new(SSLv23_server_method()));

    if(clientContext && clientVerContext && serverContext && serverVerContext) {

        SSL_CTX_set_options(clientContext, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
        SSL_CTX_set_cipher_list(clientContext, ciphersuites);
        SSL_CTX_set1_curves_list(clientContext, "P-256");
        SSL_CTX_set_options(serverContext, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
        SSL_CTX_set_cipher_list(serverContext, ciphersuites);
        SSL_CTX_set1_curves_list(serverContext, "P-256");
        SSL_CTX_set_options(clientVerContext, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
        SSL_CTX_set_cipher_list(clientVerContext, ciphersuites);
        SSL_CTX_set1_curves_list(clientVerContext, "P-256");
        SSL_CTX_set_options(serverVerContext, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
        SSL_CTX_set_cipher_list(serverVerContext, ciphersuites);
        SSL_CTX_set1_curves_list(serverVerContext, "P-256");

        EC_KEY* tmp_ecdh;
        if ((tmp_ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1/* is not secure more secure is  NID_secp384r1 or NID_secp521r1*/)) != NULL) {
            SSL_CTX_set_options(serverContext, SSL_OP_SINGLE_ECDH_USE);
            SSL_CTX_set_tmp_ecdh(serverContext, tmp_ecdh);
            SSL_CTX_set_options(serverVerContext, SSL_OP_SINGLE_ECDH_USE);
            SSL_CTX_set_tmp_ecdh(serverVerContext, tmp_ecdh);

            EC_KEY_free(tmp_ecdh);
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

    if(!ssl::SSL_CTX_use_certificate_file(serverContext, cert.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }
    if(!ssl::SSL_CTX_use_certificate_file(clientContext, cert.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }

    if(!ssl::SSL_CTX_use_certificate_file(serverVerContext, cert.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }
    if(!ssl::SSL_CTX_use_certificate_file(clientVerContext, cert.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load certificate file"));
        return;
    }

    if(!ssl::SSL_CTX_use_PrivateKey_file(serverContext, key.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }
    if(!ssl::SSL_CTX_use_PrivateKey_file(clientContext, key.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }

    if(!ssl::SSL_CTX_use_PrivateKey_file(serverVerContext, key.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }
    if(!ssl::SSL_CTX_use_PrivateKey_file(clientVerContext, key.c_str(), SSL_FILETYPE_PEM)) {
        LogManager::getInstance()->message(_("Failed to load private key"));
        return;
    }

    StringList certs = File::findFiles(SETTING(TLS_TRUSTED_CERTIFICATES_PATH), "*.pem");
    StringList certs2 = File::findFiles(SETTING(TLS_TRUSTED_CERTIFICATES_PATH), "*.crt");
    certs.insert(certs.end(), certs2.begin(), certs2.end());

    for(auto& i: certs) {
        if(
            SSL_CTX_load_verify_locations(clientContext, i.c_str(), NULL) != SSL_SUCCESS ||
            SSL_CTX_load_verify_locations(clientVerContext, i.c_str(), NULL) != SSL_SUCCESS ||
            SSL_CTX_load_verify_locations(serverContext, i.c_str(), NULL) != SSL_SUCCESS ||
            SSL_CTX_load_verify_locations(serverVerContext, i.c_str(), NULL) != SSL_SUCCESS
        ) {
            LogManager::getInstance()->message(str(F_("Failed to load trusted certificate from %1%") % Util::addBrackets(i)));
        }
    }

    loadKeyprint(cert.c_str());

    certsLoaded = true;
}

bool CryptoManager::checkCertificate() noexcept {
    auto x509 = ssl::getX509(SETTING(TLS_CERTIFICATE_FILE).c_str());
    if(!x509) {
        return false;
    }
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
    auto x509 = ssl::getX509(SETTING(TLS_CERTIFICATE_FILE).c_str());
    if(x509) {
        keyprint = ssl::X509_digest(x509, EVP_sha256());
    }
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
    std::unique_ptr<char[]> buf(new char[bufsize]);

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
    std::unique_ptr<uint8_t[]> temp(new uint8_t[len + n * 10]);
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

    std::unique_ptr<uint8_t[]> temp(new uint8_t[aLock.length()]);
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
