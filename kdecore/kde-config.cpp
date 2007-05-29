// -*- c++ -*-

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <qdir.h>
#include <stdio.h>
#include <kaboutdata.h>
#include <kdeversion.h>
#include <QDir>
#include <config.h>
#include <config-prefix.h>
#include <kconfiggroup.h>

#ifdef Q_WS_WIN
#include <win32_utils.h>
#endif

static const char *description = I18N_NOOP("A little program to output installation paths");

static KCmdLineOptions options[] =
{
    { "expandvars", I18N_NOOP("Left for legacy support"), 0 },
    { "prefix",	   I18N_NOOP("Compiled in prefix for KDE libraries"), 0 },
    { "exec-prefix", I18N_NOOP("Compiled in exec_prefix for KDE libraries"), 0 },
    { "libsuffix", I18N_NOOP("Compiled in library path suffix"), 0 },
    { "localprefix", I18N_NOOP("Prefix in $HOME used to write files"), 0},
    { "version",   I18N_NOOP("Compiled in version string for KDE libraries"), 0 },
    { "types",     I18N_NOOP("Available KDE resource types"), 0 },
    { "path type", I18N_NOOP("Search path for resource type"), 0 },
    { "locate filename", I18N_NOOP("Find filename inside the resource type given to --path"), 0},
    { "userpath type", I18N_NOOP("User path: desktop|autostart|document"), 0 },
    { "install type", I18N_NOOP("Prefix to install resource files to"), 0},
    { 0,0,0 }
};

static void printResult(const QString &s)
{
    if (s.isEmpty())
        printf("\n");
	else {
		QString path = QDir::convertSeparators( s );
        printf("%s\n", path.toLocal8Bit().constData());
	}
}

int main(int argc, char **argv)
{
    KLocale::setMainCatalog("kdelibs");
    KAboutData about("kde4-config", "kde4-config", "1.0", description, KAboutData::License_GPL, "(C) 2000 Stephan Kulow");
    KCmdLineArgs::init( argc, argv, &about);

    KCmdLineArgs::addCmdLineOptions( options ); // Add my own options.

    KComponentData a("kde4-config");
    (void)KGlobal::dirs(); // trigger the creation
    (void)KGlobal::config();

    // Get application specific arguments
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("prefix"))
    {
        printResult(KDEDIR);
        return 0;
    }

    if (args->isSet("exec-prefix"))
    {
        printResult(EXEC_INSTALL_PREFIX);
        return 0;
    }

    if (args->isSet("libsuffix"))
    {
        QString tmp(KDELIBSUFF);
        tmp.remove('"');
        printResult(tmp.toLocal8Bit());
        return 0;
    }

    if (args->isSet("localprefix"))
    {
        printResult(KGlobal::dirs()->localkdedir());
        return 0;
    }

    if (args->isSet("version"))
    {
        printf("%s\n", KDE_VERSION_STRING);
        return 0;
    }

    if (args->isSet("types"))
    {
        QStringList types = KGlobal::dirs()->allTypes();
        types.sort();
        const char *helptexts[] = {
            "apps", I18N_NOOP("Applications menu (.desktop files)"),
            "cgi", I18N_NOOP("CGIs to run from kdehelp"),
            "config", I18N_NOOP("Configuration files"),
            "data", I18N_NOOP("Where applications store data"),
            "emoticons", I18N_NOOP("Emoticons"),
            "exe", I18N_NOOP("Executables in $prefix/bin"),
            "html", I18N_NOOP("HTML documentation"),
            "icon", I18N_NOOP("Icons"),
            "kcfg", I18N_NOOP("Configuration description files"),
            "lib", I18N_NOOP("Libraries"),
            "include", I18N_NOOP("Includes/Headers"),
            "locale", I18N_NOOP("Translation files for KLocale"),
            "mime", I18N_NOOP("Mime types"),
            "module", I18N_NOOP("Loadable modules"),
            "pixmap", I18N_NOOP("Legacy pixmaps"),
            "qtplugins", I18N_NOOP("Qt plugins"),
            "services", I18N_NOOP("Services"),
            "servicetypes", I18N_NOOP("Service types"),
            "sound", I18N_NOOP("Application sounds"),
            "templates", I18N_NOOP("Templates"),
            "wallpaper", I18N_NOOP("Wallpapers"),
            "xdgdata-apps", I18N_NOOP("XDG Application menu (.desktop files)"),
            "xdgdata-dirs", I18N_NOOP("XDG Menu descriptions (.directory files)"),
            "xdgdata-icon", I18N_NOOP("XDG Icons"),
            "xdgdata-pixmaps", I18N_NOOP("Legacy pixmaps"),
            "xdgdata-mime", I18N_NOOP("XDG Mime Types"),
            "xdgconf-menu", I18N_NOOP("XDG Menu layout (.menu files)"),
            "tmp", I18N_NOOP("Temporary files (specific for both current host and current user)"),
            "socket", I18N_NOOP("UNIX Sockets (specific for both current host and current user)"),
            0, 0
        };
        for (QStringList::ConstIterator it = types.begin(); it != types.end(); ++it)
        {
            int index = 0;
            while (helptexts[index] && *it != helptexts[index]) {
                index += 2;
            }
            if (helptexts[index]) {
                printf("%s - %s\n", helptexts[index], i18n(helptexts[index+1]).toLocal8Bit().data());
            } else {
                printf("%s", i18n("%1 - unknown type\n", *it).toLocal8Bit().data());
            }
        }
        return 0;
    }

    QString type = args->getOption("path");
    if (!type.isEmpty())
    {
        QString fileName = args->getOption("locate");
        if (!fileName.isEmpty())
        {
            QString result = KStandardDirs::locate(type.toLatin1(), fileName);
            if (!result.isEmpty())
                printf("%s\n", result.toLocal8Bit().constData());
            return result.isEmpty() ? 1 : 0;
        }

        printResult(KGlobal::dirs()->resourceDirs(type.toLatin1()).join(QString(KPATH_SEPARATOR)));
        return 0;
    }

    type = args->getOption("userpath");
    if (!type.isEmpty())
    {
        //code duplicated with KGlobalSettings::initPath()
        if ( type == "desktop" )
        {
            KConfigGroup g( KGlobal::config(), "Paths" );
            QString path=QDir::homePath() + "/Desktop/";
            path=g.readPathEntry( "Desktop", path);
            path=QDir::cleanPath( path );
            if ( !path.endsWith("/") )
              path.append(QLatin1Char('/'));
            printResult(path);
        }
        else if ( type == "autostart" )
        {
            KConfigGroup g( KGlobal::config(), "Paths" );
            QString path=QDir::homePath() + "/Autostart/";
            path=g.readPathEntry( "Autostart", path);
            path=QDir::cleanPath( path );
            if ( !path.endsWith("/") )
              path.append(QLatin1Char('/'));
            printResult(path);

        }
        else if ( type == "document" )
        {
            KConfigGroup g( KGlobal::config(), "Paths" );
#ifdef Q_WS_WIN
            QString path=getWin32ShellFoldersPath("Personal");
#else
            QString path=QDir::homePath();
#endif
            path=g.readPathEntry( "Desktop", path);
            path=QDir::cleanPath( path );
            if ( !path.endsWith("/") )
              path.append(QLatin1Char('/'));
            printResult(path);
        }
        else
            fprintf(stderr, "%s", i18n("%1 - unknown type of userpath\n", type).toLocal8Bit().data() );
        return 0;
    }

    type = args->getOption("install");
    if (!type.isEmpty())
    {
        printResult( KGlobal::dirs()->installPath(type.toLocal8Bit()) );
    }
    return 0;
}
