/*
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcmdlineargs.h"

#include <config.h>

#include <sys/param.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QHash>

#include "kaboutdata.h"
#include "klocale.h"
#include "kdeversion.h"
#include "kcomponentdata.h"
#include "kglobal.h"
#include "kstringhandler.h"
#include "kurl.h"

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#else
#define DISPLAY "NODISPLAY"
#endif

#ifdef Q_WS_WIN
#include <win32_utils.h>
#endif

static const KCmdLineOptions qt_options[] =
{
  //FIXME: Check if other options are specific to Qt/X11
#ifdef Q_WS_X11
   { "display <displayname>", I18N_NOOP("Use the X-server display 'displayname'"), 0},
#elif defined(Q_WS_QWS)
   { "display <displayname>", I18N_NOOP("Use the QWS display 'displayname'"), 0},
#else
#endif
   { "session <sessionId>", I18N_NOOP("Restore the application for the given 'sessionId'"), 0},
   { "cmap", I18N_NOOP("Causes the application to install a private color\nmap on an 8-bit display"), 0},
   { "ncols <count>", I18N_NOOP("Limits the number of colors allocated in the color\ncube on an 8-bit display, if the application is\nusing the QApplication::ManyColor color\nspecification"), 0},
   { "nograb", I18N_NOOP("tells Qt to never grab the mouse or the keyboard"), 0},
   { "dograb", I18N_NOOP("running under a debugger can cause an implicit\n-nograb, use -dograb to override"), 0},
   { "sync", I18N_NOOP("switches to synchronous mode for debugging"), 0},
   { "fn", 0, 0},
   { "font <fontname>", I18N_NOOP("defines the application font"), 0},
   { "bg", 0, 0},
   { "background <color>", I18N_NOOP("sets the default background color and an\napplication palette (light and dark shades are\ncalculated)"), 0},
   { "fg", 0, 0},
   { "foreground <color>", I18N_NOOP("sets the default foreground color"), 0},
   { "btn", 0, 0},
   { "button <color>", I18N_NOOP("sets the default button color"), 0},
   { "name <name>", I18N_NOOP("sets the application name"), 0},
   { "title <title>", I18N_NOOP("sets the application title (caption)"), 0},
#ifdef Q_WS_X11
   { "visual TrueColor", I18N_NOOP("forces the application to use a TrueColor visual on\nan 8-bit display"), 0},
   { "inputstyle <inputstyle>", I18N_NOOP("sets XIM (X Input Method) input style. Possible\nvalues are onthespot, overthespot, offthespot and\nroot"), 0 },
   { "im <XIM server>", I18N_NOOP("set XIM server"),0},
   { "noxim", I18N_NOOP("disable XIM"), 0 },
#endif
#ifdef Q_WS_QWS
   { "qws", I18N_NOOP("forces the application to run as QWS Server"), 0},
#endif
   { "reverse", I18N_NOOP("mirrors the whole layout of widgets"), 0},
   KCmdLineLastOption
};

static const KCmdLineOptions kde_options[] =
{
   { "caption <caption>",       I18N_NOOP("Use 'caption' as name in the titlebar"), 0},
   { "icon <icon>",             I18N_NOOP("Use 'icon' as the application icon"), 0},
   { "config <filename>",       I18N_NOOP("Use alternative configuration file"), 0},
   { "nocrashhandler",          I18N_NOOP("Disable crash handler, to get core dumps"), 0},
   { "waitforwm",          I18N_NOOP("Waits for a WM_NET compatible windowmanager"), 0},
   { "style <style>", I18N_NOOP("sets the application GUI style"), 0},
   { "geometry <geometry>", I18N_NOOP("sets the client geometry of the main widget - see man X for the argument format"), 0},
   { "smkey <sessionKey>", 0, 0}, // this option is obsolete and exists only to allow smooth upgrades from sessions
                                  // saved under Qt 3.0.x -- Qt 3.1.x includes the session key now automatically in
                                  // the session id (Simon)
   KCmdLineLastOption
};



class KCmdLineParsedOptions : public QHash<QByteArray,QByteArray>
{
public:
   KCmdLineParsedOptions() { }
};

class KCmdLineParsedArgs : public QList<QByteArray>
{
public:
   KCmdLineParsedArgs() { }
};


class KCmdLineArgsList: public QList<KCmdLineArgs*>
{
public:
   KCmdLineArgsList() { }
   ~KCmdLineArgsList() {
	   while (count())
		delete takeFirst();
   }
};

class KCmdLineArgs::Private
{
    public:
        Private(const KCmdLineOptions *_options, const char *_name, const char *_id)
            : options(_options)
            , name(_name)
            , id(_id)
            , parsedOptionList(0)
            , parsedArgList(0)
            , isQt(::qstrcmp(id, "qt") == 0)
        {}
        ~Private()
        {
            delete parsedOptionList;
            delete parsedArgList;
        }
        const KCmdLineOptions *options;
        const char *name;
        const char *id;
        KCmdLineParsedOptions *parsedOptionList;
        KCmdLineParsedArgs *parsedArgList;
        bool isQt;
};

KCmdLineArgsList *KCmdLineArgs::argsList = 0;
int KCmdLineArgs::argc = 0;
char **KCmdLineArgs::argv = 0;
QString KCmdLineArgs::mCwd = QString();
const KAboutData *KCmdLineArgs::about = 0;
bool KCmdLineArgs::parsed = false;
bool KCmdLineArgs::ignoreUnknown = false;
KCmdLineArgs::StdCmdLineArgs KCmdLineArgs::mStdargs = 0;

//
// Static functions
//

void
KCmdLineArgs::init(int _argc, char **_argv, const char *_appname, const char* programName,
                   const char *_description, const char *_version, StdCmdLineArgs stdargs)
{
   init(_argc, _argv,
        new KAboutData(_appname, programName, _version, _description),
        stdargs);
}

void
KCmdLineArgs::initIgnore(int _argc, char **_argv, const char *_appname )
{
   init(_argc, _argv,
        new KAboutData(_appname, _appname, "unknown", "KDE Application"),CmdLineArgNone);
   ignoreUnknown = true;
}

void
KCmdLineArgs::init(const KAboutData* ab)
{
   char **_argv = (char **) malloc(sizeof(char *));
   _argv[0] = (char *) ab->appName();
   init(1,_argv,ab, CmdLineArgNone);
}


void
KCmdLineArgs::init(int _argc, char **_argv, const KAboutData *_about, StdCmdLineArgs stdargs)
{
   argc = _argc;
   argv = _argv;

   if (!argv)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Passing null-pointer to 'argv' is not allowed.\n\n");

      assert( 0 );
      exit(255);
   }

   // Strip path from argv[0]
   if (argc) {
     char *p = strrchr( argv[0], '/');
     if (p)
       argv[0] = p+1;
   }

   about = _about;
   parsed = false;
   mCwd = QDir::currentPath();
   addStdCmdLineOptions(stdargs);
}

const QString &KCmdLineArgs::cwd()
{
   return mCwd;
}

const char * KCmdLineArgs::appName()
{
   if (!argc) return 0;
   return argv[0];
}

/**
  * Add Qt and KDE command line options to KCmdLineArgs.
  */
void KCmdLineArgs::addStdCmdLineOptions(StdCmdLineArgs stdargs) {
   if (stdargs & KCmdLineArgs::CmdLineArgQt) {
       KCmdLineArgs::addCmdLineOptions(qt_options, "Qt", "qt");
   }
   if (stdargs & KCmdLineArgs::CmdLineArgKDE) {
       KCmdLineArgs::addCmdLineOptions(kde_options, "KDE", "kde");
   }
   mStdargs = stdargs;
}

void
KCmdLineArgs::addCmdLineOptions( const KCmdLineOptions *options, const char *name,
         const char *id, const char *afterId)
{
   if (!argsList)
      argsList = new KCmdLineArgsList;

   int pos = argsList->count();

   if (pos && id && argsList->last() && !argsList->last()->d->name)
      pos--;

   KCmdLineArgsList::Iterator args;
   int i = 0;
   for(args = argsList->begin(); args != argsList->end(); ++args, i++)
   {
      if (!id && !(*args)->d->id)
         return; // Options already present.

      if (id && (*args)->d->id && (::qstrcmp(id, (*args)->d->id) == 0))
   return; // Options already present.

      if (afterId && (*args)->d->id && (::qstrcmp(afterId, (*args)->d->id) == 0))
         pos = i+1;
   }

   Q_ASSERT( parsed == false ); // You must add _ALL_ cmd line options
                                // before accessing the arguments!
   argsList->insert(pos, new KCmdLineArgs(options, name, id));
}

void
KCmdLineArgs::saveAppArgs( QDataStream &ds)
{
   if (!parsed)
      parseAllArgs();

   // Remove Qt and KDE options.
   removeArgs("qt");
   removeArgs("kde");

   QByteArray qCwd = QFile::encodeName(mCwd);
   ds << qCwd;

   uint count = argsList ? argsList->count() : 0;
   ds << count;

   if (!count) return;

   KCmdLineArgsList::Iterator args;
   for(args = argsList->begin(); args != argsList->end(); ++args)
   {
      ds << QByteArray((*args)->d->id);
      (*args)->save(ds);
   }
}

void
KCmdLineArgs::loadAppArgs( QDataStream &ds)
{
   parsed = true; // don't reparse argc/argv!

   // Remove Qt and KDE options.
   removeArgs("qt");
   removeArgs("kde");

   KCmdLineArgsList::Iterator args;
   if ( argsList ) {
      for(args = argsList->begin(); args != argsList->end(); ++args)
      {
         (*args)->clear();
      }
   }

   if (ds.atEnd())
      return;

   QByteArray qCwd;
   ds >> qCwd;

   mCwd = QFile::decodeName(qCwd);

   uint count;
   ds >> count;

   while(count--)
   {
     QByteArray id;
     ds >> id;
     Q_ASSERT( argsList );
     for(args = argsList->begin(); args != argsList->end(); ++args)
     {
       if ((*args)->d->id  == id)
       {
          (*args)->load(ds);
          break;
       }
     }
   }
   parsed = true;
}

KCmdLineArgs *KCmdLineArgs::parsedArgs(const char *id)
{
   if (!argsList)
      return 0;
   KCmdLineArgsList::Iterator args = argsList->begin();
   while(args != argsList->end())
   {
      if ((id && ::qstrcmp((*args)->d->id, id) == 0) || (!id && !(*args)->d->id))
      {
          if (!parsed)
             parseAllArgs();
          return *args;
      }
      ++args;
   }

   return 0;
}

void KCmdLineArgs::removeArgs(const char *id)
{
   if (!argsList)
      return;
   KCmdLineArgsList::Iterator args = argsList->begin();
   while(args != argsList->end())
   {
      if ((*args)->d->id && id && ::qstrcmp((*args)->d->id, id) == 0)
      {
          if (!parsed)
             parseAllArgs();
          break;
      }
      ++args;
   }

   if (args != argsList->end()) {
      KCmdLineArgs *a = *args;
      argsList->erase(args);
      delete a;
   }
}

/*
 * @return:
 *  0 - option not found.
 *  1 - option found      // -fork
 *  2 - inverse option found ('no') // -nofork
 *  3 - option + arg found    // -fork now
 *
 *  +4 - no more options follow         // !fork
 */
static int
findOption(const KCmdLineOptions *options, QByteArray &opt,
           const char *&opt_name, const char *&def, bool &enabled)
{
   int result;
   bool inverse;
   int len = opt.length();
   while(options && options->name)
   {
      result = 0;
      inverse = false;
      opt_name = options->name;
      if ((opt_name[0] == ':') || (opt_name[0] == 0))
      {
         options++;
         continue;
      }

      if (opt_name[0] == '!')
      {
         opt_name++;
         result = 4;
      }
      if ((opt_name[0] == 'n') && (opt_name[1] == 'o'))
      {
         opt_name += 2;
         inverse = true;
      }
      if (strncmp(opt.data(), opt_name, len) == 0)
      {
         opt_name += len;
         if (!opt_name[0])
         {
            if (inverse)
               return result+2;

            if (!options->description)
            {
               options++;
               if (!options->name)
                  return result+0;
               QByteArray nextOption = options->name;
               int p = nextOption.indexOf(' ');
               if (p > 0)
                  nextOption = nextOption.left(p);
               if (strncmp(nextOption.data(), "no", 2) == 0)
               {
                  nextOption = nextOption.mid(2);
                  enabled = !enabled;
               }
               result = findOption(options, nextOption, opt_name, def, enabled);
               Q_ASSERT(result);
               opt = nextOption;
               return result;
            }

            return 1;
         }
         if (opt_name[0] == ' ')
         {
            opt_name++;
            def = options->def;
            return result+3;
         }
      }

      options++;
   }
   return 0;
}


void
KCmdLineArgs::findOption(const char *_opt, QByteArray opt, int &i, bool _enabled, bool &moreOptions) // krazy:exclude=passbyvalue
{
   KCmdLineArgsList::Iterator args = argsList->begin();
   const char *opt_name;
   const char *def;
   QByteArray argument;
   int j = opt.indexOf('=');
   if (j != -1)
   {
      argument = opt.mid(j+1);
      opt = opt.left(j);
   }

   bool enabled = true;
   int result = 0;
   while (args != argsList->end())
   {
      enabled = _enabled;
      result = ::findOption((*args)->d->options, opt, opt_name, def, enabled);
      if (result) break;
      ++args;
   }
   if ((args == argsList->end()) &&
       (_opt[0] == '-') && _opt[1] && (_opt[1] != '-'))
   {
      // Option not found check if it is a valid option
      // in the style of -Pprinter1 or ps -aux
      int p = 1;
      while (true)
      {
         QByteArray singleCharOption = " ";
         singleCharOption[0] = _opt[p];
         args = argsList->begin();
         while (args != argsList->end())
         {
            enabled = _enabled;
            result = ::findOption((*args)->d->options, singleCharOption,
			          opt_name, def, enabled);
            if (result) break;
            ++args;
         }
         if (args == argsList->end())
            break; // Unknown argument

         p++;
         if (result == 1) // Single option
         {
            (*args)->setOption(singleCharOption, enabled);
            if (_opt[p])
               continue; // Next option
            else
               return; // Finished
         }
         else if (result == 3) // This option takes an argument
         {
            if (argument.isEmpty())
            {
               argument = _opt+p;
            }
            (*args)->setOption(singleCharOption, argument.data());
            return;
         }
         break; // Unknown argument
      }
      args = argsList->end();
      result = 0;
   }

   if (args == argsList->end() || !result)
   {
      if (ignoreUnknown)
         return;
#ifdef Q_WS_MACX
		if (QString::fromLocal8Bit(_opt).startsWith("-psn_", Qt::CaseInsensitive))
			return;
#endif
      enable_i18n();
      usage( i18n("Unknown option '%1'.", QString::fromLocal8Bit(_opt)));
   }

   if ((result & 4) != 0)
   {
      result &= ~4;
      moreOptions = false;
   }

   if (result == 3) // This option takes an argument
   {
      if (!enabled)
      {
         if (ignoreUnknown)
            return;
#ifdef Q_WS_MACX
			if (QString::fromLocal8Bit(_opt).startsWith("-psn_", Qt::CaseInsensitive))
				return;
#endif
         enable_i18n();
         usage( i18n("Unknown option '%1'.", QString::fromLocal8Bit(_opt)));
      }
      if (argument.isEmpty())
      {
         i++;
         if (i >= argc)
         {
            enable_i18n();
            usage( i18n("'%1' missing.",  opt_name));
         }
         argument = argv[i];
      }
      (*args)->setOption(opt, argument.data());
   }
   else
   {
      (*args)->setOption(opt, enabled);
   }
}

void
KCmdLineArgs::printQ(const QString &msg)
{
   QByteArray localMsg = msg.toLocal8Bit();
   fprintf(stdout, "%s", localMsg.data());
}

void
KCmdLineArgs::parseAllArgs()
{
   bool allowArgs = false;
   bool inOptions = true;
   bool everythingAfterArgIsArgs = false;
   KCmdLineArgs *appOptions = argsList->last();
   if (!appOptions->d->id)
   {
     const KCmdLineOptions *option = appOptions->d->options;
     while(option && option->name)
     {
       if (option->name[0] == '+')
           allowArgs = true;
       if ( option->name[0] == '!' && option->name[1] == '+' )
       {
           allowArgs = true;
           everythingAfterArgIsArgs = true;
       }
       option++;
     }
   }
   for(int i = 1; i < argc; i++)
   {
      if (!argv[i])
         continue;

      if ((argv[i][0] == '-') && argv[i][1] && inOptions)
      {
         bool enabled = true;
         const char *option = &argv[i][1];
         const char *orig = argv[i];
         if (option[0] == '-')
         {
            option++;
            argv[i]++;
            if (!option[0])
            {
               inOptions = false;
               continue;
            }
         }
         if (::qstrcmp(option, "help") == 0)
         {
            usage(0);
         }
         else if (strncmp(option, "help-",5) == 0)
         {
            usage(option+5);
         }
         else if ( (::qstrcmp(option, "version") == 0) ||
                   (::qstrcmp(option, "v") == 0))
         {
            printQ( QString("Qt: %1\n").arg(qVersion()));
            printQ( QString("KDE: %1\n").arg(KDE_VERSION_STRING));
            printQ( QString("%1: %2\n").
      arg(about->programName()).arg(about->version()));
            exit(0);
         } else if ( (::qstrcmp(option, "license") == 0) )
         {
            enable_i18n();
            printQ( about->license() );
            printQ( "\n" );
            exit(0);
         } else if ( ::qstrcmp( option, "author") == 0 ) {
             enable_i18n();
       if ( about ) {
         const QList<KAboutPerson> authors = about->authors();
         if ( !authors.isEmpty() ) {
           QString authorlist;
           for (QList<KAboutPerson>::ConstIterator it = authors.begin(); it != authors.end(); ++it ) {
             QString email;
             if ( !(*it).emailAddress().isEmpty() )
               email = " <" + (*it).emailAddress() + '>';
             authorlist += QString("    ") + (*it).name() + email + '\n';
           }
           printQ( i18nc("the 2nd argument is a list of name+address, one on each line","%1 was written by\n%2",   QString(about->programName()) ,  authorlist ) );
         }
       } else {
         printQ( i18n("This application was written by somebody who wants to remain anonymous.") );
       }
       if (about)
       {
         if (!about->customAuthorTextEnabled ())
         {
           if (about->bugAddress().isEmpty() || about->bugAddress() == "submit@bugs.kde.org" )
             printQ( i18n( "Please use http://bugs.kde.org to report bugs.\n" ) );
           else {
             if( about->authors().count() == 1 && about->authors().first().emailAddress() == about->bugAddress() )
               printQ( i18n( "Please report bugs to %1.\n" ,  about->authors().first().emailAddress() ) );
             else
               printQ( i18n( "Please report bugs to %1.\n" , about->bugAddress()) );
           }
         }
         else
         {
           printQ(about->customAuthorPlainText());
         }
       }
       exit(0);
         } else {
           if ((option[0] == 'n') && (option[1] == 'o'))
           {
              option += 2;
              enabled = false;
           }
           findOption(orig, option, i, enabled, inOptions);
         }
      }
      else
      {
         // Check whether appOptions allows these arguments
         if (!allowArgs)
         {
            if (ignoreUnknown)
               continue;
            enable_i18n();
            usage( i18n("Unexpected argument '%1'.", QString::fromLocal8Bit(argv[i])));
         }
         else
         {
            appOptions->addArgument(argv[i]);
            if (everythingAfterArgIsArgs)
                inOptions = false;
         }
      }
   }
   parsed = true;
}

int * KCmdLineArgs::qt_argc()
{
    return &qtArgc();
}

int & KCmdLineArgs::qtArgc()
{
   if (!argsList)
      addStdCmdLineOptions(CmdLineArgKDE|CmdLineArgQt); // Lazy bastards!

   static int qt_argc = -1;
   if( qt_argc != -1 )
      return qt_argc;

   if (!(mStdargs & KCmdLineArgs::CmdLineArgQt))
   {
     qt_argc = 2;
     return qt_argc;
   }

   KCmdLineArgs *args = parsedArgs("qt");
   Q_ASSERT(args); // No qt options have been added!
   if (!argv)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application has not called KCmdLineArgs::init(...).\n\n");

      assert( 0 );
      exit(255);
   }

   Q_ASSERT(argc >= (args->count()+1));
   qt_argc = args->count() +1;
   return qt_argc;
}

static char** s_qt_argv;

char ***
KCmdLineArgs::qt_argv()
{
    qtArgv();
    return &s_qt_argv;
}

char **
KCmdLineArgs::qtArgv()
{
   if (!argsList)
      addStdCmdLineOptions(CmdLineArgKDE|CmdLineArgQt); // Lazy bastards!

   if( s_qt_argv != NULL )
      return s_qt_argv;

   if (!(mStdargs & KCmdLineArgs::CmdLineArgQt))
   {
     s_qt_argv = new char*[2];
     s_qt_argv[0] = qstrdup(appName());
     s_qt_argv[1] = 0;

     return s_qt_argv;
   }

   KCmdLineArgs *args = parsedArgs("qt");
   Q_ASSERT(args); // No qt options have been added!
   if (!argv)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application has not called KCmdLineArgs::init(...).\n\n");

      assert( 0 );
      exit(255);
   }

   s_qt_argv = new char*[ args->count() + 2 ];
   s_qt_argv[ 0 ] = qstrdup( appName());
   int i = 0;
   for(; i < args->count(); i++)
   {
      s_qt_argv[i+1] = qstrdup((char *) args->arg(i));
   }
   s_qt_argv[i+1] = 0;

   return s_qt_argv;
}

void
KCmdLineArgs::enable_i18n()
{
    // called twice or too late
    if (KGlobal::hasLocale())
      return;

    if (!KGlobal::hasMainComponent()) {
        KComponentData mainComponentData(about);
        mainComponentData.config();
        // mainComponentData is now the main component and won't disappear until KGlobal deletes it
    }
}

void
KCmdLineArgs::usage(const QString &error)
{
    Q_ASSERT(KGlobal::hasLocale());
    QByteArray localError = error.toLocal8Bit();
    if (localError[error.length()-1] == '\n')
  localError = localError.left(error.length()-1);
    fprintf(stderr, "%s: %s\n", argv[0], localError.data());

    QString tmp = i18n("Use --help to get a list of available command line options.");
    localError = tmp.toLocal8Bit();
    fprintf(stderr, "%s: %s\n", argv[0], localError.data());
    exit(254);
}

void
KCmdLineArgs::usage(const char *id)
{
   enable_i18n();
   Q_ASSERT(argsList != 0); // It's an error to call usage(...) without
                          // having done addCmdLineOptions first!

   QString optionFormatString   = "  %1 %2\n";
   QString optionFormatStringDef  = "  %1 %2 [%3]\n";
   QString tmp;
   QString usage;

   KCmdLineArgsList::Iterator args = --(argsList->end());

   if (!((*args)->d->id) && ((*args)->d->options) &&
       ((*args)->d->options->name) && ((*args)->d->options->name[0] != '+'))
   {
      usage = i18n("[options] ")+usage;
   }

   while(true)
   {
      if ((*args)->d->name)
      {
         usage = i18n("[%1-options]", (*args)->d->name)+' '+usage;
      }
      if (args == argsList->begin())
         break;
      --args;
   }

   KCmdLineArgs *appOptions = argsList->last();
   if (!appOptions->d->id)
   {
     const KCmdLineOptions *option = appOptions->d->options;
     while(option && option->name)
     {
       if (option->name[0] == '+')
          usage = usage + (option->name+1) + ' ';
       else if ( option->name[0] == '!' && option->name[1] == '+' )
          usage = usage + (option->name+2) + ' ';

       option++;
     }
   }

   printQ(i18n("Usage: %1 %2\n", argv[0], usage));
   printQ('\n'+about->shortDescription()+'\n');

   printQ(i18n("\nGeneric options:\n"));
   printQ(optionFormatString.arg("--help", -25).arg(i18n("Show help about options")));

   args = argsList->begin();
   while(args != argsList->end())
   {
      if ((*args)->d->name && (*args)->d->id)
      {
         QString option = QString("--help-%1").arg((*args)->d->id);
         QString desc = i18n("Show %1 specific options", (*args)->d->name);

         printQ(optionFormatString.arg(option, -25).arg(desc));
      }
      ++args;
   }

   printQ(optionFormatString.arg("--help-all",-25).arg(i18n("Show all options")));
   printQ(optionFormatString.arg("--author",-25).arg(i18n("Show author information")));
   printQ(optionFormatString.arg("-v, --version",-25).arg(i18n("Show version information")));
   printQ(optionFormatString.arg("--license",-25).arg(i18n("Show license information")));
   printQ(optionFormatString.arg("--", -25).arg(i18n("End of options")));

   args = argsList->begin(); // Sets current to 1st.

   bool showAll = id && (::qstrcmp(id, "all") == 0);

   if (!showAll)
   {
     while(args != argsList->end())
     {
       if (!id && !(*args)->d->id) break;
       if (id && (::qstrcmp((*args)->d->id, id) == 0)) break;
       ++args;
     }
   }

   while(args != argsList->end())
   {
     bool hasArgs = false;
     bool hasOptions = false;
     QString optionsHeader;
     if ((*args)->d->name)
        optionsHeader = i18n("\n%1 options:\n", QLatin1String((*args)->d->name));
     else
        optionsHeader = i18n("\nOptions:\n");

     while (args != argsList->end())
     {
       const KCmdLineOptions *option = (*args)->d->options;
       QByteArray opt = "";

       while(option && option->name)
       {
         QString description;
         QString descriptionRest;
         QStringList dl;

         // Option header
         if (option->name[0] == ':')
         {
            if (option->description)
            {
               optionsHeader = '\n'+i18n(option->description);
               if (!optionsHeader.endsWith("\n"))
                  optionsHeader.append("\n");
               hasOptions = false;
            }
            option++;
            continue;
         }

         // Free-form comment
         if (option->name[0] == 0)
         {
            if (option->description)
            {
               tmp = '\n'+i18n(option->description);
               if (!tmp.endsWith("\n"))
                  tmp.append("\n");
               printQ(tmp);
            }
            option++;
            continue;
         }

         // Options
         if (option->description)
         {
            description = i18n(option->description);
            dl = description.split( "\n", QString::KeepEmptyParts);
            description = dl.first();
            dl.erase( dl.begin() );
         }
         QByteArray name = option->name;
         if (name[0] == '!')
             name = name.mid(1);

         if (name[0] == '+')
         {
            if (!hasArgs)
            {
               printQ(i18n("\nArguments:\n"));
               hasArgs = true;
            }

            name = name.mid(1);
            if ((name[0] == '[') && (name[name.length()-1] == ']'))
                name = name.mid(1, name.length()-2);
            printQ(optionFormatString.arg(QString( name ), -25)
                                     .arg(description));
         }
         else
         {
            if (!hasOptions)
            {
               printQ(optionsHeader);
               hasOptions = true;
            }

            if ((name.length() == 1) || (name[1] == ' '))
               name = '-'+name;
            else
               name = "--"+name;
            if (!option->description)
            {
               opt = name + ", ";
            }
            else
            {
               opt = opt + name;
               if (!option->def)
               {
                  printQ(optionFormatString.arg(QString( opt ), -25)
                         .arg(description));
               }
               else
               {
                  printQ(optionFormatStringDef.arg(QString( opt ), -25)
                         .arg(description).arg(option->def));
               }
               opt = "";
            }
         }
         for(QStringList::Iterator it = dl.begin();
             it != dl.end();
             ++it)
         {
            printQ(optionFormatString.arg("", -25).arg(*it));
         }

         option++;
       }
       ++args;
       if (args == argsList->end() || (*args)->d->name || (*args)->d->id == 0)
        break;
     }
     if (!showAll) break;
   }

   exit(0);
}

//
// Member functions
//

/**
 *  Constructor.
 *
 *  The given arguments are assumed to be constants.
 */
KCmdLineArgs::KCmdLineArgs( const KCmdLineOptions *_options,
                            const char *_name, const char *_id)
  : d(new Private(_options, _name, _id))
{
}

/**
 *  Destructor.
 */
KCmdLineArgs::~KCmdLineArgs()
{
  if (argsList)
     argsList->removeAll(this);
  delete d;
}

void
KCmdLineArgs::setCwd( const char * cwd )
{
    mCwd = QFile::decodeName( cwd );
}

void
KCmdLineArgs::setCwd( const QString & cwd )
{
    mCwd = cwd;
}

void
KCmdLineArgs::clear()
{
   delete d->parsedArgList;
   d->parsedArgList = 0;
   delete d->parsedOptionList;
   d->parsedOptionList = 0;
}

void
KCmdLineArgs::reset()
{
   if ( argsList ) {
      delete argsList;
      argsList = 0;
   }
   parsed = false;
}

void
KCmdLineArgs::save( QDataStream &ds) const
{
   if (d->parsedOptionList)
      ds << (*(d->parsedOptionList));
   else
      ds << quint32(0);

   if (d->parsedArgList)
      ds << (*(d->parsedArgList));
   else
      ds << quint32(0);
}

void
KCmdLineArgs::load( QDataStream &ds)
{
   if (!d->parsedOptionList) d->parsedOptionList = new KCmdLineParsedOptions;
   if (!d->parsedArgList) d->parsedArgList = new KCmdLineParsedArgs;

   ds >> (*(d->parsedOptionList));
   ds >> (*(d->parsedArgList));

   if (d->parsedOptionList->count() == 0)
   {
      delete d->parsedOptionList;
      d->parsedOptionList = 0;
   }
   if (d->parsedArgList->count() == 0)
   {
      delete d->parsedArgList;
      d->parsedArgList = 0;
   }
}

void
KCmdLineArgs::setOption(const QByteArray &opt, bool enabled)
{
   if (d->isQt)
   {
      // Qt does it own parsing.
      QByteArray argString = "-";
      if( !enabled )
          argString += "no";
      argString += opt;
      addArgument(argString);
   }
   if (!d->parsedOptionList) {
      d->parsedOptionList = new KCmdLineParsedOptions;
   }

   if (enabled)
      d->parsedOptionList->insert( opt, QByteArray("t") );
   else
      d->parsedOptionList->insert( opt, QByteArray("f") );
}

void
KCmdLineArgs::setOption(const QByteArray &opt, const char *value)
{
   if (d->isQt)
   {
      // Qt does it's own parsing.
      QByteArray argString = "-";
      argString += opt;
      addArgument(argString);
      addArgument(value);

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
      // Hack coming up!
      if (argString == "-display")
      {
         setenv(DISPLAY, value, true);
      }
#endif
   }
   if (!d->parsedOptionList) {
      d->parsedOptionList = new KCmdLineParsedOptions;
   }

   d->parsedOptionList->insertMulti( opt, value );
}

QByteArray
KCmdLineArgs::getOption(const char *_opt) const
{
   QByteArray value;
   if (d->parsedOptionList)
   {
      value = d->parsedOptionList->value(_opt);
   }

   if (!value.isEmpty())
      return value;

   // Look up the default.
   const char *opt_name;
   const char *def;
   bool dummy = true;
   QByteArray opt = _opt;
   int result = ::findOption( d->options, opt, opt_name, def, dummy) & ~4;

   if (result != 3)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for getOption(\"%s\") but the \"%s\" option\n",
                      _opt, _opt);
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");

      Q_ASSERT( 0 );
      exit(255);
   }
   return QByteArray(def);
}

QByteArrayList
KCmdLineArgs::getOptionList(const char *_opt) const
{
   QByteArrayList result;
   if (!d->parsedOptionList)
      return result;

   while(true)
   {
      QByteArray value = d->parsedOptionList->take(_opt);
      if (value.isEmpty())
         break;
      result.prepend(value);
   }

   // Reinsert items in dictionary
   // WABA: This is rather silly, but I don't want to add restrictions
   // to the API like "you can only call this function once".
   // I can't access all items without taking them out of the list.
   // So taking them out and then putting them back is the only way.
   for(QByteArrayList::ConstIterator it=result.begin();
       it != result.end();
       ++it)
   {
      d->parsedOptionList->insertMulti(_opt, QByteArray(*it));
   }
   return result;
}

bool
KCmdLineArgs::isSet(const char *_opt) const
{
   // Look up the default.
   const char *opt_name;
   const char *def;
   QByteArray opt = _opt;
   int result = 0;
   KCmdLineArgsList::Iterator args = argsList->begin();
   while (args != argsList->end())
   {
      bool dummy = true;
      result = ::findOption((*args)->d->options, opt, opt_name, def, dummy) & ~4;
      if (result) break;
      ++args;
   }

   if (result == 0)
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs):\n");
      fprintf(stderr, "Application requests for isSet(\"%s\") but the \"%s\" option\n",
                      _opt, _opt);
      fprintf(stderr, "has never been specified via addCmdLineOptions( ... )\n\n");

      Q_ASSERT( 0 );
      exit(255);
   }

   QByteArray value;
   if (d->parsedOptionList)
   {
      value = d->parsedOptionList->value(opt);
   }

   if (!value.isEmpty())
   {
      if (result == 3)
         return true;
      else
         return (value[0] == 't');
   }

   if (result == 3)
      return false; // String option has 'false' as default.

   // We return 'true' as default if the option was listed as '-nofork'
   // We return 'false' as default if the option was listed as '-fork'
   return (result == 2);
}

int
KCmdLineArgs::count() const
{
   if (!d->parsedArgList)
      return 0;
   return d->parsedArgList->count();
}

const char *
KCmdLineArgs::arg(int n) const
{
   if (!d->parsedArgList || (n >= (int) d->parsedArgList->count()))
   {
      fprintf(stderr, "\n\nFAILURE (KCmdLineArgs): Argument out of bounds\n");
      fprintf(stderr, "Application requests for arg(%d) without checking count() first.\n",
                      n);

      Q_ASSERT( 0 );
      exit(255);
   }

   return d->parsedArgList->at(n);
}

KUrl
KCmdLineArgs::url(int n) const
{
   return makeURL( arg(n) );
}

KUrl KCmdLineArgs::makeURL(const char *_urlArg)
{
   QString urlArg = QFile::decodeName(_urlArg);
   if (!QDir::isRelativePath(urlArg))
   {
      KUrl result;
      result.setPath(urlArg);
      return result; // Absolute path.
   }

   if ( !KUrl::isRelativeUrl(urlArg) )
     return KUrl(urlArg); // Argument is a URL

   KUrl result;
   result.setPath( cwd()+'/'+urlArg );
   result.cleanPath();
   return result;  // Relative path
}

void
KCmdLineArgs::addArgument(const char *argument)
{
   if (!d->parsedArgList)
      d->parsedArgList = new KCmdLineParsedArgs;

   d->parsedArgList->append(argument);
}

static const KCmdLineOptions kde_tempfile_option[] =
{
   { "tempfile",       I18N_NOOP("The files/URLs opened by the application will be deleted after use"), 0},
   KCmdLineLastOption
};

void
KCmdLineArgs::addTempFileOption()
{
    KCmdLineArgs::addCmdLineOptions( kde_tempfile_option, "KDE-tempfile", "kde-tempfile" );
}

bool KCmdLineArgs::isTempFileSet()
{
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs( "kde-tempfile" );
    if ( args )
        return args->isSet( "tempfile" );
    return false;
}
