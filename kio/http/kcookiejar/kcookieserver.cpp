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
// $Id$

#define SAVE_DELAY 3 // Save after 3 minutes

#include "kcookieserver.h"
#include "kcookiejar.h"
#include "kcookiewin.h"

#include <kdebug.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <kstddirs.h>
#include <qtimer.h>
#include <unistd.h>
#include <qlist.h>

#include <dcopclient.h>
#include <kconfig.h>

// Cookie field indexes
enum CookieDetails { CF_DOMAIN=0, CF_PATH, CF_NAME, CF_HOST,
                     CF_VALUE, CF_EXPIRE, CF_PROVER, CF_SECURE };


class CookieRequest {
public:
   DCOPClientTransaction *transaction;
   QString url;
   bool DOM;
};

template class  QList<CookieRequest>;

class RequestList : public QList<CookieRequest>
{
public:
   RequestList() : QList<CookieRequest>() { }
};

KCookieServer::KCookieServer()
              :KUniqueApplication()
{
   mCookieJar = new KCookieJar;
   mPendingCookies = new KHttpCookieList;
   mRequestList = new RequestList;
   mAdvicePending = false;
   mTimer = 0;
   mCookieJar->loadConfig( kapp->config());

   QString filename = locateLocal("appdata", "cookies");

   // Stay backwards compatible!
   QString filenameOld = locate("data", "kfm/cookies");
   if (!filenameOld.isEmpty())
   {
      mCookieJar->loadCookies( filenameOld );
      if (mCookieJar->saveCookies( filename))
      {
         unlink(filenameOld.ascii()); // Remove old kfm cookie file
      }
   }
   else
   {
      mCookieJar->loadCookies( filename);
   }
}

KCookieServer::~KCookieServer()
{
   if (mCookieJar->changed())
      slotSave();
   delete mCookieJar;
   delete mTimer;
   delete mPendingCookies;
}

int KCookieServer::newInstance()
{
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   if (args->isSet("remove-all"))
   {
        mCookieJar->eatAllCookies();
        slotSave();
   }
   if (args->isSet("remove"))
   {
        mCookieJar->eatCookiesForDomain(args->getOption("remove"));
        slotSave();
   }

   if (args->isSet("shutdown"))
   {
        shutdown();
   }
   if(args->isSet("reload-config"))
   {
        mCookieJar->loadConfig( kapp->config(), true );
   }
   return 0;
}

// DCOP function
QString
KCookieServer::findCookies(QString url)
{
   if (cookiesPending(url))
   {
      CookieRequest *request = new CookieRequest;
      request->transaction = dcopClient()->beginTransaction();
      request->url = url;
      request->DOM = false;
      mRequestList->append( request );
      return QString::null; // Talk to you later :-)
   }
   return mCookieJar->findCookies(url, false);
}

// DCOP function
QStringList 
KCookieServer::findDomains()
{
   QStringList result;
   const QStringList *domains = mCookieJar->getDomainList();
   for (QStringList::ConstIterator domIt = domains->begin(); domIt != domains->end(); ++domIt)
   {
       // Ignore domains that have policy set for but contain
       // no cookies whatsoever...
       if ( mCookieJar->getCookieList(*domIt)->count() != 0 )
           result << *domIt;
   }
   return result;
}

// DCOP function
QStringList
KCookieServer::findCookies(QValueList<int> fields, 
                           QString domain,
                           QString cdomain,
                           QString path,
                           QString name)
{
   bool allDomCookies = name.isEmpty();

   QStringList result;
   KHttpCookieList *domcookies =  const_cast<KHttpCookieList*>(mCookieJar->getCookieList(domain));
   if (domcookies)
   {
      for (KHttpCookiePtr cIt = domcookies->first(); cIt; cIt = domcookies->next() )
      {
         if (!allDomCookies)
         {
            if ( cookieMatches(cIt, cdomain, path, name) )
            {
               putCookie(&result, cIt, &fields);
               break;
            }
         }
         else
            putCookie(&result, cIt, &fields);
      }
   }
   return result;
}

// DCOP function
QString
KCookieServer::findDOMCookies(QString url)
{
   if (cookiesPending(url))
   {
      CookieRequest *request = new CookieRequest;
      request->transaction = dcopClient()->beginTransaction();
      request->url = url;
      request->DOM = true;
      mRequestList->append( request );
      return QString::null; // Talk to you later :-)
   }
   return mCookieJar->findCookies(url, true);
}

// DCOP function
void
KCookieServer::addCookies(QString arg1, QCString arg2, long arg3)
{
   addCookies(arg1, arg2, arg3, false);
}

// DCOP function
void
KCookieServer::deleteCookie(QString domain, QString cdomain, 
                            QString path, QString name)
{
   KHttpCookieList *domcookies =  const_cast<KHttpCookieList*>(mCookieJar->getCookieList(domain));
   if (domcookies)
   {
      for (KHttpCookiePtr cIt = domcookies->first(); cIt; cIt = domcookies->next() )
      {
         if (cookieMatches(cIt, cdomain, path, name))
         {
            mCookieJar->eatCookie(cIt);
            if (!mTimer)
               saveCookieJar();
            break;
         }
      }
   }
}

// DCOP function
void
KCookieServer::deleteCookiesFromDomain(QString domain)
{
   mCookieJar->eatCookiesForDomain(domain);
   if (!mTimer)
      saveCookieJar();
}

// DCOP function
void
KCookieServer::deleteAllCookies()
{
   mCookieJar->eatAllCookies();
   if (!mTimer)
      saveCookieJar();
}

// DCOP function
void
KCookieServer::addDOMCookies(QString arg1, QCString arg2, long arg3)
{
   addCookies(arg1, arg2, arg3, true);
}

// DCOP function
void
KCookieServer::setDomainAdvice(QString url, QString advice)
{
   QString fqdn;
   QString dummy;
   if (KCookieJar::parseURL(url, fqdn, dummy))
   {
      QStringList domains;
      KCookieJar::extractDomains(fqdn, domains);
      mCookieJar->setDomainAdvice(domains[0], 
                                  KCookieJar::strToAdvice(advice));
   }
}

// DCOP function
QString 
KCookieServer::getDomainAdvice(QString url)
{
   KCookieAdvice advice = KCookieDunno;
   QString fqdn;
   QString dummy;
   if (KCookieJar::parseURL(url, fqdn, dummy))
   {
      QStringList domains;
      KCookieJar::extractDomains(fqdn, domains);
      advice = mCookieJar->getDomainAdvice(domains[0]);
   }
   return KCookieJar::adviceToStr(advice); 
}

// DCOP function
void
KCookieServer::reloadPolicy()
{
   mCookieJar->loadConfig( kapp->config(), true );
}

// DCOP function
void
KCookieServer::shutdown()
{
   quit();
}

bool KCookieServer::cookiesPending( const QString &url )
{
  QString fqdn;
  QStringList domains;
  QString path;
  // Check whether 'url' has cookies on the pending list
  if (mPendingCookies->isEmpty())
     return false;
  if (!KCookieJar::parseURL(url, fqdn, path))
     return false;
  if (!KCookieJar::extractDomains( fqdn, domains))
     return false;
  for( KHttpCookie *cookie = mPendingCookies->first();
       cookie;
       cookie = mPendingCookies->next())
  {
       if (cookie->match( fqdn, domains, path))
          return true;
  }
  return false;
}

void KCookieServer::addCookies( const QString &url, const QCString &cookieHeader,
                               long windowId, bool useDOMFormat )
{
    KHttpCookiePtr cookie = 0;
    if (useDOMFormat)
       cookie = mCookieJar->makeDOMCookies(url, cookieHeader, windowId);
    else
       cookie = mCookieJar->makeCookies(url, cookieHeader, windowId);

    if (mAdvicePending)
    {
       checkCookies(cookie, true);
    }
    else
    {
       mAdvicePending = true;
       do {
          checkCookies(cookie, false);
          cookie = mPendingCookies->count() ? mPendingCookies->take(0) : 0;
       }
       while (cookie);
       mAdvicePending = false;
    }

}

void KCookieServer::checkCookies( KHttpCookie *cookie, bool queue )
{
    QString host;
    KCookieAdvice userAdvice = KCookieDunno;
    if (cookie) host = cookie->host();
    while (cookie)
    {
        KHttpCookiePtr next_cookie = cookie->next();
        KCookieAdvice advice = mCookieJar->cookieAdvice(cookie);
        if ((advice == KCookieAsk) || (advice == KCookieDunno))
        {
            // We only ask the user once, even if we get multiple
            // cookies from the same site.
            if (userAdvice == KCookieDunno)
            {
                if (queue)
                {
                    mPendingCookies->append(cookie);
                    return;
                }
                else
                {
                    mPendingCookies->prepend(cookie);
                    KCookieWin *kw = new KCookieWin( 0L, cookie,
                                                     mCookieJar->defaultRadioButton,
                                                     mCookieJar->showCookieDetails );
                    userAdvice = kw->advice(mCookieJar, cookie);
                    delete kw;
                    mPendingCookies->take(0);
                    // Save the cookie config if it has changed
                    mCookieJar->saveConfig( kapp->config() );
                }
            }
            advice = userAdvice;
        }
        switch(advice)
        {
        case KCookieAccept:
            mCookieJar->addCookie(cookie);
            break;

        case KCookieReject:
        default:
            delete cookie;
            break;
        }
        cookie = next_cookie;
        if (!cookie && !queue)
        {
           // Check if there are cookies on the pending list from the
           // same host.
           for( cookie = mPendingCookies->first();
                cookie;
                cookie = mPendingCookies->next())
           {
               if (cookie->host() == host)
                  break;
           }
           if (cookie)
           {
               // Found a matching cookie, remove it from the pending list.
               cookie = mPendingCookies->take();
           }
        }
    }

    // Check if we can handle any request
    for ( CookieRequest *request = mRequestList->first(); request;)
    {
        if (!cookiesPending( request->url ))
        {
           QCString replyType;
           QByteArray replyData;
           QString res = mCookieJar->findCookies( request->url, request->DOM );

           QDataStream stream2(replyData, IO_WriteOnly);
           stream2 << res;
           replyType = "QString";
           dcopClient()->endTransaction( request->transaction,
                                       replyType, replyData);
           CookieRequest *tmp = request;
           request = mRequestList->next();
           mRequestList->removeRef( tmp );
           delete tmp;
        }
        else
        {
          request = mRequestList->next();
        }
    }
    if (mCookieJar->changed() && !mTimer)
        saveCookieJar();
}

void KCookieServer::slotSave()
{
   delete mTimer;
   mTimer = 0;
   QString filename = locateLocal("appdata", "cookies");
   mCookieJar->saveCookies(filename);
}

void KCookieServer::saveCookieJar()
{
    if( mTimer )
        return;

    mTimer = new QTimer();
    connect( mTimer, SIGNAL( timeout()), SLOT( slotSave()));
    mTimer->start( 1000*60*SAVE_DELAY );
}

void KCookieServer::putCookie( QStringList *out, KHttpCookie *cookie,
                               const QValueList<int> *fields )
{
    QValueList<int>::ConstIterator i = fields->begin();
    for ( ; i != fields->end(); ++i )
    {
        switch(*i) {
         case CF_DOMAIN :
            *out << cookie->domain();
            break;
         case CF_NAME :
            *out << cookie->name();
            break;
         case CF_PATH :
            *out << cookie->path();
            break;
         case CF_HOST :
            *out << cookie->host();
            break;
         case CF_VALUE :
            *out << cookie->value();
            break;
         case CF_EXPIRE :
            *out << QString::number(cookie->expireDate());
            break;
         case CF_PROVER :
            *out << QString::number(cookie->protocolVersion());
            break;
         case CF_SECURE :
            *out << QString::number( cookie->isSecure() ? 1 : 0 );
            break;
         default :
            *out << QString::null;
        }
    }
}

bool KCookieServer::cookieMatches( KHttpCookiePtr c, QString domain,
                                   QString path, QString name )
{
    if( c )
    {
        return
       (c->domain() == domain) &&
       (c->path()   == path)   &&
       (c->name()   == name);
    }
    return false;
}

#include "kcookieserver.moc"

