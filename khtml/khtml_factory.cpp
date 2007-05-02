/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "khtml_factory.h"
#include "khtml_part.h"
#include "khtml_settings.h"

#include "css/cssstyleselector.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include "rendering/break_lines.h"
#include "misc/loader.h"
#include "misc/arena.h"
#include "misc/paintbuffer.h"

#include <QtCore/QLinkedList>

#include <kcomponentdata.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <assert.h>

#include <kdebug.h>

KHTMLFactory *KHTMLFactory::s_self = 0;
unsigned long int KHTMLFactory::s_refcnt = 0;
KComponentData *KHTMLFactory::s_componentData = 0;
KIconLoader *KHTMLFactory::s_iconLoader = 0;
KAboutData *KHTMLFactory::s_about = 0;
KHTMLSettings *KHTMLFactory::s_settings = 0;
QLinkedList<KHTMLPart*> *KHTMLFactory::s_parts = 0;
QString *KHTMLSettings::avFamilies = 0;

KHTMLFactory::KHTMLFactory( bool clone )
{
    if ( clone )
        ref();
}

KHTMLFactory::~KHTMLFactory()
{
    if ( s_self == this )
    {
        assert( !s_refcnt );

        delete s_iconLoader;
        delete s_componentData;
        delete s_about;
        delete s_settings;
	delete KHTMLSettings::avFamilies;
        if ( s_parts )
        {
            assert( s_parts->isEmpty() );
            delete s_parts;
        }

        s_iconLoader = 0;
        s_componentData = 0;
        s_about = 0;
        s_settings = 0;
        s_parts = 0;
	KHTMLSettings::avFamilies = 0;

        // clean up static data
        khtml::CSSStyleSelector::clear();
        khtml::RenderStyle::cleanup();
        khtml::PaintBuffer::cleanup();
        khtml::Cache::clear();
        khtml::cleanup_thaibreaks();
        khtml::ArenaFinish();
    }
    else
        deref();
}

KParts::Part *KHTMLFactory::createPartObject( QWidget *parentWidget, QObject *parent, const char *className, const QStringList & )
{
  KHTMLPart::GUIProfile prof = KHTMLPart::DefaultGUI;
  if ( strcmp( className, "Browser/View" ) == 0 )
    prof = KHTMLPart::BrowserViewGUI;

  return new KHTMLPart( parentWidget, parent, prof );
}

void KHTMLFactory::ref()
{
    if ( !s_refcnt && !s_self )
    {
        // we can't use a staticdeleter here, because that would mean
        // that the factory gets deleted from within a qPostRoutine, called
        // from the QApplication destructor. That however is too late, because
        // we want to destruct a KComponentData object, which involves destructing
        // a KConfig object, which might call KGlobal::dirs() (in sync()) which
        // probably is not going to work ;-)
        // well, perhaps I'm wrong here, but as I'm unsure I try to stay on the
        // safe side ;-) -> let's use a simple reference counting scheme
        // (Simon)
        s_self = new KHTMLFactory;
        khtml::Cache::init();
    }

    s_refcnt++;
}

void KHTMLFactory::deref()
{
    if ( !--s_refcnt && s_self )
    {
        delete s_self;
        s_self = 0;
    }
}

void KHTMLFactory::registerPart( KHTMLPart *part )
{
    if ( !s_parts )
        s_parts = new QLinkedList<KHTMLPart*>;

    if ( !s_parts->contains( part ) )
    {
        s_parts->append( part );
        ref();
    }
}

void KHTMLFactory::deregisterPart( KHTMLPart *part )
{
    assert( s_parts );

    if ( s_parts->removeAll( part ) )
    {
        if ( s_parts->isEmpty() )
        {
            delete s_parts;
            s_parts = 0;
        }
        deref();
    }
}

const KComponentData &KHTMLFactory::componentData()
{
  assert( s_self );

  if ( !s_componentData )
  {
    s_about = new KAboutData( "khtml", I18N_NOOP( "KHTML" ), "4.0",
                              I18N_NOOP( "Embeddable HTML component" ),
                              KAboutData::License_LGPL );
    s_about->addAuthor( "Lars Knoll", 0, "knoll@kde.org" );
    s_about->addAuthor( "Antti Koivisto", 0, "koivisto@kde.org" );
    s_about->addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );
    s_about->addAuthor( "Dirk Mueller", 0, "mueller@kde.org" );
    s_about->addAuthor( "Peter Kelly", 0, "pmk@kde.org" );
    s_about->addAuthor( "Torben Weis", 0, "weis@kde.org" );
    s_about->addAuthor( "Martin Jones", 0, "mjones@kde.org" );
    s_about->addAuthor( "Simon Hausmann", 0, "hausmann@kde.org" );
    s_about->addAuthor( "Tobias Anton", 0, "anton@stud.fbi.fh-darmstadt.de" );

    s_componentData = new KComponentData( s_about );
  }

  return *s_componentData;
}

KIconLoader *KHTMLFactory::iconLoader()
{
  if ( !s_iconLoader )
  {
    s_iconLoader = new KIconLoader(componentData().componentName(), componentData().dirs());
  }

  return s_iconLoader;
}

KHTMLSettings *KHTMLFactory::defaultHTMLSettings()
{
  assert( s_self );
  if ( !s_settings )
    s_settings = new KHTMLSettings();

  return s_settings;
}

using namespace KParts;
#include "khtml_factory.moc"

