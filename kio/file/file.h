#ifndef __file_h__
#define __file_h__ "$Id$"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <unistd.h>

#include <qintdict.h>
#include <qstring.h>
#include <qvaluelist.h>

#include "../kio_interface.h"
#include "../kio_base.h"

class FileProtocol : public KIOProtocol
{
public:
  FileProtocol( KIOConnection *_conn );
//   virtual ~FileProtocol() { }

  virtual void slotGet( const char *_url );
  virtual void slotGetSize( const char *_url );

  virtual void slotPut( const char *_url, int _mode,
			bool _overwrite, bool _resume, int _size );

  virtual void slotMkdir( const char *_url, int _mode );

  virtual void slotCopy( const char *_source, const char *_dest );
  virtual void slotCopy( QStringList& _source, const char *_dest );

  virtual void slotMove( const char *_source, const char *_dest );
  virtual void slotMove( QStringList& _source, const char *_dest );

  virtual void slotDel( QStringList& _source );

  virtual void slotListDir( const char *_url );
  virtual void slotTestDir( const char *_url );

  virtual void slotUnmount( const char *_point );
  virtual void slotMount( bool _ro, const char *_fstype, const char* _dev, const char *_point );

  virtual void slotData( void *_p, int _len );
  virtual void slotDataEnd();

  KIOConnection* connection() { return KIOConnectionSignals::m_pConnection; }

  void jobError( int _errid, const char *_txt );

protected:
  struct Copy
  {
    QString m_strAbsSource;
    QString m_strRelDest;
    mode_t m_mode;
    off_t m_size;
    time_t m_mtime;
  };

  struct CopyDir
  {
    QString m_strAbsSource;
    QString m_strRelDest;
    mode_t m_mode;
    ino_t m_ino;
    time_t m_mtime;
  };

  struct Del
  {
    QString m_strAbsSource;
    QString m_strRelDest;
    mode_t m_mode;
    off_t m_size;
  };

  void doCopy( QStringList& _source, const char *_dest, bool _rename, bool _move = false );

  /**
   * Main entry point for listing recursively
   */
  long listRecursive( const char *_path, QValueList<Copy>& _files,
		      QValueList<CopyDir>& _dirs, bool _rename );
  /**
   * Internally called by listRecursive
   */
  long listRecursive2( const char *_abs_path, const char *_rel_path,
		      QValueList<Copy>& _files, QValueList<CopyDir>& _dirs );

  int m_cmd;
  bool m_bIgnoreJobErrors;

  FILE* m_fPut;
  QIntDict<QString> usercache;      // maps long ==> QString *
  QIntDict<QString> groupcache;

};

class FileIOJob : public KIOJobBase
{
public:
  FileIOJob( KIOConnection *_conn, FileProtocol *_File );

  virtual void slotError( int _errid, const char *_txt );

protected:
  FileProtocol* m_pFile;
};

#endif
