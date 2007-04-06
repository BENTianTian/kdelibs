/*
 * This file is part of the KDE project
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtCore/QTimer>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kconfiggroup.h>

#include "kimagefilepreview.h"
#include "config-kfile.h"

/**** KImageFilePreview ****/

KImageFilePreview::KImageFilePreview( QWidget *parent )
    : KPreviewWidgetBase( parent ),
      m_job( 0L )
{
    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    autoMode = cg.readEntry( "Automatic Preview", true );

    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setMargin( 0 );
    vb->setSpacing( KDialog::spacingHint() );

    imageLabel = new QLabel( this );
    imageLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    imageLabel->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    vb->addWidget( imageLabel );

    QHBoxLayout *hb = new QHBoxLayout();
    hb->setSpacing( 0 );
    vb->addLayout( hb );

    autoPreview = new QCheckBox( i18n("&Automatic preview"), this );
    autoPreview->setChecked( autoMode );
    hb->addWidget( autoPreview );
    connect( autoPreview, SIGNAL(toggled(bool)), SLOT(toggleAuto(bool)) );

    previewButton = new KPushButton( KIcon("thumbnail-show"), i18n("&Preview"), this );
    hb->addWidget( previewButton );
    connect( previewButton, SIGNAL(clicked()), SLOT(showPreview()) );

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(showPreview()) );

    setSupportedMimeTypes( KIO::PreviewJob::supportedMimeTypes() );
}

KImageFilePreview::~KImageFilePreview()
{
    if ( m_job )
        m_job->kill();

    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    cg.writeEntry( "Automatic Preview", autoPreview->isChecked() );
}

void KImageFilePreview::showPreview()
{
    // Pass a copy since clearPreview() will clear currentURL
    KUrl url = currentURL;
    showPreview( url, true );
}

// called via KPreviewWidgetBase interface
void KImageFilePreview::showPreview( const KUrl& url )
{
    showPreview( url, false );
}

void KImageFilePreview::showPreview( const KUrl &url, bool force )
{
    if ( !url.isValid() ) {
        clearPreview();
        return;
    }

    if ( url != currentURL || force )
    {
        clearPreview();
	currentURL = url;

	if ( autoMode || force )
	{
            int w = imageLabel->contentsRect().width() - 4;
            int h = imageLabel->contentsRect().height() - 4;

            m_job =  createJob( url, w, h );
            if ( force ) // explicitly requested previews shall always be generated!
                m_job->setIgnoreMaximumSize( true );

            connect( m_job, SIGNAL( result( KJob * )),
                     this, SLOT( slotResult( KJob * )));
            connect( m_job, SIGNAL( gotPreview( const KFileItem*,
                                                const QPixmap& )),
                     SLOT( gotPreview( const KFileItem*, const QPixmap& ) ));

            connect( m_job, SIGNAL( failed( const KFileItem* )),
                     this, SLOT( slotFailed( const KFileItem* ) ));
	}
    }
}

void KImageFilePreview::toggleAuto( bool a )
{
    autoMode = a;
    if ( autoMode )
    {
        // Pass a copy since clearPreview() will clear currentURL
        KUrl url = currentURL;
        showPreview( url, true );
    }
}

void KImageFilePreview::resizeEvent( QResizeEvent * )
{
    timer->setSingleShot( true );
    timer->start( 100 ); // forces a new preview
}

QSize KImageFilePreview::sizeHint() const
{
    return QSize( 20, 200 ); // otherwise it ends up huge???
}

KIO::PreviewJob * KImageFilePreview::createJob( const KUrl& url, int w, int h )
{
    KUrl::List urls;
    urls.append( url );
    return KIO::filePreview( urls, w, h, 0, 0, true, false );
}

void KImageFilePreview::gotPreview( const KFileItem* item, const QPixmap& pm )
{
    if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( pm );
}

void KImageFilePreview::slotFailed( const KFileItem* item )
{
    if ( item->isDir() )
        imageLabel->clear();
    else if ( item->url() == currentURL ) // should always be the case
        imageLabel->setPixmap( SmallIcon( "file-broken", K3Icon::SizeLarge,
                                          K3Icon::DisabledState ));
}

void KImageFilePreview::slotResult( KJob *job )
{
    if ( job == m_job )
        m_job = 0L;
}

void KImageFilePreview::clearPreview()
{
    if ( m_job ) {
        m_job->kill();
        m_job = 0L;
    }

    imageLabel->clear();
    currentURL = KUrl();
}

#include "kimagefilepreview.moc"
