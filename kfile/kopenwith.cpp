/*  This file is part of the KDE libraries

    Copyright (C) 1997 Torben Weis <weis@stud.uni-frankfurt.de>
    Copyright (C) 1999 Dirk A. Mueller <dmuell@gmx.net>
    Portions copyright (C) 1999 Preston Brown <pbrown@kde.org>

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

// $Id$

#include <qfile.h>
#include <qdir.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <kapp.h>
#include <kbuttonbox.h>
#include <kdesktopfile.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimemagic.h>
#include <krun.h>
#include <kstddirs.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <dcopclient.h>
#include <kmimetype.h>
#include <klistview.h>

#include "kopenwith.h"

template class QList<QString>;

#define SORT_SPEC (QDir::DirsFirst | QDir::Name | QDir::IgnoreCase)


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( KListView* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, QString p, QString c )
    : QListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( QListViewItem* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, QString p, QString c )
    : QListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

void KAppTreeListItem::init(const QPixmap& pixmap, bool parse, bool dir, QString _path, QString _exec)
{
    setPixmap(0, pixmap);
    parsed = parse;
    directory = dir;
    path = _path; // relative path
    exec = _exec;
    exec.simplifyWhiteSpace();
    exec.truncate(exec.find(' '));
}


// ----------------------------------------------------------------------
// Ensure that dirs are sorted in front of files and case is ignored

QString KAppTreeListItem::key(int column, bool /*ascending*/) const
{
    if(directory)
        return QString::fromLatin1(" ") + text(column).upper();
    else
        return text(column).upper();
}

void KAppTreeListItem::activate()
{
    if ( directory )
        setOpen(!isOpen());
}

void KAppTreeListItem::setOpen( bool o )
{
    if( o && !parsed ) { // fill the children before opening
        ((KApplicationTree *) parent())->parseDesktopDir( path + QString::fromLatin1("/"), this );
        parsed = true;
    }
    QListViewItem::setOpen( o );
}

// ----------------------------------------------------------------------

KApplicationTree::KApplicationTree( QWidget *parent )
    : KListView( parent )
{
    addColumn( i18n("Known Applications") );
    setRootIsDecorated( true );

    parseDesktopDir( QString::null );

    connect( this, SIGNAL( currentChanged(QListViewItem*) ), SLOT( slotItemHighlighted(QListViewItem*) ) );
    connect( this, SIGNAL( selectionChanged(QListViewItem*) ), SLOT( slotSelectionChanged(QListViewItem*) ) );
}


// ----------------------------------------------------------------------

bool KApplicationTree::isDesktopFile( const QString& filename )
{
    QFile file(filename);
    if (!file.open(IO_Raw | IO_ReadOnly))
        return false;

    QByteArray buffer(100);
    file.readBlock(buffer.data(), 100);
    file.close();

    KMimeMagicResult *result = KMimeMagic::self()->findBufferType( buffer );
    if (!result)
        return false;

    if (result->mimeType() != QString::fromLatin1("application/x-desktop"))
        return false;

    return true;
}


// ----------------------------------------------------------------------

void KApplicationTree::parseDesktopFile( QFileInfo *fi, KAppTreeListItem *item, QString relPath )
{
    QPixmap pixmap;
    QString text_name, pixmap_name, mini_pixmap_name, big_pixmap_name, command_name, comment;

    QString file = fi->absFilePath();

    if( fi->isDir() ) {
        // don't create empty directory items
        QDir dir( file );
	if ( dir.entryList().count() == 2 ) // . and ..
	    return;
	
        text_name = fi->fileName();
        file += QString::fromLatin1("/.directory");
    }
    else {
        int pos = fi->fileName().find( QString::fromLatin1(".desktop") );
        if( pos >= 0 )
            text_name = fi->fileName().left( pos );
        else {
            pos = fi->fileName().find(QString::fromLatin1(".kdelnk"));
            if (pos >= 0)
                text_name = fi->fileName().left(pos);
            else
                text_name = fi->fileName();
        }
    }

    QFile config( file );

    if( config.exists() ) {
        KSimpleConfig kconfig( file, true );
        kconfig.setDesktopGroup();
        command_name      = kconfig.readEntry(QString::fromLatin1("Exec"));
        mini_pixmap_name  = kconfig.readEntry(QString::fromLatin1("MiniIcon"));
        big_pixmap_name   = kconfig.readEntry(QString::fromLatin1("Icon"));
        comment           = kconfig.readEntry(QString::fromLatin1("Comment"));
        text_name         = kconfig.readEntry(QString::fromLatin1("Name"), text_name);

        if( !mini_pixmap_name.isEmpty() )
	  pixmap = SmallIcon(mini_pixmap_name);
        if( pixmap.isNull() && !big_pixmap_name.isEmpty() )
	  pixmap = SmallIcon(big_pixmap_name);
        if( pixmap.isNull() )
            pixmap = SmallIcon(QString::fromLatin1("default"));
    }
    else {
        command_name = text_name;
	pixmap = SmallIcon(QString::fromLatin1("default"));
    }

    KAppTreeListItem * newItem;
    if(item)
      newItem = new KAppTreeListItem( item, text_name, pixmap, false, fi->isDir(),
                                      relPath + fi->fileName(), command_name );
    else
      newItem = new KAppTreeListItem( this, text_name, pixmap, false, fi->isDir(),
                                      relPath + fi->fileName(), command_name );

    if ( fi->isDir() )
      newItem->setExpandable( true );
}


// ----------------------------------------------------------------------

void KApplicationTree::parseDesktopDir( QString relPath, KAppTreeListItem *item)
{
    // for one relative path, like "Applications", there can be several real
    // dirs (ex : a global one and a local one). Parse them both.
    QStringList list = KGlobal::dirs()->findDirs("apps", relPath);
    for (QStringList::ConstIterator dirsit = list.begin(); dirsit != list.end(); dirsit++) {
        //debug(QString("(*dirsit): '%1'").arg((*dirsit)));
        QDir d( (*dirsit) );
        if( d.exists() ) {
            debug("dirs exists");
            d.setSorting( SORT_SPEC );
            QList <QString> item_list;

            const QFileInfoList *list = d.entryInfoList();
            QFileInfoListIterator it( *list );
            QFileInfo *fi;

            while( ( fi = it.current() ) ) {
                if( fi->fileName() == QString::fromLatin1(".") || fi->fileName() == QString::fromLatin1("..") ) {
                    ++it;
                    continue;
                }
                // check if item already in list (e.g. parsed from ~/.kde)
                QListViewItem * pChild = item ? item->firstChild() : this->firstChild();
                bool found = false;
                while( pChild && !found ) {
                    found = ( ((KAppTreeListItem *)pChild)->path == relPath + fi->fileName() );
                    pChild = pChild->nextSibling();
                }
                if ( found ) {
                    kdDebug() << "skipping " << *dirsit << " | " << fi->fileName() << endl;
                    ++it;
                    continue;
                }

                if( fi->isDir() ) {
                    parseDesktopFile( fi, item, relPath );
                }
                else {
                    if( !isDesktopFile( fi->absFilePath() ) ) {
                        ++it;
                        continue;
                    }
                    parseDesktopFile( fi, item, relPath );
                }
                ++it;
            }
        }
    }
}


// ----------------------------------------------------------------------

void KApplicationTree::slotItemHighlighted(QListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    if( (!item->directory ) && (!item->exec.isEmpty()) )
        emit highlighted( item->text(0), item->exec );
}


// ----------------------------------------------------------------------

void KApplicationTree::slotSelectionChanged(QListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    if( ( !item->directory ) && (!item->exec.isEmpty() ) )
        emit selected( item->text(0), item->exec );
}

// ----------------------------------------------------------------------

void KApplicationTree::resizeEvent( QResizeEvent * e)
{
    setColumnWidth(0, width()-QApplication::style().scrollBarExtent().width());
    KListView::resizeEvent(e);
}


/***************************************************************
 *
 * KOpenWithDlg
 *
 ***************************************************************/

KOpenWithDlg::KOpenWithDlg( const KURL::List& _urls, const QString&_text,
			    const QString& _value, QWidget *parent)
    : QDialog( parent, 0L, true )
{
  if ( _urls.count() == 1 )
  {
    qServiceType = KMimeType::findByURL(_urls.first())->name();
    if (qServiceType == QString::fromLatin1("application/octet-stream"))
      qServiceType = QString::null;
  }
  else
      qServiceType = QString::null;

  m_pTree = 0L;
  m_pService = 0L;
  haveApp = false;

  QString caption = _urls.first().decodedURL();
  if (_urls.count() > 1)
      caption += QString::fromLatin1("...");
  setCaption(caption);

  QBoxLayout* topLayout = new QVBoxLayout(this, KDialog::marginHint(),
					  KDialog::spacingHint());
  label = new QLabel( _text , this );
  topLayout->addWidget(label);

  QBoxLayout* l = new QHBoxLayout(topLayout);

  edit = new KURLRequester( _value, this );
  l->addWidget(edit);

  KURLCompletion *comp = new KURLCompletion( KURLCompletion::ExeCompletion );
  edit->lineEdit()->setCompletionObject( comp );
  edit->lineEdit()->setAutoDeleteCompletionObject( true );

  connect ( edit, SIGNAL(returnPressed()), SLOT(accept()) );

  terminal = new QCheckBox( i18n("Run in terminal"), this );
  l->addWidget(terminal);

  m_pTree = new KApplicationTree( this );
  topLayout->addWidget(m_pTree);

  connect( m_pTree, SIGNAL( selected( const QString&, const QString& ) ), this, SLOT( slotSelected( const QString&, const QString& ) ) );
  connect( m_pTree, SIGNAL( highlighted( const QString&, const QString& ) ), this, SLOT( slotHighlighted( const QString&, const QString& ) ) );

  if (!qServiceType.isNull()) {
    remember = new QCheckBox(i18n("Remember application association for this file"), this);
    //    remember->setChecked(true);
    topLayout->addWidget(remember);
  } else
    remember = 0L;

  // Use KButtonBox for the aligning pushbuttons nicely
  KButtonBox* b = new KButtonBox(this);
  clear = b->addButton( i18n("C&lear") );
  b->addStretch(1);
  connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

  ok = b->addButton( i18n ("&OK") );
  ok->setDefault(true);
  connect( ok, SIGNAL(clicked()), SLOT(slotOK()) );

  cancel = b->addButton( i18n("&Cancel") );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

  b->layout();
  topLayout->addWidget(b);

  //edit->setText( _value );
  edit->setFocus();
}


// ----------------------------------------------------------------------

KOpenWithDlg::~KOpenWithDlg()
{
}

// ----------------------------------------------------------------------

void KOpenWithDlg::slotClear()
{
    edit->setURL(QString::null);
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotSelected( const QString& /*_name*/, const QString& _exec )
{
    edit->setURL( _exec );
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotHighlighted( const QString& _name, const QString& )
{
    qName = _name;
    haveApp = true;
}


// ----------------------------------------------------------------------

void KOpenWithDlg::slotOK()
{
  if (haveApp)
    m_pService = KService::serviceByName( qName );
  else {
    m_pService = 0L;
    // no service was found, maybe they typed the name into the text field
    KService::List sList = KService::allServices();
    QValueListIterator<KService::Ptr> it(sList.begin());
    QString text = edit->url();
    for (; it != sList.end(); ++it)
      if ((*it)->exec() == text ||
	  (*it)->name().lower() == text.lower())
	m_pService = *it;
    if (m_pService) {
      edit->setURL(m_pService->exec());
      haveApp = true;
    }
  }

  if (terminal->isChecked()) {
    KSimpleConfig conf(QString::fromLatin1("konquerorrc"), true);
    conf.setGroup(QString::fromLatin1("Misc Defaults"));
    m_command = conf.readEntry(QString::fromLatin1("Terminal"), QString::fromLatin1("konsole"));

    m_command += QString::fromLatin1(" -e ");
    m_command += edit->url();
  }

  if (haveApp && !remember) {
    haveApp = false;
    accept();
    return;
  }

  if (remember)
    if (!remember->isChecked()) {
      haveApp = false;
      accept();
      return;
    }

  // if we got here, we can't seem to find a service for what they
  // wanted.  The other possibility is that they have asked for the
  // association to be remembered.  Create/update service.
  QString keepExec(edit->url());
  QString serviceName;
  if (!haveApp) {
    if (keepExec.contains('/'))
      serviceName = keepExec.right(keepExec.length() -
				   keepExec.findRev('/') + 1);
    else
      serviceName = keepExec;
  } else
    serviceName = m_pService->desktopEntryPath();

  QString pathName (serviceName);
  if (pathName.right(8) != QString::fromLatin1(".desktop"))
    pathName += QString::fromLatin1(".desktop");
  QString path(locateLocal("apps", pathName));

  KDesktopFile desktop(path);
  desktop.writeEntry(QString::fromLatin1("Type"), QString::fromLatin1("Application"));
  desktop.writeEntry(QString::fromLatin1("Name"), haveApp ? qName : serviceName);
  desktop.writeEntry(QString::fromLatin1("Exec"), keepExec);
  if (remember)
    if (remember->isChecked()) {
      QStringList mimeList;
      KDesktopFile oldDesktop(locate("apps", pathName), true);
      mimeList = oldDesktop.readEntry(QString::fromLatin1("MimeType"));
      if (!mimeList.contains(qServiceType))
	mimeList.append(qServiceType);
      desktop.writeEntry(QString::fromLatin1("MimeType"), mimeList, ';');
      if (terminal->isChecked())
	desktop.writeEntry(QString::fromLatin1("Terminal"), 1);
      else
	desktop.writeEntry(QString::fromLatin1("Terminal"), 0);
    }


  // write it all out to the file
  desktop.sync();

  // rebuild the database
  DCOPClient *dcc = kapp->dcopClient();
  QByteArray replyData;
  QCString retType;
  dcc->call("kded", "kbuildsycoca", "recreate()", QByteArray(),
	    retType, replyData);

  // get the new service pointer
  m_pService = KService::serviceByDesktopPath( path );

  haveApp = false;
  accept();
}

QString KOpenWithDlg::text()
{
    if (!m_command.isEmpty())
        return m_command;
    else
        return edit->url();
}

///////////////

bool KFileOpenWithHandler::displayOpenWithDialog( const KURL::List& urls )
{
    KOpenWithDlg l( urls, i18n("Open With:"), QString::null, 0L );
    if ( l.exec() )
    {
      KService::Ptr service = l.service();
      if ( !!service )
        return KRun::run( *service, urls );
	
      QString exec = l.text();
      exec += QString::fromLatin1(" %f");
      return KRun::run( exec, urls );
    }
    return false;
}

#include "kopenwith.moc"

