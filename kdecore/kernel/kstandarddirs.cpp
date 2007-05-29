/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999,2007 Stephan Kulow <coolo@kde.org>
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

/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 * Generated: Thu Mar  5 16:05:28 EST 1998
 */

#include "kstandarddirs.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kdebug.h"
#include "kcomponentdata.h"
#include "kshell.h"
#include "kuser.h"
#include "kde_file.h"
#include "kkernel_win.h"

#include <config.h>
#include <config-prefix.h> // for KDESYSCONFDIR, __KDE_BINDIR and KDEDIR

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QCharRef>
#include <QtCore/QMutableStringListIterator>

class KStandardDirs::KStandardDirsPrivate
{
public:
    KStandardDirsPrivate(const KComponentData &cd)
        : restrictionsActive(false),
          dataRestrictionActive(false),
          checkRestrictions(true),
          componentData(cd),
          addedCustoms(false)
    { }

    bool restrictionsActive;
    bool dataRestrictionActive;
    bool checkRestrictions;
    QMap<QByteArray, bool> restrictions;
    QStringList xdgdata_prefixes;
    QStringList xdgconf_prefixes;

    QStringList prefixes;
    KComponentData componentData;

    // Directory dictionaries
    QMap<QByteArray, QStringList> absolutes;
    QMap<QByteArray, QStringList> relatives;

    mutable QMap<QByteArray, QStringList> dircache;
    mutable QMap<QByteArray, QString> savelocations;

    bool addedCustoms;
};

// Singleton, with data shared by all kstandarddirs instances.
// Used in static methods like findExe()
class KStandardDirsSingleton
{
public:
    QString defaultprefix;
    QString defaultbindir;
    QString defaultlibexecdir;
    static KStandardDirsSingleton* self();
};

KStandardDirsSingleton* KStandardDirsSingleton::self()
{
    K_GLOBAL_STATIC(KStandardDirsSingleton, s_self)
    return s_self;
}

/**
 * @internal
 * Returns the default toplevel directory where KDE is installed.
 */
static QString kfsstnd_defaultprefix()
{
   KStandardDirsSingleton* s = KStandardDirsSingleton::self();
   if (!s->defaultprefix.isEmpty())
      return s->defaultprefix;
#ifdef Q_WS_WIN
   s->defaultprefix = getKde4Prefix();
#else //UNIX
   s->defaultprefix = KDEDIR;
#endif

   if (s->defaultprefix.isEmpty())
      kWarning() << "KStandardDirs::kfsstnd_defaultprefix(): default KDE prefix not found!" << endl;
   return s->defaultprefix;
}

/**
 * @internal
 * Returns the default bin directory in which KDE executables are stored.
 */
static QString kfsstnd_defaultbindir()
{
   KStandardDirsSingleton* s = KStandardDirsSingleton::self();
   if (!s->defaultbindir.isEmpty())
      return s->defaultbindir;
#ifdef Q_WS_WIN
   s->defaultbindir = kfsstnd_defaultprefix() + QLatin1String("/bin");
#else //UNIX
   s->defaultbindir = __KDE_BINDIR;
   if (s->defaultbindir.isEmpty())
      s->defaultbindir = kfsstnd_defaultprefix() + QLatin1String("/bin");
#endif
   if (s->defaultbindir.isEmpty())
      kWarning() << "KStandardDirs::kfsstnd_defaultbindir(): default binary KDE dir not found!" << endl;
  return s->defaultbindir;
}

/**
 * @internal
 * Returns the default libexec directory in which KDE tool executables are stored.
 */
static QString kfsstnd_defaultlibexecdir()
{
    KStandardDirsSingleton* s = KStandardDirsSingleton::self();
    if (!s->defaultlibexecdir.isEmpty())
        return s->defaultlibexecdir;
    s->defaultlibexecdir = kfsstnd_defaultprefix() + QLatin1String("/lib" KDELIBSUFF "/kde4/libexec");
    //if (s->defaultlibexecdir.isEmpty())
    //   kWarning() << "KStandardDirs::kfsstnd_defaultlibexecdir(): default libexec KDE dir not found!" << endl;
    return s->defaultlibexecdir;
}

/* If you add a new resource type here, make sure to
 * 1) regenerate using "generate_string_table.pl types" and the data below.
 * 2) update the KStandardDirs class documentation
 * 3) update the kde_default code
 * 4) update the kde_default documentation
 * 5) update the list in kde-config.cpp.in

html
icon
apps
sound
data
locale
services
mime
servicetypes
config
exe
wallpaper
lib
pixmap
templates
module
qtplugins
kcfg
emoticons
xdgdata-apps
xdgdata-dirs
xdgconf-menu
xdgdata-icon
xdgdata-pixmap
xdgdata-mime
 */
static const char types[] =
    "html\0"
    "icon\0"
    "apps\0"
    "sound\0"
    "data\0"
    "locale\0"
    "services\0"
    "mime\0"
    "servicetypes\0"
    "config\0"
    "exe\0"
    "wallpaper\0"
    "lib\0"
    "pixmap\0"
    "templates\0"
    "module\0"
    "qtplugins\0"
    "kcfg\0"
    "emoticons\0"
    "xdgdata-apps\0"
    "xdgdata-dirs\0"
    "xdgconf-menu\0"
    "xdgdata-icon\0"
    "xdgdata-pixmap\0"
    "xdgdata-mime\0"
    "\0";

static const int types_indices[] = {
       0,    5,   10,   15,   21,   26,   33,   42,
      47,   60,   67,   71,   81,   85,   92,  102,
     109,  119,  124,  134,  147,  160,  173,  186,
     201,   -1
};

static int tokenize( QStringList& token, const QString& str,
                     const QString& delim );

KStandardDirs::KStandardDirs(const KComponentData &componentData)
    : d(new KStandardDirsPrivate(componentData))
{
    addKDEDefaults();
}

KStandardDirs::~KStandardDirs()
{
    delete d;
}

bool KStandardDirs::isRestrictedResource(const char *type, const QString& relPath) const
{
   if (!d->restrictionsActive)
      return false;

   if (d->restrictions.value(type, false))
      return true;

   if (strcmp(type, "data")==0)
   {
      applyDataRestrictions(relPath);
      if (d->dataRestrictionActive)
      {
         d->dataRestrictionActive = false;
         return true;
      }
   }
   return false;
}

void KStandardDirs::applyDataRestrictions(const QString &relPath) const
{
   QString key;
   int i = relPath.indexOf('/');
   if (i != -1)
      key = "data_"+relPath.left(i);
   else
      key = "data_"+relPath;

   if (d->restrictions.value(key.toLatin1(), false))
      d->dataRestrictionActive = true;
}


QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    for (int i = 0; types_indices[i] != -1; ++i)
        list.append(QLatin1String(types + types_indices[i]));
    return list;
}

static void priorityAdd(QStringList &prefixes, const QString& dir, bool priority)
{
    if (priority && !prefixes.isEmpty())
    {
        // Add in front but behind $KDEHOME
        QStringList::iterator it = prefixes.begin();
        it++;
        prefixes.insert(it, dir);
    }
    else
    {
        prefixes.append(dir);
    }
}

void KStandardDirs::addPrefix( const QString& _dir )
{
    addPrefix(_dir, false);
}

void KStandardDirs::addPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
        dir += '/';

    if (!d->prefixes.contains(dir)) {
        priorityAdd(d->prefixes, dir, priority);
        d->dircache.clear();
    }
}

void KStandardDirs::addXdgConfigPrefix( const QString& _dir )
{
    addXdgConfigPrefix(_dir, false);
}

void KStandardDirs::addXdgConfigPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
        dir += '/';

    if (!d->xdgconf_prefixes.contains(dir)) {
        priorityAdd(d->xdgconf_prefixes, dir, priority);
        d->dircache.clear();
    }
}

void KStandardDirs::addXdgDataPrefix( const QString& _dir )
{
    addXdgDataPrefix(_dir, false);
}

void KStandardDirs::addXdgDataPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
        dir += '/';

    if (!d->xdgdata_prefixes.contains(dir)) {
        priorityAdd(d->xdgdata_prefixes, dir, priority);
        d->dircache.clear();
    }
}

QString KStandardDirs::kfsstnd_prefixes()
{
   return d->prefixes.join(QString(QChar(KPATH_SEPARATOR)));
}

QString KStandardDirs::kfsstnd_xdg_conf_prefixes()
{
   return d->xdgconf_prefixes.join(QString(QChar(KPATH_SEPARATOR)));
}

QString KStandardDirs::kfsstnd_xdg_data_prefixes()
{
   return d->xdgdata_prefixes.join(QString(QChar(KPATH_SEPARATOR)));
}

bool KStandardDirs::addResourceType( const char *type,
                                     const QString& relativename )
{
    return addResourceType(type, relativename, true);
}

bool KStandardDirs::addResourceType( const char *type,
                                     const QString& relativename,
                                     bool priority )
{
    if (relativename.isEmpty())
       return false;

    QStringList& rels = d->relatives[type]; // find or insert
    QString copy = relativename;
    if (copy.at(copy.length() - 1) != '/')
        copy += '/';
    if (!rels.contains(copy)) {
        if (priority)
            rels.prepend(copy);
        else
            rels.append(copy);
        d->dircache.remove(type); // clean the cache
        return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const char *type,
                                    const QString& absdir)
{
    return addResourceDir(type, absdir, true);
}

bool KStandardDirs::addResourceDir( const char *type,
                                    const QString& absdir,
                                    bool priority)
{
    // find or insert entry in the map
    QStringList &paths = d->absolutes[type];
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != '/')
      copy += '/';

    if (!paths.contains(copy)) {
        if (priority)
            paths.prepend(copy);
        else
            paths.append(copy);
        d->dircache.remove(type); // clean the cache
        return true;
    }
    return false;
}

QString KStandardDirs::findResource( const char *type,
                                     const QString& filename ) const
{
    if (!QDir::isRelativePath(filename))
        return filename; // absolute dirs are absolute dirs, right? :-/

#if 0
kDebug(180) << "Find resource: " << type << endl;
for (QStringList::ConstIterator pit = prefixes.begin();
     pit != prefixes.end();
     pit++)
{
  kDebug(180) << "Prefix: " << *pit << endl;
}
#endif

    const QString dir = findResourceDir(type, filename);
    if (dir.isEmpty())
        return dir;
    else return dir + filename;
}

static quint32 updateHash(const QString &file, quint32 hash)
{
    QByteArray cFile = QFile::encodeName(file);
    KDE_struct_stat buff;
    if ((access(cFile, R_OK) == 0) && (KDE_stat(cFile, &buff) == 0) && (S_ISREG(buff.st_mode))) {
        hash = hash + static_cast<quint32>(buff.st_ctime);
    }
    return hash;
}

quint32 KStandardDirs::calcResourceHash( const char *type,
                                         const QString& filename,
                                         SearchOptions options ) const
{
    quint32 hash = 0;

    if (!QDir::isRelativePath(filename))
    {
        // absolute dirs are absolute dirs, right? :-/
        return updateHash(filename, hash);
    }
    if (d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    QStringList candidates = resourceDirs(type);
    QString fullPath;

    foreach ( const QString& candidate, candidates )
    {
        hash = updateHash(candidate + filename, hash);
        if (  !( options & Recursive ) && hash ) {
           return hash;
        }
    }
    return hash;
}


QStringList KStandardDirs::findDirs( const char *type,
                                     const QString& reldir ) const
{
    QDir testdir;
    QStringList list;
    if (!QDir::isRelativePath(reldir))
    {
        testdir.setPath(reldir);
        if (testdir.exists())
        {
            if (reldir.endsWith("/"))
               list.append(reldir);
            else
               list.append(reldir+'/');
        }
        return list;
    }

    checkConfig();

    if (d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(reldir);
    QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absolutePath() + '/');
    }

    return list;
}

QString KStandardDirs::findResourceDir( const char *type,
                                        const QString& filename) const
{
#ifndef NDEBUG
    if (filename.isEmpty()) {
      kWarning() << "filename for type " << type << " in KStandardDirs::findResourceDir is not supposed to be empty!!" << endl;
      return QString();
    }
#endif

    if (d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    QStringList candidates = resourceDirs(type);
    QString fullPath;

    for (QStringList::ConstIterator it = candidates.begin();
      it != candidates.end(); ++it) {
      if (exists(*it + filename)) {
          return *it;
      }
    }

#ifndef NDEBUG
    if(false && strcmp(type, "locale"))
      kDebug(180) << "KStdDirs::findResDir(): can't find \"" << filename << "\" in type \"" << type << "\"." << endl;
#endif

    return QString();
}

bool KStandardDirs::exists(const QString &fullPath)
{
    KDE_struct_stat buff;
    if (access(QFile::encodeName(fullPath), R_OK) == 0 && KDE_stat( QFile::encodeName(fullPath), &buff ) == 0)
        if (fullPath.at(fullPath.length() - 1) != '/') {
            if (S_ISREG( buff.st_mode ))
                return true;
        } else
            if (S_ISDIR( buff.st_mode ))
                return true;
    return false;
}

static void lookupDirectory(const QString& path, const QString &relPart,
                            const QRegExp &regexp,
                            QStringList& list,
                            QStringList& relList,
                            bool recursive, bool unique)
{
  QString pattern = regexp.pattern();
  if (recursive || pattern.contains('?') || pattern.contains('*'))
  {
    if (path.isEmpty()) //for sanity
      return;
    // We look for a set of files.
    DIR *dp = opendir( QFile::encodeName(path));
    if (!dp)
      return;

#ifdef Q_WS_WIN
    assert(path.at(path.length() - 1) == '/' || path.at(path.length() - 1) == '\\');
#else
    assert(path.at(path.length() - 1) == '/');
#endif

    struct dirent *ep;

    while( ( ep = readdir( dp ) ) != 0L )
    {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == "." || fn == ".." || fn.at(fn.length() - 1).toLatin1() == '~')
        continue;

      if (!recursive && !regexp.exactMatch(fn))
        continue; // No match

      bool isDir;
      bool isReg;

      QString pathfn = path + fn;
#ifdef HAVE_DIRENT_D_TYPE
      isDir = ep->d_type == DT_DIR;
      isReg = ep->d_type == DT_REG;

      if (ep->d_type == DT_UNKNOWN)
#endif
      {
        KDE_struct_stat buff;
        if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
          kDebug(180) << "Error stat'ing " << pathfn << " : " << perror << endl;
          continue; // Couldn't stat (e.g. no read permissions)
        }
        isReg = S_ISREG (buff.st_mode);
        isDir = S_ISDIR (buff.st_mode);
      }

      if ( recursive ) {
        if ( isDir ) {
          lookupDirectory(pathfn + '/', relPart + fn + '/', regexp, list, relList, recursive, unique);
        }
        if (!regexp.exactMatch(fn))
          continue; // No match
      }
      if ( isReg )
      {
        if (!unique || !relList.contains(relPart + fn))
        {
            list.append( pathfn );
            relList.append( relPart + fn );
        }
      }
    }
    closedir( dp );
  }
  else
  {
     // We look for a single file.
     QString fn = pattern;
     QString pathfn = path + fn;
     KDE_struct_stat buff;
     if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 )
        return; // File not found
     if ( S_ISREG( buff.st_mode))
     {
       if (!unique || !relList.contains(relPart + fn))
       {
         list.append( pathfn );
         relList.append( relPart + fn );
       }
     }
  }
}

static void lookupPrefix(const QString& prefix, const QString& relpath,
                         const QString& relPart,
                         const QRegExp &regexp,
                         QStringList& list,
                         QStringList& relList,
                         bool recursive, bool unique)
{
    if (relpath.isEmpty()) {
       lookupDirectory(prefix, relPart, regexp, list,
                       relList, recursive, unique);
       return;
    }
    QString path;
    QString rest;

    if (relpath.length())
    {
       int slash = relpath.indexOf('/');
       if (slash < 0)
           rest = relpath.left(relpath.length() - 1);
       else {
           path = relpath.left(slash);
           rest = relpath.mid(slash + 1);
       }
    }

    if (prefix.isEmpty()) //for sanity
      return;
#ifdef Q_WS_WIN
    assert(prefix.at(prefix.length() - 1) == '/' || prefix.at(prefix.length() - 1) == '\\');
#else
    assert(prefix.at(prefix.length() - 1) == '/');
#endif
    if (path.contains('*') || path.contains('?')) {

        QRegExp pathExp(path, Qt::CaseSensitive, QRegExp::Wildcard);
        DIR *dp = opendir( QFile::encodeName(prefix) );
        if (!dp) {
            return;
        }

        struct dirent *ep;

        while( ( ep = readdir( dp ) ) != 0L )
        {
          QString fn( QFile::decodeName(ep->d_name));
          if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
            continue;

          if ( !pathExp.exactMatch(fn) )
            continue; // No match
          QString rfn = relPart+fn;
          fn = prefix + fn;

          bool isDir;

#ifdef HAVE_DIRENT_D_TYPE
          isDir = ep->d_type == DT_DIR;

          if (ep->d_type == DT_UNKNOWN)
#endif
          {
              QString pathfn = path + fn;
              KDE_struct_stat buff;
              if ( KDE_stat( QFile::encodeName(fn), &buff ) != 0 ) {
                  kDebug(180) << "Error stat'ing " << fn << " : " << perror << endl;
                  continue; // Couldn't stat (e.g. no read permissions)
              }
              isDir = S_ISDIR (buff.st_mode);
          }
          if ( isDir )
              lookupPrefix(fn + '/', rest, rfn + '/', regexp, list, relList, recursive, unique);
        }

        closedir( dp );
    } else {
        // Don't stat, if the dir doesn't exist we will find out
        // when we try to open it.
        lookupPrefix(prefix + path + '/', rest,
                     relPart + path + '/', regexp, list,
                     relList, recursive, unique);
    }
}

QStringList
KStandardDirs::findAllResources( const char *type,
                                 const QString& filter,
                                 SearchOptions options,
                                 QStringList &relList) const
{
    QStringList list;
    QString filterPath;
    QString filterFile;

    if ( filter.length() )
    {
       int slash = filter.lastIndexOf('/');
       if (slash < 0) {
           filterFile = filter;
       } else {
           filterPath = filter.left(slash + 1);
           filterFile = filter.mid(slash + 1);
       }
    }

    checkConfig();

    QStringList candidates;
    if ( !QDir::isRelativePath(filter) ) // absolute path
    {
#ifdef Q_OS_WIN
        candidates << filterPath.left(3); //e.g. "C:\"
        filterPath = filterPath.mid(3);
#else
        candidates << "/";
        filterPath = filterPath.mid(1);
#endif
    }
    else
    {
        if (d->restrictionsActive && (strcmp(type, "data")==0)) {
            applyDataRestrictions(filter);
        }
        candidates = resourceDirs(type);
    }

    if (filterFile.isEmpty()) {
        filterFile = "*";
    }

    QRegExp regExp(filterFile, Qt::CaseSensitive, QRegExp::Wildcard);

    foreach ( const QString& candidate, candidates )
    {
        lookupPrefix(candidate, filterPath, "", regExp, list,
                     relList, options & Recursive, options & NoDuplicates);
    }

    return list;
}

QStringList
KStandardDirs::findAllResources( const char *type,
                                 const QString& filter,
                                 SearchOptions options ) const
{
    QStringList relList;
    return findAllResources(type, filter, options, relList);
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString
KStandardDirs::realPath(const QString &dirname)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(dirname).data(), realpath_buffer) != 0) {
        // success, use result from realpath
        int len = strlen(realpath_buffer);
        realpath_buffer[len] = '/';
        realpath_buffer[len+1] = 0;
        return QFile::decodeName(realpath_buffer);
    }

    return dirname;
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString
KStandardDirs::realFilePath(const QString &filename)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(filename).data(), realpath_buffer) != 0) {
        // success, use result from realpath
        return QFile::decodeName(realpath_buffer);
    }

    return filename;
}


void KStandardDirs::createSpecialResource(const char *type)
{
   char hostname[256];
   hostname[0] = 0;
   gethostname(hostname, 255);
   QString dir = QString("%1%2-%3").arg(localkdedir()).arg(type).arg(hostname);
   char link[1024];
   link[1023] = 0;
   int result = readlink(QFile::encodeName(dir).data(), link, 1023);
   bool relink = (result == -1) && (errno == ENOENT);
   if (result > 0)
   {
      link[result] = 0;
      if (!QDir::isRelativePath(link))
      {
         KDE_struct_stat stat_buf;
         int res = KDE_lstat(link, &stat_buf);
         if ((res == -1) && (errno == ENOENT))
         {
            relink = true;
         }
         else if ((res == -1) || (!S_ISDIR(stat_buf.st_mode)))
         {
            fprintf(stderr, "Error: \"%s\" is not a directory.\n", link);
            relink = true;
         }
         else if (stat_buf.st_uid != getuid())
         {
            fprintf(stderr, "Error: \"%s\" is owned by uid %d instead of uid %d.\n", link, stat_buf.st_uid, getuid());
            relink = true;
         }
      }
   }
#ifdef Q_WS_WIN
   if (relink)
   {
      if (!makeDir(dir, 0700))
         fprintf(stderr, "failed to create \"%s\"", qPrintable(dir));
      else
         result = readlink(QFile::encodeName(dir).data(), link, 1023);
   }
#else //UNIX
   if (relink)
   {
      QString srv = findExe(QLatin1String("lnusertemp"), kfsstnd_defaultlibexecdir());
      if (srv.isEmpty())
         srv = findExe(QLatin1String("lnusertemp"));
      if (!srv.isEmpty())
      {
         system(QFile::encodeName(srv)+' '+type);
         result = readlink(QFile::encodeName(dir).data(), link, 1023);
      }
   }
   if (result > 0)
   {
      link[result] = 0;
      if (link[0] == '/')
         dir = QFile::decodeName(link);
      else
         dir = QDir::cleanPath(dir+QFile::decodeName(link));
   }
#endif
   addResourceDir(type, dir+'/', false);
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    QMap<QByteArray, QStringList>::const_iterator dirCacheIt = d->dircache.find(type);

    QStringList candidates;

    if (dirCacheIt != d->dircache.end())
    {
        candidates = *dirCacheIt;
    }
    else // filling cache
    {
        if (strcmp(type, "socket") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "tmp") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "cache") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);

        QDir testdir;

        bool restrictionActive = false;
        if (d->restrictionsActive)
        {
           if (d->dataRestrictionActive)
              restrictionActive = true;
           else if (d->restrictions.value("all", false))
              restrictionActive = true;
           else if (d->restrictions.value(type, false))
              restrictionActive = true;
           d->dataRestrictionActive = false; // Reset
        }

        QStringList dirs;
        dirs = d->relatives.value(type);
        if (!dirs.isEmpty())
        {
            bool local = true;
            const QStringList *prefixList = 0;
            if (strncmp(type, "xdgdata-", 8) == 0)
                prefixList = &(d->xdgdata_prefixes);
            else if (strncmp(type, "xdgconf-", 8) == 0)
                prefixList = &(d->xdgconf_prefixes);
            else
                prefixList = &d->prefixes;

            QDir dir;
            for (QStringList::ConstIterator pit = prefixList->begin();
                 pit != prefixList->end();
                 ++pit)
            {
                for (QStringList::ConstIterator it = dirs.begin();
                     it != dirs.end(); ++it) {
                    QString path = realPath( *pit + *it );
                    testdir.setPath(path);
                    if (local && restrictionActive)
                       continue;
                    if ((local || testdir.exists()) && !candidates.contains(path))
                        candidates.append(path);
                }
                local = false;
            }
        }

        // make sure we find the path where it's installed
        QString installdir = installPath( type );
        if (!candidates.contains(installdir))
            candidates.append(installdir);

        dirs = d->absolutes.value(type);
        if (!dirs.isEmpty())
            for (QStringList::ConstIterator it = dirs.begin();
                 it != dirs.end(); ++it)
            {
                testdir.setPath(*it);
                if (testdir.exists()) {
                    QString filename = realPath( *it );
                    if (!candidates.contains(filename)) {
                        candidates.append(filename);
                    }
                }
            }

        d->dircache.insert(type, candidates);
    }

#if 0
    kDebug(180) << "found dirs for resource " << type << ":" << endl;
    for (QStringList::ConstIterator pit = candidates.begin();
         pit != candidates.end();
         pit++)
    {
        fprintf(stderr, "%s\n", qPrintable(*pit));
    }
#endif

  return candidates;
}

QStringList KStandardDirs::systemPaths( const QString& pstr )
{
    QStringList tokens;
    QString p = pstr;

    if( p.isNull() )
    {
        p = getenv( "PATH" );
    }

    QString delimiters(QChar(KPATH_SEPARATOR));
    delimiters += "\b";
    tokenize( tokens, p, delimiters );

    QStringList exePaths;

    // split path using : or \b as delimiters
    for( int i = 0; i < tokens.count(); i++ )
    {
        exePaths << KShell::tildeExpand( tokens[ i ] );
    }

    return exePaths;
}

#ifdef Q_WS_MAC
static QString getBundle( const QString& path, bool ignore )
{
    kDebug(180) << "getBundle(" << path << ", " << ignore << ") called" << endl;
    QFileInfo info;
    QString bundle = path;
    bundle += ".app/Contents/MacOS/" + bundle.section('/', -1);
    info.setFile( bundle );
    if ( info.exists() && ( ignore || info.isExecutable() )
         && ( info.isFile() || info.isSymLink() ) ) {
        kDebug(180) << "getBundle(): returning " << bundle << endl;
        return bundle;
    }
    return QString();
}
#endif

static QString checkExecutable( const QString& path, bool ignoreExecBit )
{
#ifdef Q_WS_MAC
    QString bundle = getBundle( path, ignoreExecBit );
    if ( !bundle.isEmpty() ) {
        //kDebug(180) << "findExe(): returning " << bundle << endl;
        return bundle;
    }
#endif
    QFileInfo info( path );
    if( info.exists() && ( ignoreExecBit || info.isExecutable() )
        && ( info.isFile() || info.isSymLink() ) ) {
        //kDebug(180) << "checkExecutable(): returning " << path << endl;
        return path;
    }
    //kDebug(180) << "checkExecutable(): failed, returning empty string" << endl;
    return QString();
}

QString KStandardDirs::findExe( const QString& appname,
                                const QString& pstr,
                                SearchOptions options )
{
    //kDebug(180) << "findExe(" << appname << ", pstr, " << ignoreExecBit << ") called" << endl;

#ifdef Q_WS_WIN
    QString real_appname = appname + ".exe";
#else
    QString real_appname = appname;
#endif
    QFileInfo info;

    // absolute path ?
    if (!QDir::isRelativePath(real_appname))
    {
        //kDebug(180) << "findExe(): absolute path given" << endl;
        return checkExecutable(real_appname, options & IgnoreExecBit);
    }

    //kDebug(180) << "findExe(): relative path given" << endl;

    // Look in the default bin and libexec dirs. Maybe we should use the "exe" resource instead?

    QString p = kfsstnd_defaultlibexecdir() + '/' + real_appname;
    QString result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        //kDebug(180) << "findExe(): returning " << result << endl;
        return result;
    }

    p = kfsstnd_defaultbindir() + '/' + real_appname;
    result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        //kDebug(180) << "findExe(): returning " << result << endl;
        return result;
    }

    //kDebug(180) << "findExe(): checking system paths" << endl;
    const QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + '/';
        p += real_appname;

        // Check for executable in this tokenized path
        result = checkExecutable(p, options & IgnoreExecBit);
        if (!result.isEmpty()) {
            //kDebug(180) << "findExe(): returning " << result << endl;
            return result;
        }
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.

    //kDebug(180) << "findExe(): failed, nothing matched" << endl;
    return QString();
}

int KStandardDirs::findAllExe( QStringList& list, const QString& appname,
                               const QString& pstr, SearchOptions options )
{
#ifdef Q_WS_WIN
    QString real_appname = appname + ".exe";
#else
    QString real_appname = appname;
#endif
    QFileInfo info;
    QString p;
    list.clear();

    QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + '/';
        p += real_appname;

        info.setFile( p );

        if( info.exists() && ( ( options & IgnoreExecBit ) || info.isExecutable())
            && info.isFile() ) {
            list.append( p );
        }
    }

    return list.count();
}

static int tokenize( QStringList& tokens, const QString& str,
                     const QString& delim )
{
    int len = str.length();
    QString token = "";

    for( int index = 0; index < len; index++)
    {
        if ( delim.contains( str[ index ] ) )
        {
            tokens.append( token );
            token = "";
        }
        else
        {
            token += str[ index ];
        }
    }
    if ( token.length() > 0 )
    {
        tokens.append( token );
    }

    return tokens.count();
}

QString KStandardDirs::kde_default(const char *type) {

    // The documentation for this method has all the information below.
    // Make sure to update it if you update the code.

    if (!strcmp(type, "data"))
        return "share/apps/";
    if (!strcmp(type, "html"))
        return "share/doc/HTML/";
    if (!strcmp(type, "icon"))
        return "share/icons/";
    if (!strcmp(type, "config"))
        return "share/config/";
    if (!strcmp(type, "pixmap"))
        return "share/pixmaps/";
    if (!strcmp(type, "apps"))
        return "share/applnk/";
    if (!strcmp(type, "sound"))
        return "share/sounds/";
    if (!strcmp(type, "locale"))
        return "share/locale/";
    if (!strcmp(type, "services"))
        return "share/kde4/services/";
    if (!strcmp(type, "servicetypes"))
        return "share/kde4/servicetypes/";
    if (!strcmp(type, "mime"))
        return "share/mimelnk/";
    if (!strcmp(type, "cgi"))
        return "cgi-bin/";
    if (!strcmp(type, "wallpaper"))
        return "share/wallpapers/";
    if (!strcmp(type, "templates"))
        return "share/templates/";
    if (!strcmp(type, "exe"))
        return "bin/";
    if (!strcmp(type, "lib"))
        return "lib" KDELIBSUFF "/";
    if (!strcmp(type, "module"))
        return "lib" KDELIBSUFF "/kde4/";
    if (!strcmp(type, "qtplugins"))
        return "lib" KDELIBSUFF "/kde4/plugins";
    if (!strcmp(type, "kcfg"))
        return "share/config.kcfg";
    if (!strcmp(type, "emoticons"))
        return "share/emoticons";
    if (!strcmp(type, "xdgdata-apps"))
        return "applications/";
    if (!strcmp(type, "xdgdata-icon"))
        return "icons/";
    if (!strcmp(type, "xdgdata-pixmap"))
        return "pixmaps/";
    if (!strcmp(type, "xdgdata-dirs"))
        return "desktop-directories/";
    if (!strcmp(type, "xdgdata-mime"))
        return "mime/";
    if (!strcmp(type, "xdgconf-menu"))
        return "menus/";


    qFatal("unknown resource type %s", type);
    return QString();
}

QString KStandardDirs::saveLocation(const char *type,
                                    const QString& suffix,
                                    bool create) const
{
    checkConfig();

    QString path = d->savelocations.value(type);
    if (path.isEmpty())
    {
       QStringList dirs = d->relatives.value(type);
       if (dirs.isEmpty() && (
                     (strcmp(type, "socket") == 0) ||
                     (strcmp(type, "tmp") == 0) ||
                     (strcmp(type, "cache") == 0) ))
       {
          (void) resourceDirs(type); // Generate socket|tmp|cache resource.
          dirs = d->relatives.value(type); // Search again.
       }
       if (!dirs.isEmpty())
       {
          // Check for existence of typed directory + suffix
          if (strncmp(type, "xdgdata-", 8) == 0) {
             path = realPath( localxdgdatadir() + dirs.last() ) ;
          } else if (strncmp(type, "xdgconf-", 8) == 0) {
             path = realPath( localxdgconfdir() + dirs.last() );
          } else {
             path = realPath( localkdedir() + dirs.last() );
          }
       }
       else {
          dirs = d->absolutes.value(type);
          if (dirs.isEmpty()) {
             qFatal("KStandardDirs: The resource type %s is not registered", type);
          }
          path = realPath(dirs.last());
       }

       d->savelocations.insert(type, path);
    }
    QString fullPath = path + (path.endsWith("/") ? "" : "/") + suffix;

    KDE_struct_stat st;
    if (KDE_stat(QFile::encodeName(fullPath), &st) != 0 || !(S_ISDIR(st.st_mode))) {
        if(!create) {
#ifndef NDEBUG
            kDebug(180) << QString("save location %1 doesn't exist").arg(fullPath) << endl;
#endif
            return fullPath;
        }
        if(!makeDir(fullPath, 0700)) {
            return fullPath;
        }
        d->dircache.remove(type);
    }
    if (!fullPath.endsWith("/"))
            fullPath += '/';
    return fullPath;
}

QString KStandardDirs::relativeLocation(const char *type, const QString &absPath)
{
    QString fullPath = absPath;
    int i = absPath.lastIndexOf('/');
    if (i != -1) {
       fullPath = realFilePath(absPath); // Normalize
    }

    QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (fullPath.startsWith(*it)) {
            return fullPath.mid((*it).length());
        }
    }
    return absPath;
}


bool KStandardDirs::makeDir(const QString& dir, int mode)
{
    // we want an absolute path
    if (QDir::isRelativePath(dir))
        return false;

    QString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != '/')
        target += '/';

    QString base("");
    uint i = 1;

    while( i < len )
    {
        KDE_struct_stat st;
        int pos = target.indexOf('/', i);
        base += target.mid(i - 1, pos - i + 1);
        QByteArray baseEncoded = QFile::encodeName(base);
        // bail out if we encountered a problem
        if (KDE_stat(baseEncoded, &st) != 0)
        {
          // Directory does not exist....
          // Or maybe a dangling symlink ?
          if (KDE_lstat(baseEncoded, &st) == 0)
              (void)unlink(baseEncoded); // try removing

          if (KDE_mkdir(baseEncoded, static_cast<mode_t>(mode)) != 0) {
            baseEncoded.prepend( "trying to create local folder " );
            perror(baseEncoded.data());
            return false; // Couldn't create it :-(
          }
        }
        i = pos + 1;
    }
    return true;
}

static QString readEnvPath(const char *env)
{
   QByteArray c_path = getenv(env);
   if (c_path.isEmpty())
      return QString();
   return QFile::decodeName(c_path);
}

#ifdef __linux__
static QString executablePrefix()
{
   char path_buffer[MAXPATHLEN + 1];
   path_buffer[MAXPATHLEN] = 0;
   int length = readlink ("/proc/self/exe", path_buffer, MAXPATHLEN);
   if (length == -1)
      return QString();

   path_buffer[length] = '\0';

   QString path = QFile::decodeName(path_buffer);

   if(path.isEmpty())
      return QString();

   int pos = path.lastIndexOf('/'); // Skip filename
   if(pos <= 0)
      return QString();
   pos = path.lastIndexOf('/', pos - 1); // Skip last directory
   if(pos <= 0)
      return QString();

   return path.left(pos);
}
#endif

void KStandardDirs::addResourcesFrom_krcdirs()
{
    QString localFile = QDir::currentPath() + QDir::separator() + ".krcdirs";
    if (!QFile::exists(localFile))
        return;

    QSettings iniFile(localFile, QSettings::IniFormat);
    iniFile.beginGroup("KStandardDirs");
    const QStringList resources = iniFile.allKeys();
    foreach(const QString &key, resources)
    {
        QDir path(iniFile.value(key).toString());
        if (!path.exists())
            continue;

        if(path.makeAbsolute())
            addResourceDir(key.toAscii(), path.path(), false);
    }
}

void KStandardDirs::addKDEDefaults()
{
    addResourcesFrom_krcdirs();

    QStringList kdedirList;

    // begin KDEDIRS
    QString kdedirs = readEnvPath("KDEDIRS");
    if (!kdedirs.isEmpty())
    {
        tokenize(kdedirList, kdedirs, QString(QChar(KPATH_SEPARATOR)));
    }
    kdedirList.append(QFile::decodeName(KDEDIR));

    QString execPrefix(EXEC_INSTALL_PREFIX);
    if (!kdedirList.contains(execPrefix))
       kdedirList.append(execPrefix);
#ifdef __linux__
    const QString linuxExecPrefix = executablePrefix();
    if ( !linuxExecPrefix.isEmpty() )
       kdedirList.append( linuxExecPrefix );
#endif

    // We treat root differently to prevent a "su" shell messing up the
    // file permissions in the user's home directory.
    QString localKdeDir = readEnvPath(getuid() ? "KDEHOME" : "KDEROOTHOME");
    if (!localKdeDir.isEmpty())
    {
       if (localKdeDir[localKdeDir.length()-1] != '/')
          localKdeDir += '/';
    }
    else
    {
#ifdef Q_WS_MACX
       localKdeDir =  QDir::homePath() + "/Library/Preferences/KDE/";
#else
       localKdeDir =  QDir::homePath() + "/.kde/";
#endif
    }

    if (localKdeDir != "-/")
    {
        localKdeDir = KShell::tildeExpand(localKdeDir);
        addPrefix(localKdeDir);
    }

    QStringList::ConstIterator end(kdedirList.end());
    for (QStringList::ConstIterator it = kdedirList.begin();
         it != end; ++it)
    {
        const QString dir = KShell::tildeExpand(*it);
        addPrefix(dir);
    }
    // end KDEDIRS

    // begin XDG_CONFIG_XXX
    QStringList xdgdirList;
    QString xdgdirs = readEnvPath("XDG_CONFIG_DIRS");
    if (!xdgdirs.isEmpty())
    {
        tokenize(xdgdirList, xdgdirs, QString(QChar(KPATH_SEPARATOR)));
    }
    else
    {
        xdgdirList.clear();
        xdgdirList.append("/etc/xdg");
#ifdef Q_WS_WIN
        xdgdirList.append(kfsstnd_defaultprefix() + "/etc/xdg");
#else
        xdgdirList.append(KDESYSCONFDIR "/xdg");
#endif
    }

    QString localXdgDir = readEnvPath("XDG_CONFIG_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != '/')
          localXdgDir += '/';
    }
    else
    {
#ifdef Q_WS_MACX
       localXdgDir =  QDir::homePath() + "/Library/Preferences/XDG/";
#else
       localXdgDir =  QDir::homePath() + "/.config/";
#endif
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.begin();
         it != xdgdirList.end(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
        addXdgConfigPrefix(dir);
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty())
    {
        tokenize(xdgdirList, xdgdirs, QString(QChar(KPATH_SEPARATOR)));
    }
    else
    {
        xdgdirList.clear();
        for (QStringList::ConstIterator it = kdedirList.begin();
           it != kdedirList.end(); ++it)
        {
           QString dir = *it;
           if (dir[dir.length()-1] != '/')
             dir += '/';
           xdgdirList.append(dir+"share/");
        }

        xdgdirList.append("/usr/local/share/");
        xdgdirList.append("/usr/share/");
    }

    localXdgDir = readEnvPath("XDG_DATA_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != '/')
          localXdgDir += '/';
    }
    else
    {
       localXdgDir = QDir::homePath() + "/.local/share/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgDataPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.begin();
         it != xdgdirList.end(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
        addXdgDataPrefix(dir);
    }
    // end XDG_DATA_XXX


    uint index = 0;
    while (types_indices[index] != -1) {
        addResourceType(types + types_indices[index], kde_default(types + types_indices[index]));
        index++;
    }
    addResourceDir("exe", LIBEXEC_INSTALL_DIR, false );

    addResourceDir("home", QDir::homePath(), false);
}

void KStandardDirs::checkConfig() const
{
    if (!d->addedCustoms) {
        if (d->componentData.isValid() && d->componentData.privateConfig()) {
            const_cast<KStandardDirs*>(this)->addCustomized(d->componentData.privateConfig().data());
        } else if (KGlobal::hasMainComponent() && KGlobal::mainComponent().privateConfig()) {
            const_cast<KStandardDirs*>(this)->addCustomized(KGlobal::mainComponent().privateConfig().data());
        }
    }
}

static QStringList lookupProfiles(const QString &mapFile, const KComponentData &componentData)
{
    QStringList profiles;

    if (mapFile.isEmpty() || !QFile::exists(mapFile))
    {
       profiles << "default";
       return profiles;
    }

    struct passwd *pw = getpwuid(geteuid());
    if (!pw)
    {
        profiles << "default";
        return profiles; // Not good
    }

    QByteArray user = pw->pw_name;

    gid_t sup_gids[512];
    int sup_gids_nr = getgroups(512, sup_gids);

    KConfig mapCfgFile(componentData, mapFile );
    KConfigGroup mapCfg(&mapCfgFile, "Users");
    if (mapCfg.hasKey(user.data()))
    {
        profiles = mapCfg.readEntry(user.data(), QStringList());
        return profiles;
    }

    const KConfigGroup generalGrp(&mapCfgFile, "General");
    QStringList groups = generalGrp.readEntry("groups", QStringList());

    const KConfigGroup groupsGrp(&mapCfgFile, "Groups");

    for( QStringList::ConstIterator it = groups.begin();
         it != groups.end(); ++it )
    {
        QByteArray grp = (*it).toUtf8();
        // Check if user is in this group
        struct group *grp_ent = getgrnam(grp);
        if (!grp_ent) continue;
        gid_t gid = grp_ent->gr_gid;
        if (pw->pw_gid == gid)
        {
            // User is in this group --> add profiles
            profiles += groupsGrp.readEntry(*it, QStringList());
        }
        else
        {
            for(int i = 0; i < sup_gids_nr; i++)
            {
                if (sup_gids[i] == gid)
                {
                    // User is in this group --> add profiles
                    profiles += groupsGrp.readEntry(*it, QStringList());
                    break;
                }
            }
        }
    }

    if (profiles.isEmpty())
        profiles << "default";
    return profiles;
}

extern bool kde_kiosk_admin;

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (d->addedCustoms && !d->checkRestrictions) // there are already customized entries
        return false; // we just quit and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    int configdirs = resourceDirs("config").count();

    if (!d->addedCustoms)
    {
        // We only add custom entries once
        d->addedCustoms = true;

        // reading the prefixes in
        QString group = QLatin1String("Directories");
        KConfigGroup cg(config, group);

        QString kioskAdmin = cg.readEntry("kioskAdmin");
        if (!kioskAdmin.isEmpty() && !kde_kiosk_admin)
        {
            int i = kioskAdmin.indexOf(':');
            QString user = kioskAdmin.left(i);
            QString host = kioskAdmin.mid(i+1);

            KUser thisUser;
            char hostname[ 256 ];
            hostname[ 0 ] = '\0';
            if (!gethostname( hostname, 255 ))
                hostname[sizeof(hostname)-1] = '\0';

            if ((user == thisUser.loginName()) &&
                (host.isEmpty() || (host == hostname)))
            {
                kde_kiosk_admin = true;
            }
        }

        bool readProfiles = true;

        if (kde_kiosk_admin && !QByteArray(getenv("KDE_KIOSK_NO_PROFILES")).isEmpty())
            readProfiles = false;

        QString userMapFile = cg.readEntry("userProfileMapFile");
        QString profileDirsPrefix = cg.readEntry("profileDirsPrefix");
        if (!profileDirsPrefix.isEmpty() && !profileDirsPrefix.endsWith("/"))
            profileDirsPrefix.append("/");

        QStringList profiles;
        if (readProfiles)
            profiles = lookupProfiles(userMapFile, d->componentData);
        QString profile;

        bool priority = false;
        while(true)
        {
            KConfigGroup cg(config, group);
            QStringList list = cg.readEntry("prefixes", QStringList());
            for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            {
                addPrefix(*it, priority);
                addXdgConfigPrefix(*it+"/etc/xdg", priority);
                addXdgDataPrefix(*it+"/share", priority);
            }
            // If there are no prefixes defined, check if there is a directory
            // for this profile under <profileDirsPrefix>
            if (list.isEmpty() && !profile.isEmpty() && !profileDirsPrefix.isEmpty())
            {
                QString dir = profileDirsPrefix + profile;
                addPrefix(dir, priority);
                addXdgConfigPrefix(dir+"/etc/xdg", priority);
                addXdgDataPrefix(dir+"/share", priority);
            }

            // iterating over all entries in the group Directories
            // to find entries that start with dir_$type
            QMap<QString, QString> entries = config->entryMap(group);
            for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
                 it2 != entries.end(); it2++)
            {
                const QString key = it2.key();
                if (key.startsWith("dir_")) {
                    // generate directory list, there may be more than 1.
                    QStringList dirs = (*it2).split(',');
                    QStringList::Iterator sIt(dirs.begin());
                    QString resType = key.mid(4, key.length());
                    for (; sIt != dirs.end(); ++sIt)
                    {
                        addResourceDir(resType.toLatin1(), *sIt, priority);
                    }
                }
            }
            if (profiles.isEmpty())
                break;
            profile = profiles.back();
            group = QString::fromLatin1("Directories-%1").arg(profile);
            profiles.pop_back();
            priority = true;
        }
    }

    // Process KIOSK restrictions.
    if (!kde_kiosk_admin || QByteArray(getenv("KDE_KIOSK_NO_RESTRICTIONS")).isEmpty())
    {
        KConfigGroup cg(config, "KDE Resource Restrictions");
        QMap<QString, QString> entries = cg.entryMap();
        for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
            it2 != entries.end(); it2++)
        {
            const QString key = it2.key();
            if (!cg.readEntry(key, true))
            {
                d->restrictionsActive = true;
                d->restrictions.insert(key.toLatin1(), true);
                d->dircache.remove(key.toLatin1());
            }
        }
    }

    // check if the number of config dirs changed
    bool configDirsChanged = (resourceDirs("config").count() != configdirs);
    // If the config dirs changed, we check kiosk restrictions again.
    d->checkRestrictions = configDirsChanged;
    // return true if the number of config dirs changed: reparse config file
    return configDirsChanged;
}

QString KStandardDirs::localkdedir() const
{
    // Return the prefix to use for saving
    return d->prefixes.first();
}

QString KStandardDirs::localxdgdatadir() const
{
    // Return the prefix to use for saving
    return d->xdgdata_prefixes.first();
}

QString KStandardDirs::localxdgconfdir() const
{
    // Return the prefix to use for saving
    return d->xdgconf_prefixes.first();
}


// just to make code more readable without macros
QString KStandardDirs::locate( const char *type,
                               const QString& filename, const KComponentData &cData)
{
    return cData.dirs()->findResource(type, filename);
}

QString KStandardDirs::locateLocal( const char *type,
                                    const QString& filename, const KComponentData &cData)
{
    return locateLocal(type, filename, true, cData);
}

QString KStandardDirs::locateLocal( const char *type,
                                    const QString& filename, bool createDir,
                                    const KComponentData &cData)
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.lastIndexOf('/')+1;
    if (!slash) { // only one filename
        return cData.dirs()->saveLocation(type, QString(), createDir) + filename;
    }

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return cData.dirs()->saveLocation(type, dir, createDir) + file;
}

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

QString KStandardDirs::installPath(const char *type)
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
    if (!installprefixes[index].isEmpty())
        return installprefixes[index+1];
    return QString();

}

bool KStandardDirs::checkAccess(const QString& pathname, int mode)
{
  int accessOK = access( QFile::encodeName(pathname), mode );
  if ( accessOK == 0 )
    return true;  // OK, I can really access the file

  // else
  // if we want to write the file would be created. Check, if the
  // user may write to the directory to create the file.
  if ( (mode & W_OK) == 0 )
    return false;   // Check for write access is not part of mode => bail out


  if (!access( QFile::encodeName(pathname), F_OK)) // if it already exists
      return false;

  //strip the filename (everything until '/' from the end
  QString dirName(pathname);
  int pos = dirName.lastIndexOf('/');
  if ( pos == -1 )
    return false;   // No path in argument. This is evil, we won't allow this
  else if ( pos == 0 ) // don't turn e.g. /root into an empty string
      pos = 1;

  dirName.truncate(pos); // strip everything starting from the last '/'

  accessOK = access( QFile::encodeName(dirName), W_OK );
  // -?- Can I write to the accessed diretory
  if ( accessOK == 0 )
    return true;  // Yes
  else
    return false; // No
}
