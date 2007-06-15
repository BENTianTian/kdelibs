/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2001 Mirko Boehm (mirko@kde.org) and
 *  Espen Sand (espen@kde.org)
 *  Holger Freyther <freyther@kde.org>
 *  2005-2006 Olivier Goffart <ogoffart at kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "kpagedialog.h"

#include <QTimer>
#include <QLayout>

class KPageDialog::Private
{
  public:
    Private( KPageDialog *parent )
      : mParent( parent ), mPageWidget( 0 )
    {
    }

    KPageDialog *mParent;
    KPageWidget *mPageWidget;

    void init()
    {
      connect( mPageWidget, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ),
               mParent, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ) );

      mParent->setMainWidget( mPageWidget );
      mPageWidget->layout()->setMargin( 0 );
    }
};

KPageDialog::KPageDialog( QWidget *parent, Qt::WFlags flags )
  : KDialog( parent, flags ),
    d( new Private( this ) )
{
  d->mPageWidget = new KPageWidget( this );

  d->init();
}

KPageDialog::KPageDialog( KPageWidget *widget, QWidget *parent, Qt::WFlags flags )
  : KDialog( parent, flags ),
    d( new Private( this ) )
{
  d->mPageWidget = widget;

  d->init();
}

KPageDialog::~KPageDialog()
{
  delete d;
}

void KPageDialog::setFaceType( FaceType faceType )
{
  d->mPageWidget->setFaceType( (KPageWidget::FaceType)faceType );
}

KPageWidgetItem* KPageDialog::addPage( QWidget *widget, const QString &name )
{
  return d->mPageWidget->addPage( widget, name );
}

void KPageDialog::addPage( KPageWidgetItem *item )
{
  d->mPageWidget->addPage( item );
}

KPageWidgetItem* KPageDialog::insertPage( KPageWidgetItem *before, QWidget *widget, const QString &name )
{
  return d->mPageWidget->insertPage( before, widget, name );
}

void KPageDialog::insertPage( KPageWidgetItem *before, KPageWidgetItem *item )
{
  d->mPageWidget->insertPage( before, item );
}

KPageWidgetItem* KPageDialog::addSubPage( KPageWidgetItem *parent, QWidget *widget, const QString &name )
{
  return d->mPageWidget->addSubPage( parent, widget, name );
}

void KPageDialog::addSubPage( KPageWidgetItem *parent, KPageWidgetItem *item )
{
  d->mPageWidget->addSubPage( parent, item );
}

void KPageDialog::removePage( KPageWidgetItem *item )
{
  d->mPageWidget->removePage( item );
}

void KPageDialog::setCurrentPage( KPageWidgetItem *item )
{
  d->mPageWidget->setCurrentPage( item );
}

KPageWidgetItem* KPageDialog::currentPage() const
{
  return d->mPageWidget->currentPage();
}

KPageWidget* KPageDialog::pageWidget()
{
  return d->mPageWidget;
}

const KPageWidget* KPageDialog::pageWidget() const
{
  return d->mPageWidget;
}

#include "kpagedialog.moc"
