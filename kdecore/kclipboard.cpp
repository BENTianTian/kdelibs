/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qclipboard.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

#include "kclipboard.h"



class KClipboard::MimeSource : public QMimeSource
{
public:
    MimeSource( const QMimeSource * src );
    ~MimeSource();

    virtual const char *format( int i ) const {
        if ( i < (int) m_formats.count() )
            return m_formats.at( i );
        else
            return 0L;
    }
    virtual bool provides( const char *mimeType ) const {
        return ( m_formats.find( mimeType ) > -1 );
    }
    virtual QByteArray encodedData( const char *format ) const;

private:
    mutable QStrList m_formats;
    QPtrList<QByteArray> m_data;
};






KClipboard * KClipboard::s_self = 0L;
bool KClipboard::s_sync = false;
bool KClipboard::s_implicitSelection = true;
bool KClipboard::s_blocked = false;

KClipboard * KClipboard::self()
{
    if ( !s_self )
        s_self = new KClipboard( kapp, "KDE Clipboard" );

    return s_self;
}

KClipboard::KClipboard( QObject *parent, const char *name )
    : QObject( parent, name )
{
    s_self = this;

    KConfigGroup config( KGlobal::config(), "General" );
    s_sync = config.readBoolEntry( "SynchronizeClipboardAndSelection", s_sync);
    s_implicitSelection = config.readBoolEntry( "ImplicitlySetSelection",
                                                s_implicitSelection );

    QClipboard *clip = QApplication::clipboard();
    connect( clip, SIGNAL( selectionChanged() ),
             SLOT( slotSelectionChanged() ));
    connect( clip, SIGNAL( dataChanged() ),
             SLOT( slotClipboardChanged() ));
}

KClipboard::~KClipboard()
{
    if ( s_self == this )
        s_self = 0L;
}

void KClipboard::slotSelectionChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** sel changed: %i", s_blocked);
    if ( s_blocked || !clip->ownsSelection() )
        return;

    if ( s_sync )
    {
        bool oldMode = clip->selectionModeEnabled();
        clip->setSelectionMode( true );

        setClipboard( new MimeSource( clip->data() ), Clipboard );

        clip->setSelectionMode( oldMode );
    }
}

void KClipboard::slotClipboardChanged()
{
    QClipboard *clip = QApplication::clipboard();

//     qDebug("*** clip changed : %i (implicit: %i, ownz: clip: %i, selection: %i)", s_blocked, s_implicitSelection, clip->ownsClipboard(), clip->ownsSelection());
    if ( s_blocked || !clip->ownsClipboard() )
        return;

    if ( s_implicitSelection || s_sync )
    {
        bool oldMode = clip->selectionModeEnabled();
        clip->setSelectionMode( true );

        setClipboard( new MimeSource( clip->data() ), Selection );

        clip->setSelectionMode( oldMode );
    }
}

void KClipboard::setClipboard( QMimeSource *data, Mode mode )
{
//     qDebug("---> setting clipboard: %p", data);

    QClipboard *clip = QApplication::clipboard();

    s_blocked = true;

    if ( mode == Clipboard )
    {
        clip->setSelectionMode( false );
        clip->setData( data );
    }
    else if ( mode == Selection )
    {
        clip->setSelectionMode( true );
        clip->setData( data );
    }

    s_blocked = false;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KClipboard::MimeSource::MimeSource( const QMimeSource * source )
    : QMimeSource(),
      m_formats( true ) // deep copies!
{
    m_formats.setAutoDelete( true );
    m_data.setAutoDelete( true );
    
    if ( source )
    {
        QByteArray *byteArray;
        const char *format;
        int i = 0;
        while ( (format = source->format( i++ )) )
        {
            byteArray = new QByteArray();
            *byteArray = source->encodedData( format ).copy();
            m_data.append( byteArray );
            m_formats.append( format );
        }
    }
}

KClipboard::MimeSource::~MimeSource()
{
}

QByteArray KClipboard::MimeSource::encodedData( const char *format ) const
{
    int index = m_formats.find( format );
    if ( index > -1 )
    {
        // grmbl, gcc (2.95.3 at least) doesn't let me call m_data.at(), due
        // to it being non-const. Even if mutable.
        QPtrList<QByteArray> *list = 
            const_cast<QPtrList<QByteArray> *>( &m_data );
        return *(list->at( index ));
    }

    return QByteArray();
}

#include "kclipboard.moc"
