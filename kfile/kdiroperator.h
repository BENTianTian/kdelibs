// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
		  2000 Carsten Pfeiffer <pfeiffer@kde.org>

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
#ifndef KDIROPERATOR_H_
#define KDIROPERATOR_H_

#include <qobject.h>
#include <qstack.h>

#include <kaction.h>
#include <kcompletion.h>

#include <kfileview.h>
#include <kfileviewitem.h>
#include <kfile.h>

class QPopupMenu;
class QTimer;

class KAction;
class KToggleAction;
class KActionSeparator;
class KActionMenu;
class KFileReader;
class QWidgetStack;
class KProgress;

class KDirOperator : public QWidget {
    Q_OBJECT
	
 public:
    KDirOperator(const KURL& urlName = KURL(),
		 QWidget *parent = 0, const char* name = 0);
    virtual ~KDirOperator();

    void setShowHiddenFiles ( bool s ) { showHiddenAction->setChecked( s ); }
    bool showHiddenFiles () const { return showHiddenAction->isChecked(); }

    void close();

    void setNameFilter(const QString& filter);

    KURL url() const;

    void setURL(const KURL& url, bool clearforward);

    //this also reads the current url(), so you better call this after setURL()
    void setView(KFileView *view);
    const KFileView * view() const { return fileView; }
    void setView(KFile::FileView view);

    void setSorting( QDir::SortSpec );
    QDir::SortSpec sorting() const { return mySorting; }

    bool isRoot() const;

    KFileReader *fileReader() const { return dir; }

    void setMode( KFile::Mode m );
    KFile::Mode mode() const;

    void setPreviewWidget(const QWidget *w);

    const KFileViewItemList * selectedItems() const {
	return ( fileView ? fileView->selectedItems() : 0L );
    }
    inline bool isSelected( const KFileViewItem *item ) const {
	return ( fileView ? fileView->isSelected( item ) : false );
    }


    int numDirs() const;
    int numFiles() const;

    /**
     * an accessor to a collection of all available Actions. The actions
     * are static, they will be there all the time (no need to connect to
     * the signals QActionCollection::inserted() or removed().
     *
     * There are the following actions:
     *
     * @li popupMenu : an ActionMenu presenting a popupmenu with all actions
     * @li up : changes to the parent directory
     * @li back : goes back to the previous directory
     * @li forward : goes forward in the history
     * @li home : changes to the user's home directory
     * @li reload : reloads the current directory
     * @li separator : a separator
     * @li mkdir : opens a dialog box to create a directory
     * @li sorting menu : an ActionMenu containing all sort-options
     * @li by name : sorts by name
     * @li by date : sorts by date
     * @li by size : sorts by size
     * @li reversed : reverses the sort order
     * @li dirs first : sorts directories before files
     * @li case insensitive : sorts case insensitively
     * @li view menu : an ActionMenu containing all actions concerning the view
     * @li short view : shows a simple fileview
     * @li detailed view : shows a detailed fileview (dates, permissions ,...)
     * @li show hidden : shows hidden files
     * @li separate dirs : shows directories in a separate pane
     *
     * The short and detailed view are in an exclusive group. The sort-by
     * actions are in an exclusive group as well.
     *
     * You can e.g. use
     * <pre>actionCollection()->action( "up" )->plug( someToolBar );</pre>
     * to add a button into a toolbar, which makes the dirOperator change to
     * its parent directory.
     *
     * @returns all available Actions
     */
    QActionCollection * actionCollection() const { return myActionCollection; }


 protected:
    void setFileReader( KFileReader *reader );
    void resizeEvent( QResizeEvent * );
    void setupActions();
    void setupMenu();


 private:
    /**
     * Contains all URLs you can reach with the back button.
     */
    QStack<KURL> backStack;

    /**
     * Contains all URLs you can reach with the forward button.
     */
    QStack<KURL> forwardStack;

    static KURL *lastDirectory;

    KFileReader *dir;

    KCompletion myCompletion;
    bool myCompleteListDirty;
    QDir::SortSpec mySorting;

    /**
      * takes action on the new location. If it's a directory, change
      * into it, if it's a file, correct the name, etc.
      */
    void checkPath(const QString& txt, bool takeFiles = false);

    void connectView(KFileView *);

    // for the handling of the cursor
    bool finished;

    KFileView *fileView;
    KFileView *oldView;

    KFileViewItemList pendingMimeTypes;

    // the enum KFile::FileView as an int
    int viewKind;

    KFile::Mode myMode;
    KProgress *progress;

    QWidget *myPreview;    // temporary pointer for the preview widget

    // actions for the popupmenus
    KActionMenu *actionMenu;

    KAction 	*backAction;
    KAction 	*forwardAction;
    KAction 	*homeAction;
    KAction 	*upAction;
    KAction 	*reloadAction;
    KActionSeparator *actionSeparator;
    KAction 	*mkdirAction;

    KActionMenu *sortActionMenu;
    KRadioAction *byNameAction;
    KRadioAction *byDateAction;
    KRadioAction *bySizeAction;
    KToggleAction *reverseAction;
    KToggleAction *dirsFirstAction;
    KToggleAction *caseInsensitiveAction;

    KActionMenu *viewActionMenu;
    KRadioAction *shortAction;
    KRadioAction *detailedAction;
    KToggleAction *showHiddenAction;
    KToggleAction *separateDirsAction;

    QActionCollection *myActionCollection;

 public slots:
    void back();
    void forward();
    void home();
    void cdUp();
    void rereadDir();
    void mkdir();
    QString makeCompletion(const QString&);

  protected slots:
    void resetCursor();
    void readNextMimeType();
    void slotKIOError(int, const QString& );
    void pathChanged();
    void filterChanged();
    void insertNewFiles(const KFileViewItemList &newone, bool ready);
    void itemsDeleted(const KFileViewItemList &);

    void selectDir(const KFileViewItem*);
    void selectFile(const KFileViewItem*);
    void activatedMenu( const KFileViewItem * );

    void sortByName() 		{ byNameAction->setChecked( true ); }
    void sortBySize() 		{ bySizeAction->setChecked( true ); }
    void sortByDate() 		{ byDateAction->setChecked( true ); }
    void sortReversed() 	{ reverseAction->setChecked( !reverseAction->isChecked() ); }
    void toggleDirsFirst() 	{ dirsFirstAction->setChecked( !dirsFirstAction->isChecked() ); }
    void toggleIgnoreCase() 	{ caseInsensitiveAction->setChecked( !caseInsensitiveAction->isChecked() ); }

    void deleteOldView();

    void slotCompletionMatch(const QString&);
    void slotCompletionMatches(const QStringList&);

private slots:
    void slotDetailedView();
    void slotSimpleView();
    void slotToggleHidden( bool );
    void slotToggleMixDirsAndFiles();
    void slotSortByName();
    void slotSortBySize();
    void slotSortByDate();
    void slotSortReversed();
    void slotToggleDirsFirst();
    void slotToggleIgnoreCase();

signals:
    void urlEntered(const KURL& );
    void updateInformation(int files, int dirs);
    void completion(const QString&);

    void fileHighlighted(const KFileViewItem*);
    void dirActivated(const KFileViewItem*);
    void fileSelected(const KFileViewItem*);

};

#endif
