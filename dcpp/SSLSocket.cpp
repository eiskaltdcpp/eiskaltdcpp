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

#include "SSLSocket.h"
#include "LogManager.h"
#include "SettingsManager.h"
#include "format.h"

#include <openssl/err.h>

namespace dcpp {

SSLSocket::SSLSocket(SSL_CTX* context) throw(SocketException) : ctx(context), ssl(0) {

}

void SSLSocket::connect(const string& aIp, uint16_t aPort) throw(SocketException) {
	Socket::connect(aIp, aPort);

	waitConnected(0);
}

bool SSLSocket::waitConnected(uint32_t millis) {
	if(!ssl) {
		if(!Socket::waitConnected(millis)) {
			return false;
		}
		ssl.reset(SSL_new(ctx));
		if(!ssl)
			checkSSL(-1);

		checkSSL(SSL_set_fd(ssl, sock));
	}

	if(SSL_is_init_finished(ssl)) {
		return true;
	}

	while(true) {
		int ret = ssl->server?SSL_accept(ssl):SSL_connect(ssl);
		if(ret == 1) {
			dcdebug("Connected to SSL server using %s as %s\n", SSL_get_cipher(ssl), ssl->server?"server":"client");
			return true;
		}
		if(!waitWant(ret, millis)) {
			return false;
		}
	}
}

void SSLSocket::accept(const Socket& listeningSocket) throw(SocketException) {
	Socket::accept(listeningSocket);

	waitAccepted(0);
}

bool SSLSocket::waitAccepted(uint32_t millis) {
	if(!ssl) {
		if(!Socket::waitAccepted(millis)) {
			return false;
		}
		ssl.reset(SSL_new(ctx));
		if(!ssl)
			checkSSL(-1);

		checkSSL(SSL_set_fd(ssl, sock));
	}

	if(SSL_is_init_finished(ssl)) {
		return true;
	}

	while(true) {
		int ret = SSL_accept(ssl);
		if(ret == 1) {
			dcdebug("Connected to SSL client using %s\n", SSL_get_cipher(ssl));
			return true;
		}
		if(!waitWant(ret, millis)) {
			return false;
		}
	}
}

bool SSLSocket::waitWant(int ret, uint32_t millis) {
	int err = SSL_get_error(ssl, ret);
	switch(err) {
	case SSL_ERROR_WANT_READ:
		return wait(millis, Socket::WAIT_READ) == WAIT_READ;
	case SSL_ERROR_WANT_WRITE:
		return wait(millis, Socket::WAIT_WRITE) == WAIT_WRITE;
	// Check if this is a fatal error...
	default: checkSSL(ret);
	}
	dcdebug("SSL: Unexpected fallthrough");
	// There was no error?
	return true;
}

int SSLSocket::read(void* aBuffer, int aBufLen) throw(SocketException) {
	if(!ssl) {
		return -1;
	}
	int len = checkSSL(SSL_read(ssl, aBuffer, aBufLen));

	if(len > 0) {
		stats.totalDown += len;
		//dcdebug("In(s): %.*s\n", len, (char*)aBuffer);
	}
	return len;
}

int SSLSocket::write(const void* aBuffer, int aLen) throw(SocketException) {
	if(!ssl) {
		return -1;
	}
	int ret = checkSSL(SSL_write(ssl, aBuffer, aLen));
	if(ret > 0) {
		stats.totalUp += ret;
		//dcdebug("Out(s): %.*s\n", ret, (char*)aBuffer);
	}
	return ret;
}

int SSLSocket::checkSSL(int ret) throw(SocketException) {
	if(!ssl) {
		return -1;
	}
	if(ret <= 0) {
		int err = SSL_get_error(ssl, ret);
		switch(err) {
			case SSL_ERROR_NONE:		// Fallthrough - YaSSL doesn't for example return an openssl compatible error on recv fail
			case SSL_ERROR_WANT_READ:	// Fallthrough
			case SSL_ERROR_WANT_WRITE:
				return -1;
			case SSL_ERROR_ZERO_RETURN:
				throw SocketException(_("Connection closed"));
			default:
				{
					ssl.reset();
					// @todo replace 80 with MAX_ERROR_SZ or whatever's appropriate for yaSSL in some nice way...
					char errbuf[80];
					throw SSLSocketException(str(F_("SSL Error: %1% (%2%, %3%)") % ERR_error_string(err, errbuf) % ret % err));
				}
		}
	}
	return ret;
}

int SSLSocket::wait(uint32_t millis, int waitFor) throw(SocketException) {
	if(ssl && (waitFor & Socket::WAIT_READ)) {
		/** @todo Take writing into account as well if reading is possible? */
		char c;
		if(SSL_peek(ssl, &c, 1) > 0)
			return WAIT_READ;
	}
	return Socket::wait(millis, waitFor);
}

bool SSLSocket::isTrusted() const throw() {
	if(!ssl) {
		return false;
	}

	if(SSL_get_verify_result(ssl) != X509_V_OK) {
		return false;
	}

	X509* cert = SSL_get_peer_certificate(ssl);
	if(!cert) {
		return false;
	}
	X509_free(cert);
	return true;
}

std::string SSLSocket::getCipherName() const throw() {
	if(!ssl)
		return Util::emptyString;

	return SSL_get_cipher_name(ssl);
}

vector<uint8_t> SSLSocket::getKeyprint() const throw() {
	if(!ssl)
		return vector<uint8_t>();
	X509* x509 = SSL_get_peer_certificate(ssl);
	if(!x509)
		return vector<uint8_t>();

	return ssl::X509_digest(x509, EVP_sha256());
}

void SSLSocket::shutdown() throw() {
	if(ssl)
		SSL_shutdown(ssl);
}

void SSLSocket::close() throw() {
	if(ssl) {
		ssl.reset();
	}
	Socket::shutdown();
	Socket::close();
}

} // namespace dcpp
