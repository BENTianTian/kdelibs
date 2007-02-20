/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "ktoolinvocation.h"
#include "klauncher_iface.h"
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kcodecs.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kshell.h"
#include "kmacroexpander.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include "kmessage.h"
#include <qcoreapplication.h>
#include <qhash.h>
#include <QtDBus/QtDBus>
#include <QDebug>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif


#include <qglobal.h>
#include <QFile>
#include <errno.h>
#include <kconfiggroup.h>

void KToolInvocation::invokeHelp( const QString& anchor,
                                  const QString& _appname,
                                  const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    QString url;
    QString appname;
    if (_appname.isEmpty()) {
        appname = QCoreApplication::instance()->applicationName();
    } else
        appname = _appname;

    if (!anchor.isEmpty())
        url = QString("help:/%1?anchor=%2").arg(appname).arg(anchor);
    else
        url = QString("help:/%1/index.html").arg(appname);

    QDBusInterface *iface = new QDBusInterface(QLatin1String("org.kde.khelpcenter"),
                                               QLatin1String("/KHelpCenter"),
                                               QLatin1String("org.kde.khelpcenter.khelpcenter"),
                                               QDBusConnection::sessionBus());
    if ( !iface->isValid() )
    {
        QString error;
        if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, false))
        {
            KMessage::message(KMessage::Error,
                              i18n("Could not launch the KDE Help Center:\n\n%1", error),
                              i18n("Could not Launch Help Center"));
            return;
        }

        delete iface;
        iface = new QDBusInterface(QLatin1String("org.kde.khelpcenter"),
                                   QLatin1String("/KHelpCenter"),
                                   QLatin1String("org.kde.khelpcenter.khelpcenter"),
                                   QDBusConnection::sessionBus());
    }

    iface->call("openUrl", url, startup_id );
    delete iface;
}

void KToolInvocation::invokeMailer(const KUrl &mailtoURL, const QByteArray& startup_id, bool allowAttachments )
{
    if (!isMainThreadActive())
        return;

    QString address = KUrl::fromPercentEncoding(mailtoURL.path().toLatin1()), subject, cc, bcc, body;
    QStringList queries = mailtoURL.query().mid(1).split( '&');
    QStringList attachURLs;
    for (QStringList::Iterator it = queries.begin(); it != queries.end(); ++it)
    {
        QString q = (*it).toLower();
        if (q.startsWith("subject="))
            subject = KUrl::fromPercentEncoding((*it).mid(8).toLatin1());
        else
            if (q.startsWith("cc="))
                cc = cc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): cc + ',' + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
            else
                if (q.startsWith("bcc="))
                    bcc = bcc.isEmpty()? KUrl::fromPercentEncoding((*it).mid(4).toLatin1()): bcc + ',' + KUrl::fromPercentEncoding((*it).mid(4).toLatin1());
                else
                    if (q.startsWith("body="))
                        body = KUrl::fromPercentEncoding((*it).mid(5).toLatin1());
                    else
                        if (allowAttachments && q.startsWith("attach="))
                            attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(7).toLatin1()));
                        else
                            if (allowAttachments && q.startsWith("attachment="))
                                attachURLs.push_back(KUrl::fromPercentEncoding((*it).mid(11).toLatin1()));
                            else
                                if (q.startsWith("to="))
                                    address = address.isEmpty()? KUrl::fromPercentEncoding((*it).mid(3).toLatin1()): address + ',' + KUrl::fromPercentEncoding((*it).mid(3).toLatin1());
    }

    invokeMailer( address, cc, bcc, subject, body, QString(), attachURLs, startup_id );
}


static QStringList splitEmailAddressList( const QString & aStr )
{
    // This is a copy of KPIM::splitEmailAddrList().
    // Features:
    // - always ignores quoted characters
    // - ignores everything (including parentheses and commas)
    //   inside quoted strings
    // - supports nested comments
    // - ignores everything (including double quotes and commas)
    //   inside comments

    QStringList list;

    if (aStr.isEmpty())
        return list;

    QString addr;
    uint addrstart = 0;
    int commentlevel = 0;
    bool insidequote = false;

    for (int index=0; index<aStr.length(); index++) {
        // the following conversion to latin1 is o.k. because
        // we can safely ignore all non-latin1 characters
        switch (aStr[index].toLatin1()) {
        case '"' : // start or end of quoted string
            if (commentlevel == 0)
                insidequote = !insidequote;
            break;
        case '(' : // start of comment
            if (!insidequote)
                commentlevel++;
            break;
        case ')' : // end of comment
            if (!insidequote) {
                if (commentlevel > 0)
                    commentlevel--;
                else {
                    //kDebug() << "Error in address splitting: Unmatched ')'"
                    //          << endl;
                    return list;
                }
            }
            break;
        case '\\' : // quoted character
            index++; // ignore the quoted character
            break;
        case ',' :
            if (!insidequote && (commentlevel == 0)) {
                addr = aStr.mid(addrstart, index-addrstart);
                if (!addr.isEmpty())
                    list += addr.simplified();
                addrstart = index+1;
            }
            break;
        }
    }
    // append the last address to the list
    if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, aStr.length()-addrstart);
        if (!addr.isEmpty())
            list += addr.simplified();
    }
    //else
    //  kDebug() << "Error in address splitting: "
    //            << "Unexpected end of address list"
    //            << endl;

    return list;
}

void KToolInvocation::invokeMailer(const QString &address, const QString &subject, const QByteArray& startup_id)
{
    if (!isMainThreadActive())
        return;

    invokeMailer(address, QString(), QString(), subject, QString(), QString(),
                 QStringList(), startup_id );
}

void KToolInvocation::invokeMailer(const QString &_to, const QString &_cc, const QString &_bcc,
                                   const QString &subject, const QString &body,
                                   const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                   const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    KConfig config("emaildefaults");
    KConfigGroup defaultsGrp(&config, "Defaults");

    QString group = defaultsGrp.readEntry("Profile","Default");

    KConfigGroup profileGrp(&config, QString("PROFILE_%1").arg(group) );
    QString command = profileGrp.readPathEntry("EmailClient");

    QString to, cc, bcc;
    if (command.isEmpty() || command == QLatin1String("kmail")
        || command.endsWith("/kmail"))
    {
        command = QLatin1String("kmail --composer -s %s -c %c -b %b --body %B --attach %A -- %t");
        if ( !_to.isEmpty() )
        {
            // put the whole address lists into RFC2047 encoded blobs; technically
            // this isn't correct, but KMail understands it nonetheless
            to = QString( "=?utf8?b?%1?=" )
                 .arg( (const char*)KCodecs::base64Encode( _to.toUtf8(), false ) );
        }
        if ( !_cc.isEmpty() )
            cc = QString( "=?utf8?b?%1?=" )
                 .arg( (const char*)KCodecs::base64Encode( _cc.toUtf8(), false ) );
        if ( !_bcc.isEmpty() )
            bcc = QString( "=?utf8?b?%1?=" )
                  .arg( (const char*)KCodecs::base64Encode( _bcc.toUtf8(), false ) );
    } else {
        to = _to;
        cc = _cc;
        bcc = _bcc;
        if( !command.contains( '%' ))
            command += " %u";
    }

    if (profileGrp.readEntry("TerminalClient", false))
    {
        KConfigGroup confGroup( KGlobal::config(), "General" );
        QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", "konsole");
        command = preferredTerminal + " -e " + command;
    }

    QStringList cmdTokens = KShell::splitArgs(command);
    QString cmd = cmdTokens[0];
    cmdTokens.erase(cmdTokens.begin());

    KUrl url;
    //QStringList qry;
    if (!to.isEmpty())
    {
        QStringList tos = splitEmailAddressList( to );
        url.setPath( tos.first() );
        tos.erase( tos.begin() );
        for (QStringList::ConstIterator it = tos.begin(); it != tos.end(); ++it)
            url.addQueryItem("to",*it);
        //qry.append( "to=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    }
    const QStringList ccs = splitEmailAddressList( cc );
    for (QStringList::ConstIterator it = ccs.begin(); it != ccs.end(); ++it)
        url.addQueryItem("cc",*it);
    //qry.append( "cc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    const QStringList bccs = splitEmailAddressList( bcc );
    for (QStringList::ConstIterator it = bccs.begin(); it != bccs.end(); ++it)
        url.addQueryItem("bcc",*it);
    //qry.append( "bcc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    for (QStringList::ConstIterator it = attachURLs.begin(); it != attachURLs.end(); ++it)
        url.addQueryItem("attach",*it);
    //qry.append( "attach=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    if (!subject.isEmpty())
        url.addQueryItem("subject",subject);
    //qry.append( "subject=" + QLatin1String(KUrl::toPercentEncoding( subject ) ));
    if (!body.isEmpty())
        url.addQueryItem("body",body);
    //qry.append( "body=" + QLatin1String(KUrl::toPercentEncoding( body ) ));
    //url.setQuery( qry.join( "&" ) );

    if ( ! (to.isEmpty() && (!url.hasQuery())) )
        url.setProtocol("mailto");

    QHash<QChar, QString> keyMap;
    keyMap.insert('t', to);
    keyMap.insert('s', subject);
    keyMap.insert('c', cc);
    keyMap.insert('b', bcc);
    keyMap.insert('B', body);
    keyMap.insert('u', url.url());

    QString attachlist = attachURLs.join(",");
    attachlist.prepend('\'');
    attachlist.append('\'');
    keyMap.insert('A', attachlist);

    for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); )
    {
        if (*it == "%A")
        {
            if (it == cmdTokens.begin()) // better safe than sorry ...
                continue;
            QStringList::ConstIterator urlit = attachURLs.begin();
            QStringList::ConstIterator urlend = attachURLs.end();
            if ( urlit != urlend )
            {
                QStringList::Iterator previt = it;
                --previt;
                *it = *urlit;
                ++it;
                while ( ++urlit != urlend )
                {
                    cmdTokens.insert( it, *previt );
                    cmdTokens.insert( it, *urlit );
                }
            } else {
                --it;
                it = cmdTokens.erase( cmdTokens.erase( it ) );
            }
        } else {
            *it = KMacroExpander::expandMacros(*it, keyMap);
            ++it;
        }
    }

    QString error;
    // TODO this should check if cmd has a .desktop file, and use data from it, together
    // with sending more ASN data
    if (kdeinitExec(cmd, cmdTokens, &error, NULL, startup_id ))
    {}
    KMessage::message(KMessage::Error,
                      i18n("Could not launch the mail client:\n\n%1", error),
                      i18n("Could not Launch Mail Client"));
}

void KToolInvocation::invokeBrowser( const QString &url, const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    QString error;

    if (startServiceByDesktopName("kfmclient", url, &error, 0, 0, startup_id, false))
    {
        KMessage::message(KMessage::Error,
                          i18n("Could not launch the browser:\n\n%1", error),
                          i18n("Could not Launch Browser"));
    }
}

static int my_system (const char *command) {
   int pid, status;

   QCoreApplication::flush();
   pid = fork();
   if (pid == -1)
      return -1;
   if (pid == 0) {
      const char* shell = "/bin/sh";
      execl(shell, shell, "-c", command, (void *)0);
      ::_exit(127);
   }
   do {
      if (waitpid(pid, &status, 0) == -1) {
         if (errno != EINTR)
            return -1;
       } else
            return status;
   } while(1);
}


void KToolInvocation::startKdeinit()
{
  // Try to launch kdeinit.
  QString srv = KStandardDirs::findExe(QLatin1String("kdeinit"));
  if (srv.isEmpty())
     return;
//   this is disabled because we are in kdecore
//  const bool gui = qApp && qApp->type() != QApplication::Tty;
//  if ( gui )
//    qApp->setOverrideCursor( Qt::WaitCursor );
  my_system(QFile::encodeName(srv)+" --suicide");
//  if ( gui )
//    qApp->restoreOverrideCursor();
}
