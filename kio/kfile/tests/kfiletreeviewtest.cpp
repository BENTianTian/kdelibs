/* This file is part of the KDE libraries
    Copyright (C) 2001 Klaas Freitag <freitag@suse.de>

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

#include <kglobal.h>
#include <kiconloader.h>
#include <kmainwindow.h>
#include <kapp.h>
#include <kurl.h>
#include <kdebug.h>
#include <kstatusbar.h>

#include <kfiletreeview.h>
#include "kfiletreeviewtest.h"


#include "kfiletreeviewtest.moc"

testFrame::testFrame():KMainWindow(0,"Test FileTreeView"),
		       dirOnlyMode(false)

{
   treeView = new KFileTreeView( this );
   treeView->setDragEnabled( true );
   treeView->setDropVisualizer( true );

   /* Connect to see the status bar */
   KStatusBar* sta = statusBar();
   connect( treeView, SIGNAL( onItem( const QString& )),
	    sta, SLOT( message( const QString& )));
   
   
   treeView->addColumn( "File" );
   treeView->addColumn( "ChildCount" );
   setCentralWidget( treeView );
}

void testFrame::showPath( KURL &url )
{
   QString fname = "TestBranch"; // url.fileName ();
   /* try a user icon */
   KIconLoader *loader = KGlobal::iconLoader();
   QPixmap pix = loader->loadIcon( "contents2", KIcon::Small );
   QPixmap pixOpen = loader->loadIcon( "contents", KIcon::Small );

   KFileTreeBranch *nb = treeView->addBranch( url, fname, pix );
   
   if( nb )
   {
      if( dirOnlyMode ) treeView->setDirOnlyMode( nb, true );
      nb->setOpenPixmap( pixOpen );
      
      connect( nb, SIGNAL(populateFinished(KFileTreeViewItem*)),
	       this, SLOT(slotPopulateFinished(KFileTreeViewItem*)));
      connect( nb, SIGNAL( directoryChildCount( KFileTreeViewItem *, int )),
	       this, SLOT( slotSetChildCount( KFileTreeViewItem*, int )));
      // nb->setChildRecurse(false );
      
      nb->setOpen(true);
   }


}

void testFrame::slotPopulateFinished(KFileTreeViewItem *item )
{
#if 0
   if( item )
   {
      int cc = item->childCount();

      kdDebug() << "setting column 2 of treeview with count " << cc << endl;

      item->setText( 1, QString::number( cc ));
   }
   else
   {
      kdDebug() << "slotPopFinished for uninitalised item" << endl;
   }
#endif
}

void testFrame::slotSetChildCount( KFileTreeViewItem *item, int c )
{
   if( item )
      item->setText(1, QString::number( c ));
}

int main(int argc, char **argv)
{
    KApplication a(argc, argv, "kfiletreeviewtest");
    QString name1;
    QStringList names;

    QString argv1;
    testFrame *tf;

    tf =  new testFrame();
    a.setMainWidget( tf );
    
    if (argc > 1)
    {
       for( int i = 1; i < argc; i++ )
       {
	  argv1 = QString::fromLatin1(argv[i]);
	  kdDebug() << "Opening " << argv1 << endl;
	  if( argv1 == "-d" )
	     tf->setDirOnly();
	  else
	  {
	     if( argv1.endsWith( "/" ))
		argv1.truncate( argv1.length() -1  );
	  KURL u( argv1 );
	  tf->showPath( u );
       }
    }
    }
    tf->show();
    int ret = a.exec();
    return( ret );
}
