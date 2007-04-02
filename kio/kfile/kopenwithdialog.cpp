/*  This file is part of the KDE libraries

    Copyright (C) 1997 Torben Weis <weis@stud.uni-frankfurt.de>
    Copyright (C) 1999 Dirk Mueller <mueller@kde.org>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kauthorized.h>
#include <kcombobox.h>
#include <kdesktopfile.h>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kmimetypetrader.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kmimetype.h>
#include <kservicegroup.h>

#include "kopenwithdialog.h"
#include "kopenwithdialog_p.h"

#include <assert.h>
#include <stdlib.h>
#include <kbuildsycocaprogressdialog.h>
#include <kconfiggroup.h>

template <> inline
void KConfigGroup::writeEntry( const char *pKey,
                              const KGlobalSettings::Completion& aValue,
                              KConfigBase::WriteConfigFlags flags)
{
  writeEntry(pKey, int(aValue), flags);
}

// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( Q3ListView* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, const QString &p, const QString &c )
    : Q3ListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

KAppTreeListItem::KAppTreeListItem( Q3ListViewItem* parent, const QString & name,
                                    const QPixmap& pixmap, bool parse, bool dir, const QString &p, const QString &c )
    : Q3ListViewItem( parent, name )
{
    init(pixmap, parse, dir, p, c);
}


// ----------------------------------------------------------------------

void KAppTreeListItem::init(const QPixmap& pixmap, bool parse, bool dir, const QString &_path, const QString &_exec)
{
    setPixmap(0, pixmap);
    parsed = parse;
    directory = dir;
    path = _path; // relative path
    exec = _exec;
}


/* Ensures that directories sort before non-directories */
int KAppTreeListItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
	KAppTreeListItem *other = static_cast<KAppTreeListItem *>(i);

	// Directories sort first
	if (directory && !other->directory)
		return -1;

	else if (!directory && other->directory)
		return 1;

	else // both directories or both not
		return Q3ListViewItem::compare(i, col, ascending);
}

// ----------------------------------------------------------------------
// Ensure that case is ignored
QString KAppTreeListItem::key(int column, bool /*ascending*/) const
{
        return text(column).toUpper();
}

void KAppTreeListItem::activate()
{
    if ( directory )
        setOpen(!isOpen());
}

void KAppTreeListItem::setOpen( bool o )
{
    if( o && !parsed ) { // fill the children before opening
        ((KApplicationTree *) parent())->addDesktopGroup( path, this );
        parsed = true;
    }
    Q3ListViewItem::setOpen( o );
}

bool KAppTreeListItem::isDirectory()
{
    return directory;
}

// ----------------------------------------------------------------------

KApplicationTree::KApplicationTree( QWidget *parent )
    : Q3ListView( parent ), currentitem(0)
{
    addColumn( i18n("Known Applications") );
    setRootIsDecorated( true );

    addDesktopGroup( QString() );
    cleanupTree();

    connect( this, SIGNAL( currentChanged(Q3ListViewItem*) ),
            SLOT( slotItemHighlighted(Q3ListViewItem*) ) );
    connect( this, SIGNAL( selectionChanged(Q3ListViewItem*) ),
            SLOT( slotSelectionChanged(Q3ListViewItem*) ) );
}

// ----------------------------------------------------------------------

bool KApplicationTree::isDirSel()
{
    if (!currentitem) return false; // if currentitem isn't set
    return currentitem->isDirectory();
}

// ----------------------------------------------------------------------

static QPixmap appIcon(const QString &iconName)
{
    QPixmap normal = KIconLoader::global()->loadIcon(iconName, K3Icon::Small, 0, K3Icon::DefaultState, 0L, true);
    // make sure they are not larger than 20x20
    if (normal.width() > 20 || normal.height() > 20)
    {
       QImage tmp = normal.toImage();
       tmp = tmp.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
       normal = QPixmap::fromImage(tmp);
    }
    return normal;
}

void KApplicationTree::addDesktopGroup( const QString &relPath, KAppTreeListItem *item)
{
   KServiceGroup::Ptr root = KServiceGroup::group(relPath);
   if (!root || !root->isValid()) return;

   KServiceGroup::List list = root->entries();

   KAppTreeListItem * newItem;
   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
   {
      QString icon;
      QString text;
      QString relPath;
      QString exec;
      bool isDir = false;
      const KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
         const KService::Ptr service = KService::Ptr::staticCast(p);

         if (service->noDisplay())
            continue;

         icon = service->icon();
         text = service->name();
         exec = service->exec();
      }
      else if (p->isType(KST_KServiceGroup))
      {
         const KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);

         if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
            continue;

         icon = serviceGroup->icon();
         text = serviceGroup->caption();
         relPath = serviceGroup->relPath();
         isDir = true;
      }
      else
      {
         kWarning(250) << "KServiceGroup: Unexpected object in list!" << endl;
         continue;
      }

      QPixmap pixmap = appIcon( icon );

      if (item)
         newItem = new KAppTreeListItem( item, text, pixmap, false, isDir,
                                         relPath, exec );
      else
         newItem = new KAppTreeListItem( this, text, pixmap, false, isDir,
                                         relPath, exec );
      if (isDir)
         newItem->setExpandable( true );
   }
}


// ----------------------------------------------------------------------

void KApplicationTree::slotItemHighlighted(Q3ListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    currentitem = item;

    if( (!item->directory ) && (!item->exec.isEmpty()) )
        emit highlighted( item->text(0), item->exec );
}


// ----------------------------------------------------------------------

void KApplicationTree::slotSelectionChanged(Q3ListViewItem* i)
{
    // i may be 0 (see documentation)
    if(!i)
        return;

    KAppTreeListItem *item = (KAppTreeListItem *) i;

    currentitem = item;

    if( ( !item->directory ) && (!item->exec.isEmpty() ) )
        emit selected( item->text(0), item->exec );
}

// ----------------------------------------------------------------------

void KApplicationTree::resizeEvent( QResizeEvent * e)
{
    setColumnWidth(0, width()-style()->pixelMetric(QStyle::PM_ScrollBarExtent)
                         -2*style()->pixelMetric(QStyle::PM_DefaultFrameWidth));
    Q3ListView::resizeEvent(e);
}

// Prune empty directories from the tree
void KApplicationTree::cleanupTree()
{
	Q3ListViewItem *item=firstChild();
	while(item!=0)
	{
		if(item->isExpandable())
		{
			Q3ListViewItem *temp=item->itemBelow();
			if(item->text(0)!=i18n("Applications"))
				item->setOpen(false);
			item=temp;
			continue;
		}
		item=item->itemBelow();
	}
}

/***************************************************************
 *
 * KOpenWithDialog
 *
 ***************************************************************/
class KOpenWithDialogPrivate
{
public:
    KOpenWithDialogPrivate() : saveNewApps(false) { }
    bool saveNewApps;
    KService::Ptr curService;
};

KOpenWithDialog::KOpenWithDialog( const KUrl::List& _urls, QWidget* parent )
             :KDialog( parent ),d(new KOpenWithDialogPrivate)
{
    setObjectName( QLatin1String( "openwith" ) );
    setModal( true );
    setCaption( i18n( "Open With" ) );

    QString text;
    if( _urls.count() == 1 )
    {
        text = i18n("<qt>Select the program that should be used to open <b>%1</b>. "
                     "If the program is not listed, enter the name or click "
                     "the browse button.</qt>",  _urls.first().fileName() );
    }
    else
        // Should never happen ??
        text = i18n( "Choose the name of the program with which to open the selected files." );
    setMimeType( _urls );
    init( text, QString() );
}

KOpenWithDialog::KOpenWithDialog( const KUrl::List& _urls, const QString&_text,
                            const QString& _value, QWidget *parent)
             :KDialog( parent ),d(new KOpenWithDialogPrivate)
{
  setObjectName( QLatin1String( "openwith" ) );
  setModal( true );
  QString caption = KStringHandler::csqueeze( _urls.first().prettyUrl() );
  if (_urls.count() > 1)
      caption += QString::fromLatin1("...");
  setCaption(caption);
  setMimeType( _urls );
  init( _text, _value );
}

KOpenWithDialog::KOpenWithDialog( const QString &mimeType, const QString& value,
                            QWidget *parent)
             :KDialog( parent ),d(new KOpenWithDialogPrivate)
{
  setObjectName( QLatin1String( "openwith" ) );
  setModal( true );
  setCaption(i18n("Choose Application for %1", mimeType));
  QString text = i18n("<qt>Select the program for the file type: <b>%1</b>. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>", mimeType);
  qMimeType = mimeType;
  init( text, value );
  if (remember)
      remember->hide();
}

KOpenWithDialog::KOpenWithDialog( QWidget *parent)
             :KDialog( parent ),d(new KOpenWithDialogPrivate)
{
  setObjectName( QLatin1String( "openwith" ) );
  setModal( true );
  setCaption(i18n("Choose Application"));
  QString text = i18n("<qt>Select a program. "
                      "If the program is not listed, enter the name or click "
                      "the browse button.</qt>");
  qMimeType.clear();
  init( text, QString() );
}

void KOpenWithDialog::setMimeType( const KUrl::List& _urls )
{
  if ( _urls.count() == 1 )
  {
    qMimeType = KMimeType::findByUrl( _urls.first())->name();
    if (qMimeType == QLatin1String("application/octet-stream"))
      qMimeType.clear();
  }
  else
      qMimeType.clear();
}

void KOpenWithDialog::init( const QString& _text, const QString& _value )
{
  bool bReadOnly = !KAuthorized::authorize("shell_access");
  m_terminaldirty = false;
  m_pTree = 0L;
  m_pService = 0L;
  d->curService = 0L;

  setButtons( Ok | Cancel );


  QWidget *mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  QBoxLayout *topLayout = new QVBoxLayout( mainWidget );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );
  label = new QLabel( _text, this );
  label->setWordWrap(true);
  topLayout->addWidget(label);

  if (!bReadOnly)
  {
    // init the history combo and insert it into the URL-Requester
    KHistoryCombo *combo = new KHistoryCombo();
    KLineEdit *lineEdit = new KLineEdit();
    lineEdit->setClearButtonShown(true);
    combo->setLineEdit(lineEdit);
    combo->setDuplicatesEnabled( false );
    KConfigGroup cg( KGlobal::config(), QString::fromLatin1("Open-with settings") );
    int max = cg.readEntry( QString::fromLatin1("Maximum history"), 15 );
    combo->setMaxCount( max );
    int mode = cg.readEntry(QString::fromLatin1("CompletionMode"),
				int(KGlobalSettings::completionMode()));
    combo->setCompletionMode((KGlobalSettings::Completion)mode);
    QStringList list = cg.readEntry( QString::fromLatin1("History"), QStringList() );
    combo->setHistoryItems( list, true );
    edit = new KUrlRequester( combo, mainWidget );
  }
  else
  {
    edit = new KUrlRequester( mainWidget );
    edit->lineEdit()->setReadOnly(true);
    edit->button()->hide();
  }

  edit->setUrl( KUrl(_value) );
  edit->setWhatsThis(i18n(
    "Following the command, you can have several place holders which will be replaced "
    "with the actual values when the actual program is run:\n"
    "%f - a single file name\n"
    "%F - a list of files; use for applications that can open several local files at once\n"
    "%u - a single URL\n"
    "%U - a list of URLs\n"
    "%d - the directory of the file to open\n"
    "%D - a list of directories\n"
    "%i - the icon\n"
    "%m - the mini-icon\n"
    "%c - the comment"));

  topLayout->addWidget(edit);

  if ( edit->comboBox() ) {
    KUrlCompletion *comp = new KUrlCompletion( KUrlCompletion::ExeCompletion );
    edit->comboBox()->setCompletionObject( comp );
    edit->comboBox()->setAutoDeleteCompletionObject( true );
  }

  connect ( edit, SIGNAL(returnPressed()), SLOT(slotOK()) );
  connect ( this, SIGNAL(okClicked()),SLOT(slotOK()));
  connect ( edit, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged()) );

  m_pTree = new KApplicationTree( mainWidget );
  topLayout->addWidget(m_pTree);

  connect( m_pTree, SIGNAL( selected( const QString&, const QString& ) ),
           SLOT( slotSelected( const QString&, const QString& ) ) );
  connect( m_pTree, SIGNAL( highlighted( const QString&, const QString& ) ),
           SLOT( slotHighlighted( const QString&, const QString& ) ) );
  connect( m_pTree, SIGNAL( doubleClicked(Q3ListViewItem*) ),
           SLOT( slotDbClick() ) );

  terminal = new QCheckBox( i18n("Run in &terminal"), mainWidget );
  if (bReadOnly)
     terminal->hide();
  connect(terminal, SIGNAL(toggled(bool)), SLOT(slotTerminalToggled(bool)));

  topLayout->addWidget(terminal);

  QBoxLayout* nocloseonexitLayout = new QHBoxLayout();
  nocloseonexitLayout->setMargin( 0 );
  nocloseonexitLayout->setSpacing( KDialog::spacingHint() );
  QSpacerItem* spacer = new QSpacerItem( 20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum );
  nocloseonexitLayout->addItem( spacer );

  nocloseonexit = new QCheckBox( i18n("&Do not close when command exits"), mainWidget );
  nocloseonexit->setChecked( false );
  nocloseonexit->setDisabled( true );

  // check to see if we use konsole if not disable the nocloseonexit
  // because we don't know how to do this on other terminal applications
  KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
  QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));

  if (bReadOnly || preferredTerminal != "konsole")
     nocloseonexit->hide();

  nocloseonexitLayout->addWidget( nocloseonexit );
  topLayout->addLayout( nocloseonexitLayout );

  if (!qMimeType.isNull())
  {
    remember = new QCheckBox(i18n("&Remember application association for this type of file"), mainWidget);
    //    remember->setChecked(true);
    topLayout->addWidget(remember);
  }
  else
    remember = 0L;

  //edit->setText( _value );
  // This is what caused "can't click on items before clicking on Name header".
  // Probably due to the resizeEvent handler using width().
  //resize( minimumWidth(), sizeHint().height() );
  edit->setFocus();
  slotTextChanged();
}


// ----------------------------------------------------------------------

KOpenWithDialog::~KOpenWithDialog()
{
    delete d;
}


// ----------------------------------------------------------------------

void KOpenWithDialog::slotSelected( const QString& /*_name*/, const QString& _exec )
{
    kDebug(250)<<"KOpenWithDialog::slotSelected"<<endl;
    KService::Ptr pService = d->curService;
    edit->setUrl( KUrl(_exec) ); // calls slotTextChanged :(
    d->curService = pService;
}


// ----------------------------------------------------------------------

void KOpenWithDialog::slotHighlighted( const QString& _name, const QString& )
{
    kDebug(250)<<"KOpenWithDialog::slotHighlighted"<<endl;
    qName = _name;
    d->curService = KService::serviceByName( qName );
    if (!m_terminaldirty)
    {
        // ### indicate that default value was restored
        terminal->setChecked(d->curService->terminal());
        QString terminalOptions = d->curService->terminalOptions();
        nocloseonexit->setChecked( (terminalOptions.contains( "--noclose" ) > 0) );
        m_terminaldirty = false; // slotTerminalToggled changed it
    }
}

// ----------------------------------------------------------------------

void KOpenWithDialog::slotTextChanged()
{
    kDebug(250)<<"KOpenWithDialog::slotTextChanged"<<endl;
    // Forget about the service
    d->curService = 0L;
    enableButton( Ok, !edit->url().isEmpty());
}

// ----------------------------------------------------------------------

void KOpenWithDialog::slotTerminalToggled(bool)
{
    // ### indicate that default value was overridden
    m_terminaldirty = true;
    nocloseonexit->setDisabled( ! terminal->isChecked() );
}

// ----------------------------------------------------------------------

void KOpenWithDialog::slotDbClick()
{
   if (m_pTree->isDirSel() ) return; // check if a directory is selected
   slotOK();
}

void KOpenWithDialog::setSaveNewApplications(bool b)
{
  d->saveNewApps = b;
}

void KOpenWithDialog::slotOK()
{
  QString typedExec(edit->url().pathOrUrl());
  QString fullExec(typedExec);

  QString serviceName;
  QString initialServiceName;
  QString preferredTerminal;
  m_pService = d->curService;
  if (!m_pService) {
    // No service selected - check the command line

    // Find out the name of the service from the command line, removing args and paths
    serviceName = KRun::binaryName( typedExec, true );
    if (serviceName.isEmpty())
    {
      // TODO add a KMessageBox::error here after the end of the message freeze
      return;
    }
    initialServiceName = serviceName;
    kDebug(250) << "initialServiceName=" << initialServiceName << endl;
    int i = 1; // We have app, app-2, app-3... Looks better for the user.
    bool ok = false;
    // Check if there's already a service by that name, with the same Exec line
    do {
        kDebug(250) << "looking for service " << serviceName << endl;
        KService::Ptr serv = KService::serviceByDesktopName( serviceName );
        ok = !serv; // ok if no such service yet
        // also ok if we find the exact same service (well, "kwrite" == "kwrite %U"
        if (serv && serv->isApplication())
        {
            QString exec = serv->exec();
            fullExec = exec;
            exec.replace("%u", "", Qt::CaseInsensitive);
            exec.replace("%f", "", Qt::CaseInsensitive);
            exec.replace("-caption %c", "");
            exec.replace("-caption \"%c\"", "");
            exec.replace("%i", "");
            exec.replace("%m", "");
            exec = exec.simplified();
            if (exec == typedExec)
            {
                ok = true;
                m_pService = serv;
                kDebug(250) << k_funcinfo << "OK, found identical service: " << serv->desktopEntryPath() << endl;
            }
        }
        if (!ok) // service was found, but it was different -> keep looking
        {
            ++i;
            serviceName = initialServiceName + '-' + QString::number(i);
        }
    }
    while (!ok);
  }
  if ( m_pService )
  {
    // Existing service selected
    serviceName = m_pService->name();
    initialServiceName = serviceName;
    fullExec = m_pService->exec();
  }

  if (terminal->isChecked())
  {
    KConfigGroup confGroup( KGlobal::config(), QString::fromLatin1("General") );
    preferredTerminal = confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));
    m_command = preferredTerminal;
    // only add --noclose when we are sure it is konsole we're using
    if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
      m_command += QString::fromLatin1(" --noclose");
    m_command += QString::fromLatin1(" -e ");
    m_command += edit->url().pathOrUrl();
    kDebug(250) << "Setting m_command to " << m_command << endl;
  }
  if ( m_pService && terminal->isChecked() != m_pService->terminal() )
      m_pService = 0L; // It's not exactly this service we're running

  bool bRemember = remember && remember->isChecked();

  if ( !bRemember && m_pService)
  {
    accept();
    return;
  }

  if (!bRemember && !d->saveNewApps)
  {
    // Create temp service
    m_pService = new KService(initialServiceName, fullExec, QString());
    if (terminal->isChecked())
    {
      m_pService->setTerminal(true);
      // only add --noclose when we are sure it is konsole we're using
      if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
         m_pService->setTerminalOptions("--noclose");
    }
    accept();
    return;
  }

  // if we got here, we can't seem to find a service for what they
  // wanted.  The other possibility is that they have asked for the
  // association to be remembered.  Create/update service.

  QString newPath;
  QString oldPath;
  QString menuId;
  if (m_pService)
  {
    oldPath = m_pService->desktopEntryPath();
    newPath = m_pService->locateLocal();
    menuId = m_pService->menuId();
    kDebug(250) << "Updating exitsing service " << m_pService->desktopEntryPath() << " ( " << newPath << " ) " << endl;
  }
  else
  {
    newPath = KService::newServicePath(false /* hidden */, serviceName, &menuId);
    kDebug(250) << "Creating new service " << serviceName << " ( " << newPath << " ) " << endl;
  }

  int maxPreference = 1;
  if (!qMimeType.isEmpty())
  {
    const KServiceOfferList offerList = KMimeTypeTrader::self()->weightedOffers( qMimeType );
    if (!offerList.isEmpty())
      maxPreference = offerList.first().preference();
  }

  KDesktopFile *desktop = 0;
  if (!oldPath.isEmpty() && (oldPath != newPath))
  {
     KDesktopFile orig( oldPath );
     desktop = orig.copyTo( newPath );
  }
  else
  {
     desktop = new KDesktopFile(newPath);
  }
  KConfigGroup cg = desktop->desktopGroup();
  cg.writeEntry("Type", QString::fromLatin1("Application"));
  cg.writeEntry("Name", initialServiceName);
  cg.writePathEntry("Exec", fullExec);
  if (terminal->isChecked())
  {
    cg.writeEntry("Terminal", true);
    // only add --noclose when we are sure it is konsole we're using
    if (preferredTerminal == "konsole" && nocloseonexit->isChecked())
      cg.writeEntry("TerminalOptions", "--noclose");
  }
  else
  {
    cg.writeEntry("Terminal", false);
  }
  cg.writeEntry("InitialPreference", maxPreference + 1);


  if (bRemember || d->saveNewApps)
  {
    QStringList mimeList = cg.readEntry("MimeType", QStringList(), ';');
    if (!qMimeType.isEmpty() && !mimeList.contains(qMimeType))
      mimeList.append(qMimeType);
    cg.writeEntry("MimeType", mimeList, ';');

    if ( !qMimeType.isEmpty() )
    {
      // Also make sure the "auto embed" setting for this mimetype is off
      KDesktopFile mimeDesktop( KStandardDirs::locateLocal( "mime", qMimeType + ".desktop" ) );
      mimeDesktop.desktopGroup().writeEntry( "X-KDE-AutoEmbed", false );
      mimeDesktop.sync();
    }
  }

  // write it all out to the file
  cg.sync();
  delete desktop;

  KBuildSycocaProgressDialog::rebuildKSycoca(this);

  m_pService = KService::serviceByMenuId( menuId );

  Q_ASSERT( m_pService );

  accept();
}

QString KOpenWithDialog::text() const
{
    if (!m_command.isEmpty())
        return m_command;
    else
        return edit->url().url();
}

void KOpenWithDialog::hideNoCloseOnExit()
{
    // uncheck the checkbox because the value could be used when "Run in Terminal" is selected
    nocloseonexit->setChecked( false );
    nocloseonexit->hide();
}

void KOpenWithDialog::hideRunInTerminal()
{
    terminal->hide();
    hideNoCloseOnExit();
}

void KOpenWithDialog::accept()
{
    KHistoryCombo *combo = static_cast<KHistoryCombo*>( edit->comboBox() );
    if ( combo ) {
        combo->addToHistory( edit->url().url() );

        KConfigGroup cg( KGlobal::config(), QString::fromLatin1("Open-with settings") );
        cg.writeEntry( QString::fromLatin1("History"), combo->historyItems() );
        cg.writeEntry(QString::fromLatin1("CompletionMode"), combo->completionMode());
        // don't store the completion-list, as it contains all of KUrlCompletion's
        // executables
        cg.sync();
    }

    QDialog::accept();
}

#include "kopenwithdialog.moc"
#include "kopenwithdialog_p.moc"
