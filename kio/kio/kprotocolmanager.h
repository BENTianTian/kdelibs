/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000- Waldo Bastain <bastain@kde.org>
   Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>

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
#ifndef __kprotocolmanager_h__
#define __kprotocolmanager_h__

#include <qstringlist.h>

#include <kapplication.h>
#include <kio/global.h>

/** @deprecated Use KProtocolManager::defaultUserAgent() instead. */
#define DEFAULT_USERAGENT_STRING ""

class KConfig;

/**
 * Provides information about I/O (Internet, etc.) settings chosen/set
 * by the end user.
 *
 * KProtocolManager has a heap of static functions that allows only read
 * access to KDE's IO related settings. These include proxy, cache, file
 * transfer resumption, timeout and user-agent related settings.
 *
 * The information provided by this class is generic enough to be applicable
 * to any application that makes use of KDE's IO sub-system.  Note that this
 * mean the proxy, timeout etc. settings are saved in a separate user-specific
 * config file and not in the config file of the application.
 *
 * Original author:
 * @author Torben Weis <weis@kde.org>
 *
 * Revised by:
 * @author Waldo Bastain <bastain@kde.org>
 * @author Dawit Alemayehu <adawit@kde.org>
 * @see KPAC
 */
class KProtocolManager
{
public:


/*=========================== USER-AGENT SETTINGS ===========================*/


  /**
   * Returns the default user-agent string.
   *
   * @return the default user-agent string
   */
  static QString defaultUserAgent();

  /**
   * Returns the default user-agent value.
   *
   * @param keys can be any of the following:
   * @li 'o'	Show OS
   * @li 'v'	Show OS Version
   * @li 'p'	Show platform
   * @li 'm'	Show machine architecture
   * @li 'l'	Show language
   * @return the default user-agent value with the given @p keys
   */
  static QString defaultUserAgent(const QString &keys);

  /**
   * Returns the userAgent string configured for the
   * specified host.
   *
   * If hostname is not found or is empty (i.e. "" or
   * QString::null) this function will return the default
   * user agent.
   *
   * @param hostname name of the host
   * @return specified userAgent string
   */
  static QString userAgentForHost( const QString &hostname );


/*=========================== TIMEOUT CONFIG ================================*/


  /**
   * Returns the preferred timeout value for reading from
   * remote connections in seconds.
   *
   * @return timeout value for remote connection in secs.
   */
  static int readTimeout();

  /**
   * Returns the preferred timeout value for remote connections
   * in seconds.
   *
   * @return timeout value for remote connection in secs.
   */
  static int connectTimeout();

  /**
   * Returns the preferred timeout value for proxy connections
   * in seconds.
   *
   * @return timeout value for proxy connection in secs.
   */
  static int proxyConnectTimeout();

  /**
   * Returns the preferred response timeout value for
   * remote connecting in seconds.
   *
   * @return timeout value for remote connection in seconds.
   */
  static int responseTimeout();


/*=============================== PROXY CONFIG ==============================*/


  /**
   * Returns whether or not the user specified the
   * use of proxy server to make connections.
   * @return true to use a proxy
   */
  static bool useProxy();

  /**
   * Returns whether or not the the proxy server
   * lookup should be reversed or not.
   * @return true to use a reversed proxy
   */
  static bool useReverseProxy();

  /**
   * Types of proxy configuration
   * @li NoProxy     - No proxy is used
   * @li ManualProxy - Proxies are manually configured
   * @li PACProxy    - A Proxy configuration URL has been given
   * @li WPADProxy   - A proxy should be automatically discovered
   * @li EnvVarProxy - Use the proxy values set through environment variables.
   */
  enum ProxyType
  {
      NoProxy,
      ManualProxy,
      PACProxy,
      WPADProxy,
      EnvVarProxy
  };

  /**
   * Returns the type of proxy configuration that is used.
   * @return the proxy type
   */
  static ProxyType proxyType();

  /**
   * Proxy authorization modes.
   *
   * @li Prompt     - Ask for authorization as needed
   * @li Automatic  - Use auto login as defined in kionetrc files.
   */
  enum ProxyAuthMode
  {
      Prompt,
      Automatic
  };

  /**
   * Returns the way proxy authorization should be handled.
   *
   * @return the proxy authorization mode
   * @see ProxyAuthMode
   */
  static ProxyAuthMode proxyAuthMode();

  /**
   * Returns the strings for hosts that should contacted
   * DIRECTLY, bypassing any proxy settings.
   * @return a list of (comma-separated) hostnames or partial host
   *         names
   */
  static QString noProxyFor();

  /*
   * Returns the proxy server address for a given
   * protocol.
   *
   * @param protocol the protocol whose proxy info is needed
   * @returns the proxy server address if one is available,
   *          or QString::null if not available
   */
  static QString proxyFor( const QString& protocol );

  /**
   * Returns the Proxy server address for a given URL
   * If automatic proxy configuration is configured, KPAC
   * is used to determine the proxy server, otherwise the return
   * value of proxyFor for the URL's protocol is used.
   * If an empty string is returned, the request is to be aborted,
   * a return value of "DIRECT" requests a direct connection.
   *
   * @param url the URL whose proxy info is needed
   * @returns the proxy server address if one is available
   *          or QString::null otherwise
   */
  static QString proxyForURL( const KURL& url );

  /**
   * Marks this proxy as bad (down). It will not be used for the
   * next 30 minutes. (The script may supply an alternate proxy)
   * @param proxy the proxy to mark as bad (as URL)
   */
  static void badProxy( const QString & proxy );

  /**
   * Returns the URL of the script for automatic proxy configuration.
   * @return the proxy configuration script
   */
  static QString proxyConfigScript();


/*========================== CACHE CONFIG ===================================*/


  /**
   * Returns true/false to indicate whether a cache
   * should be used
   *
   * @return true to use the cache, false otherwisea
   */
  static bool useCache();

  /**
   * Returns the maximum age in seconds cached files should be
   * kept before they are deleted as necessary.
   *
   * @return the maximum cache age in seconds
   */
  static int maxCacheAge();

  /**
   * Returns the maximum size that can be used for caching.
   *
   * By default this function returns the DEFAULT_MAX_CACHE_SIZE
   * value as defined in http_slave_defaults.h.  Not that the
   * value returned is in bytes, hence a value of 5120 would mean
   * 5 Kb.
   *
   * @return the maximum cache size in bytes
   */
  static int maxCacheSize(); // Maximum cache size in Kb.

  /**
   * The directory which contains the cache files.
   * @return the directory that contains the cache files
   */
  static QString cacheDir();

  /**
   * Returns the Cache control directive to be used.
   * @return the cache control value
   */
  static KIO::CacheControl cacheControl();


/*============================ DOWNLOAD CONFIG ==============================*/

  /**
   * Returns true if partial downloads should be
   * automatically resumed.
   * @return true to resume partial downloads
   */
  static bool autoResume();

  /**
   * Returns true if partial downloads should be marked
   * with a ".part" extension.
   * @return true if partial downloads should get an ".part" extension
   */
  static bool markPartial();

  /**
   * Returns the minimum file size for keeping aborted
   * downloads.
   *
   * Any data downloaded that does not meet this minimum
   * requirement will simply be discarded. The default size
   * is 5 KB.
   *
   * @return the minimum keep size for aborted downloads in bytes
   */
  static int minimumKeepSize();


  /*============================ NETWORK CONNECTIONS ==========================*/
  /**
   * Returns true if proxy connections should be persistent.
   * @return true if proxy connections should be persistent
   * @since 3.1
   */
  static bool persistentProxyConnection();

  /**
   * Returns true if connections should be persistent
   * @return true if the connections should be persistent
   */
  static bool persistentConnections();

/*=============================== OTHERS ====================================*/


  /**
   * Force a reload of the general config file of
   * io-slaves ( kioslaverc).
   */
  static void reparseConfiguration();

  /**
   * Return the protocol to use in order to handle the given @p url
   * It's usually the same, except that FTP, when handled by a proxy,
   * needs an HTTP ioslave.
   *
   * When a proxy is to be used, proxy contains the URL for the proxy.
   * @param url the url to check
   * @param proxy the URL of the proxy to use
   * @return the slave protocol (e.g. 'http'), can be null if unknown
   */
  static QString slaveProtocol(const KURL &url, QString &proxy);

  /**
   * @internal
   * (Shared with SlaveConfig)
   */
  static KConfig *config();
private:
  static KConfig *http_config();
};
#endif
