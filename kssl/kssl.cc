/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#undef crypt
#endif

#include "kssl.h"

#include <kdebug.h>

class KSSLPrivate {
public:
  KSSLPrivate() {

  }

  ~KSSLPrivate() {

  }

  #ifdef HAVE_SSL
    SSL *m_ssl;
    SSL_CTX *m_ctx;
    SSL_METHOD *m_meth;
  #endif
};


KSSL::KSSL(bool init) {
  d = new KSSLPrivate;
  m_bInit = false;
  m_bAutoReconfig = true;
  m_cfg = new KSSLSettings();
  if (init) initialize();
}


KSSL::~KSSL() {
  close();
  delete m_cfg;
  delete d;
}


bool KSSL::initialize() {
#ifdef HAVE_SSL
  if (m_bInit) return false;

  if (m_bAutoReconfig)
    m_cfg->load();
  // TLS1 goes first - it excludes SSL2/3
  // FIXME: we should be able to force SSL off entirely.
  //        This logic here makes v2 a "default" if no other SSL
  //        version is turned on.  IMHO this is the safest one to
  //        use as the default anyways, so I'm not changing it yet.
  if (m_cfg->tlsv1())
    d->m_meth = TLSv1_client_method();
  else if (m_cfg->sslv2() && m_cfg->sslv3())
    d->m_meth = SSLv23_client_method();
  else if (m_cfg->sslv3())
    d->m_meth = SSLv3_client_method();
  else
    d->m_meth = SSLv2_client_method();
 
  SSLeay_add_ssl_algorithms();
  d->m_ctx=SSL_CTX_new(d->m_meth);
  if (d->m_ctx == NULL) {
    return false;
  }

  // set cipher list
  QString clist = m_cfg->getCipherList();
  if (!clist.isEmpty()) 
    SSL_CTX_set_cipher_list(d->m_ctx, clist.ascii());

  m_bInit = true;
return true;
#else
return false;
#endif
}




void KSSL::close() {
#ifdef HAVE_SSL
  if (!m_bInit) return;
  SSL_shutdown(d->m_ssl);
  SSL_free(d->m_ssl);
  SSL_CTX_free(d->m_ctx);
  m_bInit = false;
#endif
}


bool KSSL::reInitialize() {
  close();
  return initialize();
}


int KSSL::connect(int sock) {
#ifdef HAVE_SSL
int rc;
  if (!m_bInit) return -1;
  d->m_ssl = SSL_new(d->m_ctx);
  if (!d->m_ssl) return -1;
  SSL_set_fd(d->m_ssl, sock);
  rc = SSL_connect(d->m_ssl);
  if (rc != -1) {
    setConnectionInfo();
    setPeerInfo();
  }
  return rc;
#else
  return -1;
#endif
}


int KSSL::read(void *buf, int len) {
#ifdef HAVE_SSL
  if (!m_bInit) return -1;
  return SSL_read(d->m_ssl, (char *)buf, len);
#else
return -1;
#endif
}


int KSSL::write(const void *buf, int len) {
#ifdef HAVE_SSL
  if (!m_bInit) return -1;
  return SSL_write(d->m_ssl, (const char *)buf, len);
#else
return -1;
#endif
}


bool KSSL::reconfig() {
  return reInitialize();
}


void KSSL::setAutoReconfig(bool ar) {
  m_bAutoReconfig = ar;
}


bool KSSL::setSettings(KSSLSettings *settings) {
  delete m_cfg;
  m_cfg = settings;
  return reconfig();
}


#ifdef HAVE_SSL
bool KSSL::m_bSSLWorks = true;
#else
bool KSSL::m_bSSLWorks = false;
#endif

bool KSSL::doesSSLWork() {
  return m_bSSLWorks;
}


void KSSL::setConnectionInfo() {
#ifdef HAVE_SSL
SSL_CIPHER *sc;
char buf[1024];

  sc = SSL_get_current_cipher(d->m_ssl);
  // set the number of bits, bits used
  m_ci.m_iCipherUsedBits = SSL_CIPHER_get_bits(sc, &(m_ci.m_iCipherBits));
  // set the cipher version
  m_ci.m_cipherVersion = SSL_CIPHER_get_version(sc);
  // set the cipher name
  m_ci.m_cipherName = SSL_CIPHER_get_name(sc);
  // set the cipher description
  m_ci.m_cipherDescription = SSL_CIPHER_description(sc, buf, 1023);  

#endif
}


void KSSL::setPeerInfo() {
#ifdef HAVE_SSL
  m_pi.m_cert.setCert(SSL_get_peer_certificate(d->m_ssl));
#endif
}


const KSSLConnectionInfo& KSSL::connectionInfo() const {
  return m_ci;
}


const KSSLPeerInfo& KSSL::peerInfo() const {
  return m_pi;
}


