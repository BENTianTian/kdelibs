
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2001 Klaas Freitag <freitag@suse.de>
		
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

#ifndef kfile_tree_branch_h
#define kfile_tree_branch_h

#include <qdict.h>
#include <qlistview.h>

#include <kfileitem.h>
#include <kio/global.h>
#include <kdirlister.h>
#include <kio/job.h>
#include <kfiletreeviewitem.h>

class KURL;
class KFileTreeView;


/**
 * This is the branch class of the KFileTreeViewWidget, which represents one
 * branch in the treeview. Every branch has a root, which is an Url and lists
 * the files unter the root. Every branch uses its own dirlister and can have
 * its own filter etc, different from other branches in the treeview.
 *
 * @short Branch object for @ref KFileTreeView object.
 *
 */

class KFileTreeBranch : public KDirLister
{
   Q_OBJECT
public:
   /**
    * constructs a branch, listing the file system.
    * @param url start url of the branch.
    * @param name the name of the branch, which is displayed in the first column of the treeview.
    * @param pix is a pixmap to display as an icon of the branch.
    * @param showHidden flag to make hidden files visible or not.
    */
   KFileTreeBranch( KFileTreeView*, const KURL& url, const QString& name,
                    const QPixmap& pix, bool showHidden = false);

   /**
    * @returns the root url of the branch.
    */
   KURL 	rootUrl() const { return( m_startURL ); }

   /**
    * sets a @ref KFileTreeViewItem as root widget for the branch.
    * That must be created outside of the branch. All KFileTreeViewItems
    * the branch is allocating will become children of that object.
    * @param the KFileTreeViewItem to become the root item.
    */
   virtual void 	setRoot( KFileTreeViewItem *r ){ m_root = r; };

   /**
    * @returns the root item.
    */
   KFileTreeViewItem *root( ) { return( m_root );}

   /**
    * @returns the name of the branch.
    */
   QString      name() const { return( m_name ); }

   /**
    * sets the name of the branch.
    */
   virtual void         setName( const QString n ) { m_name = n; };

   const QPixmap& pixmap(){ return(m_rootIcon); }

   /**
    * @returns whether the items in the branch show their file extensions in the
    * tree or not. See @ref setShowExtensions for more information.
    */
   bool showExtensions( ) const;
   
public slots:
   /**
    * populates a branch. This method must be called after a branch was added
    * to  a @ref KFileTreeView using method @ref addBranch.
    * @param url is the url of the root item where the branch starts.
    * @param currItem is the current parent.
    */
   virtual void populate( const KURL &url, KFileTreeViewItem* currItem );

   void populate( );

   /**
    * sets printing of the file extensions on or off. If you pass false to this
    * slot, all items of this branch will not show their file extensions in the
    * tree.
    * @param visible flags if the extensions should be visible or not.
    */
   virtual void setShowExtensions( bool visible = true );

protected:
   /**
    * virtual method that allocates a @ref KFileTreeViewItem for the branch
    * for new items. 
    */ 
   virtual KFileTreeViewItem *createTreeViewItem( KFileTreeViewItem *parent,
						  KFileItem *fileItem );

   virtual KFileTreeViewItem *createBranchRoot( KFileTreeView *parent, const KURL& url );
   
signals:
   /**
    * emitted with the item of a directory which was finished to populate
    */
   void populateFinished( KFileTreeViewItem * );

   /**
    * emitted with a list of new or updated @ref KFileTreeViewItem which were
    * found in a branch. Note that this signal is emitted very often and may slow
    * down the performance of the treeview !
    */
   void newTreeViewItems( KFileTreeBranch*, const KFileTreeViewItemList& );

private slots:
   void addItems( const KFileItemList& );
   void slCompleted();
   void slotCanceled();
   void slotListerStarted( const KURL& );
   void slotDeleteItem( KFileItem* );

private:
   KFileTreeViewItem 	*m_root;
   KURL 		m_startURL;
   QString 		m_name;
   QPixmap 		m_rootIcon;
   KFileTreeViewItem    *m_currParent;
   KFileTreeViewItem    *m_nextChild;

   bool 		m_wantDotFiles    :1;
   bool                 m_recurseChildren :1;
   bool                 m_showExtensions  :1;
   
   class KFileTreeBranchPrivate;
   KFileTreeBranchPrivate *d;
};


/**
 * List of KFileTreeBranches
 */
typedef QPtrList<KFileTreeBranch> KFileTreeBranchList;

/**
 * Iterator for KFileTreeBranchLists
 */
typedef QPtrListIterator<KFileTreeBranch> KFileTreeBranchIterator;




#endif

