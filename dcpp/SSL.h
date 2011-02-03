/*
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

#ifndef SSL_H_
#define SSL_H_

#include <openssl/ssl.h>

namespace dcpp {

namespace ssl {

template<typename T, void (*Release)(T*)>
class scoped_handle {
public:
	explicit scoped_handle(T* t_ = 0) : t(t_) { }
	~scoped_handle() { Release(t); }

	operator T*() { return t; }
	operator const T*() const { return t; }

	T* operator->() { return t; }
	const T* operator->() const { return t; }

	void reset(T* t_ = NULL) { Release(t); t = t_; }
private:
	scoped_handle(const scoped_handle<T, Release>&);
	scoped_handle<T, Release>& operator=(const scoped_handle<T, Release>&);

	T* t;
};

typedef scoped_handle<ASN1_INTEGER, ASN1_INTEGER_free> ASN1_INTEGER;
typedef scoped_handle<BIGNUM, BN_free> BIGNUM;
typedef scoped_handle<DH, DH_free> DH;
typedef scoped_handle<DSA, DSA_free> DSA;
typedef scoped_handle<EVP_PKEY, EVP_PKEY_free> EVP_PKEY;
typedef scoped_handle<RSA, RSA_free> RSA;
typedef scoped_handle<SSL, SSL_free> SSL;
typedef scoped_handle<SSL_CTX, SSL_CTX_free> SSL_CTX;
typedef scoped_handle<X509, X509_free> X509;
typedef scoped_handle<X509_NAME, X509_NAME_free> X509_NAME;

vector<uint8_t> X509_digest(::X509* x509, const ::EVP_MD* md);

}
}
#endif /*SSL_H_*/
