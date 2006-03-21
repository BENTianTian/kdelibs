/* This file is part of the KDEproject
   Copyright (C) 2000 David Faure <faure@kde.org>
   2000 Carsten Pfeiffer <pfeiffer@kde.org>
   2002 Klaas Freitag <freitag@suse.de>

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

#include <qfile.h>

#include <kfileitem.h>
#include <kdebug.h>
#include <kde_file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kfiletreeviewitem.h"
#include "kfiletreebranch.h"


/* --- KFileTreeViewToplevelItem --- */
KFileTreeBranch::KFileTreeBranch( KFileTreeView *parent, const KUrl& url,
                                  const QString& name,
				  const QPixmap& pix, bool showHidden,
				  KFileTreeViewItem *branchRoot )

    : KDirLister( false ),
      m_root( branchRoot ),
      m_startURL( url ),
      m_name ( name ),
      m_rootIcon( pix ),
      m_openRootIcon( pix ),
      m_recurseChildren(true),
      m_showExtensions(true)
{
    kDebug( 250) << "Creating branch for url " << url.prettyURL() << endl;

    /* if non exists, create one */
    if( ! branchRoot )
    {
        m_root =  new KFileTreeViewItem( parent,
                                         new KFileItem( url, "inode/directory",
                                                        S_IFDIR  ),
                                         this );
    }

    m_root->setExpandable( true );
    m_root->setPixmap( 0, pix );
    m_root->setText( 0, name );

    setShowingDotFiles( showHidden );

    connect( this, SIGNAL( refreshItems(const KFileItemList&)),
             this, SLOT  ( slotRefreshItems( const KFileItemList& )));

    connect( this, SIGNAL( newItems(const KFileItemList&)),
             this, SLOT  ( addItems( const KFileItemList& )));

    connect( this, SIGNAL( completed(const KUrl& )),
             this,   SLOT(slCompleted(const KUrl&)));

    connect( this, SIGNAL( started( const KUrl& )),
             this,   SLOT( slotListerStarted( const KUrl& )));

    connect( this, SIGNAL( deleteItem( KFileItem* )),
             this,   SLOT( slotDeleteItem( KFileItem* )));

    connect( this, SIGNAL( canceled(const KUrl&) ),
             this,   SLOT( slotCanceled(const KUrl&) ));

    connect( this, SIGNAL( clear()),
             this, SLOT( slotDirlisterClear()));

    connect( this, SIGNAL( clear(const KUrl&)),
             this, SLOT( slotDirlisterClearURL(const KUrl&)));

    connect( this, SIGNAL( redirection( const KUrl& , const KUrl& ) ),
             this, SLOT( slotRedirect( const KUrl&, const KUrl& )));

    m_openChildrenURLs.append( url );
}

void KFileTreeBranch::setOpenPixmap( const QPixmap& pix )
{
    m_openRootIcon = pix;

    if( root()->isOpen())
    {
        root()->setPixmap( 0, pix );
    }
}

void KFileTreeBranch::slotListerStarted( const KUrl &url )
{
    /* set the parent correct if it is zero. */
    kDebug( 250) << "Starting to list " << url.prettyURL() << endl;
}


KFileTreeViewItem *KFileTreeBranch::parentKFTVItem( KFileItem *item )
{
    KFileTreeViewItem *parent = 0;

    if( ! item ) return 0;

    /* If it is a directory, check, if it exists in the dict. If not, go one up
     * and check again.
     */
    KUrl url = item->url();
    // kDebug(250) << "Item's url is " << url.prettyURL() << endl;
    KUrl dirUrl( url );
    dirUrl.setFileName( QString() );
    // kDebug(250) << "Directory url is " << dirUrl.prettyURL() << endl;

    parent  = findTVIByURL( dirUrl );
    // kDebug(250) << "Returning as parent item <" << parent <<  ">" << endl;
    return( parent );
}


void KFileTreeBranch::slotRefreshItems( const KFileItemList& list )
{
    kDebug(250) << "Refreshing " << list.count() << " items !" << endl;

    KFileItemList::const_iterator kit = list.begin();
    const KFileItemList::const_iterator kend = list.end();
    for ( ; kit != kend; ++kit )
    {
        KFileTreeViewItem *item = findTVIByURL((*kit)->url());
        if (item) {
            item->setPixmap(0, item->fileItem()->pixmap( K3Icon::SizeSmall ));
            item->setText( 0, item->fileItem()->text());
        }
    }
}

void KFileTreeBranch::addItems( const KFileItemList& list )
{
    kDebug(250) << "Adding " << list.count() << " items !" << endl;
    KFileTreeViewItemList treeViewItList;
    KFileTreeViewItem *parentItem = 0;

    KFileItemList::const_iterator kit = list.begin();
    const KFileItemList::const_iterator kend = list.end();
    for ( ; kit != kend; ++kit )
    {
        KFileItem* currItem = *kit;
        parentItem = parentKFTVItem( currItem );

        /* Only create a new KFileTreeViewItem if it does not yet exist */
        KFileTreeViewItem *newKFTVI =
            static_cast<KFileTreeViewItem *>(currItem->extraData( this ));

        if( ! newKFTVI )
        {
            newKFTVI = createTreeViewItem( parentItem, currItem );
            if (!newKFTVI)
            {
                // TODO: Don't fail if parentItem == 0
                continue;
            }
            currItem->setExtraData( this, newKFTVI );

            /* Cut off the file extension in case it is not a directory */
            if( !m_showExtensions && !currItem->isDir() )	/* Need to cut the extension */
            {
                QString name = currItem->text();
                int mPoint = name.lastIndexOf( '.' );
                if( mPoint > 0 )
                    name = name.left( mPoint );
                newKFTVI->setText( 0, name );
            }
        }

        /* Now try to find out if there are children for dirs in the treeview */
        /* This stats a directory on the local file system and checks the */
        /* hardlink entry in the stat-buf. This works only for local directories. */
        if( dirOnlyMode() && !m_recurseChildren && currItem->isLocalFile( ) && currItem->isDir() )
        {
            KUrl url = currItem->url();
            QString filename = url.directory( false, true ) + url.fileName();
            /* do the stat trick of Carsten. The problem is, that the hardlink
             *  count only contains directory links. Thus, this method only seem
             * to work in dir-only mode */
            kDebug(250) << "Doing stat on " << filename << endl;
            KDE_struct_stat statBuf;
            if( KDE_stat( QFile::encodeName( filename ), &statBuf ) == 0 )
            {
                int hardLinks = statBuf.st_nlink;  /* Count of dirs */
                kDebug(250) << "stat succeeded, hardlinks: " << hardLinks << endl;
                // If the link count is > 2, the directory likely has subdirs. If it's < 2
                // it's something weird like a mounted SMB share. In that case we don't know
                // if there are subdirs, thus show it as expandable.

                if( hardLinks != 2 )
                {
                    newKFTVI->setExpandable(true);
                }
                else
                {
                    newKFTVI->setExpandable(false);
                }
                if( hardLinks >= 2 ) // "Normal" directory with subdirs
                {
                    kDebug(250) << "Emitting for " << url.prettyURL() << endl;
                    emit( directoryChildCount( newKFTVI, hardLinks-2)); // parentItem, hardLinks-1 ));
                }
            }
            else
            {
                kDebug(250) << "stat of " << filename << " failed !" << endl;
            }
        }
        treeViewItList.append( newKFTVI );
    }

    emit newTreeViewItems( this, treeViewItList );
}

KFileTreeViewItem* KFileTreeBranch::createTreeViewItem( KFileTreeViewItem *parent,
							KFileItem *fileItem )
{
    KFileTreeViewItem  *tvi = 0;
    if( parent && fileItem )
    {
        tvi = new KFileTreeViewItem( parent,
                                     fileItem,
                                     this );
    }
    else
    {
        kDebug(250) << "createTreeViewItem: Have no parent" << endl;
    }
    return( tvi );
}

void KFileTreeBranch::setChildRecurse( bool t )
{
    m_recurseChildren = t;
    if( t == false )
        m_openChildrenURLs.clear();
}


void KFileTreeBranch::setShowExtensions( bool visible )
{
    m_showExtensions = visible;
}

bool KFileTreeBranch::showExtensions( ) const
{
    return( m_showExtensions );
}

/*
 * The signal that tells that a directory was deleted may arrive before the signal
 * for its children arrive. Thus, we must walk through the children of a dir and
 * remove them before removing the dir itself.
 */
void KFileTreeBranch::slotDeleteItem( KFileItem *it )
{
    if( !it ) return;
    kDebug(250) << "Slot Delete Item hitted for " << it->url().prettyURL() << endl;

    KFileTreeViewItem *kfti = static_cast<KFileTreeViewItem*>(it->extraData(this));

    if( kfti )
    {
        kDebug( 250 ) << "Child count: " << kfti->childCount() << endl;
        if( kfti->childCount() > 0 )
        {
            KFileTreeViewItem *child = static_cast<KFileTreeViewItem*>(kfti->firstChild());

            while( child )
            {
                kDebug(250) << "Calling child to be deleted !" << endl;
                KFileTreeViewItem *nextChild = static_cast<KFileTreeViewItem*>(child->nextSibling());
                slotDeleteItem( child->fileItem());
                child = nextChild;
            }
        }

        kDebug(250) << "Found corresponding KFileTreeViewItem" << endl;
        if( m_lastFoundURL.equals(it->url(), true ))
        {
          m_lastFoundURL = KUrl();
          m_lastFoundItem = 0L;
        }
        delete( kfti );
    }
    else
    {
        kDebug(250) << "Error: kfiletreeviewitem: "<< kfti << endl;
    }
}


void KFileTreeBranch::slotCanceled( const KUrl& url )
{
    // ### anything else to do?
    // remove the url from the childrento-recurse-list
    m_openChildrenURLs.removeAll( url);

    // stop animations etc.
    KFileTreeViewItem *item = findTVIByURL(url);
    if (!item) return; // Uh oh...
    emit populateFinished(item);
}

void KFileTreeBranch::slotDirlisterClear()
{
    kDebug(250)<< "*** Clear all !" << endl;
    /* this slots needs to clear all listed items, but NOT the root item */
    if( m_root )
        deleteChildrenOf( m_root );
}

void KFileTreeBranch::slotDirlisterClearURL( const KUrl& url )
{
    kDebug(250)<< "*** Clear for URL !" << url.prettyURL() << endl;
    KFileItem *item = findByURL( url );
    if( item )
    {
        KFileTreeViewItem *ftvi =
            static_cast<KFileTreeViewItem *>(item->extraData( this ));
        deleteChildrenOf( ftvi );
    }
}

void KFileTreeBranch::deleteChildrenOf( Q3ListViewItem *parent )
{
    // for some strange reason, slotDirlisterClearURL() sometimes calls us
    // with a 0L parent.
    if ( !parent )
        return;

    while ( parent->firstChild() )
        delete parent->firstChild();
}

void KFileTreeBranch::slotRedirect( const KUrl& oldUrl, const KUrl&newUrl )
{
    if( oldUrl.equals( m_startURL, true ))
    {
        m_startURL = newUrl;
    }
}

KFileTreeViewItem* KFileTreeBranch::findTVIByURL( const KUrl& url )
{
    KFileTreeViewItem *resultItem = 0;

    if( m_startURL.equals(url, true) )
    {
        kDebug(250) << "findByURL: Returning root as a parent !" << endl;
        resultItem = m_root;
    }
    else if( m_lastFoundURL.equals( url, true ))
    {
        kDebug(250) << "findByURL: Returning from lastFoundURL!" << endl;
        resultItem = m_lastFoundItem;
    }
    else
    {
        kDebug(250) << "findByURL: searching by dirlister: " << url.url() << endl;

        KFileItem *it = findByURL( url );

        if( it )
        {
            resultItem = static_cast<KFileTreeViewItem*>(it->extraData(this));
            m_lastFoundItem = resultItem;
            m_lastFoundURL = url;
        }
    }

    return( resultItem );
}


void KFileTreeBranch::slCompleted( const KUrl& url )
{
    kDebug(250) << "SlotCompleted hit for " << url.prettyURL() << endl;
    KFileTreeViewItem *currParent = findTVIByURL( url );
    if( ! currParent ) return;

    kDebug(250) << "current parent " << currParent << " is already listed: "
                 << currParent->alreadyListed() << endl;

    emit( populateFinished(currParent));
    emit( directoryChildCount(currParent, currParent->childCount()));

    /* This is a walk through the children of the last populated directory.
     * Here we start the dirlister on every child of the dir and wait for its
     * finish. When it has finished, we go to the next child.
     * This must be done for non local file systems in dirOnly- and Full-Mode
     * and for local file systems only in full mode, because the stat trick
     * (see addItem-Method) does only work for dirs, not for files in the directory.
     */
    /* Set bit that the parent dir was listed completely */
    currParent->setListed(true);

    kDebug(250) << "recurseChildren: " << m_recurseChildren << endl;
    kDebug(250) << "isLocalFile: " << m_startURL.isLocalFile() << endl;
    kDebug(250) << "dirOnlyMode: " << dirOnlyMode() << endl;


    if( m_recurseChildren && (!m_startURL.isLocalFile() || ! dirOnlyMode()) )
    {
        bool wantRecurseUrl = false;
        /* look if the url is in the list for url to recurse */
        for ( KUrl::List::Iterator it = m_openChildrenURLs.begin();
              it != m_openChildrenURLs.end(); ++it )
        {
            /* it is only interesting that the url _is_in_ the list. */
            if( (*it).equals( url, true ) )
                wantRecurseUrl = true;
        }

        KFileTreeViewItem    *nextChild = 0;
        kDebug(250) << "Recursing " << url.prettyURL() << "? " << wantRecurseUrl << endl;

        if( wantRecurseUrl && currParent )
        {

            /* now walk again through the tree and populate the children to get +-signs */
            /* This is the starting point. The visible folder has finished,
               processing the children has not yet started */
            nextChild = static_cast<KFileTreeViewItem*>
                        (static_cast<Q3ListViewItem*>(currParent)->firstChild());

            if( ! nextChild )
            {
                /* This happens if there is no child at all */
                kDebug( 250 ) << "No children to recuse" << endl;
            }

            /* Since we have listed the children to recurse, we can remove the entry
             * in the list of the URLs to see the children.
             */
            m_openChildrenURLs.removeAll(url);
        }

        if( nextChild ) /* This implies that idx > -1 */
        {
            /* Next child is defined. We start a dirlister job on every child item
             * which is a directory to find out how much children are in the child
             * of the last opened dir
             */

            /* Skip non directory entries */
            while( nextChild )
            {
                if( nextChild->isDir() && ! nextChild->alreadyListed())
                {
                    KFileItem *kfi = nextChild->fileItem();
                    if( kfi && kfi->isReadable())
                    {
                        KUrl recurseUrl = kfi->url();
                        kDebug(250) << "Starting to recurse NOW " << recurseUrl.prettyURL() << endl;
                        openURL( recurseUrl, true );
                    }
                }
                nextChild = static_cast<KFileTreeViewItem*>(static_cast<Q3ListViewItem*>(nextChild->nextSibling()));
                // kDebug(250) << "Next child " << m_nextChild << endl;
            }
        }
    }
    else
    {
        kDebug(250) << "skipping to recurse in complete-slot" << endl;
    }
}

/* This slot is called when a treeviewitem is expanded in the gui */
bool KFileTreeBranch::populate( const KUrl& url,  KFileTreeViewItem *currItem )
{
    bool ret = false;
    if( ! currItem )
        return ret;

    kDebug(250) << "Populating <" << url.prettyURL() << ">" << endl;

    /* Add this url to the list of urls to recurse for children */
    if( m_recurseChildren )
    {
        m_openChildrenURLs.append( url );
        kDebug(250) << "Appending to list " << url.prettyURL() << endl;
    }

    if( ! currItem->alreadyListed() )
    {
        /* start the lister */
        ret = openURL( url, true );
    }
    else
    {
        kDebug(250) << "Children already existing in treeview!" << endl;
        slCompleted( url );
        ret = true;
    }
    return ret;
}

void KFileTreeBranch::virtual_hook( int id, void* data )
{ KDirLister::virtual_hook( id, data ); }

#include "kfiletreebranch.moc"

