/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <kcrash.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kuniqueapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <soundserver.h>
#include <dispatcher.h>
#include <qiomanager.h>

#include "knotify.h"
#include "knotify.moc"

#include <qfile.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <iostream.h>
#include <qtextstream.h>

class KNotifyPrivate
{
public:
    KConfig* globalEvents;
    KConfig* globalConfig;
    QMap<QString, KConfig*> events;
    QMap<QString, KConfig*> configs;
    QString externalPlayer;
    Arts::SimpleSoundServer soundServer;
    bool useExternal;
};

Arts::SimpleSoundServer connectSoundServer()
{
    static bool firstTime = true;

    /*
     * obtain an object reference to the soundserver - if we're doing it
     * for the first time, retry sometimes, so it will work during the
     * startup sequence, even if artsd is started some time after the first
     * process requests knotify to do some notifications
     */
    Arts::SimpleSoundServer result;
    result = Arts::Reference("global:Arts_SimpleSoundServer");
    if ( firstTime && result.isNull() )
        for( int tries=0; tries<7; tries++ )
        {
            sleep( 1 );
            result = Arts::Reference("global:Arts_SimpleSoundServer");
            if( !result.isNull() ) break;
        }

    firstTime = false;
    return result;
}

int main(int argc, char **argv)
{
    KAboutData aboutdata("knotify", I18N_NOOP("KNotify"),
                         "2.0pre", I18N_NOOP("KDE Notification Server"),
                         KAboutData::License_GPL, "(C) 1997-2000, KDE Developers");
    aboutdata.addAuthor("Christian Esken",0,"esken@kde.org");
    aboutdata.addAuthor("Stefan Westerfeld",I18N_NOOP("Sound support"),"stefan@space.twc.de");
    aboutdata.addAuthor("Charles Samuels",I18N_NOOP("Current Maintainer"),"charles@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutdata );
    KUniqueApplication::addCmdLineOptions();


    // initialize application
    if ( !KUniqueApplication::start() ) {
        kdDebug() << "Running knotify found" << endl;
        return 0;
    }

    KUniqueApplication app;
    app.disableSessionManagement();

    // setup mcop communication
    Arts::Dispatcher dispatcher;

    // start notify service
    KNotify notify;
    app.dcopClient()->setDefaultObject( "Notify" );
    app.dcopClient()->setDaemonMode( true );
    return app.exec();
}


KNotify::KNotify()
    : QObject(), DCOPObject("Notify")
{
    d = new KNotifyPrivate;
    d->soundServer = Arts::SimpleSoundServer::null();
    d->globalEvents = new KConfig("knotify/eventsrc", true, false, "data");
    d->globalConfig = new KConfig("knotify.eventsrc", true, false);
    loadConfig();
}

KNotify::~KNotify()
{
    reconfigure();
    delete d->globalEvents;
    delete d->globalConfig;
    delete d;
}


void KNotify::loadConfig() {
    // load external player settings
    KConfig *kc = kapp->config();
    kc->setGroup("Misc");
    d->useExternal = kc->readBoolEntry( "Use external player", false );
    d->externalPlayer = kc->readEntry("External player");

    // try to locate a suitable player if none is configured
    if ( d->externalPlayer.isEmpty() ) {
	QStringList players;
	players << "wavplay" << "aplay" << "auplay";
	QStringList::Iterator it = players.begin();
	while ( d->externalPlayer.isEmpty() && it != players.end() ) {
	    d->externalPlayer = KStandardDirs::findExe( *it );
	    ++it;
	}
    }
}


void KNotify::reconfigure()
{
    kapp->config()->reparseConfiguration();
    loadConfig();

    // clear loaded config files
    d->globalConfig->reparseConfiguration();
    for ( QMapIterator<QString,KConfig*> it = d->configs.begin(); it != d->configs.end(); ++it )
        delete it.data();
    d->configs.clear();
}


void KNotify::notify(const QString &event, const QString &fromApp,
                     const QString &text, QString sound, QString file,
                     int present, int level)
{
//    kdDebug() << "event=" << event << " fromApp=" << fromApp << " text=" << text << " sound=" << sound <<
//        " file=" << file << " present=" << present << " level=" << level << endl;

    // check for valid events
    if ( event.length()>0 )     {

        // get config file
        KConfig *eventsFile;
        KConfig *configFile;
	if ( d->events.contains( fromApp ) ) {
	    eventsFile = d->events[fromApp];
	} else {
	    eventsFile=new KConfig(locate("data", fromApp+"/eventsrc"),true,false);
	    d->events.insert( fromApp, eventsFile );
        }
        if ( d->configs.contains( fromApp) ) {
            configFile = d->configs[fromApp];
        } else {
	    configFile=new KConfig(fromApp+".eventsrc",true,false);
	    d->configs.insert( fromApp, configFile );
	}

	if ( !eventsFile->hasGroup( event ) && isGlobal(event) )
        {
	    eventsFile = d->globalEvents;
            configFile = d->globalConfig;
        }

        eventsFile->setGroup( event );
        configFile->setGroup( event );

        // get event presentation
        if ( present==-1 )
            present = configFile->readNumEntry( "presentation", -1 );
        if ( present==-1 )
            present = eventsFile->readNumEntry( "default_presentation", 0 );

        // get sound file name
        sound = configFile->readEntry( "soundfile" );
        if ( sound.length()==0 )
            sound = eventsFile->readEntry( "default_sound" );

        // get log file name
        file = configFile->readEntry( "logfile" );
        if ( file.length()==0 )
            file = eventsFile->readEntry( "default_logfile" );

        // get default event level
        level = eventsFile->readNumEntry( "level", 0 );
    }

    // emit event
    if ( present & KNotifyClient::Sound ) // && QFile(sound).isReadable()
        notifyBySound( sound );

    if ( present & KNotifyClient::Messagebox )
        notifyByMessagebox( text, level );

    if ( present & KNotifyClient::Logfile ) // && QFile(file).isWritable()
        notifyByLogfile( text, file );

    if ( present & KNotifyClient::Stderr )
        notifyByStderr( text );
}


bool KNotify::notifyBySound( const QString &sound )
{
    bool external = d->useExternal && !d->externalPlayer.isEmpty();
    // get file name
    QString soundFile(sound);
    if ( QFileInfo(sound).isRelative() )
	soundFile = locate( "sound", sound );

    // Oh dear! we seem to have lost our connection to artsd!
    if( !external && (d->soundServer.isNull() || d->soundServer.error()) )
        d->soundServer = connectSoundServer();

    // kdDebug() << "KNotify::notifyBySound - trying to play file " << soundFile << endl;

    if (!external && !d->soundServer.isNull() && !d->soundServer.error()) {
        // play sound finally
        d->soundServer.play( QFile::encodeName(soundFile).data() );

        return true;

    } else if(!d->externalPlayer.isEmpty()) {
        // use an external player to play the sound
        system( QFile::encodeName( d->externalPlayer ) + " " +
                QFile::encodeName( soundFile ));
        return true;
    }
    return false;
}


bool KNotify::notifyByMessagebox(const QString &text, int level)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return false;

    // display message box for specified event level
    switch( level ) {
    default:
    case KNotifyClient::Notification:
        KMessageBox::information( 0, text, i18n("Notification"), 0, false );
        break;
    case KNotifyClient::Warning:
        KMessageBox::sorry( 0, text, i18n("Warning"), false );
        break;
    case KNotifyClient::Error:
        KMessageBox::error( 0, text, i18n("Error"), false );
        break;
    case KNotifyClient::Catastrophe:
        KMessageBox::error( 0, text, i18n("Catastrophe!"), false );
        break;
    }

    return true;
}


bool KNotify::notifyByLogfile(const QString &text, const QString &file)
{
    // open file in append mode
    QFile logFile(file);
    if ( !logFile.open(IO_WriteOnly | IO_Append) ) return false;

    // append msg
    QTextStream strm( &logFile );
    strm << "- KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    // close file
    logFile.close();
    return true;
}

bool KNotify::notifyByStderr(const QString &text)
{
    // open stderr for output
    QTextStream strm( stderr, IO_WriteOnly );

    // output msg
    strm << "KNotify " << QDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    return true;
}


bool KNotify::isGlobal(const QString &eventname)
{
    return d->globalEvents->hasGroup( eventname );
}
