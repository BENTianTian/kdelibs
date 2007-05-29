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

#ifdef Q_OS_WIN
#include <windows.h>

static QString getKde4Prefix()
{
    static QString modFilePath;
    if(modFilePath.isEmpty()) {
        wchar_t module_name[256];
        GetModuleFileNameW(0, module_name, sizeof(module_name) / sizeof(wchar_t));
        modFilePath = QString::fromUtf16((ushort *)module_name);
        int idx = modFilePath.lastIndexOf('\\');
        if(idx != -1)
            modFilePath = modFilePath.left(idx);
        modFilePath = QDir(modFilePath + "/../").canonicalPath();
    }
    return modFilePath;
}

static const QString prefix        = getKde4Prefix();
static const QString exec_prefix   = getKde4Prefix();
static const QString libdir        = getKde4Prefix() + "/lib";
static const QString includedir    = getKde4Prefix() + "/include";
static const QString sysconfdir    = getKde4Prefix() + "/etc";
static const QString share         = getKde4Prefix() + "/share";
static const QString datadir       = share + QLatin1String("/apps");
static const QString kde_appsdir   = share + QLatin1String("/applnk");
static const QString kde_confdir   = share + QLatin1String("/config");
static const QString kde_kcfgdir   = share + QLatin1String("/config.kcfg");
static const QString kde_datadir   = share + QLatin1String("/apps");
static const QString kde_bindir    = getKde4Prefix();
static const QString kde_htmldir   = share + QLatin1String("/doc/HTML");
static const QString kde_icondir   = share + QLatin1String("/icons");
static const QString kde_moduledir = libdir + QLatin1String("/kde4");
static const QString kde_locale    = share + QLatin1String("/locale");
static const QString kde_mimedir   = share + QLatin1String("/mimelnk");
static const QString kde_servicesdir     = share + QLatin1String("/services");
static const QString kde_servicetypesdir = share + QLatin1String("/servicetypes");
static const QString kde_sounddir        = share + QLatin1String("/sounds");
static const QString kde_templatesdir    = share + QLatin1String("/templates");
static const QString kde_wallpaperdir    = share + QLatin1String("/wallpapers");
static const QString xdg_menudir         = share + QLatin1String("/currently/undefined");
static const QString xdg_appsdir         = share + QLatin1String("/applications/kde4");
static const QString xdg_directorydir    = share + QLatin1String("/desktop-directories");
#else
static const QString prefix        = QString(QLatin1String(KDEDIR));
static const QString exec_prefix   = QString(QLatin1String(EXEC_INSTALL_PREFIX));
static const QString libdir        = QString(QLatin1String(LIB_INSTALL_DIR));
static const QString includedir    = QString(QLatin1String(INCLUDE_INSTALL_DIR));
static const QString sysconfdir    = QString(QLatin1String(SYSCONF_INSTALL_DIR));
static const QString datadir       = QString(QLatin1String(DATA_INSTALL_DIR));
static const QString kde_appsdir   = QString(QLatin1String(APPLNK_INSTALL_DIR));
static const QString kde_confdir   = QString(QLatin1String(CONFIG_INSTALL_DIR));
static const QString kde_kcfgdir   = QString(QLatin1String(KCFG_INSTALL_DIR));
static const QString kde_datadir   = QString(QLatin1String(DATA_INSTALL_DIR));
static const QString kde_bindir    = QString(QLatin1String(BIN_INSTALL_DIR));
static const QString kde_htmldir   = QString(QLatin1String(HTML_INSTALL_DIR));
static const QString kde_icondir   = QString(QLatin1String(ICON_INSTALL_DIR));
static const QString kde_moduledir = QString(QLatin1String(PLUGIN_INSTALL_DIR));
static const QString kde_locale    = QString(QLatin1String(LOCALE_INSTALL_DIR));
static const QString kde_mimedir   = QString(QLatin1String(MIME_INSTALL_DIR));
static const QString kde_servicesdir     = QString(QLatin1String(SERVICES_INSTALL_DIR));
static const QString kde_servicetypesdir = QString(QLatin1String(SERVICETYPES_INSTALL_DIR));
static const QString kde_sounddir        = QString(QLatin1String(SOUND_INSTALL_DIR));
static const QString kde_templatesdir    = QString(QLatin1String(TEMPLATES_INSTALL_DIR));
static const QString kde_wallpaperdir    = QString(QLatin1String(WALLPAPER_INSTALL_DIR));
static const QString xdg_menudir         = QString(QLatin1String(SYSCONF_INSTALL_DIR "/xdg/menus"));
static const QString xdg_appsdir         = QString(QLatin1String(XDG_APPS_DIR));
static const QString xdg_directorydir    = QString(QLatin1String(XDG_DIRECTORY_DIR));
#endif

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
        printResult(prefix.toLocal8Bit());
        return 0;
    }

    if (args->isSet("exec-prefix"))
    {
        printResult(exec_prefix.toLocal8Bit());
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
        static QString installprefixes[] = {
            "apps",   kde_appsdir,
            "config", kde_confdir,
            "kcfg",   kde_kcfgdir,
            "data",   kde_datadir,
            "exe",    kde_bindir,
            "html",   kde_htmldir,
            "icon",   kde_icondir,
            "lib",    libdir,
            "module", kde_moduledir,
            "qtplugins", kde_moduledir + QLatin1String("/plugins"),
            "locale", kde_locale,
            "mime",   kde_mimedir,
            "services", kde_servicesdir,
            "servicetypes", kde_servicetypesdir,
            "sound", kde_sounddir,
            "templates", kde_templatesdir,
            "wallpaper", kde_wallpaperdir,
            "xdgconf-menu", xdg_menudir,
            "xdgdata-apps", xdg_appsdir,
            "xdgdata-dirs", xdg_directorydir,
            "include", includedir,
            QString(), QString()
        };
        int index = 0;
        while (!installprefixes[index].isEmpty() && type != installprefixes[index]) {
            index += 2;
        }
        if (!installprefixes[index].isEmpty()) {
            printResult(installprefixes[index+1].toLocal8Bit());
        } else {
            printResult("NONE"); // no i18n here as for scripts
        }
    }
    return 0;
}
