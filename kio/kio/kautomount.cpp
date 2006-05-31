/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kautomount.h"
#include "krun.h"
#include "kdirwatch.h"
#include "kio/job.h"
#include <kdirnotify.h>
#include <kdebug.h>

/***********************************************************************
 *
 * Utility classes
 *
 ***********************************************************************/

KAutoMount::KAutoMount( bool _readonly, const QByteArray& _format, const QString& _device,
                        const QString&  _mountpoint, const QString & _desktopFile,
                        bool _show_filemanager_window )
  : m_strDevice( _device ),
    m_desktopFile( _desktopFile )
{
  //kDebug(7015) << "KAutoMount device=" << _device << " mountpoint=" << _mountpoint << endl;
  m_bShowFilemanagerWindow = _show_filemanager_window;

  KIO::Job* job = KIO::mount( _readonly, _format, _device, _mountpoint );
  connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
}

void KAutoMount::slotResult( KJob * job )
{
  if ( job->error() ) {
    emit error();
    static_cast<KIO::Job*>( job )->showErrorDialog();
  }
  else
  {
    KUrl mountpoint;
    mountpoint.setPath( KIO::findDeviceMountPoint( m_strDevice ) );
    //kDebug(7015) << "KAutoMount: m_strDevice=" << m_strDevice << " -> mountpoint=" << mountpoint << endl;
    Q_ASSERT( mountpoint.isValid() );

    if ( mountpoint.path().isEmpty() )
        kWarning(7015) << m_strDevice << " was correctly mounted, but KIO::findDeviceMountPoint didn't find it. "
                        << "This looks like a bug, please report it on http://bugs.kde.org, together with your /etc/fstab line" << endl;
    else if ( m_bShowFilemanagerWindow )
      KRun::runUrl( mountpoint, "inode/directory", 0 /*TODO - window*/ );

    // Notify about the new stuff in that dir, in case of opened windows showing it
    org::kde::KDirNotify::emitFilesAdded( mountpoint.url() );

    // Update the desktop file which is used for mount/unmount (icon change)
    kDebug(7015) << " mount finished : updating " << m_desktopFile << endl;
    KUrl dfURL;
    dfURL.setPath( m_desktopFile );
    org::kde::KDirNotify::emitFilesChanged( QStringList() << dfURL.url() );
    //KDirWatch::self()->setFileDirty( m_desktopFile );

    emit finished();
  }
  delete this;
}

KAutoUnmount::KAutoUnmount( const QString & _mountpoint, const QString & _desktopFile )
  : m_desktopFile( _desktopFile ), m_mountpoint( _mountpoint )
{
  KIO::Job * job = KIO::unmount( m_mountpoint );
  connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
}

void KAutoUnmount::slotResult( KJob * job )
{
  if ( job->error() ) {
    emit error();
    static_cast<KIO::Job*>( job )->showErrorDialog();
  }
  else
  {
    // Update the desktop file which is used for mount/unmount (icon change)
    kDebug(7015) << "unmount finished : updating " << m_desktopFile << endl;
    KUrl dfURL;
    dfURL.setPath( m_desktopFile );
    org::kde::KDirNotify::emitFilesChanged( QStringList() << dfURL.url() );
    //KDirWatch::self()->setFileDirty( m_desktopFile );

    // Notify about the new stuff in that dir, in case of opened windows showing it
    // You may think we removed files, but this may have also readded some
    // (if the mountpoint wasn't empty). The only possible behavior on FilesAdded
    // is to relist the directory anyway.
    KUrl mp;
    mp.setPath( m_mountpoint );
    org::kde::KDirNotify::emitFilesAdded( mp.url() );

    emit finished();
  }

  delete this;
}

#include "kautomount.moc"
