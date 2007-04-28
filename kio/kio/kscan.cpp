/* This file is part of the KDE libraries
    Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include "kscan.h"

#include <QtCore/QFile>

#include <klocale.h>
#include <kservicetypetrader.h>

// static factory method
KScanDialog * KScanDialog::getScanDialog( QWidget *parent )
{
    return KServiceTypeTrader::createInstanceFromQuery<KScanDialog>( "KScan/KScanDialog", QString(), parent );
}


KScanDialog::KScanDialog( int dialogFace, int buttonMask,
			  QWidget *parent )
    : KPageDialog( parent ),
      m_currentId( 1 )
{
  setFaceType( (KPageDialog::FaceType)dialogFace );
  setCaption( i18n("Acquire Image") );
  setButtons( (KDialog::ButtonCodes)buttonMask );
  setDefaultButton( Close );
}

KScanDialog::~KScanDialog()
{
}

bool KScanDialog::setup()
{
    return true;
}

///////////////////////////////////////////////////////////////////


// static factory method
KOCRDialog * KOCRDialog::getOCRDialog( QWidget *parent )
{
    return KServiceTypeTrader::createInstanceFromQuery<KOCRDialog>( "KScan/KOCRDialog", QString(), parent );
}


KOCRDialog::KOCRDialog( int dialogFace, int buttonMask,
			  QWidget *parent, bool modal )
    : KPageDialog( parent ),
      m_currentId( 1 )
{
  setFaceType( (KPageDialog::FaceType)dialogFace );
  setCaption( i18n("OCR Image") );
  setButtons( (KDialog::ButtonCodes)buttonMask );
  setDefaultButton( Close );
  setModal( modal );
  showButtonSeparator( true );
}

KOCRDialog::~KOCRDialog()
{
}


///////////////////////////////////////////////////////////////////


KScanDialogFactory::KScanDialogFactory( QObject *parent )
    : KLibFactory(parent)
{
}

KScanDialogFactory::~KScanDialogFactory()
{
}

QObject *KScanDialogFactory::createObject( QObject *parent,
                                           const char *classname,
                                           const QStringList &args )
{
    if ( strcmp( classname, "KScanDialog" ) != 0 )
        return 0;

    if ( parent && !parent->isWidgetType() )
       return 0;

    Q_UNUSED( args );

    return createDialog( static_cast<QWidget *>( parent ) );
}


///////////////////////////////////////////////////////////////////


KOCRDialogFactory::KOCRDialogFactory( QObject *parent )
    : KLibFactory(parent)
{
}

KOCRDialogFactory::~KOCRDialogFactory()
{
}

QObject *KOCRDialogFactory::createObject( QObject *parent,
                                          const char *classname,
                                          const QStringList &args )
{
    if ( strcmp( classname, "KOCRDialog" ) != 0 )
        return 0;

    if ( parent && !parent->isWidgetType() )
       return 0;

    Q_UNUSED( args );

    return createDialog( static_cast<QWidget *>( parent ) );
}



#include "kscan.moc"
