/* This file is part of the KDE libraries
   Copyright (C) 2001 George Staikos <staikos@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#undef crypt
#endif

#include "kopenssl.h"

#ifdef HAVE_SSL
extern "C" {
static int (*K_SSL_connect)     (SSL *ssl) = NULL;
static int (*K_SSL_read)        (SSL *ssl, void *buf, int num) = NULL;
static int (*K_SSL_write)       (SSL *ssl, const void *buf, int num) = NULL;
static SSL *(*K_SSL_new)        (SSL_CTX *ctx) = NULL;
static void (*K_SSL_free)       (SSL *ssl) = NULL;
static int (*K_SSL_shutdown)    (SSL *ssl) = NULL;
static SSL_CTX *(*K_SSL_CTX_new)(SSL_METHOD *method) = NULL;
static void (*K_SSL_CTX_free)   (SSL_CTX *ctx) = NULL;
static int (*K_SSL_set_fd)      (SSL *ssl, int fd) = NULL;
static int (*K_SSL_pending)     (SSL *ssl) = NULL;
static int (*K_SSL_CTX_set_cipher_list)(SSL_CTX *ctx, const char *str) = NULL;
static void (*K_SSL_CTX_set_verify)(SSL_CTX *ctx, int mode,
                         int (*verify_callback)(int, X509_STORE_CTX *)) = NULL;
static int (*K_SSL_CTX_use_certificate)(SSL_CTX *ctx, X509 *x) = NULL;
static SSL_CIPHER *(*K_SSL_get_current_cipher)(SSL *ssl) = NULL;
static long (*K_SSL_ctrl)      (SSL *ssl,int cmd, long larg, char *parg) = NULL;
static int (*K_RAND_egd)        (const char *path) = NULL;
static SSL_METHOD * (*K_TLSv1_client_method) () = NULL;
static SSL_METHOD * (*K_SSLv2_client_method) () = NULL;
static SSL_METHOD * (*K_SSLv3_client_method) () = NULL;
static SSL_METHOD * (*K_SSLv23_client_method) () = NULL;
static X509 * (*K_SSL_get_peer_certificate) (SSL *) = NULL;
static int (*K_SSL_CIPHER_get_bits) (SSL_CIPHER *,int *) = NULL;
static char * (*K_SSL_CIPHER_get_version) (SSL_CIPHER *) = NULL;
static const char * (*K_SSL_CIPHER_get_name) (SSL_CIPHER *) = NULL;
static char * (*K_SSL_CIPHER_description) (SSL_CIPHER *, char *, int) = NULL;
static X509 * (*K_d2i_X509) (X509 **,unsigned char **,long) = NULL;
static int (*K_i2d_X509) (X509 *,unsigned char **) = NULL;
static int (*K_X509_cmp) (X509 *, X509 *) = NULL;
static void (*K_X509_STORE_CTX_free) (X509_STORE_CTX *) = NULL;
static int (*K_X509_verify_cert) (X509_STORE_CTX *) = NULL;
static X509_STORE_CTX *(*K_X509_STORE_CTX_new) (void) = NULL;
static void (*K_X509_STORE_free) (X509_STORE *) = NULL;
static X509_STORE *(*K_X509_STORE_new) (void) = NULL;
static void (*K_X509_free) (X509 *) = NULL;
static char *(*K_X509_NAME_oneline) (X509_NAME *,char *,int) = NULL;
static X509_NAME *(*K_X509_get_subject_name) (X509 *) = NULL;
static X509_NAME *(*K_X509_get_issuer_name) (X509 *) = NULL;
static X509_LOOKUP *(*K_X509_STORE_add_lookup) (X509_STORE *, X509_LOOKUP_METHOD *) = NULL;
static X509_LOOKUP_METHOD *(*K_X509_LOOKUP_file)(void) = NULL;
static int (*K_X509_LOOKUP_ctrl)(X509_LOOKUP *, int, const char *, long, char **) = NULL;
static void (*K_X509_STORE_CTX_init)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *) = NULL;
static void (*K_CRYPTO_free)       (void *) = NULL;
static X509* (*K_X509_dup)         (X509 *) = NULL;
};
#endif


KOpenSSLProxy::KOpenSSLProxy() {
KLibLoader *ll = KLibLoader::self();
_ok = false;

   _sslLib = ll->library("/usr/lib/libssl.so");
   _cryptoLib = ll->library("/usr/lib/libcrypto.so");

   if (_sslLib) {
      // stand back from your monitor and look at this.  it's fun! :)
      K_SSL_connect = (int (*)(SSL *)) _sslLib->symbol("SSL_connect");
      K_SSL_read = (int (*)(SSL *, void *, int)) _sslLib->symbol("SSL_read");
      K_SSL_write = (int (*)(SSL *, const void *, int)) 
                            _sslLib->symbol("SSL_write");
      K_SSL_new = (SSL* (*)(SSL_CTX *)) _sslLib->symbol("SSL_new");
      K_SSL_free = (void (*)(SSL *)) _sslLib->symbol("SSL_free");
      K_SSL_shutdown = (int (*)(SSL *)) _sslLib->symbol("SSL_shutdown");
      K_SSL_CTX_new = (SSL_CTX* (*)(SSL_METHOD*)) _sslLib->symbol("SSL_CTX_new");
      K_SSL_CTX_free = (void (*)(SSL_CTX*)) _sslLib->symbol("SSL_CTX_free");
      K_SSL_set_fd = (int (*)(SSL *, int)) _sslLib->symbol("SSL_set_fd");
      K_SSL_pending = (int (*)(SSL *)) _sslLib->symbol("SSL_pending");
      K_SSL_CTX_set_cipher_list = (int (*)(SSL_CTX *, const char *))
                                  _sslLib->symbol("SSL_CTX_set_cipher_list");
      K_SSL_CTX_set_verify = (void (*)(SSL_CTX*, int, int (*)(int, X509_STORE_CTX*))) _sslLib->symbol("SSL_CTX_set_verify");
      K_SSL_CTX_use_certificate = (int (*)(SSL_CTX*, X509*)) 
                                  _sslLib->symbol("SSL_CTX_use_certificate");
      K_SSL_get_current_cipher = (SSL_CIPHER *(*)(SSL *)) 
                                  _sslLib->symbol("SSL_get_current_cipher");
      K_SSL_ctrl = (long (*)(SSL * ,int, long, char *))
                                  _sslLib->symbol("SSL_ctrl");
      K_TLSv1_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("TLSv1_client_method");
      K_SSLv2_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("SSLv2_client_method");
      K_SSLv3_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("SSLv3_client_method");
      K_SSLv23_client_method = (SSL_METHOD *(*)()) _sslLib->symbol("SSLv23_client_method");
      K_SSL_get_peer_certificate = (X509 *(*)(SSL *)) _sslLib->symbol("SSL_get_peer_certificate");
      K_SSL_CIPHER_get_bits = (int (*)(SSL_CIPHER *,int *)) _sslLib->symbol("SSL_CIPHER_get_bits");
      K_SSL_CIPHER_get_version = (char * (*)(SSL_CIPHER *)) _sslLib->symbol("SSL_CIPHER_get_version");
      K_SSL_CIPHER_get_name = (const char * (*)(SSL_CIPHER *)) _sslLib->symbol("SSL_CIPHER_get_name");
      K_SSL_CIPHER_description = (char * (*)(SSL_CIPHER *, char *, int)) _sslLib->symbol("SSL_CIPHER_description");
      K_d2i_X509 = (X509 * (*)(X509 **,unsigned char **,long)) _sslLib->symbol("d2i_X509");
      K_i2d_X509 = (int (*)(X509 *,unsigned char **)) _sslLib->symbol("i2d_X509");
      K_X509_cmp = (int (*)(X509 *, X509 *)) _sslLib->symbol("X509_cmp");
      K_X509_STORE_CTX_new = (X509_STORE_CTX * (*) (void)) _sslLib->symbol("X509_STORE_CTX_new");
      K_X509_STORE_CTX_free = (void (*) (X509_STORE_CTX *)) _sslLib->symbol("X509_STORE_CTX_free");
      K_X509_verify_cert = (int (*) (X509_STORE_CTX *)) _sslLib->symbol("X509_verify_cert");
      K_X509_STORE_new = (X509_STORE * (*) (void)) _sslLib->symbol("X509_STORE_new");
      K_X509_STORE_free = (void (*) (X509_STORE *)) _sslLib->symbol("X509_STORE_free");
      K_X509_free = (void (*) (X509 *)) _sslLib->symbol("X509_free");
      K_X509_NAME_oneline = (char * (*) (X509_NAME *,char *,int)) _sslLib->symbol("X509_NAME_oneline");
      K_X509_get_subject_name = (X509_NAME * (*) (X509 *)) _sslLib->symbol("X509_get_subject_name");
      K_X509_get_issuer_name = (X509_NAME * (*) (X509 *)) _sslLib->symbol("X509_get_issuer_name");
      K_X509_STORE_add_lookup = (X509_LOOKUP *(*) (X509_STORE *, X509_LOOKUP_METHOD *)) _sslLib->symbol("X509_STORE_add_lookup");
      K_X509_LOOKUP_file = (X509_LOOKUP_METHOD *(*)(void)) _sslLib->symbol("X509_LOOKUP_file");
      K_X509_LOOKUP_ctrl = (int (*)(X509_LOOKUP *, int, const char *, long, char **)) _sslLib->symbol("X509_LOOKUP_ctrl");
      K_X509_STORE_CTX_init = (void (*)(X509_STORE_CTX *, X509_STORE *, X509 *, STACK_OF(X509) *)) _sslLib->symbol("X509_STORE_CTX_init");
      K_X509_dup = (X509* (*)(X509*)) _sslLib->symbol("X509_dup");


      // Initialize the library (once only!)
      void *x;
      x = _sslLib->symbol("SSL_library_init");
      if (x) ((int (*)())x)();
      _sslLib->symbol("OpenSSL_add_all_algorithms");
      if (x) ((void (*)())x)();
      _sslLib->symbol("OpenSSL_add_all_ciphers");
      if (x) ((void (*)())x)();
      _sslLib->symbol("OpenSSL_add_all_digests");
      if (x) ((void (*)())x)();
   }


   if (_cryptoLib) {
      K_RAND_egd = (int (*)(const char *)) _cryptoLib->symbol("RAND_egd");
      K_CRYPTO_free = (void (*) (void *)) _cryptoLib->symbol("CRYPTO_free");
   }
}


KOpenSSLProxy::~KOpenSSLProxy() {

}


KOpenSSLProxy* KOpenSSLProxy::_me = NULL;


KOpenSSLProxy *KOpenSSLProxy::self() {
#ifdef HAVE_SSL
   if (!_me) {
      _me = new KOpenSSLProxy;
   }
#endif
   return _me;
}







#ifdef HAVE_SSL



int KOpenSSLProxy::SSL_connect(SSL *ssl) {
   if (K_SSL_connect) return (K_SSL_connect)(ssl);
   return -1;
}


int KOpenSSLProxy::SSL_read(SSL *ssl, void *buf, int num) {
   if (K_SSL_read) return (K_SSL_read)(ssl, buf, num);
   return -1;
}


int KOpenSSLProxy::SSL_write(SSL *ssl, const void *buf, int num) {
   if (K_SSL_write) return (K_SSL_write)(ssl, buf, num);
   return -1;
}


SSL *KOpenSSLProxy::SSL_new(SSL_CTX *ctx) {
   if (K_SSL_new) return (K_SSL_new)(ctx);
   return NULL;
}


void KOpenSSLProxy::SSL_free(SSL *ssl) {
   if (K_SSL_free) (K_SSL_free)(ssl);
}


int KOpenSSLProxy::SSL_shutdown(SSL *ssl) {
   if (K_SSL_shutdown) return (K_SSL_shutdown)(ssl);
   return -1;
}


SSL_CTX *KOpenSSLProxy::SSL_CTX_new(SSL_METHOD *method) {
   if (K_SSL_CTX_new) return (K_SSL_CTX_new)(method);
   return NULL;
}


void KOpenSSLProxy::SSL_CTX_free(SSL_CTX *ctx) {
   if (K_SSL_CTX_free) (K_SSL_CTX_free)(ctx);
}


int KOpenSSLProxy::SSL_set_fd(SSL *ssl, int fd) {
   if (K_SSL_set_fd) return (K_SSL_set_fd)(ssl, fd);
   return -1;
}


int KOpenSSLProxy::SSL_pending(SSL *ssl) {
   if (K_SSL_pending) return (K_SSL_pending)(ssl);
   return -1;
}


int KOpenSSLProxy::SSL_CTX_set_cipher_list(SSL_CTX *ctx, const char *str) {
   if (K_SSL_CTX_set_cipher_list) return (K_SSL_CTX_set_cipher_list)(ctx, str);
   return -1;
}


void KOpenSSLProxy::SSL_CTX_set_verify(SSL_CTX *ctx, int mode,
                              int (*verify_callback)(int, X509_STORE_CTX *)) {
   if (K_SSL_CTX_set_verify) (K_SSL_CTX_set_verify)(ctx, mode, verify_callback);
}


int KOpenSSLProxy::SSL_CTX_use_certificate(SSL_CTX *ctx, X509 *x) {
   if (K_SSL_CTX_use_certificate) return (K_SSL_CTX_use_certificate)(ctx, x);
   return -1;
}


SSL_CIPHER *KOpenSSLProxy::SSL_get_current_cipher(SSL *ssl) {
   if (K_SSL_get_current_cipher) return (K_SSL_get_current_cipher)(ssl);
   return NULL;
}


long KOpenSSLProxy::SSL_ctrl(SSL *ssl,int cmd, long larg, char *parg) {
   if (K_SSL_ctrl) return (K_SSL_ctrl)(ssl, cmd, larg, parg);
   return -1;
}


int KOpenSSLProxy::RAND_egd(const char *path) {
   if (K_RAND_egd) return (K_RAND_egd)(path);
   return -1;
}


SSL_METHOD *KOpenSSLProxy::TLSv1_client_method() {
   if (K_TLSv1_client_method) return (K_TLSv1_client_method)();
   return NULL;
}


SSL_METHOD *KOpenSSLProxy::SSLv2_client_method() {
   if (K_SSLv2_client_method) return (K_SSLv2_client_method)();
   return NULL;
}


SSL_METHOD *KOpenSSLProxy::SSLv3_client_method() {
   if (K_SSLv3_client_method) return (K_SSLv3_client_method)();
   return NULL;
}


SSL_METHOD *KOpenSSLProxy::SSLv23_client_method() {
   if (K_SSLv23_client_method) return (K_SSLv23_client_method)();
   return NULL;
}


X509 *KOpenSSLProxy::SSL_get_peer_certificate(SSL *s) {
   if (K_SSL_get_peer_certificate) return (K_SSL_get_peer_certificate)(s);
   return NULL;
}


int KOpenSSLProxy::SSL_CIPHER_get_bits(SSL_CIPHER *c,int *alg_bits) {
   if (K_SSL_CIPHER_get_bits) return (K_SSL_CIPHER_get_bits)(c, alg_bits);
   return -1;
}


char * KOpenSSLProxy::SSL_CIPHER_get_version(SSL_CIPHER *c) {
   if (K_SSL_CIPHER_get_version) return (K_SSL_CIPHER_get_version)(c);
   return NULL;
}


const char * KOpenSSLProxy::SSL_CIPHER_get_name(SSL_CIPHER *c) {
   if (K_SSL_CIPHER_get_name) return (K_SSL_CIPHER_get_name)(c);
   return NULL;
}


char * KOpenSSLProxy::SSL_CIPHER_description(SSL_CIPHER *c,char *buf,int size) {
   if (K_SSL_CIPHER_description) return (K_SSL_CIPHER_description)(c,buf,size);
   return NULL;
}


X509 * KOpenSSLProxy::d2i_X509(X509 **a,unsigned char **pp,long length) {
   if (K_d2i_X509) return (K_d2i_X509)(a,pp,length);
   return NULL;
}


int KOpenSSLProxy::i2d_X509(X509 *a,unsigned char **pp) {
   if (K_i2d_X509) return (K_i2d_X509)(a,pp);
   return -1;
}


int KOpenSSLProxy::X509_cmp(X509 *a, X509 *b) {
   if (K_X509_cmp) return (K_X509_cmp)(a,b);
   return 0;
}


X509_STORE *KOpenSSLProxy::X509_STORE_new(void) {
   if (K_X509_STORE_new) return (K_X509_STORE_new)();
   return NULL;
}


void KOpenSSLProxy::X509_STORE_free(X509_STORE *v) {
   if (K_X509_STORE_free) (K_X509_STORE_free)(v);
}


X509_STORE_CTX *KOpenSSLProxy::X509_STORE_CTX_new(void) {
   if (K_X509_STORE_CTX_new) return (K_X509_STORE_CTX_new)();
   return NULL;
}


void KOpenSSLProxy::X509_STORE_CTX_free(X509_STORE_CTX *ctx) {
   if (K_X509_STORE_CTX_free) (K_X509_STORE_CTX_free)(ctx);
}


int KOpenSSLProxy::X509_verify_cert(X509_STORE_CTX *ctx) {
   if (K_X509_verify_cert) return (K_X509_verify_cert)(ctx);
   return -1;
}


void KOpenSSLProxy::X509_free(X509 *a) {
   if (K_X509_free) (K_X509_free)(a);
}


char *KOpenSSLProxy::X509_NAME_oneline(X509_NAME *a,char *buf,int size) {
   if (K_X509_NAME_oneline) return (K_X509_NAME_oneline)(a,buf,size);
   return NULL;
}


X509_NAME *KOpenSSLProxy::X509_get_subject_name(X509 *a) {
   if (K_X509_get_subject_name) return (K_X509_get_subject_name)(a);
   return NULL;
}


X509_NAME *KOpenSSLProxy::X509_get_issuer_name(X509 *a) {
   if (K_X509_get_issuer_name) return (K_X509_get_issuer_name)(a);
   return NULL;
}


X509_LOOKUP *KOpenSSLProxy::X509_STORE_add_lookup(X509_STORE *v, X509_LOOKUP_METHOD *m) {
   if (K_X509_STORE_add_lookup) return (K_X509_STORE_add_lookup)(v,m);
   return NULL;
}


X509_LOOKUP_METHOD *KOpenSSLProxy::X509_LOOKUP_file(void) {
   if (K_X509_LOOKUP_file) return (K_X509_LOOKUP_file)();
   return NULL;
}


int KOpenSSLProxy::X509_LOOKUP_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc, long argl, char **ret) {
   if (K_X509_LOOKUP_ctrl) return (K_X509_LOOKUP_ctrl)(ctx,cmd,argc,argl,ret);
   return -1;
}


void KOpenSSLProxy::X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509, STACK_OF(X509) *chain) {
   if (K_X509_STORE_CTX_init) (K_X509_STORE_CTX_init)(ctx,store,x509,chain);
}


void KOpenSSLProxy::CRYPTO_free(void *x) {
   if (K_CRYPTO_free) (K_CRYPTO_free)(x);
}


X509 *KOpenSSLProxy::X509_dup(X509 *x509) {
   if (K_X509_dup) return (K_X509_dup)(x509);
   return NULL;
}




#endif

