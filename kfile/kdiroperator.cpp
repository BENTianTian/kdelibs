/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Stephan Kulow <coolo@kde.org>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kdiroperator.h"
#include "kcombiview.h"
#include "kfilepreview.h"
#include "k3fileiconview.h"
#include "k3filedetailview.h"
#include "kfileview.h"
#include "kfileitem.h"
#include "kfilemetapreview.h"

#include <config-kfile.h>

#include <unistd.h>

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtGui/QProgressBar>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdirlister.h>
#include <kicon.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/previewjob.h>
#include <kio/renamedialog.h>
#include <kpropertiesdialog.h>
#include <kmimetypefactory.h>
#include <kstandardshortcut.h>
#include <kde_file.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kconfiggroup.h>


template class QHash<QString,KFileItem*>;

class KDirOperator::KDirOperatorPrivate
{
public:
    KDirOperatorPrivate() {
        onlyDoubleClickSelectsFiles = false;
        progressDelayTimer = 0L;
        dirHighlighting = false;
        dropOptions = 0;
        configGroup=0;
        viewActionSeparator = new QAction(0);
        viewActionSeparator->setSeparator(true);
    }

    ~KDirOperatorPrivate() {
        delete progressDelayTimer;
        delete viewActionSeparator;
    }

    bool dirHighlighting;
    QString lastURL; // used for highlighting a directory on cdUp
    bool onlyDoubleClickSelectsFiles;
    QTimer *progressDelayTimer;
    QAction *viewActionSeparator;
    int dropOptions;

    KConfigGroup *configGroup;
};

KDirOperator::KDirOperator(const KUrl& _url, QWidget *parent)
    : QWidget(parent),
      dir(0),
      m_fileView(0),
      progress(0),d(new KDirOperatorPrivate)
{
    myPreview = 0L;
    myMode = KFile::ModeMax; //try to make KFileDialog::getOpenURL (without s) to work again
    m_viewKind = KFile::Simple;
    mySorting = QDir::Name | QDir::DirsFirst;

    if (_url.isEmpty()) { // no dir specified -> current dir
        QString strPath = QDir::currentPath();
        strPath.append(QChar('/'));
        currUrl = KUrl();
        currUrl.setProtocol(QLatin1String("file"));
        currUrl.setPath(strPath);
    }
    else {
        currUrl = _url;
        if ( currUrl.protocol().isEmpty() )
            currUrl.setProtocol(QLatin1String("file"));

        currUrl.addPath("/"); // make sure we have a trailing slash!
    }

    setDirLister( new KDirLister() );

    connect(&myCompletion, SIGNAL(match(const QString&)),
            SLOT(slotCompletionMatch(const QString&)));

    progress = new QProgressBar(this);
    progress->setObjectName("progress");
    progress->adjustSize();
    progress->move(2, height() - progress->height() -2);

    d->progressDelayTimer = new QTimer( this );
    d->progressDelayTimer->setObjectName( QLatin1String( "progress delay timer" ) );
    connect( d->progressDelayTimer, SIGNAL( timeout() ),
        SLOT( slotShowProgress() ));

    myCompleteListDirty = false;

    // action stuff
    setupActions();
    setupMenu();

    setFocusPolicy(Qt::WheelFocus);
}

KDirOperator::~KDirOperator()
{
    resetCursor();
    if ( m_fileView )
    {
        if ( d->configGroup )
            m_fileView->writeConfig( d->configGroup );

        delete m_fileView;
        m_fileView = 0L;
    }

    qDeleteAll( backStack );
    qDeleteAll( forwardStack );
    delete myPreview;
    delete dir;
    delete d->configGroup;
    delete d;
}


void KDirOperator::setSorting( QDir::SortFlags spec )
{
    if ( m_fileView )
        m_fileView->setSorting( spec );
    mySorting = spec;
    updateSortActions();
}

QDir::SortFlags KDirOperator::sorting() const
{
    return mySorting;
}

bool KDirOperator::isRoot() const
{
    return url().path() == QString(QLatin1Char('/'));
}

KDirLister *KDirOperator::dirLister() const
{
    return dir;
}

void KDirOperator::resetCursor()
{
   QApplication::restoreOverrideCursor();
   progress->hide();
}

void KDirOperator::insertViewDependentActions()
{
   // If we have a new view actionCollection(), insert its actions
   // into viewActionMenu.

   if( !m_fileView )
      return;

   if ( (viewActionMenu->menu()->actions().count() == 0) || 			// Not yet initialized or...
        (viewActionCollection != m_fileView->actionCollection()) )	// ...changed since.
   {
      if (viewActionCollection)
      {
         disconnect( viewActionCollection, SIGNAL( inserted( KAction * )),
               this, SLOT( slotViewActionAdded( KAction * )));
         disconnect( viewActionCollection, SIGNAL( removed( KAction * )),
               this, SLOT( slotViewActionRemoved( KAction * )));
      }

      viewActionMenu->menu()->clear();
//      viewActionMenu->addAction( shortAction );
//      viewActionMenu->addAction( detailedAction );
//      viewActionMenu->addAction( actionSeparator );
      viewActionMenu->addAction( myActionCollection->action( "short view" ) );
      viewActionMenu->addAction( myActionCollection->action( "detailed view" ) );
      viewActionMenu->addAction( actionSeparator );
      viewActionMenu->addAction( showHiddenAction );
//      viewActionMenu->addAction( myActionCollection->action( "single" ));
      viewActionMenu->addAction( separateDirsAction );
      // Warning: adjust slotViewActionAdded() and slotViewActionRemoved()
      // when you add/remove actions here!

      viewActionCollection = m_fileView->actionCollection();
      if (!viewActionCollection)
         return;

      if ( !viewActionCollection->isEmpty() )
      {
         viewActionMenu->addAction( d->viewActionSeparator );

         // first insert the normal actions, then the grouped ones
         QList<QActionGroup*> groups = viewActionCollection->actionGroups();

         foreach (QAction* action, viewActionCollection->actionsWithoutGroup())
            viewActionMenu->addAction( action );

         foreach (QActionGroup* group, groups)
         {
            viewActionMenu->addSeparator();

            foreach (QAction* action, group->actions())
               viewActionMenu->addAction( action );
         }
      }

      connect( viewActionCollection, SIGNAL( inserted( KAction * )),
               SLOT( slotViewActionAdded( KAction * )));
      connect( viewActionCollection, SIGNAL( removed( KAction * )),
               SLOT( slotViewActionRemoved( KAction * )));
   }
}

void KDirOperator::activatedMenu( const KFileItem *, const QPoint& pos )
{
    setupMenu();
    updateSelectionDependentActions();

    actionMenu->menu()->exec( pos );
}

void KDirOperator::sortByName()
{
    byNameAction->setChecked( true );
}

void KDirOperator::sortBySize()
{
    bySizeAction->setChecked( true );
}

void KDirOperator::sortByDate()
{
    byDateAction->setChecked( true );
}

void KDirOperator::sortReversed()
{
    reverseAction->setChecked( !reverseAction->isChecked() );
}

void KDirOperator::toggleDirsFirst()
{
    dirsFirstAction->setChecked( !dirsFirstAction->isChecked() );
}

void KDirOperator::toggleIgnoreCase()
{
    caseInsensitiveAction->setChecked( !caseInsensitiveAction->isChecked() );
}

void KDirOperator::updateSelectionDependentActions()
{
    bool hasSelection = m_fileView && m_fileView->selectedItems() &&
                        !m_fileView->selectedItems()->isEmpty();
    myActionCollection->action( "trash" )->setEnabled( hasSelection );
    myActionCollection->action( "delete" )->setEnabled( hasSelection );
    myActionCollection->action( "properties" )->setEnabled( hasSelection );
}

void KDirOperator::setPreviewWidget(const QWidget *w)
{
    if(w != 0L)
        m_viewKind = (m_viewKind | KFile::PreviewContents);
    else
        m_viewKind = (m_viewKind & ~KFile::PreviewContents);

    delete myPreview;
    myPreview = w;

    KToggleAction *preview = static_cast<KToggleAction*>(myActionCollection->action("preview"));
    preview->setEnabled( w != 0L );
    preview->setChecked( w != 0L );
    setView( static_cast<KFile::FileView>(m_viewKind) );
}

const KFileItemList * KDirOperator::selectedItems() const
{
    return ( m_fileView ? m_fileView->selectedItems() : 0L );
}

bool KDirOperator::isSelected( const KFileItem *item ) const
{
    return ( m_fileView ? m_fileView->isSelected( item ) : false );
}

int KDirOperator::numDirs() const
{
    return m_fileView ? m_fileView->numDirs() : 0;
}

int KDirOperator::numFiles() const
{
    return m_fileView ? m_fileView->numFiles() : 0;
}

KCompletion * KDirOperator::completionObject() const
{
    return const_cast<KCompletion *>( &myCompletion );
}

KCompletion *KDirOperator::dirCompletionObject() const
{
    return const_cast<KCompletion *>( &myDirCompletion );
}

KActionCollection * KDirOperator::actionCollection() const
{
    return myActionCollection;
}

void KDirOperator::slotDetailedView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (m_viewKind & ~KFile::Simple) | KFile::Detail );
    setView( view );
}

void KDirOperator::slotSimpleView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (m_viewKind & ~KFile::Detail) | KFile::Simple );
    setView( view );
}

void KDirOperator::slotToggleHidden( bool show )
{
    dir->setShowingDotFiles( show );
    updateDir();
    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::slotSeparateDirs()
{
    if (separateDirsAction->isChecked())
    {
        KFile::FileView view = static_cast<KFile::FileView>( m_viewKind | KFile::SeparateDirs );
        setView( view );
    }
    else
    {
        KFile::FileView view = static_cast<KFile::FileView>( m_viewKind & ~KFile::SeparateDirs );
        setView( view );
    }
}

void KDirOperator::slotDefaultPreview()
{
    m_viewKind = m_viewKind | KFile::PreviewContents;
    if ( !myPreview ) {
        myPreview = new KFileMetaPreview( this );
        (static_cast<KToggleAction*>( myActionCollection->action("preview") ))->setChecked(true);
    }

    setView( static_cast<KFile::FileView>(m_viewKind) );
}

void KDirOperator::slotSortByName()
{
    QDir::SortFlags sorting = (m_fileView->sorting()) & ~QDir::SortByMask;
    m_fileView->setSorting( sorting | QDir::Name );
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( true );
}

void KDirOperator::slotSortBySize()
{
    QDir::SortFlags sorting = (m_fileView->sorting()) & ~QDir::SortByMask;
    m_fileView->setSorting( sorting | QDir::Size );
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( false );
}

void KDirOperator::slotSortByDate()
{
    QDir::SortFlags sorting = (m_fileView->sorting()) & ~QDir::SortByMask;
    m_fileView->setSorting( sorting | QDir::Time );
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( false );
}

void KDirOperator::slotSortReversed()
{
    if ( m_fileView )
        m_fileView->sortReversed();
}

void KDirOperator::slotToggleDirsFirst()
{
    if ( !m_fileView )
      return;

    QDir::SortFlags sorting = m_fileView->sorting();
    if ( !KFile::isSortDirsFirst( sorting ) )
        m_fileView->setSorting( sorting | QDir::DirsFirst );
    else
        m_fileView->setSorting( sorting & ~QDir::DirsFirst );
    mySorting = m_fileView->sorting();
}

void KDirOperator::slotToggleIgnoreCase()
{
    if ( !m_fileView )
      return;

    QDir::SortFlags sorting = m_fileView->sorting();
    if ( !KFile::isSortCaseInsensitive( sorting ) )
        m_fileView->setSorting( sorting | QDir::IgnoreCase );
    else
        m_fileView->setSorting( sorting & ~QDir::IgnoreCase );
    mySorting = m_fileView->sorting();
}

void KDirOperator::mkdir()
{
    bool ok;
    QString where = url().pathOrUrl();
    QString name = i18n( "New Folder" );
#ifdef Q_WS_WIN
    if ( url().isLocalFile() && QFileInfo( url().toLocalFile() + name ).exists() )
#else
    if ( url().isLocalFile() && QFileInfo( url().path(KUrl::AddTrailingSlash) + name ).exists() )
#endif
         name = KIO::RenameDialog::suggestName( url(), name );

    QString dir = KInputDialog::getText( i18n( "New Folder" ),
                                         i18n( "Create new folder in:\n%1" ,  where ),
                                         name, &ok, this);
    if (ok)
      mkdir( KIO::encodeFileName( dir ), true );
}

bool KDirOperator::mkdir( const QString& directory, bool enterDirectory )
{
    // Creates "directory", relative to the current directory (currUrl).
    // The given path may contain any number directories, existant or not.
    // They will all be created, if possible.

    bool writeOk = false;
    bool exists = false;
    KUrl url( currUrl );

    QStringList dirs = directory.split( QDir::separator(), QString::SkipEmptyParts );
    QStringList::ConstIterator it = dirs.begin();

    for ( ; it != dirs.end(); ++it )
    {
        url.addPath( *it );
        exists = KIO::NetAccess::exists( url, false, 0 );
        writeOk = !exists && KIO::NetAccess::mkdir( url, topLevelWidget() );
    }

    if ( exists ) // url was already existant
    {
        KMessageBox::sorry(viewWidget(), i18n("A file or folder named %1 already exists.", url.pathOrUrl()));
        enterDirectory = false;
    }
    else if ( !writeOk ) {
        KMessageBox::sorry(viewWidget(), i18n("You do not have permission to "
                                              "create that folder." ));
    }
    else if ( enterDirectory ) {
        setUrl( url, true );
    }

    return writeOk;
}

KIO::DeleteJob * KDirOperator::del( const KFileItemList& items,
                                    bool ask, bool showProgress )
{
    return del( items, this, ask, showProgress );
}

KIO::DeleteJob * KDirOperator::del( const KFileItemList& items,
                                    QWidget *parent,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( parent,
                                i18n("You did not select a file to delete."),
                                i18n("Nothing to Delete") );
        return 0L;
    }

    const KUrl::List urls = items.urlList();
    QStringList files;
    foreach( const KUrl& url, urls )
        files.append( url.pathOrUrl() );

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to delete\n <b>'%1'</b>?</qt>" ,
                  files.first() ),
                                                      i18n("Delete File"),
                                                      KStandardGuiItem::del(),
                                                      KStandardGuiItem::cancel(), "AskForDelete" );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18np("Do you really want to delete this item?", "Do you really want to delete these %1 items?", items.count() ),
                                                    files,
                                                    i18n("Delete Files"),
                                                    KStandardGuiItem::del(),
                                                    KStandardGuiItem::cancel(), "AskForDelete" );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt ) {
        KIO::DeleteJob *job = KIO::del( urls, false, showProgress );
        job->ui()->setWindow (topLevelWidget());
        job->ui()->setAutoErrorHandlingEnabled( true );
        return job;
    }

    return 0L;
}

void KDirOperator::deleteSelected()
{
    if ( !m_fileView )
        return;

    const KFileItemList *list = m_fileView->selectedItems();
    if ( list )
        del( *list );
}

KIO::CopyJob * KDirOperator::trash( const KFileItemList& items,
                                    QWidget *parent,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( parent,
                                i18n("You did not select a file to trash."),
                                i18n("Nothing to Trash") );
        return 0L;
    }

    const KUrl::List urls = items.urlList();
    QStringList files;
    foreach( const KUrl& url, urls )
        files.append( url.pathOrUrl() );

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to trash\n <b>'%1'</b>?</qt>" ,
                  files.first() ),
                                                      i18n("Trash File"),
                                                      KGuiItem(i18nc("to trash", "&Trash"),"edit-trash"),
                                                      KStandardGuiItem::cancel(), "AskForTrash" );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18np("translators: not called for n == 1", "Do you really want to trash these %1 items?", items.count() ),
                                                    files,
                                                    i18n("Trash Files"),
                                                    KGuiItem(i18nc("to trash", "&Trash"),"edit-trash"),
                                                    KStandardGuiItem::cancel(), "AskForTrash" );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt ) {
        KIO::CopyJob *job = KIO::trash( urls, showProgress );
        job->ui()->setWindow (topLevelWidget());
        job->ui()->setAutoErrorHandlingEnabled( true );
        return job;
    }

    return 0L;
}

void KDirOperator::trashSelected()
{
    if ( !m_fileView )
        return;

    /*if ( reason == KAction::PopupMenuActivation && ( modifiers & Qt::ShiftModifier ) ) {
        deleteSelected();
        return;
    }*/

    const KFileItemList *list = m_fileView->selectedItems();
    if ( list )
        trash( *list, this );
}

void KDirOperator::close()
{
    resetCursor();
    pendingMimeTypes.clear();
    myCompletion.clear();
    myDirCompletion.clear();
    myCompleteListDirty = true;
    dir->stop();
}

void KDirOperator::checkPath(const QString &, bool /*takeFiles*/) // SLOT
{
#if 0
    // copy the argument in a temporary string
    QString text = _txt;
    // it's unlikely to happen, that at the beginning are spaces, but
    // for the end, it happens quite often, I guess.
    text = text.trimmed();
    // if the argument is no URL (the check is quite fragil) and it's
    // no absolute path, we add the current directory to get a correct url
    if (text.find(':') < 0 && text[0] != '/')
        text.insert(0, currUrl);

    // in case we have a selection defined and someone patched the file-
    // name, we check, if the end of the new name is changed.
    if (!selection.isNull()) {
        int position = text.lastIndexOf('/');
        ASSERT(position >= 0); // we already inserted the current dir in case
        QString filename = text.mid(position + 1, text.length());
        if (filename != selection)
            selection = QString();
    }

    KUrl u(text); // I have to take care of entered URLs
    bool filenameEntered = false;

    if (u.isLocalFile()) {
        // the empty path is kind of a hack
        KFileItem i("", u.toLocalFile());
        if (i.isDir())
            setUrl(text, true);
        else {
            if (takeFiles)
                if (acceptOnlyExisting && !i.isFile())
                    warning("you entered an invalid URL");
                else
                    filenameEntered = true;
        }
    } else
        setUrl(text, true);

    if (filenameEntered) {
        filename_ = u.url();
        emit fileSelected(filename_);

        QApplication::restoreOverrideCursor();

        accept();
    }
#endif
    kDebug(kfile_area) << "TODO KDirOperator::checkPath()" << endl;
}

void KDirOperator::setUrl(const KUrl& _newurl, bool clearforward)
{
    KUrl newurl;

    if ( !_newurl.isValid() )
        newurl.setPath( QDir::homePath() );
    else
        newurl = _newurl;

#ifdef Q_WS_WIN
    QString pathstr = newurl.toLocalFile();
#else
    QString pathstr = newurl.path(KUrl::AddTrailingSlash);
#endif
    newurl.setPath(pathstr);

    // already set
    if ( newurl.equals( currUrl, KUrl::CompareWithoutTrailingSlash ) )
        return;

    if ( !isReadable( newurl ) ) {
        // maybe newurl is a file? check its parent directory
        newurl.cd(QLatin1String(".."));
        if ( !isReadable( newurl ) ) {
            resetCursor();
            KMessageBox::error(viewWidget(),
                               i18n("The specified folder does not exist "
                                    "or was not readable."));
            return;
        }
    }

    if (clearforward) {
        // autodelete should remove this one
        backStack.push(new KUrl(currUrl));
        qDeleteAll( forwardStack );
        forwardStack.clear();
    }

    d->lastURL = currUrl.url(KUrl::RemoveTrailingSlash);
    currUrl = newurl;

    pathChanged();
    emit urlEntered(newurl);

    // enable/disable actions
    forwardAction->setEnabled( !forwardStack.isEmpty() );
    backAction->setEnabled( !backStack.isEmpty() );
    upAction->setEnabled( !isRoot() );

    openUrl( newurl );
}

void KDirOperator::updateDir()
{
    dir->emitChanges();
    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::rereadDir()
{
    pathChanged();
    openUrl( currUrl, false, true );
}


bool KDirOperator::openUrl( const KUrl& url, bool keep, bool reload )
{
    bool result = dir->openUrl( url, keep, reload );
    if ( !result ) // in that case, neither completed() nor canceled() will be emitted by KDL
        slotCanceled();

    return result;
}

// Protected
void KDirOperator::pathChanged()
{
    if (!m_fileView)
        return;

    pendingMimeTypes.clear();
    m_fileView->clear();
    myCompletion.clear();
    myDirCompletion.clear();

    // it may be, that we weren't ready at this time
    QApplication::restoreOverrideCursor();

    // when KIO::Job emits finished, the slot will restore the cursor
    QApplication::setOverrideCursor( Qt::WaitCursor );

    if ( !isReadable( currUrl )) {
        KMessageBox::error(viewWidget(),
                           i18n("The specified folder does not exist "
                                "or was not readable."));
        if (backStack.isEmpty())
            home();
        else
            back();
    }
}

void KDirOperator::slotRedirected( const KUrl& newURL )
{
    currUrl = newURL;
    pendingMimeTypes.clear();
    myCompletion.clear();
    myDirCompletion.clear();
    myCompleteListDirty = true;
    emit urlEntered( newURL );
}

// Code pinched from kfm then hacked
void KDirOperator::back()
{
    if ( backStack.isEmpty() )
        return;

    forwardStack.push( new KUrl(currUrl) );

    KUrl *s = backStack.pop();

    setUrl(*s, false);
    delete s;
}

// Code pinched from kfm then hacked
void KDirOperator::forward()
{
    if ( forwardStack.isEmpty() )
        return;

    backStack.push(new KUrl(currUrl));

    KUrl *s = forwardStack.pop();
    setUrl(*s, false);
    delete s;
}

KUrl KDirOperator::url() const
{
    return currUrl;
}

void KDirOperator::cdUp()
{
    KUrl tmp(currUrl);
    tmp.cd(QLatin1String(".."));
    setUrl(tmp, true);
}

void KDirOperator::home()
{
    KUrl u;
    u.setPath( QDir::homePath() );
    setUrl(u, true);
}

void KDirOperator::clearFilter()
{
    dir->setNameFilter( QString() );
    dir->clearMimeFilter();
    checkPreviewSupport();
}

void KDirOperator::setNameFilter(const QString& filter)
{
    dir->setNameFilter(filter);
    checkPreviewSupport();
}

QString KDirOperator::nameFilter() const
{
    return dir->nameFilter();
}

void KDirOperator::setMimeFilter( const QStringList& mimetypes )
{
    dir->setMimeFilter( mimetypes );
    checkPreviewSupport();
}

QStringList KDirOperator::mimeFilter() const
{
    return dir->mimeFilters();
}

bool KDirOperator::checkPreviewSupport()
{
    KToggleAction *previewAction = static_cast<KToggleAction*>( myActionCollection->action( "preview" ));

    bool hasPreviewSupport = false;
    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    if ( cg.readEntry( "Show Default Preview", true ) )
        hasPreviewSupport = checkPreviewInternal();

    previewAction->setEnabled( hasPreviewSupport );
    return hasPreviewSupport;
}

bool KDirOperator::checkPreviewInternal() const
{
    QStringList supported = KIO::PreviewJob::supportedMimeTypes();
    // no preview support for directories?
    if ( dirOnlyMode() && supported.indexOf( "inode/directory" ) == -1 )
        return false;

    QStringList mimeTypes = dir->mimeFilters();
    QStringList nameFilter = dir->nameFilter().split( " ", QString::SkipEmptyParts );

    if ( mimeTypes.isEmpty() && nameFilter.isEmpty() && !supported.isEmpty() )
        return true;
    else {
        QRegExp r;
        r.setPatternSyntax( QRegExp::Wildcard ); // the "mimetype" can be "image/*"

        if ( !mimeTypes.isEmpty() ) {
            QStringList::Iterator it = supported.begin();

            for ( ; it != supported.end(); ++it ) {
                r.setPattern( *it );

                QStringList result = mimeTypes.filter( r );
                if ( !result.isEmpty() ) { // matches! -> we want previews
                    return true;
                }
            }
        }

        if ( !nameFilter.isEmpty() ) {
            // find the mimetypes of all the filter-patterns and
            KMimeTypeFactory *fac = KMimeTypeFactory::self();
            QStringList::Iterator it1 = nameFilter.begin();
            for ( ; it1 != nameFilter.end(); ++it1 ) {
                if ( (*it1) == "*" ) {
                    return true;
                }

                KMimeType::Ptr mt = fac->findFromPattern( *it1 );
                if ( !mt )
                    continue;
                QString mime = mt->name();

                // the "mimetypes" we get from the PreviewJob can be "image/*"
                // so we need to check in wildcard mode
                QStringList::Iterator it2 = supported.begin();
                for ( ; it2 != supported.end(); ++it2 ) {
                    r.setPattern( *it2 );
                    if ( r.indexIn( mime ) != -1 ) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

KFileView* KDirOperator::createView( QWidget* parent, KFile::FileView view )
{
    KFileView* new_view = 0L;
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview = ( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if ( separateDirs || preview ) {
        KCombiView *combi = 0L;
        if (separateDirs)
        {
            combi = new KCombiView( parent );
            combi->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
        }

        KFileView* v = 0L;
        if ( KFile::isSimpleView( view ) )
            v = createView( combi, KFile::Simple );
        else
            v = createView( combi, KFile::Detail );

        v->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);

        if (combi)
            combi->setRight( v );

        if (preview)
        {
            KFilePreview* pView = new KFilePreview( combi ? combi : v, parent );
            pView->setObjectName( QLatin1String( "preview" ) );
            pView->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
            new_view = pView;
        }
        else
            new_view = combi;
    }
    else if ( KFile::isDetailView( view ) && !preview ) {
        K3FileDetailView *ndw = new K3FileDetailView( parent );
        ndw->setObjectName( "detail view" );
        new_view = ndw;
        new_view->setViewName( i18n("Detailed View") );
    }
    else /* if ( KFile::isSimpleView( view ) && !preview ) */ {
        K3FileIconView* iconView = new K3FileIconView(parent, "simple view");
        new_view = iconView;
        new_view->setViewName( i18n("Short View") );
    }

    new_view->widget()->setAcceptDrops(acceptDrops());
    return new_view;
}

void KDirOperator::setAcceptDrops(bool b)
{
    if (m_fileView)
       m_fileView->widget()->setAcceptDrops(b);
    QWidget::setAcceptDrops(b);
}

void KDirOperator::setDropOptions(int options)
{
    d->dropOptions = options;
    if (m_fileView)
       m_fileView->setDropOptions(options);
}

void KDirOperator::setView( KFile::FileView view )
{
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview=( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if (view == KFile::Default) {
        if ( KFile::isDetailView( (KFile::FileView) defaultView ) )
            view = KFile::Detail;
        else
            view = KFile::Simple;

        separateDirs = KFile::isSeparateDirs( static_cast<KFile::FileView>(defaultView) );
        preview = ( KFile::isPreviewInfo( static_cast<KFile::FileView>(defaultView) ) ||
                    KFile::isPreviewContents( static_cast<KFile::FileView>(defaultView) ) )
                  && myActionCollection->action("preview")->isEnabled();

        if ( preview ) { // instantiates KFileMetaPreview and calls setView()
            m_viewKind = defaultView;
            slotDefaultPreview();
            return;
        }
        else if ( !separateDirs )
            separateDirsAction->setChecked(true);
    }

    // if we don't have any files, we can't separate dirs from files :)
    if ( (mode() & KFile::File) == 0 &&
         (mode() & KFile::Files) == 0 ) {
        separateDirs = false;
        separateDirsAction->setEnabled( false );
    }

    m_viewKind = static_cast<int>(view) | (separateDirs ? KFile::SeparateDirs : 0);
    view = static_cast<KFile::FileView>(m_viewKind);

    KFileView *new_view = createView( this, view );
    if ( preview ) {
        // we keep the preview-_widget_ around, but not the KFilePreview.
        // KFilePreview::setPreviewWidget handles the reparenting for us
        static_cast<KFilePreview*>(new_view)->setPreviewWidget(myPreview, url());
    }

    setView( new_view );
}

KFileView * KDirOperator::view() const
{
    return m_fileView;
}

QWidget * KDirOperator::viewWidget() const
{
    return m_fileView ? m_fileView->widget() : 0L;
}


void KDirOperator::connectView(KFileView *view)
{
    // TODO: do a real timer and restart it after that
    pendingMimeTypes.clear();
    bool listDir = true;

    if ( dirOnlyMode() )
         view->setViewMode(KFileView::Directories);
    else
        view->setViewMode(KFileView::All);

    if ( myMode & KFile::Files )
        view->setSelectionMode( KFile::Extended );
    else
        view->setSelectionMode( KFile::Single );

    if (m_fileView)
    {
        if ( d->configGroup ) // save and restore the views' configuration
        {
            m_fileView->writeConfig(d->configGroup);
            view->readConfig(d->configGroup);
        }

        // transfer the state from old view to new view
        view->clear();
        view->addItemList( *m_fileView->items() );
        listDir = false;

        if ( m_fileView->widget()->hasFocus() )
            view->widget()->setFocus();

        KFileItem *oldCurrentItem = m_fileView->currentFileItem();
        if ( oldCurrentItem ) {
            view->setCurrentItem( oldCurrentItem );
            view->setSelected( oldCurrentItem, false );
            view->ensureItemVisible( oldCurrentItem );
        }

        const KFileItemList *oldSelected = m_fileView->selectedItems();
        if ( !oldSelected->isEmpty() ) {
            KFileItemList::const_iterator kit = oldSelected->begin();
            const KFileItemList::const_iterator kend = oldSelected->end();
            for ( ; kit != kend; ++kit )
                view->setSelected( *kit, true );
        }

        m_fileView->widget()->hide();
        delete m_fileView;
    }

    else
    {
        if ( d->configGroup )
            view->readConfig( d->configGroup );
    }

    m_fileView = view;
    m_fileView->setDropOptions(d->dropOptions);
    viewActionCollection = 0L;
    KFileViewSignaler *sig = view->signaler();

    connect(sig, SIGNAL( activatedMenu(const KFileItem *, const QPoint& ) ),
            this, SLOT( activatedMenu(const KFileItem *, const QPoint& )));
    connect(sig, SIGNAL( dirActivated(const KFileItem *) ),
            this, SLOT( selectDir(const KFileItem*) ) );
    connect(sig, SIGNAL( fileSelected(const KFileItem *) ),
            this, SLOT( selectFile(const KFileItem*) ) );
    connect(sig, SIGNAL( fileHighlighted(const KFileItem *) ),
            this, SLOT( highlightFile(const KFileItem*) ));
    connect(sig, SIGNAL( sortingChanged( QDir::SortFlags ) ),
            this, SLOT( slotViewSortingChanged( QDir::SortFlags )));
    connect(sig, SIGNAL( dropped(const KFileItem *, QDropEvent*, const KUrl::List&) ),
            this, SIGNAL( dropped(const KFileItem *, QDropEvent*, const KUrl::List&)) );

    if ( reverseAction->isChecked() != m_fileView->isReversed() )
        slotSortReversed();

    updateViewActions();
    m_fileView->widget()->resize(size());
    m_fileView->widget()->show();

    if ( listDir ) {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        openUrl( currUrl );
    }
    else
        view->listingCompleted();
}

KFile::Modes KDirOperator::mode() const
{
    return myMode;
}

void KDirOperator::setMode(KFile::Modes m)
{
    if (myMode == m)
        return;

    myMode = m;

    dir->setDirOnlyMode( dirOnlyMode() );

    // reset the view with the different mode
    setView( static_cast<KFile::FileView>(m_viewKind) );
}

void KDirOperator::setView(KFileView *view)
{
    if ( view == m_fileView ) {
        return;
    }

    setFocusProxy(view->widget());
    view->setSorting( mySorting );
    view->setOnlyDoubleClickSelectsFiles( d->onlyDoubleClickSelectsFiles );
    connectView(view); // also deletes the old view

    emit viewChanged( view );
}

void KDirOperator::setDirLister( KDirLister *lister )
{
    if ( lister == dir ) // sanity check
        return;

    delete dir;
    dir = lister;

    dir->setAutoUpdate( true );
    dir->setDelayedMimeTypes(true);

    QWidget* mainWidget = topLevelWidget();
    dir->setMainWindow (mainWidget);
    kDebug (kfile_area) << "mainWidget=" << mainWidget << endl;

    connect( dir, SIGNAL( percent( int )),
             SLOT( slotProgress( int ) ));
    connect( dir, SIGNAL(started( const KUrl& )), SLOT(slotStarted()));
    connect( dir, SIGNAL(newItems(const KFileItemList &)),
             SLOT(insertNewFiles(const KFileItemList &)));
    connect( dir, SIGNAL(completed()), SLOT(slotIOFinished()));
    connect( dir, SIGNAL(canceled()), SLOT(slotCanceled()));
    connect( dir, SIGNAL(deleteItem(KFileItem *)),
             SLOT(itemDeleted(KFileItem *)));
    connect( dir, SIGNAL(redirection( const KUrl& )),
             SLOT( slotRedirected( const KUrl& )));
    connect( dir, SIGNAL( clear() ), SLOT( slotClearView() ));
    connect( dir, SIGNAL( refreshItems( const KFileItemList& ) ),
             SLOT( slotRefreshItems( const KFileItemList& ) ) );
}

void KDirOperator::insertNewFiles(const KFileItemList &newone)
{
    if ( newone.isEmpty() || !m_fileView )
        return;

    myCompleteListDirty = true;
    m_fileView->addItemList( newone );
    emit updateInformation(m_fileView->numDirs(), m_fileView->numFiles());

    KFileItemList::const_iterator kit = newone.begin();
    const KFileItemList::const_iterator kend = newone.end();
    for ( ; kit != kend; ++kit ) {
        const KFileItem* item = *kit;
        // highlight the dir we come from, if possible
        if ( d->dirHighlighting && item->isDir() &&
                    item->url().url(KUrl::RemoveTrailingSlash) == d->lastURL ) {
            m_fileView->setCurrentItem( item );
            m_fileView->ensureItemVisible( item );
        }
    }

    QTimer::singleShot(200, this, SLOT(resetCursor()));
}

void KDirOperator::selectDir(const KFileItem *item)
{
    setUrl(item->url(), true);
}

void KDirOperator::itemDeleted(KFileItem *item)
{
    pendingMimeTypes.removeAll( item );
    if ( m_fileView )
    {
        m_fileView->removeItem( item );
        emit updateInformation(m_fileView->numDirs(), m_fileView->numFiles());
    }
}

void KDirOperator::selectFile(const KFileItem *item)
{
    QApplication::restoreOverrideCursor();

    emit fileSelected( item );
}

void KDirOperator::highlightFile(const KFileItem* i)
{
    fileHighlighted( i );
}

void KDirOperator::setCurrentItem( const QString& filename )
{
    if ( m_fileView ) {
        const KFileItem *item = 0;

        if ( !filename.isNull() )
            item = dir->findByName( filename );

        m_fileView->clearSelection();
        if ( item ) {
            m_fileView->setCurrentItem( item );
            m_fileView->setSelected( item, true );
            m_fileView->ensureItemVisible( item );
        }
    }
}

QString KDirOperator::makeCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        m_fileView->clearSelection();
        return QString();
    }

    prepareCompletionObjects();
    return myCompletion.makeCompletion( string );
}

QString KDirOperator::makeDirCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        m_fileView->clearSelection();
        return QString();
    }

    prepareCompletionObjects();
    return myDirCompletion.makeCompletion( string );
}

void KDirOperator::prepareCompletionObjects()
{
    if ( !m_fileView )
        return;

    if ( myCompleteListDirty ) { // create the list of all possible completions
        const KFileItemList* itemList = m_fileView->items();
        KFileItemList::const_iterator kit = itemList->begin();
        const KFileItemList::const_iterator kend = itemList->end();
        for ( ; kit != kend; ++kit ) {
            KFileItem *item = *kit;
            myCompletion.addItem( item->name() );
            if ( item->isDir() )
                myDirCompletion.addItem( item->name() );
        }
        myCompleteListDirty = false;
    }
}

void KDirOperator::slotCompletionMatch(const QString& match)
{
    setCurrentItem( match );
    emit completion( match );
}

void KDirOperator::setupActions()
{
    myActionCollection = new KActionCollection( this );
    myActionCollection->setObjectName( "KDirOperator::myActionCollection" );
    myActionCollection->setAssociatedWidget( topLevelWidget() );

    actionMenu = new KActionMenu( i18n("Menu"), this );
    myActionCollection->addAction( "popupMenu", actionMenu );

    upAction = myActionCollection->addAction( KStandardAction::Up, "up", this, SLOT( cdUp() ) );
    upAction->setText( i18n("Parent Folder") );

    backAction = myActionCollection->addAction( KStandardAction::Back, "back", this, SLOT( back() ) );

    forwardAction = myActionCollection->addAction( KStandardAction::Forward, "forward", this, SLOT( forward() ) );

    homeAction = myActionCollection->addAction( KStandardAction::Home, "home", this, SLOT( home() ) );
    homeAction->setText(i18n("Home Folder"));

    reloadAction = myActionCollection->addAction( KStandardAction::Redisplay, "reload", this, SLOT( rereadDir() ) );
    reloadAction->setText( i18n("Reload") );
    reloadAction->setShortcuts( KStandardShortcut::shortcut( KStandardShortcut::Reload ));

    actionSeparator = new QAction( this );
    actionSeparator->setSeparator(true);
    myActionCollection->addAction( "separator", actionSeparator );


    mkdirAction = new KAction( i18n("New Folder..."), this );
    myActionCollection->addAction( "mkdir", mkdirAction );
    mkdirAction->setIcon( KIcon( QLatin1String("folder-new") ) );
    connect( mkdirAction, SIGNAL( triggered( bool ) ), this, SLOT( mkdir() ) );

    KAction* trash = new KAction( i18n( "Move to Trash" ), this );
    myActionCollection->addAction( "trash", trash );
    trash->setIcon( KIcon( "edit-trash" ) );
    trash->setShortcuts( KShortcut(Qt::Key_Delete) );
    connect( trash, SIGNAL( triggered(bool) ), SLOT( trashSelected() ) );

    KAction* action = new KAction( i18n( "Delete" ), this );
    myActionCollection->addAction( "delete", action );
    action->setIcon( KIcon( "edit-delete" ) );
    action->setShortcuts( KShortcut(Qt::SHIFT+Qt::Key_Delete) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( deleteSelected() ) );

    // the sort menu actions
    sortActionMenu = new KActionMenu( i18n("Sorting"), this );
    myActionCollection->addAction( "sorting menu",  sortActionMenu );

    byNameAction = new KAction( i18n("By Name"), this );
    myActionCollection->addAction( "by name", byNameAction );
    connect( byNameAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortByName() ) );

    byDateAction = new KAction( i18n("By Date"), this );
    myActionCollection->addAction( "by date", byDateAction );
    connect( byDateAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortByDate() ) );

    bySizeAction = new KAction( i18n("By Size"), this );
    myActionCollection->addAction( "by size", bySizeAction );
    connect( bySizeAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortBySize() ) );

    reverseAction = new KToggleAction( i18n("Reverse"), this );
    myActionCollection->addAction( "reversed", reverseAction );
    connect( reverseAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortReversed() ) );

    QActionGroup* sortGroup = new QActionGroup(this);
    byNameAction->setActionGroup(sortGroup);
    byDateAction->setActionGroup(sortGroup);
    bySizeAction->setActionGroup(sortGroup);

    dirsFirstAction = new KToggleAction( i18n("Folders First"), this );
    myActionCollection->addAction( "dirs first", dirsFirstAction );
    connect( dirsFirstAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleDirsFirst() ));

    caseInsensitiveAction = new KToggleAction(i18n("Case Insensitive"), this );
    myActionCollection->addAction( "case insensitive", caseInsensitiveAction );
    connect( caseInsensitiveAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleIgnoreCase() ));

    // the view menu actions
    viewActionMenu = new KActionMenu( i18n("&View"), this );
    myActionCollection->addAction( "view menu", viewActionMenu );
    connect( viewActionMenu->menu(), SIGNAL( aboutToShow() ), SLOT( insertViewDependentActions() ));

    shortAction = new KAction(i18n("Short View"), this );
    myActionCollection->addAction( "short view",  shortAction );
    shortAction->setIcon( KIcon( QLatin1String("fileview-multicolumn") ) );
    connect( shortAction, SIGNAL( activated() ), SLOT( slotSimpleView() ));

    detailedAction = new KAction( i18n("Detailed View"), this );
    myActionCollection->addAction( "detailed view", detailedAction );
    detailedAction->setIcon( KIcon( QLatin1String("fileview-detailed") ) );
    connect( detailedAction, SIGNAL( activated() ), SLOT( slotDetailedView() ));

    showHiddenAction = new KToggleAction( i18n("Show Hidden Files"), this );
    myActionCollection->addAction( "show hidden", showHiddenAction );
    connect( showHiddenAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleHidden( bool ) ));

    separateDirsAction = new KToggleAction( i18n("Separate Folders"), this );
    myActionCollection->addAction( "separate dirs", separateDirsAction );
    connect( separateDirsAction, SIGNAL( triggered( bool ) ), this, SLOT(slotSeparateDirs()) );

    KToggleAction *previewAction = new KToggleAction(i18n("Show Preview"), this );
    myActionCollection->addAction( "preview", previewAction );
    previewAction->setIcon( KIcon( "thumbnail-show" ) );
    connect( previewAction, SIGNAL( toggled( bool )),
             SLOT( togglePreview( bool )));


    QActionGroup* detailGroup = new QActionGroup(this);
    shortAction->setActionGroup( detailGroup );
    detailedAction->setActionGroup( detailGroup );

    action = new KAction( i18n("Properties"), this );
    myActionCollection->addAction( "properties", action );
    action->setShortcut( KShortcut(Qt::ALT+Qt::Key_Return) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT(slotProperties()) );
}

void KDirOperator::setupMenu()
{
    setupMenu(AllActions);
}

void KDirOperator::setupMenu(int whichActions)
{
    // first fill the submenus (sort and view)
    sortActionMenu->menu()->clear();
    sortActionMenu->addAction( byNameAction );
    sortActionMenu->addAction( byDateAction );
    sortActionMenu->addAction( bySizeAction );
    sortActionMenu->addAction( actionSeparator );
    sortActionMenu->addAction( reverseAction );
    sortActionMenu->addAction( dirsFirstAction );
    sortActionMenu->addAction( caseInsensitiveAction );

    // now plug everything into the popupmenu
    actionMenu->menu()->clear();
    if (whichActions & NavActions)
    {
        actionMenu->addAction( upAction );
        actionMenu->addAction( backAction );
        actionMenu->addAction( forwardAction );
        actionMenu->addAction( homeAction );
        actionMenu->addAction( actionSeparator );
    }

    if (whichActions & FileActions)
    {
        actionMenu->addAction( mkdirAction );
        if (currUrl.isLocalFile() && !(QApplication::keyboardModifiers() & Qt::ShiftModifier))
            actionMenu->addAction( myActionCollection->action( "trash" ) );
        KConfigGroup cg( KGlobal::config(), QLatin1String("KDE") );
        if (!currUrl.isLocalFile() || (QApplication::keyboardModifiers() & Qt::ShiftModifier) ||
            cg.readEntry("ShowDeleteCommand", false))
            actionMenu->addAction( myActionCollection->action( "delete" ) );
        actionMenu->addAction( actionSeparator );
    }

    if (whichActions & SortActions)
    {
        actionMenu->addAction( sortActionMenu );
        actionMenu->addAction( actionSeparator );
    }

    if (whichActions & ViewActions)
    {
        actionMenu->addAction( viewActionMenu );
        actionMenu->addAction( actionSeparator );
    }

    if (whichActions & FileActions)
    {
        actionMenu->addAction( myActionCollection->action( "properties" ) );
    }
}

void KDirOperator::updateSortActions()
{
    if ( KFile::isSortByName( mySorting ) )
        byNameAction->setChecked( true );
    else if ( KFile::isSortByDate( mySorting ) )
        byDateAction->setChecked( true );
    else if ( KFile::isSortBySize( mySorting ) )
        bySizeAction->setChecked( true );

    dirsFirstAction->setChecked( KFile::isSortDirsFirst( mySorting ) );
    caseInsensitiveAction->setChecked( KFile::isSortCaseInsensitive(mySorting) );
    caseInsensitiveAction->setEnabled( KFile::isSortByName( mySorting ) );

    if ( m_fileView )
        reverseAction->setChecked( m_fileView->isReversed() );
}

void KDirOperator::updateViewActions()
{
    KFile::FileView fv = static_cast<KFile::FileView>( m_viewKind );

    separateDirsAction->setChecked( KFile::isSeparateDirs( fv ) &&
                                    separateDirsAction->isEnabled() );

    shortAction->setChecked( KFile::isSimpleView( fv ));
    detailedAction->setChecked( KFile::isDetailView( fv ));
}

void KDirOperator::readConfig( const KConfigGroup& configGroup)
{
    defaultView = 0;
    QDir::SortFlags sorting = QDir::Name;

    QString viewStyle = configGroup.readEntry( "View Style", "Simple" );
    if ( viewStyle == QLatin1String("Detail") )
        defaultView |= KFile::Detail;
    else
        defaultView |= KFile::Simple;
    if ( configGroup.readEntry( QLatin1String("Separate Directories"),
                        DefaultMixDirsAndFiles ) )
        defaultView |= KFile::SeparateDirs;
    if ( configGroup.readEntry(QLatin1String("Show Preview"), false) )
        defaultView |= KFile::PreviewContents;

    if ( configGroup.readEntry( QLatin1String("Sort case insensitively"),
                        DefaultCaseInsensitive ) )
        sorting |= QDir::IgnoreCase;
    if ( configGroup.readEntry( QLatin1String("Sort directories first"),
                        DefaultDirsFirst ) )
        sorting |= QDir::DirsFirst;


    QString name = QLatin1String("Name");
    QString sortBy = configGroup.readEntry( QLatin1String("Sort by"), name );
    if ( sortBy == name )
        sorting |= QDir::Name;
    else if ( sortBy == QLatin1String("Size") )
        sorting |= QDir::Size;
    else if ( sortBy == QLatin1String("Date") )
        sorting |= QDir::Time;

    mySorting = sorting;
    setSorting( mySorting );


    if ( configGroup.readEntry( QLatin1String("Show hidden files"),
                        DefaultShowHidden ) ) {
         showHiddenAction->setChecked( true );
         dir->setShowingDotFiles( true );
    }
    if ( configGroup.readEntry( QLatin1String("Sort reversed"),
                        DefaultSortReversed ) )
        reverseAction->setChecked( true );

}

void KDirOperator::writeConfig( KConfigGroup& configGroup)
{
    QString sortBy = QLatin1String("Name");
    if ( KFile::isSortBySize( mySorting ) )
        sortBy = QLatin1String("Size");
    else if ( KFile::isSortByDate( mySorting ) )
        sortBy = QLatin1String("Date");
    configGroup.writeEntry( QLatin1String("Sort by"), sortBy );

    configGroup.writeEntry( QLatin1String("Sort reversed"),
                    reverseAction->isChecked() );
    configGroup.writeEntry( QLatin1String("Sort case insensitively"),
                    caseInsensitiveAction->isChecked() );
    configGroup.writeEntry( QLatin1String("Sort directories first"),
                    dirsFirstAction->isChecked() );

    // don't save the separate dirs or preview when an application specific
    // preview is in use.
    bool appSpecificPreview = false;
    if ( myPreview ) {
        QWidget *preview = const_cast<QWidget*>( myPreview ); // grmbl
        KFileMetaPreview *tmp = dynamic_cast<KFileMetaPreview*>( preview );
        appSpecificPreview = (tmp == 0L);
    }

    if ( !appSpecificPreview ) {
        if ( separateDirsAction->isEnabled() )
            configGroup.writeEntry( QLatin1String("Separate Directories"),
                            separateDirsAction->isChecked() );

        KToggleAction *previewAction = static_cast<KToggleAction*>(myActionCollection->action("preview"));
        if ( previewAction->isEnabled() ) {
            bool hasPreview = previewAction->isChecked();
            configGroup.writeEntry( QLatin1String("Show Preview"), hasPreview );
        }
    }

    configGroup.writeEntry( QLatin1String("Show hidden files"),
                    showHiddenAction->isChecked() );

    KFile::FileView fv = static_cast<KFile::FileView>( m_viewKind );
    QString style;
    if ( KFile::isDetailView( fv ) )
        style = QLatin1String("Detail");
    else if ( KFile::isSimpleView( fv ) )
        style = QLatin1String("Simple");
    configGroup.writeEntry( QLatin1String("View Style"), style );

}


void KDirOperator::resizeEvent( QResizeEvent * )
{
    if (m_fileView)
        m_fileView->widget()->resize( size() );

    if ( progress->parent() == this ) // might be reparented into a statusbar
        progress->move(2, height() - progress->height() -2);
}

void KDirOperator::setOnlyDoubleClickSelectsFiles( bool enable )
{
    d->onlyDoubleClickSelectsFiles = enable;
    if ( m_fileView )
        m_fileView->setOnlyDoubleClickSelectsFiles( enable );
}

bool KDirOperator::onlyDoubleClickSelectsFiles() const
{
    return d->onlyDoubleClickSelectsFiles;
}

void KDirOperator::slotStarted()
{
    progress->setValue( 0 );
    // delay showing the progressbar for one second
    d->progressDelayTimer->setSingleShot( true );
    d->progressDelayTimer->start( 1000 );
}

void KDirOperator::slotShowProgress()
{
    progress->raise();
    progress->show();
    QApplication::flush();
}

void KDirOperator::slotProgress( int percent )
{
    progress->setValue( percent );
    // we have to redraw this as fast as possible
    if ( progress->isVisible() )
        QApplication::flush();
}


void KDirOperator::slotIOFinished()
{
    d->progressDelayTimer->stop();
    slotProgress( 100 );
    progress->hide();
    emit finishedLoading();
    resetCursor();

    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::slotCanceled()
{
    emit finishedLoading();
    resetCursor();

    if ( m_fileView )
        m_fileView->listingCompleted();
}

QProgressBar * KDirOperator::progressBar() const
{
    return progress;
}

void KDirOperator::clearHistory()
{
    qDeleteAll( backStack );
    backStack.clear();
    backAction->setEnabled( false );
    qDeleteAll( forwardStack );
    forwardStack.clear();
    forwardAction->setEnabled( false );
}

void KDirOperator::slotViewActionAdded( KAction *action )
{
    if ( viewActionMenu->menu()->actions().count() == 5 ) // need to add a separator
        viewActionMenu->addAction( d->viewActionSeparator );

    viewActionMenu->addAction( action );
}

void KDirOperator::slotViewActionRemoved( KAction *action )
{
    viewActionMenu->removeAction( action );

    if ( viewActionMenu->menu()->actions().count() == 6 ) // remove the separator
        viewActionMenu->removeAction( d->viewActionSeparator );
}

void KDirOperator::slotViewSortingChanged( QDir::SortFlags sort )
{
    mySorting = sort;
    updateSortActions();
}

void KDirOperator::setEnableDirHighlighting( bool enable )
{
    d->dirHighlighting = enable;
}

bool KDirOperator::dirHighlighting() const
{
    return d->dirHighlighting;
}

bool KDirOperator::dirOnlyMode() const
{
    return dirOnlyMode( myMode );
}

bool KDirOperator::dirOnlyMode( uint mode )
{
    return ( (mode & KFile::Directory) &&
        (mode & (KFile::File | KFile::Files)) == 0 );
}

void KDirOperator::slotProperties()
{
    if ( m_fileView ) {
        const KFileItemList *list = m_fileView->selectedItems();
        if ( !list->isEmpty() ) {
            KPropertiesDialog::showDialog( *list, this );
        }
    }
}

void KDirOperator::slotClearView()
{
    if ( m_fileView )
        m_fileView->clearView();
}

// ### temporary code
#include <dirent.h>
bool KDirOperator::isReadable( const KUrl& url )
{
    if ( !url.isLocalFile() )
        return true; // what else can we say?

    KDE_struct_stat buf;
#ifdef Q_WS_WIN
    QString ts = url.toLocalFile();
#else
    QString ts = url.path(KUrl::AddTrailingSlash);
#endif
    bool readable = ( KDE_stat( QFile::encodeName( ts ), &buf) == 0 );
    if (readable) { // further checks
        DIR *test;
        test = opendir( QFile::encodeName( ts )); // we do it just to test here
        readable = (test != 0);
        if (test)
            closedir(test);
    }
    return readable;
}

void KDirOperator::togglePreview( bool on )
{
    if ( on )
        slotDefaultPreview();
    else
        setView( (KFile::FileView) (m_viewKind & ~(KFile::PreviewContents|KFile::PreviewInfo)) );
}

void KDirOperator::slotRefreshItems( const KFileItemList& items )
{
    if ( !m_fileView )
        return;

    KFileItemList::const_iterator kit = items.begin();
    const KFileItemList::const_iterator kend = items.end();
    for ( ; kit != kend; ++kit )
        m_fileView->updateView( *kit );
}

void KDirOperator::setViewConfig( KConfigGroup& configGroup)
{
    delete d->configGroup;
    d->configGroup = new KConfigGroup(configGroup);
}

KConfigGroup* KDirOperator::viewConfigGroup() const
{
    return d->configGroup;
}

void KDirOperator::setShowHiddenFiles( bool s )
{
    showHiddenAction->setChecked( s );
}

bool KDirOperator::showHiddenFiles( ) const
{
    return showHiddenAction->isChecked();
}

#include "kdiroperator.moc"
