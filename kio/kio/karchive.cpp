/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   Moved from ktar.cpp by Roberto Teixeira <maragato@kde.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <assert.h>

#include <qptrlist.h>
#include <qptrstack.h>
#include <qvaluestack.h>
#include <qmap.h>
#include <qcstring.h>
#include <qdir.h>
#include <qfile.h>

#include <kdebug.h>
#include <kfilterdev.h>
#include <kfilterbase.h>

#include "karchive.h"
#include "klimitediodevice.h"

template class QDict<KArchiveEntry>;


class KArchive::KArchivePrivate
{
public:
    KArchiveDirectory* rootDir;
};

class PosSortedPtrList : public QPtrList<KArchiveFile> {
protected:
    int compareItems( QPtrCollection::Item i1,
                      QPtrCollection::Item i2 )
    {
        int pos1 = static_cast<KArchiveFile*>( i1 )->position();
        int pos2 = static_cast<KArchiveFile*>( i2 )->position();
        return ( pos1 - pos2 );
    }
};


////////////////////////////////////////////////////////////////////////
/////////////////////////// KArchive ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchive::KArchive( QIODevice * dev )
{
    d = new KArchivePrivate;
    d->rootDir = 0;
    m_dev = dev;
    m_open = false;
}

KArchive::~KArchive()
{
    if ( m_open )
        close();
    delete d->rootDir;
    delete d;
}

bool KArchive::open( int mode )
{
    if(0 == m_dev)
        return false; // Fail w/o segfaulting if the device is no good

    if ( !m_dev->open( mode ) )
        return false;

    if ( m_open )
        close();

    m_mode = mode;
    m_open = true;

    Q_ASSERT( d->rootDir == 0L );
    d->rootDir = 0L;

    return openArchive( mode );
}

void KArchive::close()
{
    if ( !m_open )
        return;
    // moved by holger to allow kzip to write the zip central dir
    // to the file in closeArchive()
    closeArchive();

    m_dev->close();

    delete d->rootDir;
    d->rootDir = 0;
    m_open = false;
}

const KArchiveDirectory* KArchive::directory() const
{
    // rootDir isn't const so that parsing-on-demand is possible
    return const_cast<KArchive *>(this)->rootDir();
}


bool KArchive::addLocalFile( const QString& fileName, const QString& destName )
{
    QFileInfo fileInfo( fileName );
    if ( !fileInfo.isFile() )
    {
        kdWarning() << "KArchive::addLocalFile " << fileName << " doesn't exist or is not a regular file." << endl;
        return false;
    }

    uint size = fileInfo.size();
    if ( !prepareWriting( destName, fileInfo.owner(), fileInfo.group(), size ) )
    {
        kdWarning() << "KArchive::addLocalFile prepareWriting " << destName << " failed" << endl;
        return false;
    }

    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) )
    {
        kdWarning() << "KArchive::addLocalFile couldn't open file " << fileName << endl;
        return false;
    }

    // Read and write data in chunks to minimize memory usage
    QByteArray array(8*1024);
    int n;
    uint total = 0;
    while ( ( n = file.readBlock( array.data(), array.size() ) ) > 0 )
    {
        if ( !writeData( array.data(), n ) )
        {
            kdWarning() << "KArchive::addLocalFile writeData failed" << endl;
            return false;
        }
        total += n;
    }
    Q_ASSERT( total == size );

    if ( !doneWriting( size ) )
    {
        kdWarning() << "KArchive::addLocalFile doneWriting failed" << endl;
        return false;
    }
    return true;
}

bool KArchive::addLocalDirectory( const QString& path, const QString& destName )
{
    QString dot = ".";
    QString dotdot = "..";
    QDir dir( path );
    if ( !dir.exists() )
        return false;
    QStringList files = dir.entryList();
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
        if ( *it != dot && *it != dotdot )
        {
            QString fileName = path + "/" + *it;
            QString dest = destName.isEmpty() ? *it : (destName + "/" + *it);
            QFileInfo fileInfo( fileName );

            if ( fileInfo.isDir() )
                addLocalDirectory( fileName, dest );
            else if ( fileInfo.isFile() )
                addLocalFile( fileName, dest );
            // We omit symlinks and sockets
        }
    }
    return true;
}

bool KArchive::writeFile( const QString& name, const QString& user, const QString& group, uint size, const char* data )
{

    if ( !prepareWriting( name, user, group, size ) )
    {
        kdWarning() << "KArchive::writeFile prepareWriting failed" << endl;
        return false;
    }

    // Write data
    // Note: if data is 0L, don't call writeBlock, it would terminate the KFilterDev
    if ( data && size && !writeData( data, size ) )
    {
        kdWarning() << "KArchive::writeFile writeData failed" << endl;
        return false;
    }

    if ( !doneWriting( size ) )
    {
        kdWarning() << "KArchive::writeFile doneWriting failed" << endl;
        return false;
    }
    return true;
}

KArchiveDirectory * KArchive::rootDir()
{
    if ( !d->rootDir )
    {
        //kdDebug() << "Making root dir " << endl;
        struct passwd* pw =  getpwuid( getuid() );
        struct group* grp = getgrgid( getgid() );
        QString username = pw ? QFile::decodeName(pw->pw_name) : QString::number( getuid() );
        QString groupname = grp ? QFile::decodeName(grp->gr_name) : QString::number( getgid() );

        d->rootDir = new KArchiveDirectory( this, QString::fromLatin1("/"), (int)(0777 + S_IFDIR), 0, username, groupname, QString::null );
    }
    return d->rootDir;
}

KArchiveDirectory * KArchive::findOrCreate( const QString & path )
{
    //kdDebug() << "KArchive::findOrCreate " << path << endl;
    if ( path == "" || path == "/" || path == "." ) // root dir => found
    {
        //kdDebug() << "KArchive::findOrCreate returning rootdir" << endl;
        return rootDir();
    }
    // Important note : for tar files containing absolute paths
    // (i.e. beginning with "/"), this means the leading "/" will
    // be removed (no KDirectory for it), which is exactly the way
    // the "tar" program works (though it displays a warning about it)
    // See also KArchiveDirectory::entry().

    // Already created ? => found
    KArchiveEntry* ent = rootDir()->entry( path );
    if ( ent && ent->isDirectory() )
    {
        //kdDebug() << "KArchive::findOrCreate found it" << endl;
        return (KArchiveDirectory *) ent;
    }

    // Otherwise go up and try again
    int pos = path.findRev( '/' );
    KArchiveDirectory * parent;
    QString dirname;
    if ( pos == -1 ) // no more slash => create in root dir
    {
        parent =  rootDir();
        dirname = path;
    }
    else
    {
        QString left = path.left( pos );
        dirname = path.mid( pos + 1 );
        parent = findOrCreate( left ); // recursive call... until we find an existing dir.
    }

    //kdDebug() << "KTar : found parent " << parent->name() << " adding " << dirname << " to ensure " << path << endl;
    // Found -> add the missing piece
    KArchiveDirectory * e = new KArchiveDirectory( this, dirname, d->rootDir->permissions(),
                                                   d->rootDir->date(), d->rootDir->user(),
                                                   d->rootDir->group(), QString::null );
    parent->addEntry( e );
    return e; // now a directory to <path> exists
}

void KArchive::setDevice( QIODevice * dev )
{
    m_dev = dev;
}

void KArchive::setRootDir( KArchiveDirectory *rootDir )
{
    Q_ASSERT( !d->rootDir ); // Call setRootDir only once during parsing please ;)
    d->rootDir = rootDir;
}

////////////////////////////////////////////////////////////////////////
/////////////////////// KArchiveEntry //////////////////////////////////
////////////////////////////////////////////////////////////////////////
KArchiveEntry::KArchiveEntry( KArchive* t, const QString& name, int access, int date,
                      const QString& user, const QString& group, const
                      QString& symlink)
{
  m_name = name;
  m_access = access;
  m_date = date;
  m_user = user;
  m_group = group;
  m_symlink = symlink;
  m_archive = t;

}

QDateTime KArchiveEntry::datetime() const
{
  QDateTime d;
  d.setTime_t( m_date );
  return d;
}

////////////////////////////////////////////////////////////////////////
/////////////////////// KArchiveFile ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchiveFile::KArchiveFile( KArchive* t, const QString& name, int access, int date,
                    const QString& user, const QString& group,
                    const QString & symlink,
                    int pos, int size )
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_pos = pos;
  m_size = size;
}

int KArchiveFile::position() const
{
  return m_pos;
}

int KArchiveFile::size() const
{
  return m_size;
}

QByteArray KArchiveFile::data() const
{
  archive()->device()->at( m_pos );

  // Read content
  QByteArray arr( m_size );
  if ( m_size )
  {
    assert( arr.data() );
    int n = archive()->device()->readBlock( arr.data(), m_size );
    if ( n != m_size )
      arr.resize( n );
  }
  return arr;
}

// ** This should be a virtual method, and this code should be in ktar.cpp
QIODevice *KArchiveFile::device() const
{
    return new KLimitedIODevice( archive()->device(), m_pos, m_size );
}

void KArchiveFile::copyTo(const QString& dest) const
{
  QFile f( dest + "/"  + name() );
  f.open( IO_ReadWrite | IO_Truncate );
  f.writeBlock( data() );
  f.close();
}

////////////////////////////////////////////////////////////////////////
//////////////////////// KArchiveDirectory /////////////////////////////////
////////////////////////////////////////////////////////////////////////


KArchiveDirectory::KArchiveDirectory( KArchive* t, const QString& name, int access,
                              int date,
                              const QString& user, const QString& group,
                              const QString &symlink)
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_entries.setAutoDelete( true );
}

QStringList KArchiveDirectory::entries() const
{
  QStringList l;

  QDictIterator<KArchiveEntry> it( m_entries );
  for( ; it.current(); ++it )
    l.append( it.currentKey() );

  return l;
}

KArchiveEntry* KArchiveDirectory::entry( QString name )
  // not "const QString & name" since we want a local copy
  // (to remove leading slash if any)
{
  int pos = name.find( '/' );
  if ( pos == 0 ) // ouch absolute path (see also KArchive::findOrCreate)
  {
    if (name.length()>1)
    {
      name = name.mid( 1 ); // remove leading slash
      pos = name.find( '/' ); // look again
    }
    else // "/"
      return this;
  }
  // trailing slash ? -> remove
  if ( pos != -1 && pos == (int)name.length()-1 )
  {
    name = name.left( pos );
    pos = name.find( '/' ); // look again
  }
  if ( pos != -1 )
  {
    QString left = name.left( pos );
    QString right = name.mid( pos + 1 );

    //kdDebug() << "KArchiveDirectory::entry left=" << left << " right=" << right << endl;

    KArchiveEntry* e = m_entries[ left ];
    if ( !e || !e->isDirectory() )
      return 0;
    return ((KArchiveDirectory*)e)->entry( right );
  }

  return m_entries[ name ];
}

const KArchiveEntry* KArchiveDirectory::entry( QString name ) const
{
  return ((KArchiveDirectory*)this)->entry( name );
}

void KArchiveDirectory::addEntry( KArchiveEntry* entry )
{
  Q_ASSERT( !entry->name().isEmpty() );
  m_entries.insert( entry->name(), entry );
}

void KArchiveDirectory::copyTo(const QString& dest, bool recursiveCopy ) const
{
  QDir root;

  PosSortedPtrList fileList;
  QMap<int, QString> fileToDir;

  QStringList::Iterator it;

  // placeholders for iterated items
  KArchiveDirectory* curDir;
  QString curDirName;

  QStringList dirEntries;
  KArchiveEntry* curEntry;
  KArchiveFile* curFile;


  QPtrStack<KArchiveDirectory> dirStack;
  QValueStack<QString> dirNameStack;

  dirStack.push( this );     // init stack at current directory
  dirNameStack.push( dest ); // ... with given path
  do {
    curDir = dirStack.pop();
    curDirName = dirNameStack.pop();
    root.mkdir(curDirName);

    dirEntries = curDir->entries();
    for ( it = dirEntries.begin(); it != dirEntries.end(); ++it ) {
      curEntry = curDir->entry(*it);
      if ( curEntry->isFile() ) {
        curFile = dynamic_cast<KArchiveFile*>( curEntry );
        fileList.append( curFile );
        fileToDir.insert( curFile->position(), curDirName );
      }

      if ( curEntry->isDirectory() )
        if ( recursiveCopy ) {
          dirStack.push( dynamic_cast<KArchiveDirectory*>( curEntry ) );
          dirNameStack.push( curDirName + "/" + curEntry->name() );
        }
    }
  } while (!dirStack.isEmpty());

  fileList.sort();  // sort on m_pos, so we have a linear access

  KArchiveFile* f;
  for ( f = fileList.first(); f; f = fileList.next() ) {
    int pos = f->position();
    f->copyTo( fileToDir[pos] );
  }
}

bool KArchive::writeData( const char* data, uint size )
{
    WriteDataParams params;
    params.data = data;
    params.size = size;
    virtual_hook( 1, &params );
    return params.retval;
}

void KArchive::virtual_hook( int id, void* data )
{
    if ( id == 1 ) {
        WriteDataParams* params = reinterpret_cast<WriteDataParams *>(data);
        params->retval = device()->writeBlock( params->data, params->size ) == (Q_LONG)params->size;
    } else {
        /*BASE::virtual_hook( id, data );*/
    }
}

void KArchiveEntry::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KArchiveFile::virtual_hook( int id, void* data )
{ KArchiveEntry::virtual_hook( id, data ); }

void KArchiveDirectory::virtual_hook( int id, void* data )
{ KArchiveEntry::virtual_hook( id, data ); }
