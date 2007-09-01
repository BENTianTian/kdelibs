/*
This file is part of KDE

  Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
//----------------------------------------------------------------------------
//
// KDE Cookie Server

#include "kcookieserver.h"

#define SAVE_DELAY 3 // Save after 3 minutes

#include <unistd.h>

#include <QtCore/QTimer>
#include <Qt3Support/Q3PtrList>
#include <QtCore/QFile>

#include <QtDBus/QtDBus>

#include <kconfig.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "kcookiejar.h"
#include "kcookiewin.h"
#include "kcookieserveradaptor.h"

extern "C" {
    KDE_EXPORT KDEDModule *create_kcookiejar()
    {
       return new KCookieServer();
    }
}


// Cookie field indexes
enum CookieDetails { CF_DOMAIN=0, CF_PATH, CF_NAME, CF_HOST,
                     CF_VALUE, CF_EXPIRE, CF_PROVER, CF_SECURE };


class CookieRequest {
public:
   QDBusMessage reply;
   QString url;
   bool DOM;
   qlonglong windowId;
};

template class  QList<CookieRequest*>;

class RequestList : public QList<CookieRequest*>
{
public:
   RequestList() : QList<CookieRequest*>() { }
};

KCookieServer::KCookieServer()
              :KDEDModule()
{
   (void)new KCookieServerAdaptor(this);
   mCookieJar = new KCookieJar;
   mPendingCookies = new KHttpCookieList;
   mPendingCookies->setAutoDelete(true);
   mRequestList = new RequestList;
   mAdvicePending = false;
   mTimer = new QTimer();
   mTimer->setSingleShot(true);
   connect(mTimer, SIGNAL( timeout()), SLOT( slotSave()));
   mConfig = new KConfig("kcookiejarrc");
   mCookieJar->loadConfig( mConfig );

   QString filename = KStandardDirs::locateLocal("data", "kcookiejar/cookies");

   // Stay backwards compatible!
   QString filenameOld = KStandardDirs::locate("data", "kfm/cookies");
   if (!filenameOld.isEmpty())
   {
      mCookieJar->loadCookies( filenameOld );
      if (mCookieJar->saveCookies( filename))
      {
         unlink(QFile::encodeName(filenameOld)); // Remove old kfm cookie file
      }
   }
   else
   {
      mCookieJar->loadCookies( filename);
   }
   connect(this, SIGNAL(windowUnregistered(qlonglong)),
           this, SLOT(slotDeleteSessionCookies(qlonglong)));
}

KCookieServer::~KCookieServer()
{
   slotSave();
   delete mCookieJar;
   delete mTimer;
   delete mPendingCookies;
   delete mConfig;
}

bool KCookieServer::cookiesPending( const QString &url, KHttpCookieList *cookieList )
{
  QString fqdn;
  QStringList domains;
  QString path;
  // Check whether 'url' has cookies on the pending list
  if (mPendingCookies->isEmpty())
     return false;
  if (!KCookieJar::parseUrl(url, fqdn, path))
     return false;

  mCookieJar->extractDomains( fqdn, domains );
  for( KHttpCookie *cookie = mPendingCookies->first();
       cookie != 0L;
       cookie = mPendingCookies->next())
  {
       if (cookie->match( fqdn, domains, path))
       {
          if (!cookieList)
             return true;
          cookieList->append(cookie);
       }
  }
  if (!cookieList)
     return false;
  return cookieList->isEmpty();
}

void KCookieServer::addCookies( const QString &url, const QByteArray &cookieHeader,
                               qlonglong windowId, bool useDOMFormat )
{
    KHttpCookieList cookieList;
    if (useDOMFormat)
       cookieList = mCookieJar->makeDOMCookies(url, cookieHeader, windowId);
    else
       cookieList = mCookieJar->makeCookies(url, cookieHeader, windowId);

    checkCookies(&cookieList);

    for(KHttpCookiePtr cookie = cookieList.first(); cookie; cookie = cookieList.first())
       mPendingCookies->append(cookieList.take());

    if (!mAdvicePending)
    {
       mAdvicePending = true;
       while (!mPendingCookies->isEmpty())
       {
          checkCookies(0);
       }
       mAdvicePending = false;
    }
}

void KCookieServer::checkCookies( KHttpCookieList *cookieList)
{
    KHttpCookieList *list;

    if (cookieList)
       list = cookieList;
    else
       list = mPendingCookies;

    KHttpCookiePtr cookie = list->first();
    while (cookie)
    {
        KCookieAdvice advice = mCookieJar->cookieAdvice(cookie);
        switch(advice)
        {
        case KCookieAccept:
            list->take();
            mCookieJar->addCookie(cookie);
            cookie = list->current();
            break;

        case KCookieReject:
            list->take();
            delete cookie;
            cookie = list->current();
            break;

        default:
            cookie = list->next();
            break;
        }
    }

    if (cookieList || list->isEmpty())
       return;

    KHttpCookiePtr currentCookie = mPendingCookies->first();

    KHttpCookieList currentList;
    currentList.append(currentCookie);
    QString currentHost = currentCookie->host();

    cookie = mPendingCookies->next();
    while (cookie)
    {
        if (cookie->host() == currentHost)
        {
            currentList.append(cookie);
        }
        cookie = mPendingCookies->next();
    }

    KCookieWin *kw = new KCookieWin( 0L, currentList,
                                     mCookieJar->preferredDefaultPolicy(),
                                     mCookieJar->showCookieDetails() );
    KCookieAdvice userAdvice = kw->advice(mCookieJar, currentCookie);
    delete kw;
    // Save the cookie config if it has changed
    mCookieJar->saveConfig( mConfig );

    // Apply the user's choice to all cookies that are currently
    // queued for this host.
    cookie = mPendingCookies->first();
    while (cookie)
    {
        if (cookie->host() == currentHost)
        {
           switch(userAdvice)
           {
           case KCookieAccept:
               mPendingCookies->take();
               mCookieJar->addCookie(cookie);
               cookie = mPendingCookies->current();
               break;

           case KCookieReject:
               mPendingCookies->take();
               delete cookie;
               cookie = mPendingCookies->current();
               break;

           default:
               qWarning(__FILE__":%d Problen!", __LINE__);
               cookie = mPendingCookies->next();
               break;
           }
        }
        else
        {
            cookie = mPendingCookies->next();
        }
    }


    // Check if we can handle any request
    RequestList reqToRemove;
    RequestList::ConstIterator it = mRequestList->constBegin();
    for ( ; it != mRequestList->constEnd(); ++it )
    {
        CookieRequest *request = *it;
        if (!cookiesPending( request->url ))
        {
           QString res = mCookieJar->findCookies( request->url, request->DOM, request->windowId );

           QDBusConnection::sessionBus().send(request->reply.createReply(res));
           reqToRemove += request;
           delete request;
        }
    }
    it = reqToRemove.constBegin();
    for( ; it != reqToRemove.constEnd(); ++it )
        mRequestList->removeAll( *it );

    saveCookieJar();
}

void KCookieServer::slotSave()
{
   if (mCookieJar->changed())
   {
      QString filename = KStandardDirs::locateLocal("data", "kcookiejar/cookies");
      mCookieJar->saveCookies(filename);
   }
}

void KCookieServer::saveCookieJar()
{
    if( mTimer->isActive() )
        return;

    mTimer->start( 1000*60*SAVE_DELAY );
}

void KCookieServer::putCookie( QStringList& out, KHttpCookie *cookie,
                               const QList<int>& fields )
{
	foreach ( int i, fields ) {
       switch(i)
        {
         case CF_DOMAIN :
            out << cookie->domain();
            break;
         case CF_NAME :
            out << cookie->name();
            break;
         case CF_PATH :
            out << cookie->path();
            break;
         case CF_HOST :
            out << cookie->host();
            break;
         case CF_VALUE :
            out << cookie->value();
            break;
         case CF_EXPIRE :
            out << QString::number(cookie->expireDate());
            break;
         case CF_PROVER :
            out << QString::number(cookie->protocolVersion());
            break;
         case CF_SECURE :
            out << QString::number( cookie->isSecure() ? 1 : 0 );
            break;
         default :
            out << QString();
        }
    }
}

bool KCookieServer::cookieMatches( KHttpCookiePtr c,
                                   const QString &domain, const QString &fqdn,
                                   const QString &path, const QString &name )
{
    if( c )
    {
        bool hasDomain = !domain.isEmpty();
        return
       ((hasDomain && c->domain() == domain) ||
        fqdn == c->host()) &&
       (c->path()   == path) &&
       (c->name()   == name) &&
       (!c->isExpired(time(0)));
    }
    return false;
}

// DBUS function
QString
KCookieServer::findCookies(const QString &url, qlonglong windowId)
{
   if (cookiesPending(url))
   {
      CookieRequest *request = new CookieRequest;
      message().setDelayedReply(true);
      request->reply = message();
      request->url = url;
      request->DOM = false;
      request->windowId = windowId;
      mRequestList->append( request );
      return QString(); // Talk to you later :-)
   }

   QString cookies = mCookieJar->findCookies(url, false, windowId);
   saveCookieJar();
   return cookies;
}

// DBUS function
QStringList
KCookieServer::findDomains()
{
   QStringList result;
   const QStringList domains = mCookieJar->getDomainList();
   for ( QStringList::ConstIterator domIt = domains.begin();
         domIt != domains.end(); ++domIt )
   {
       // Ignore domains that have policy set for but contain
       // no cookies whatsoever...
       const KHttpCookieList* list =  mCookieJar->getCookieList(*domIt, "");
       if ( list && !list->isEmpty() )
          result << *domIt;
   }
   return result;
}

// DBUS function
QStringList
KCookieServer::findCookies(const QList<int> &fields,
                           const QString &domain,
                           const QString &fqdn,
                           const QString &path,
                           const QString &name)
{
   QStringList result;
   bool allDomCookies = name.isEmpty();

   const KHttpCookieList* list =  mCookieJar->getCookieList(domain, fqdn);
   if ( list && !list->isEmpty() )
   {
      Q3PtrListIterator<KHttpCookie>it( *list );
      for ( ; it.current(); ++it )
      {
         if ( !allDomCookies )
         {
            if ( cookieMatches(it.current(), domain, fqdn, path, name) )
            {
               putCookie(result, it.current(), fields);
               break;
            }
         }
         else
            putCookie(result, it.current(), fields);
      }
   }
   return result;
}

// DBUS function
QString
KCookieServer::findDOMCookies(const QString &url)
{
   return findDOMCookies(url, 0);
}

// DBUS function
QString
KCookieServer::findDOMCookies(const QString &url, qlonglong windowId)
{
   // We don't wait for pending cookies because it locks up konqueror
   // which can cause a deadlock if it happens to have a popup-menu up.
   // Instead we just return pending cookies as if they had been accepted already.
   KHttpCookieList pendingCookies;
   cookiesPending(url, &pendingCookies);

   return mCookieJar->findCookies(url, true, windowId, &pendingCookies);
}

// DBUS function
void
KCookieServer::addCookies(const QString &arg1, const QByteArray &arg2, qlonglong arg3)
{
   addCookies(arg1, arg2, arg3, false);
}

// DBUS function
void
KCookieServer::deleteCookie(const QString &domain, const QString &fqdn,
                            const QString &path, const QString &name)
{
   const KHttpCookieList* list = mCookieJar->getCookieList( domain, fqdn );
   if ( list && !list->isEmpty() )
   {
      Q3PtrListIterator<KHttpCookie>it (*list);
      for ( ; it.current(); ++it )
      {
         if( cookieMatches(it.current(), domain, fqdn, path, name) )
         {
            mCookieJar->eatCookie( it.current() );
            saveCookieJar();
            break;
         }
      }
   }
}

// DBUS function
void
KCookieServer::deleteCookiesFromDomain(const QString &domain)
{
   mCookieJar->eatCookiesForDomain(domain);
   saveCookieJar();
}


// Qt function
void
KCookieServer::slotDeleteSessionCookies( qlonglong windowId )
{
   deleteSessionCookies(windowId);
}

// DBUS function
void
KCookieServer::deleteSessionCookies( qlonglong windowId )
{
  mCookieJar->eatSessionCookies( windowId );
  saveCookieJar();
}

void
KCookieServer::deleteSessionCookiesFor(const QString &fqdn, qlonglong windowId)
{
  mCookieJar->eatSessionCookies( fqdn, windowId );
  saveCookieJar();
}

// DBUS function
void
KCookieServer::deleteAllCookies()
{
   mCookieJar->eatAllCookies();
   saveCookieJar();
}

// DBUS function
void
KCookieServer::addDOMCookies(const QString &arg1, const QByteArray &arg2, qlonglong arg3)
{
   addCookies(arg1, arg2, arg3, true);
}

// DBUS function
void
KCookieServer::setDomainAdvice(const QString &url, const QString &advice)
{
   QString fqdn;
   QString dummy;
   if (KCookieJar::parseUrl(url, fqdn, dummy))
   {
      QStringList domains;
      mCookieJar->extractDomains(fqdn, domains);

      mCookieJar->setDomainAdvice(domains[domains.count() > 3 ? 3 : 0],
                                  KCookieJar::strToAdvice(advice));
      // Save the cookie config if it has changed
      mCookieJar->saveConfig( mConfig );
   }
}

// DBUS function
QString
KCookieServer::getDomainAdvice(const QString &url)
{
   KCookieAdvice advice = KCookieDunno;
   QString fqdn;
   QString dummy;
   if (KCookieJar::parseUrl(url, fqdn, dummy))
   {
      QStringList domains;
      mCookieJar->extractDomains(fqdn, domains);

      QStringList::ConstIterator it = domains.begin();
      while ( (advice == KCookieDunno) && (it != domains.end()) )
      {
         // Always check advice in both ".domain" and "domain". Note
         // that we only want to check "domain" if it matches the
         // fqdn of the requested URL.
         if ( (*it)[0] == '.' || (*it) == fqdn )
            advice = mCookieJar->getDomainAdvice(*it);
         ++it;
      }
      if (advice == KCookieDunno)
         advice = mCookieJar->getGlobalAdvice();
   }
   return KCookieJar::adviceToStr(advice);
}

// DBUS function
void
KCookieServer::reloadPolicy()
{
   mCookieJar->loadConfig( mConfig, true );
}

// DBUS function
void
KCookieServer::shutdown()
{
   deleteLater();
}

#include "kcookieserver.moc"

