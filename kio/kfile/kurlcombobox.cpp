/* This file is part of the KDE libraries
    Copyright (C) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kurlcombobox.h"

#include <QtCore/QDir>

#include <kdebug.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>
#include <kmimetype.h>

class KUrlComboBox::KUrlComboBoxPrivate
{
public:
    KUrlComboBoxPrivate()
        : dirIcon(QLatin1String("folder"))
    {}

    KIcon dirIcon;
    bool urlAdded;
    int myMaximum;
    Mode myMode; // can be used as parameter to KUR::path( int ) or url( int )
                 // to specify if we want a trailing slash or not
};


KUrlComboBox::KUrlComboBox( Mode mode, QWidget *parent)
    : KComboBox( parent),d(new KUrlComboBoxPrivate())
{
    init( mode );
}


KUrlComboBox::KUrlComboBox( Mode mode, bool rw, QWidget *parent)
    : KComboBox( rw, parent),d(new KUrlComboBoxPrivate())
{
    init( mode );
}


KUrlComboBox::~KUrlComboBox()
{
    qDeleteAll( itemList );
    qDeleteAll( defaultList );
    delete d;
}


void KUrlComboBox::init( Mode mode )
{
    d->myMode = mode;
    d->urlAdded = false;
    d->myMaximum = 10; // default
    setInsertPolicy( NoInsert );
    setTrapReturnKey( true );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));

    opendirIcon = KIcon(QLatin1String("folder-open"));

    connect( this, SIGNAL( activated( int )), SLOT( slotActivated( int )));
}


QStringList KUrlComboBox::urls() const
{
    kDebug(250) << "::urls()";
    //static const QString &fileProt = KGlobal::staticQString("file:");
    QStringList list;
    QString url;
    for ( int i = defaultList.count(); i < count(); i++ ) {
        url = itemText( i );
        if ( !url.isEmpty() ) {
            //if ( url.at(0) == '/' )
            //    list.append( url.prepend( fileProt ) );
            //else
                list.append( url );
        }
    }

    return list;
}


void KUrlComboBox::addDefaultUrl( const KUrl& url, const QString& text )
{
    addDefaultUrl( url, getIcon( url ), text );
}


void KUrlComboBox::addDefaultUrl( const KUrl& url, const QIcon& icon,
                                  const QString& text )
{
    KUrlComboItem *item = new KUrlComboItem;
    item->url = url;
    item->icon = icon;
    if ( text.isEmpty() ) {
        KUrl::AdjustPathOption mode = KUrl::LeaveTrailingSlash;
        if (d->myMode == Directories)
          mode = KUrl::AddTrailingSlash;
        else
          mode = KUrl::RemoveTrailingSlash;
        if ( url.isLocalFile() )
          item->text = url.path( mode );
        else
          item->text = url.prettyUrl( mode );
    }
    else
        item->text = text;

    defaultList.append( item );
}


void KUrlComboBox::setDefaults()
{
    clear();
    itemMapper.clear();

    const KUrlComboItem *item;
    for ( int id = 0; id < defaultList.count(); id++ ) {
        item = defaultList.at( id );
        insertUrlItem( item );
    }
}

void KUrlComboBox::setUrls( const QStringList &urls )
{
    setUrls( urls, RemoveBottom );
}

void KUrlComboBox::setUrls( const QStringList &_urls, OverLoadResolving remove )
{
    setDefaults();
    qDeleteAll( itemList );
    itemList.clear();
    d->urlAdded = false;

    if ( _urls.isEmpty() )
        return;

    QStringList urls;
    QStringList::ConstIterator it = _urls.constBegin();

    // kill duplicates
    while ( it != _urls.constEnd() ) {
        if ( !urls.contains( *it ) )
            urls += *it;
        ++it;
    }

    // limit to myMaximum items
    /* Note: overload is an (old) C++ keyword, some compilers (KCC) choke
       on that, so call it Overload (capital 'O').  (matz) */
    int Overload = urls.count() - d->myMaximum + defaultList.count();
    while ( Overload > 0 ) {
        if (remove == RemoveBottom)
            urls.removeLast();
        else
            urls.removeFirst();
        Overload--;
    }

    it = urls.begin();

    KUrlComboItem *item = 0L;

    while ( it != urls.end() ) {
        if ( (*it).isEmpty() ) {
            ++it;
            continue;
        }
        KUrl u = *it;

        // Don't restore if file doesn't exist anymore
        if (u.isLocalFile() && !QFile::exists(u.path())) {
            ++it;
            continue;
        }

        item = new KUrlComboItem;
        item->url = u;
        item->icon = getIcon( u );

        if ( u.isLocalFile() )
        {
          KUrl::AdjustPathOption mode = KUrl::LeaveTrailingSlash;
          if (d->myMode == Directories)
              mode = KUrl::AddTrailingSlash;
          else
              mode = KUrl::RemoveTrailingSlash;
          item->text = u.path( mode ); // don't show file:/
        }
        else
            item->text = *it;

        insertUrlItem( item );
        itemList.append( item );
        ++it;
    }
}


void KUrlComboBox::setUrl( const KUrl& url )
{
    if ( url.isEmpty() )
        return;

    bool blocked = blockSignals( true );

    // check for duplicates
    QMap<int,const KUrlComboItem*>::ConstIterator mit = itemMapper.begin();
    QString urlToInsert = url.url(KUrl::RemoveTrailingSlash);
    while ( mit != itemMapper.end() ) {
      if ( urlToInsert == mit.value()->url.url(KUrl::RemoveTrailingSlash) ) {
            setCurrentIndex( mit.key() );

            if (d->myMode == Directories)
                updateItem( mit.value(), mit.key(), opendirIcon );

            blockSignals( blocked );
            return;
        }
        ++mit;
    }

    // not in the combo yet -> create a new item and insert it

    // first remove the old item
    if (d->urlAdded) {
        Q_ASSERT(!itemList.isEmpty());
        itemList.removeLast();
        d->urlAdded = false;
    }

    setDefaults();

    QListIterator<const KUrlComboItem*> it( itemList );
    while ( it.hasNext() )
        insertUrlItem( it.next() );

    KUrl::AdjustPathOption mode = KUrl::LeaveTrailingSlash;
    if (d->myMode == Directories)
      mode = KUrl::AddTrailingSlash;
    else
      mode = KUrl::RemoveTrailingSlash;
    KUrlComboItem *item = new KUrlComboItem;
    item->url = url;
    item->icon = getIcon( url );
    if ( url.isLocalFile() )
      item->text = url.path( mode );
    else
      item->text = url.prettyUrl( mode );
     kDebug(250) << "setURL: text=" << item->text;

    int id = count();
    QString text = /*isEditable() ? item->url.prettyUrl( (KUrl::AdjustPathOption)myMode ) : */ item->text;

    if (d->myMode == Directories)
        KComboBox::insertItem( id,opendirIcon, text);
    else
        KComboBox::insertItem( id,item->icon, text);
    itemMapper.insert( id, item );
    itemList.append( item );

    setCurrentIndex( id );
    Q_ASSERT(!itemList.isEmpty());
    d->urlAdded = true;
    blockSignals( blocked );
}


void KUrlComboBox::slotActivated( int index )
{
    const KUrlComboItem *item = itemMapper[ index ];

    if ( item ) {
        setUrl( item->url );
        emit urlActivated( item->url );
    }
}


void KUrlComboBox::insertUrlItem( const KUrlComboItem *item )
{
// kDebug(250) << "insertURLItem " << item->text;
    int id = count();
    KComboBox::insertItem(id, item->icon, item->text);
    itemMapper.insert( id, item );
}


void KUrlComboBox::setMaxItems( int max )
{
    d->myMaximum = max;

    if (count() > d->myMaximum) {
        int oldCurrent = currentIndex();

        setDefaults();

        QListIterator<const KUrlComboItem*> it( itemList );
        int Overload = itemList.count() - d->myMaximum + defaultList.count();
        for ( int i = 0; i <= Overload; i++ )
            it.next();

        while ( it.hasNext() )
            insertUrlItem( it.next() );

        if ( count() > 0 ) { // restore the previous currentItem
            if ( oldCurrent >= count() )
                oldCurrent = count() -1;
            setCurrentIndex( oldCurrent );
        }
    }
}

int KUrlComboBox::maxItems() const
{
    return d->myMaximum;
}

void KUrlComboBox::removeUrl( const KUrl& url, bool checkDefaultURLs )
{
    QMap<int,const KUrlComboItem*>::ConstIterator mit = itemMapper.begin();
    while ( mit != itemMapper.end() ) {
      if ( url.url(KUrl::RemoveTrailingSlash) == mit.value()->url.url(KUrl::RemoveTrailingSlash) ) {
            if ( !itemList.removeAll( mit.value() ) && checkDefaultURLs )
                defaultList.removeAll( mit.value() );
        }
        ++mit;
    }

    bool blocked = blockSignals( true );
    setDefaults();
    QListIterator<const KUrlComboItem*> it( itemList );
    while ( it.hasNext() ) {
        insertUrlItem( it.next() );
    }
    blockSignals( blocked );
}


QIcon KUrlComboBox::getIcon( const KUrl& url ) const
{
    if (d->myMode == Directories)
        return d->dirIcon;
    else
        return KIcon(KMimeType::iconNameForUrl(url, 0));
}


// updates "item" with icon "icon" and sets the URL instead of text
void KUrlComboBox::updateItem( const KUrlComboItem *item,
                               int index, const QIcon& icon)
{
    setItemIcon(index,icon);

    if ( isEditable() ) {
        KUrl::AdjustPathOption mode = KUrl::LeaveTrailingSlash;
        if (d->myMode == Directories)
            mode = KUrl::AddTrailingSlash;
        else
            mode = KUrl::RemoveTrailingSlash;

        setItemText( index, item->url.isLocalFile() ? item->url.path( mode ) :
                                                      item->url.prettyUrl( mode ));
    }
    else {
        setItemText(index,item->text);
    }
}


#include "kurlcombobox.moc"
