/* This file is part of the KDEproject
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qfile.h>

#include <kfileitem.h>
#include <kdebug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kfiletreeviewitem.h"
#include "kfiletreebranch.h"

class KFileTreeView;


/* --- KFileTreeViewToplevelItem --- */
KFileTreeBranch::KFileTreeBranch( KFileTreeView *parent, const KURL& url,
                                  const QString& name,
				  const QPixmap& pix, bool showHidden  )
    : KDirLister( false ),
      m_root( 0L ),
      m_startURL( url ),
      m_name ( name ),
      m_rootIcon( pix ),
      m_currParent( 0L ),
      m_nextChild( 0L ),
      m_recurseChildren( false )
{
   m_root = createBranchRoot( parent, url );
   
   m_root->setPixmap( 0, pix );
   m_root->setText( 0, name );
   m_root->setOpen( true );
   m_currParent= m_root;

   setShowingDotFiles( showHidden );

   connect( this, SIGNAL( newItems(const KFileItemList&)),
	    this, SLOT  ( addItems( const KFileItemList& )));

   connect( this, SIGNAL( completed()),
	    this,   SLOT(slCompleted()));

   connect( this, SIGNAL( started( const KURL& )),
	    this,   SLOT( slotListerStarted( const KURL& )));

   connect( this, SIGNAL( deleteItem( KFileItem* )),
	    this,   SLOT( slotDeleteItem( KFileItem* )));

   connect( this, SIGNAL( canceled() ),
            this,   SLOT( slotCanceled() ));
}


void KFileTreeBranch::slotListerStarted( const KURL &url )
{
   /* set the parent correct if it is zero. */
   kdDebug( 250) << "Find parent for " << url.prettyURL() << endl;
   kdDebug(250) << "Root URL is " << m_root->url().prettyURL() << endl;
   KFileItem *fi = findByURL( url );

   m_currParent = 0;
   
   if( fi )
   {
      m_currParent = static_cast<KFileTreeViewItem*>( fi->extraData( this ));
      kdDebug(250) << "Current parent changed!" << endl;
   }

   if( !fi && url.cmp(m_root->url(), true )) // compare and ignore trailing /
   {
      m_currParent = m_root;
      kdDebug(250) << "slotListerStarted: Parent is branch-root!" << endl;
   }

   if( ! m_currParent )
   {
      kdDebug(250) << "No parent found in KFileItem for " << url.prettyURL() << endl;
   }
}



void KFileTreeBranch::addItems( const KFileItemList& list )
{
   /* Put the new items under the current url */
   KFileItemListIterator it( list );
   kdDebug(250) << "Adding items !" << endl;
   KFileItem *currItem;
   KFileTreeViewItemList treeViewItList;
   
   
   while ( m_currParent && (currItem = it.current()) != 0 )
   {
      /* Only create a new KFileTreeViewItem if it does not yet exist */
      KFileTreeViewItem *newKFTVI =
	 static_cast<KFileTreeViewItem *>(currItem->extraData( this ));

      if( ! newKFTVI )
      {
	 newKFTVI = createTreeViewItem( m_currParent, currItem );
	 
	 currItem->setExtraData( this, newKFTVI );

	 /* Cut off the file extension */
	 if( !m_showExtensions && !currItem->isDir() )
	 {
	    QString n = currItem->text();
	    int mPoint = n.findRev( '.' );
	    if( mPoint > 0 ) n = n.left( mPoint );
	    newKFTVI->setText( 0, n );
	 }
      }

      /* Now try to find out if there are children for dirs in the treeview */
      /* This stats a directory on the local file system and checks the */
      /* hardlink entry in the stat-buf. This works only for directory. */
      if( currItem->isLocalFile( ) && currItem->isDir() )
      {
	 KURL url = currItem->url();
	 QString filename = url.directory( false, true ) + url.filename();
	 /* do the stat trick of Carsten. The problem is, that the hardlink
	 *  count only contains directory links. Thus, this method only seem
         * to work in dir-only mode */
	 struct stat statBuf;
	 if( stat( QFile::encodeName( filename ), &statBuf ) == 0 )
	 {
	    int hardLinks = statBuf.st_nlink;  /* Count of dirs */
	    if( hardLinks > 2 )
	    {
	       newKFTVI->setExpandable(true);
	    }
	    else
	    {
	       newKFTVI->setExpandable(false);
	    }
	 }
	 else
	 {
	    kdDebug(250) << "stat of " << filename << " failed !" << endl;
	 }
      }
      ++it;

      treeViewItList.append( newKFTVI );
   }
      
   emit( newTreeViewItems( this, treeViewItList ));
   
}

KFileTreeViewItem* KFileTreeBranch::createTreeViewItem( KFileTreeViewItem *parent,
							KFileItem *fileItem )
{
   return( new KFileTreeViewItem( parent,
				  fileItem,
				  this ));
}

KFileTreeViewItem* KFileTreeBranch::createBranchRoot( KFileTreeView *parent, const KURL& url )
{
   return( new KFileTreeViewItem( parent,
				  new KFileItem( url, "inode/directory",
                                                  S_IFDIR  ),
				  this ));
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
   kdDebug(250) << "Slot Delete Item hitted for " << it->url().prettyURL() << endl;
   
   KFileTreeViewItem *kfti = static_cast<KFileTreeViewItem*>(it->extraData(this));

   if( kfti )
   {
      kdDebug( 250 ) << "Child count: " << kfti->childCount() << endl;
      if( kfti->childCount() > 0 )
      {
	 KFileTreeViewItem *child = static_cast<KFileTreeViewItem*>(kfti->firstChild());
      
	 while( child )
	 {
	    kdDebug(250) << "Calling child to be deleted !" << endl;
	    KFileTreeViewItem *nextChild = static_cast<KFileTreeViewItem*>(child->nextSibling());
	    slotDeleteItem( child->fileItem());
	    child = nextChild;
	 }
      }
      
      kdDebug(250) << "Found corresponding KFileTreeViewItem" << endl;
      delete( kfti );
      it->removeExtraData( this );
   }
   else
   {
      kdDebug(250) << "Error: kfiletreeviewitem: "<< kfti << endl;
   }
}


void KFileTreeBranch::slotCanceled()
{
    // ### anything else to do?
    emit populateFinished( m_currParent );
}

void KFileTreeBranch::slCompleted()
{
   kdDebug(250) << "SlotCompleted hit !" << endl;
   emit( populateFinished( m_currParent));

   /* This is a walk through the children of the last populated directory.
    * Here we start the dirlister on every child of the dir and wait for its
    * finish. When it has finished, we go to the next child.
    * This must be done for non local file systems in dirOnly- and Full-Mode
    * and for local file systems only in full mode, because the stat trick
    * (see addItem-Method).
    * does only work for dirs, not for files in the directory.
    */
   if( !m_startURL.isLocalFile() || ! dirOnlyMode() )
   {
      /* now walk again through the tree and populate the children to get +-signs */
      if( ! m_currParent ) return;

      if( m_recurseChildren && ! m_nextChild )
      {
	 /* This is the starting point. The visible folder has finished,
            processing the children has not yet started */
	 m_nextChild = static_cast<KFileTreeViewItem*>
	    (static_cast<QListViewItem*>(m_currParent)->firstChild());

	 m_recurseChildren = false;
	 if( ! m_nextChild )
	 {
	    /* This happens if there is no child at all */
	    kdDebug( 250 ) << "No children to recuse" << endl;
	 }
// 	 else
// 	 {
// 	    KFileItem *fi = m_currParent->fileItem();
// 	 }

      }

      if( m_nextChild )
      {
	 /* Next child is defined, the dirlister job must be started with on the
	  * closed child of an open item to find out if there are children to make
	  * the +-sign in the treeview. */

	 /* Skip non directory entries */
	 while( m_nextChild && !m_nextChild->isDir())
	 {
	    m_nextChild = static_cast<KFileTreeViewItem*>(static_cast<QListViewItem*>(m_nextChild->nextSibling()));
	    // kdDebug(250) << "Next child " << m_nextChild << endl;
	 }

	 /* at this point, m_nextChildren is a dir if defined or m_nextChild is zero, both OK. */
	 if( m_nextChild )
	 {
	    KFileItem *kfi = m_nextChild->fileItem();
	    if( kfi )
	    {
	       KURL url = kfi->url();
	       kdDebug(250) << "Looking for children of <" << url.prettyURL() << ">" << endl;
	       m_currParent = m_nextChild;

	       /* now switch the nextChild pointer to the next to have the next item when
		* this slot is hit again.
		*/
	       m_nextChild = static_cast<KFileTreeViewItem*>(static_cast<QListViewItem*>(m_nextChild->nextSibling()));
	       if( kfi->isReadable() )
		  openURL( url, true );
	       else
		  kdDebug(250) << "Can not recurse to " << url.prettyURL() << endl;
	    }
	 }
	 else
	 {
	    kdDebug(250) << "** parsing of children finished" << endl;
	    m_recurseChildren = false;
	    m_currParent = 0L;
	 }
      }
   }
   else
   {
      kdDebug(250) << "skipping to recurse in complete-slot" << endl;
      m_currParent = 0L;

   }
}

void KFileTreeBranch::populate( const KURL& url,  KFileTreeViewItem *currItem )
{
   if( ! currItem )
      return;
   m_currParent = currItem;

   kdDebug(250) << "Populating <" << url.prettyURL() << ">" << endl;

   /* Recurse to the first row of children of the new files to get info about children */
   m_recurseChildren = true;

   openURL( url, true );
}

void KFileTreeBranch::populate( )
{
   populate( m_startURL, m_currParent );
}

#include "kfiletreebranch.moc"

