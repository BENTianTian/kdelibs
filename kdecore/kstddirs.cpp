/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 * Version:	$Id$
 * Generated:	Thu Mar  5 16:05:28 EST 1998
 */


#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <iostream.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/types.h>
#include <dirent.h>

#include <qregexp.h>
#include <qdict.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kstddirs.h"
#include "kglobal.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kinstance.h"

template class QDict<QStringList>;

static const char* types[] = {"html", "icon", "apps", "sound",
			      "data", "locale", "services", "mime",
			      "servicetypes", "cgi", "config", "exe",
			      "toolbar", "wallpaper", "lib", 0};

static int tokenize( QStringList& token, const QString& str,
		const QString& delim );

KStandardDirs::KStandardDirs( ) : addedCustoms(false)
{
    dircache.setAutoDelete(true);
    relatives.setAutoDelete(true);
    absolutes.setAutoDelete(true);
}

KStandardDirs::~KStandardDirs()
{
}

void KStandardDirs::addPrefix( QString dir )
{
    if (dir.isNull())
      return;

    if (dir.at(dir.length() - 1) != '/')
	dir += '/';

    if (!prefixes.contains(dir)) {
	prefixes.prepend(dir);
	dircache.clear();
    }
}

bool KStandardDirs::addResourceType( const char *type,
				     const QString& relativename )
{
    if (relativename.isNull())
       return false;

    QStringList *rels = relatives.find(type);
    if (!rels) {
	rels = new QStringList();
	relatives.insert(type, rels);
    }
    QString copy = relativename;
    if (copy.at(copy.length() - 1) != '/')
	copy += '/';
    if (!rels->contains(copy)) {
	rels->prepend(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const char *type,
				    const QString& absdir)
{
    QStringList *paths = absolutes.find(type);
    if (!paths) {
	paths = new QStringList();
	absolutes.insert(type, paths);
    }
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != '/')
      copy += '/';
    if (!paths->contains(copy)) {
	paths->append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

QString KStandardDirs::findResource( const char *type,
				     const QString& filename ) const
{
    if (filename.at(0) == '/')
	return filename; // absolute dirs are absolute dirs, right? :-/

    QString dir = findResourceDir(type, filename);
    if (dir.isNull())
	return dir;
    else return dir + filename;
}

QStringList KStandardDirs::findDirs( const char *type,
                                     const QString& reldir ) const
{
    QStringList list;

    QStringList candidates = resourceDirs(type);
    QDir testdir;

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); it++) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absPath() + '/');
    }
    return list;
}

QString KStandardDirs::findResourceDir( const char *type,
					const QString& filename) const
{
#ifndef NDEBUG
    if (filename.isEmpty()) {
      warning("filename for type %s in KStandardDirs::findResourceDir is not supposed to be empty!!", type);
      return QString::null;
    }
#endif

    QStringList candidates = resourceDirs(type);
    QString fullPath;
   
    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++)
      if (exists(*it + filename))
	return *it;
    
#ifndef NDEBUG
    if(false && type != "locale")
      debug("KStdDirs::findResDir(): can't find \"%s\" in type \"%s\".",
            filename.ascii(), type);
#endif

    return QString::null;
}

bool KStandardDirs::exists(const QString &fullPath)
{
    struct stat buff;
    if (access(fullPath.ascii(), R_OK) == 0 && stat( fullPath.ascii(), &buff ) == 0)
	if ( S_ISREG( buff.st_mode ))
	    return true;
    return false;
}

static void lookupDirectory(const QString& path, const QString &relPart,
			    const QRegExp &regexp,
			    QStringList& list,
			    QStringList& relList,
			    bool recursive, bool uniq)
{
  QString pattern = regexp.pattern();
  if (recursive || pattern.contains('?') || pattern.contains('*'))
  {
    // We look for a set of files.
    DIR *dp = opendir( QFile::encodeName(path));
    if (!dp)
      return;

    assert(path.at(path.length() - 1) == '/');

    struct dirent *ep;
    struct stat buff;

    while( ( ep = readdir( dp ) ) != 0L )
    {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
	continue;

      if (!recursive && (regexp.match(fn) == -1))
	continue; // No match

      QString pathfn = path + fn;
      if ( stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
	QString tmp = QString("Error stat'ing %1").arg( pathfn );
	perror(tmp.ascii());
	continue; // Couldn't stat (Why not?)
      }
      if ( recursive ) {
	if ( S_ISDIR( buff.st_mode )) {
	  lookupDirectory(pathfn + '/', relPart + fn + '/', regexp, list, relList, recursive, uniq);
	}
	if (regexp.match(fn) == -1)
	  continue; // No match
      }
      if ( S_ISREG( buff.st_mode))
      {
        if (!uniq || !relList.contains(relPart + fn))
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
     struct stat buff;
     if ( stat( QFile::encodeName(pathfn), &buff ) != 0 ) 
        return; // File not found
     if ( S_ISREG( buff.st_mode))
     {
       if (!uniq || !relList.contains(relPart + fn))
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
			 bool recursive, bool uniq)
{
    if (relpath.isNull()) {
       lookupDirectory(prefix, relPart, regexp, list, 
		       relList, recursive, uniq);
       return;
    }
    QString path;
    QString rest;

    if (relpath.length())
    {
       int slash = relpath.find('/');
       if (slash < 0)
	   rest = relpath.left(relpath.length() - 1);
       else {
	   path = relpath.left(slash);
	   rest = relpath.mid(slash + 1);
       }
    }

    assert(prefix.at(prefix.length() - 1) == '/');
    
    struct stat buff;

    if (path.contains('*') || path.contains('?')) {
	
	QRegExp pathExp(path, true, true);
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
		
		if (pathExp.match(fn) == -1)
		    continue; // No match
		QString rfn = relPart+fn;
		fn = prefix + fn;
		if ( stat( fn.ascii(), &buff ) != 0 ) {
		    QString tmp = QString("Error statting %1:").arg( fn );
		    perror(tmp.ascii());
		    continue; // Couldn't stat (Why not?)
		}
		if ( S_ISDIR( buff.st_mode ))
		    lookupPrefix(fn + '/', rest, rfn + '/', regexp, list, relList, recursive, uniq);
	    }
	
	closedir( dp );
    } else {
        // Don't stat, if the dir doesn't exist we will find out
        // when we try to open it.
        lookupPrefix(prefix + path + '/', rest, 
                     relPart + path + '/', regexp, list, 
                     relList, recursive, uniq);
    }
}

QStringList
KStandardDirs::findAllResources( const char *type,
			         const QString& filter,
				 bool recursive,
			         bool uniq,
                                 QStringList &relList) const
{
    QStringList list;
    if (filter.at(0) == '/') // absolute paths we return
    {
        list.append( filter);
	return list;
    }

    QString filterPath;
    QString filterFile;

    if (filter.length())
    {
       int slash = filter.findRev('/');
       if (slash < 0)
	   filterFile = filter;
       else {
	   filterPath = filter.left(slash + 1);
	   filterFile = filter.mid(slash + 1);
       }
    }

    QStringList candidates = resourceDirs(type);

    if (filterFile.isEmpty())
	filterFile = "*";

    if (filterFile.find('*') || filterPath.find('*')) {
	QRegExp regExp(filterFile, true, true);
	
	for (QStringList::ConstIterator it = candidates.begin();
	     it != candidates.end(); it++)
	    lookupPrefix(*it, filterPath, "", regExp, list, 
			 relList, recursive, uniq);
    } else  {
	for (QStringList::ConstIterator it = candidates.begin();
	     it != candidates.end(); it++) {
	    QString fullpath = *it + filter;
	    if (exists(fullpath))
		list.append(fullpath);
	}
    }

    return list;
}

QStringList
KStandardDirs::findAllResources( const char *type,
			         const QString& filter,
				 bool recursive,
			         bool uniq) const
{
   QStringList relList;
   return findAllResources(type, filter, recursive, uniq, relList);
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
  QStringList *candidates = dircache.find(type);
  
  if (!candidates) { // filling cache
    QDir testdir;
    candidates = new QStringList();
    QStringList *dirs = absolutes.find(type);
    if (dirs)
      for (QStringList::ConstIterator it = dirs->begin();
	   it != dirs->end(); ++it) {
	testdir.setPath(*it);
	if (testdir.exists() && !candidates->contains(*it))
	  candidates->append(*it);
      }
    dirs = relatives.find(type);
    if (dirs)
      for (QStringList::ConstIterator pit = prefixes.fromLast();
	   pit != prefixes.end(); --pit)
	for (QStringList::ConstIterator it = dirs->begin();
	     it != dirs->end(); ++it) {
	  QString path = *pit + *it;
	  testdir.setPath(path);
	  if (testdir.exists() && !candidates->contains(path))
	    candidates->append(path);
	}
    dircache.insert(type, candidates);
  }
  return *candidates;
}

QString KStandardDirs::findExe( const QString& appname,
				const QString& pstr, bool ignore)
{
    QFileInfo info;
    QStringList tokens;
    QString p = pstr;

    if( p == QString::null ) {
	p = getenv( "PATH" );
    }

    tokenize( tokens, p, ":\b" );

    // split path using : or \b as delimiters
    for( unsigned i = 0; i < tokens.count(); i++ ) {
	p = tokens[ i ];
	p += "/";
	p += appname;
	
	// Check for executable in this tokenized path
	info.setFile( p );
	
	if( info.exists() && ( ignore || info.isExecutable() )
	    && info.isFile() ) {
	    return p;
	}
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.

    return QString::null;
}

int KStandardDirs::findAllExe( QStringList& list, const QString& appname,
			const QString& pstr, bool ignore )
{
    QString p = pstr;
    QFileInfo info;
    QStringList tokens;

    if( p == QString::null ) {
	p = getenv( "PATH" );
    }

    list.clear();
    tokenize( tokens, p, ":\b" );

    for ( unsigned i = 0; i < tokens.count(); i++ ) {
	p = tokens[ i ];
	p += "/";
	p += appname;
	
	info.setFile( p );
	
	if( info.exists() && (ignore || info.isExecutable())
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
	if ( delim.find( str[ index ] ) >= 0 )
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
    if (!strcmp(type, "data"))
	return "share/apps/";
    if (!strcmp(type, "html"))
	return "share/doc/HTML/";
    if (!strcmp(type, "icon"))
	return "share/icons/";
    if (!strcmp(type, "config"))
	return "share/config/";
    if (!strcmp(type, "toolbar"))
	return "share/toolbar/";
    if (!strcmp(type, "apps"))
	return "share/applnk/";
    if (!strcmp(type, "sound"))
	return "share/sounds/";
    if (!strcmp(type, "locale"))
	return "share/locale/";
    if (!strcmp(type, "services"))
	return "share/services/";
    if (!strcmp(type, "servicetypes"))
	return "share/servicetypes/";
    if (!strcmp(type, "mime"))
	return "share/mimelnk/";
    if (!strcmp(type, "cgi"))
	return "cgi-bin/";
    if (!strcmp(type, "wallpaper"))
	return "share/wallpapers/";
    if (!strcmp(type, "exe"))
	return "bin/";
    if (!strcmp(type, "lib"))
	return "lib/";
    fatal("unknown resource type %s", type);
    return QString::null;
}

QString KStandardDirs::saveLocation(const char *type,
				    const QString& suffix,
				    bool create) const
{
    QStringList *dirs = relatives.find(type);
    if (!dirs)
	fatal("there are no relative suffixes for type %s registered", type);

    struct stat st;
    QString local = localkdedir();

    // Check for existance of typed directory + suffix
    QString fullPath = local + dirs->last() + suffix;
    if (stat(fullPath.ascii(), &st) != 0 || !(S_ISDIR(st.st_mode))) {
	if(!create) {
	    debug("save location %s doesn't exist", fullPath.ascii());
	    return local;
	}
	if(!makeDir(fullPath, 0700)) {
	    debug("failed to create %s", fullPath.ascii());
	    return local;
	}
    }
    return fullPath;

    // I can't think of a case where this happens
    debug("couldn't find save location for type %s", type);
    return local;
}

bool KStandardDirs::makeDir(const QString& dir, int mode)
{
    // we want an absolute path
    if (dir.at(0) != '/')
        return false;

    QString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != '/')
        target += '/';

    QString base("/");
    uint i = 1;

    while( i < len )
    {
        struct stat st;
        int pos = target.find('/', i);
        base += target.mid(i, pos - i + 1);
        // bail out if we encountered a problem
        if (stat(base.ascii(), &st) != 0)
        {
           // Directory does not exist....
	  if ( mkdir(base.ascii(), (mode_t) mode) != 0) {
	    perror("trying to create local folder");
	    return false; // Couldn't create it :-(
	  }
        }
        i = pos + 1;
    }
    return true;
}

void KStandardDirs::addKDEDefaults()
{
    QStringList kdedirList;

    const char *kdedirs = getenv("KDEDIRS");
    if (kdedirs && strlen(kdedirs))
	tokenize(kdedirList, kdedirs, ":");
    else {
	QString kdedir = getenv("KDEDIR");
	if (!kdedir.isEmpty())
	  kdedirList.append(kdedir);
        addPrefix(QDir::homeDirPath() + "/.kde/");
    }
    kdedirList.append(KDEDIR);  // Location of kdelibs.

    for (QStringList::ConstIterator it = kdedirList.begin();
	 it != kdedirList.end(); it++)
	addPrefix(*it);

    uint index = 0;
    while (types[index] != 0) {
	addResourceType(types[index], kde_default(types[index]));
	index++;
    }

}

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (addedCustoms) // there are already customized entries
        return false; // we just quite and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    uint configdirs = resourceDirs("config").count();

    // reading the prefixes in
    KConfigGroupSaver(config, "Directories");
    QStringList list;
    QStringList::ConstIterator it;
    list = config->readListEntry("prefixes");
    for (it = list.begin(); it != list.end(); it++)
	addPrefix(*it);

    // iterating over all entries in the group Directories
    // to find entries that start with dir_$type
    QMap<QString, QString> entries = config->entryMap("Directories");
    QMap<QString, QString>::ConstIterator it2;
    for (it2 = entries.begin(); it2 != entries.end(); it2++)
    {
	QString key = it2.key();
	if (key.left(4) == "dir_")
	{
	    if (addResourceDir(key.mid(4, key.length()), it2.data()))
	      debug("adding custom dir %s of type %s", it2.data().ascii(),
		    key.mid(4,key.length()).data());
	    else
	      debug("couldn't add custom dir %s of type %s", it2.data().ascii(),
		    key.mid(4,key.length()).data());
	}
    }

    // save it for future calls - that will return
    addedCustoms = true;

    // return true if the number of config dirs changed
    return (resourceDirs("config").count() != configdirs);
}

QString KStandardDirs::localkdedir() const
{
    // The last dir in KDEDIRS is the local dir.
    return prefixes.last();
}

// just to make code more readable without macros
QString locate( const char *type,
		const QString& filename, const KInstance* inst )
{
    return inst->dirs()->findResource(type, filename);
}

QString locateLocal( const char *type,
	             const QString& filename, const KInstance* inst )
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.findRev('/')+1;
    if (!slash) // only one filename
	return inst->dirs()->saveLocation(type) + filename;

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return inst->dirs()->saveLocation(type, dir) + file;
}
