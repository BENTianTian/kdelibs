/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

/*
 * kio_propsdlg.cpp
 * View/Edit Properties of files (later : URLs)
 *
 * some FilePermissionsPropsPage-changes by 
 *  Henner Zeller <zeller@think.de>
 * some layout management by 
 *  Bertrand Leconte <B.Leconte@mail.dotcom.fr>
 * the rest of the layout management, bug fixes, adaptation to libkio by
 *  David Faure <faure@kde.org>
 */

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <kconfig.h>

#include <qfile.h>
#include <qdir.h>
#include <qdict.h>
#include <qmessagebox.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qpainter.h>

#include <kurl.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmimetypes.h>
#include <kservices.h>
#include <kbookmark.h> // for encode. hum.

#include "kio_propsdlg.h"
#include "kfmpaths.h"

#include <X11/Xlib.h> // for XSetTransientForHint

#define SEPARATION 10

mode_t FilePermissionsPropsPage::fperm[3][4] = {
        {S_IRUSR, S_IWUSR, S_IXUSR, S_ISUID},
        {S_IRGRP, S_IWGRP, S_IXGRP, S_ISGID},
        {S_IROTH, S_IWOTH, S_IXOTH, S_ISVTX}
    };

PropertiesDialog::PropertiesDialog( const char *_url, mode_t _mode ) : QObject(), url(_url), mode(_mode)
{
    pageList.setAutoDelete( true );
  
    kurl = new KURL( url );
    if ( kurl->isMalformed() )
	delete this;

    if ( mode == (mode_t) -1 )
    {
      if ( kurl->isLocalFile() )
      {
          struct stat buf;
          stat( kurl->path(), &buf );
          mode = buf.st_mode;
      }
      else mode = 0;
    }

    tab = new QTabDialog( 0L, 0L );

    // Matthias: let the dialog look like a modal dialog
    XSetTransientForHint(qt_xdisplay(), tab->winId(), tab->winId());

    tab->setGeometry( tab->x(), tab->y(), 400, 400 );

    insertPages();

    tab->setOKButton(i18n("OK")); 
    tab->setCancelButton(i18n("Cancel"));

    connect( tab, SIGNAL( applyButtonPressed() ), this, SLOT( slotApply() ) );
    connect( tab, SIGNAL( cancelButtonPressed() ), this, SLOT( slotCancel() ) );
    
    tab->show();
}


PropertiesDialog::~PropertiesDialog()
{
    pageList.clear();
    
    if ( kurl != 0L )
	delete kurl;
}

void PropertiesDialog::emitPropertiesChanged( const QString& _new_name )
{
    emit propertiesChanged( url, _new_name );
}

void PropertiesDialog::slotApply()
{
    PropsPage *page;
    for ( page = pageList.last(); page != 0L; page = pageList.prev() )
	page->applyChanges();

    QString s = getURL();

    // Strip the filename
    if ( !S_ISDIR( mode ) )
    {
	int i = s.findRev( "/" );
	// Should never happen
	if ( i == -1 )
	return;
	s = s.left( i + 1 );
    }
    
    /* still needed ?
    KIOServer::sendNotify( s.data() );
    */

    // make sure the desktop icon get's unselected
    emit propertiesClosed();
    delete this;
}

void PropertiesDialog::slotCancel(){
   
  emit propertiesClosed();
  delete this;
}

void PropertiesDialog::insertPages()
{ 
    if ( FilePropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new FilePropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( FilePermissionsPropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new FilePermissionsPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( ExecPropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new ExecPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( ApplicationPropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new ApplicationPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( BindingPropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new BindingPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( URLPropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new URLPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( DirPropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new DirPropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }

    if ( DevicePropsPage::supports( kurl, mode ) )
    {
	PropsPage *p = new DevicePropsPage( this );
	tab->addTab( p, p->getTabName() );
	pageList.append( p );
    }
}


PropsPage::PropsPage( PropertiesDialog *_props ) : QWidget( _props->getTab(), 0L )
{
    properties = _props;
    fontHeight = 2*fontMetrics().height();
}

FilePropsPage::FilePropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    QString path = properties->getKURL()->path();

    // Extract the directories name without path
    QString filename;
    QString tmp2 = properties->getKURL()->path();
    if ( tmp2.right(1) == "/" )
	tmp2.truncate( tmp2.length() - 1 );
    int i = tmp2.findRev( "/" );
    // Is is not the root directory ?
    if ( i != -1 )
	filename = tmp2.mid( i + 1, tmp2.length() );
    else
	filename = "/";
    // decodeFileName( filename );

    QString tmp = path.data();
    if ( tmp.right(1) != "/" )
      tmp += "/";
    bool isTrash = false;
    // is it the trash bin ?
    if ( strcmp( properties->getKURL()->protocol(), "file" ) == 0L &&
	 tmp == KfmPaths::trashPath())
           isTrash = true;
    
    /* directories may not have a slash at the end if
     * we want to stat() them; it requires that we
     * change into it .. which may not be allowed
     * stat("/is/unaccessible")  -> rwx------
     * stat("/is/unaccessible/") -> EPERM            H.Z.
     */
    if ( path.length() > 1 && path.right(1) == "/" )
	path = path.left( path.length() - 1 );             

    struct stat buff;
    stat( path.data(), &buff );

    struct stat lbuff;
    lstat( path.data(), &lbuff );

    QLabel *l;
 
    layout = new QBoxLayout(this, QBoxLayout::TopToBottom, SEPARATION); 

    l = new QLabel( i18n("File Name"), this );
    l->setFixedSize(l->sizeHint());
    layout->addWidget(l, 0, AlignLeft);
    
    name = new QLineEdit( this );
    name->setMinimumSize(200, fontHeight);
    name->setMaximumSize(QLayout::unlimited, fontHeight);
    layout->addWidget(name, 0, AlignLeft);
    name->setText( filename );
    // Dont rename trash or root directory
    if ( isTrash || filename == "/" )
	name->setEnabled( false );
    oldName = filename;

    l = new QLabel( i18n("Full Name"), this );
    l->setFixedSize(l->sizeHint());
    layout->addWidget(l, 0, AlignLeft);
    
    QLabel *fname = new QLabel( this );
    fname->setText( path );
    fname->setMinimumSize(200, fontHeight);
    fname->setMaximumSize(QLayout::unlimited, fontHeight);
    fname->setLineWidth(1);
    fname->setFrameStyle(QFrame::Box | QFrame::Raised);
    layout->addWidget(fname, 0, AlignLeft);
    
    layout->addSpacing(SEPARATION);

    if ( isTrash )
    {
	l = new QLabel( i18n( "Is the Trash Bin"), this );
	l->setFixedSize(l->sizeHint());
	layout->addWidget(l, 0, AlignLeft);
    }
    else if ( S_ISDIR( buff.st_mode ) )
    {
	l = new QLabel( i18n("Is a Directory"), this );
	l->setFixedSize(l->sizeHint());
	layout->addWidget(l, 0, AlignLeft);
    }
    if ( S_ISLNK( lbuff.st_mode ) )
    {
	l = new QLabel( i18n( "Points to" ), this );
	l->setFixedSize(l->sizeHint());
	layout->addWidget(l, 0, AlignLeft);
    
        QLabel *lname = new QLabel( this );
        lname->setText( path );
        lname->setMinimumSize(200, fontHeight);
        lname->setMaximumSize(QLayout::unlimited, fontHeight);
        lname->setLineWidth(1);
        lname->setFrameStyle(QFrame::Box | QFrame::Raised);
        layout->addWidget(lname, 0, AlignLeft);

	char buffer[1024];
	int n = readlink( path.data(), buffer, 1022 );
	if ( n > 0 )
	{
	    buffer[ n ] = 0;
	    lname->setText( buffer );
	}
    }
    else if ( S_ISREG( buff.st_mode ) )
    {
        QString tempstr;
	int size = buff.st_size;
	tempstr.sprintf(i18n("Size: %i"), size );
	l = new QLabel( tempstr.data(), this );
	l->setFixedSize(l->sizeHint());
	layout->addWidget(l, 0, AlignLeft);
    }
    
    char buffer[1024];
    struct tm *t = localtime( &lbuff.st_atime );
    sprintf( buffer, "%s: %02i:%02i %02i.%02i.%04i", 
	     (const char*)i18n("Last Access"),
	     t->tm_hour,t->tm_min,
	     t->tm_mday,t->tm_mon + 1,t->tm_year + 1900 );             
    l = new QLabel( buffer, this );
    l->setFixedSize(l->sizeHint());
    layout->addWidget(l, 0, AlignLeft);

    t = localtime( &lbuff.st_mtime );
    sprintf( buffer, "%s: %02i:%02i %02i.%02i.%04i", 
	     (const char*)i18n("Last Modified"),
	     t->tm_hour,t->tm_min,
	     t->tm_mday,t->tm_mon + 1,t->tm_year + 1900 );          
    l = new QLabel( buffer, this );
    l->setFixedSize(l->sizeHint());
    layout->addWidget(l, 0, AlignLeft);

    layout->addStretch(10);
    layout->activate();
}

bool FilePropsPage::supports( KURL *_kurl, mode_t _mode )
{
// Only local files for now - to be extended
    return _kurl->isLocalFile();
/*
    KURL u( _kurl->url().data() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
    return true;
*/

}

void FilePropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    QString fname = properties->getKURL()->filename();
    QString n = name->text();
    KBookmark::encode( n ); // hum this has nothing to do with bookmarks. We'll need to move it

    // Do we need to rename the file ?
    if ( strcmp( oldName.data(), n.data() ) != 0 )
    {
	QString s = path.data();
	if ( s.right(1) == "/") 
	  // It's a directory, so strip the trailing slash first
	  s.truncate( s.length() - 1);

	int i = s.findRev( "/" );
	// Should never happen
	if ( i == -1 )
	    return;
	s.truncate( i );
	s += "/";
	s += n.data();
	if ( path.right(1) == "/") 
	  // It's a directory, so strip the trailing slash (in case it's a
          // symlink)
	  path.truncate( path.length() - 1);
	if ( rename( path, s ) != 0 ) {
            QString tmp;
            tmp.sprintf(i18n("Could not rename the file or directory\n%s\n"), strerror(errno));
            QMessageBox::warning( this, i18n( "KFM Error" ), tmp );
        }
	properties->emitPropertiesChanged( n );
    }
}

FilePermissionsPropsPage::FilePermissionsPropsPage( PropertiesDialog *_props ) 
: PropsPage( _props )
{
    QString path = properties->getKURL()->path();
    QString fname = properties->getKURL()->filename();

    /* remove appended '/' .. see comment in FilePropsPage */
    if ( path.length() > 1 && path.right( 1 ) == "/" )
	path = path.left ( path.length() - 1 );          

    struct stat buff;
    lstat( path.data(), &buff ); // display uid/gid of the link, if it's a link
    struct passwd * user = getpwuid( buff.st_uid );
    struct group * ge = getgrgid( buff.st_gid );

    bool IamRoot = (geteuid() == 0);
    bool IsMyFile  = (geteuid() == buff.st_uid);
    bool IsDir =  S_ISDIR (buff.st_mode);
    bool IsLink =  S_ISLNK( buff.st_mode );

    permissions = buff.st_mode & ( S_IRWXU | S_IRWXG | S_IRWXO |
				   S_ISUID | S_ISGID | S_ISVTX );
    strOwner = "";
    strGroup = "";
    if ( user != 0L )
    {
	strOwner = user->pw_name;
    }    
    if ( ge != 0L )
    {
	strGroup = ge->gr_name;
	if (strGroup.isEmpty())
	    strGroup.sprintf("%d",ge->gr_gid);
    } else 
	strGroup.sprintf("%d",buff.st_gid);

    QBoxLayout *box = new QVBoxLayout( this, SEPARATION );

    QLabel *l;
    QGroupBox *gb;
    QGridLayout *gl;

    /* Group: Access Permissions */
    gb = new QGroupBox ( i18n("Access permissions"), this );
    box->addWidget (gb);

    gl = new QGridLayout (gb, 4, 6, 15);

    if (IsDir)
	    l = new QLabel( i18n("Show\nEntries"), gb );
    else
	    l = new QLabel( i18n("Read"), gb );
    l->setMinimumSize( l->sizeHint() );
    gl->addWidget (l, 0, 1);

    if (IsDir) 
	    l = new QLabel( i18n("Write\nEntries"), gb );
    else
	    l = new QLabel( i18n("Write"), gb );
    l->setMinimumSize( l->sizeHint() );
    gl->addWidget (l, 0, 2);

    if (IsDir)
	    l = new QLabel( i18n("Change\nInto"), gb );
    else
	    l = new QLabel( i18n("Exec"), gb );
    l->setMinimumSize( l->sizeHint() );
    gl->addWidget (l, 0, 3);

    l = new QLabel( i18n("Special"), gb );
    l->setMinimumSize( l->sizeHint() );
    gl->addWidget (l, 0, 4);

    l = new QLabel( i18n("User"), gb );
    l->setMinimumSize( l->sizeHint() );
    l->setEnabled (IamRoot || IsMyFile);
    gl->addWidget (l, 1, 0);

    l = new QLabel( i18n("Group"), gb );
    l->setMinimumSize( l->sizeHint() );
    l->setEnabled (IamRoot || IsMyFile);
    gl->addWidget (l, 2, 0);

    l = new QLabel( i18n("Others"), gb );
    l->setMinimumSize( l->sizeHint() );
    l->setEnabled (IamRoot || IsMyFile);
    gl->addWidget (l, 3, 0);

    /* Draw Checkboxes */
    for (int row = 0; row < 3 ; ++row) {
	    for (int col = 0; col < 4; ++col) {
	            QCheckBox *cb;
		    QString desc;

		    /* some boxes need further description .. */
		    switch (fperm[row][col]) {
		    case S_ISUID : desc = i18n("Set UID"); break;
		    case S_ISGID : desc = i18n("Set GID"); break;
		    case S_ISVTX : desc = i18n("Sticky"); break;
		    default      : desc = "";
		    }
		    
		    cb = new QCheckBox (desc, gb);
		    cb->setChecked ((buff.st_mode & fperm[row][col]) 
				    == fperm[row][col]);
		    cb->setEnabled ((IsMyFile || IamRoot) && (!IsLink));
		    cb->setMinimumSize (cb->sizeHint());
		    permBox[row][col] = cb;
		    gl->addWidget (permBox[row][col], row+1, col+1);
	    }
    }
    gl->setColStretch(5, 10);
    gl->activate();
    gb->adjustSize();

    /**** Group: Ownership ****/
    gb = new QGroupBox ( i18n("Ownership"), this );
    box->addWidget (gb);

    gl = new QGridLayout (gb, 4, 3, 15);

    /*** Set Owner ***/
    l = new QLabel( i18n("Owner"), gb );
    l->setMinimumSize( l->sizeHint() );
    gl->addWidget (l, 1, 0);
    
    /* maybe this should be a combo-box, but this isn't handy
     * if there are 2000 Users (tiny scrollbar!)
     * anyone developing a combo-box with incremental search .. ?
     */
    owner = new QLineEdit( gb );
    owner->setMinimumSize( owner->sizeHint().width(), fontHeight );
    gl->addWidget (owner, 1, 1);
    owner->setText( strOwner );
    owner->setEnabled ( IamRoot ); // we can just change the user if we're root

    /*** Set Group ***/
    /* get possible groups .. */
    QStrList *groupList = new QStrList (true);

    if (IsMyFile || IamRoot) {  // build list just if we have to
      setgrent();
      while ((ge = getgrent()) != 0L) {
	if (IamRoot)
	  groupList->inSort (ge->gr_name);
	else {
	  /* pick just the groups the user can change the file to */
	  char ** members = ge->gr_mem;
	  char * member;
	  while ((member = *members) != 0L) {
	    if (strcmp (member, strOwner.data()) == 0) {
		groupList->inSort (ge->gr_name);
		break;
	    }
	    ++members;
	  }
	}
      }
      endgrent();
      
      /* add the effective Group to the list .. */
      ge = getgrgid (getegid());
      if (ge) {
	  QString name = ge->gr_name;
	  if (name.isEmpty())
	      name.sprintf("%d",ge->gr_gid);
	if (groupList->find (name) < 0)
	  groupList->inSort (name); 
      }
    }
    
    /* add the group the file currently belongs to ..
     * .. if its not there already 
     */
    if (groupList->find (strGroup) < 0) 
	groupList->inSort (strGroup);
    
    l = new QLabel( i18n("Group"), gb );
    l->setMinimumSize( l->sizeHint() );
    gl->addWidget (l, 2, 0);

    if (groupList->count() > 1 && 
	(IamRoot || IsMyFile)) { 
      /* Combo-Box .. if there is anything to change */
      if (groupList->count() <= 10)
	/* Motif-Style looks _much_ nicer for few items */
	grp = new QComboBox( gb ); 
      else
	grp = new QComboBox( false, gb );
      
      grp->insertStrList ( groupList );
      grp->setCurrentItem (groupList->find ( strGroup ));
      grp->setMinimumSize( grp->sizeHint().width(), fontHeight );
      gl->addWidget (grp, 2, 1);
      grp->setEnabled (IamRoot || IsMyFile);
    }
    else {
      // no choice; just present the group in a disabled edit-field
      QLineEdit *e = new QLineEdit ( gb );
      e->setMinimumSize (e->sizeHint().width(), fontHeight);
      e->setText ( strGroup );
      e->setEnabled (false);
      gl->addWidget (e, 2, 1);
      grp = 0L;
    }
    
    delete groupList;

    gl->setColStretch(2, 10);
    gl->activate();
    gb->adjustSize();

    box->addStretch (10);
    box->activate();
}

bool FilePermissionsPropsPage::supports( KURL *_kurl, mode_t _mode )
{
    return _kurl->isLocalFile();
/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;

    return true;
*/
}

void FilePermissionsPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    QString fname = properties->getKURL()->filename();

    mode_t p = 0L;
    for (int row = 0;row < 3; ++row)
        for (int col = 0; col < 4; ++col)
	    if (permBox[row][col]->isChecked())
	        p |= fperm[row][col];

    // First update group / owner
    // (permissions have to set after, in case of suid and sgid)
    if ( strcmp( owner->text(), strOwner.data() ) != 0 || 
	 (grp && strcmp( grp->text(grp->currentItem()), strGroup.data() ) != 0 ))
    {
	struct passwd* pw = getpwnam( owner->text() );
	struct group* g;
	if (grp)
	    g = getgrnam( grp->text(grp->currentItem()) );
	else 
	    g = 0L;
	
	if ( pw == 0L )
	{
	    warning(i18n(" ERROR: No user %s"),(const char*)owner->text() );
	    return;
	}
	if ( g == 0L )
	{
	    warning(i18n(" ERROR: No group %s"),
		    (const char*)grp->text(grp->currentItem()) ); // should never happen
	    return;
	}
	if ( chown( path, pw->pw_uid, g->gr_gid ) != 0 )
	    QMessageBox::warning( 0, i18n( "KFM Error" ),
				  i18n( "Could not change owner/group\nPerhaps access denied." ) );
    }    

    if ( p != permissions )
    {
	if ( chmod( path, p ) != 0 )
	    QMessageBox::warning( 0, i18n( "KFM Error" ),
				  i18n( "Could not change permissions\nPerhaps access denied." ) );	    
    }

}

ExecPropsPage::ExecPropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    QVBoxLayout * mainlayout = new QVBoxLayout(this, SEPARATION);

    // Now the widgets in the top layout

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setText( i18n("Execute") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    mainlayout->addWidget(tmpQLabel, 1);

    QHBoxLayout * hlayout;
    hlayout = new QHBoxLayout(SEPARATION);
    mainlayout->addLayout(hlayout, 1);

    execEdit = new QLineEdit( this, "LineEdit_1" );
    execEdit->setText( "" );
    execEdit->setMaxLength( 256 );
    execEdit->setFixedHeight(fontHeight);
    execEdit->setMinimumSize( execEdit->sizeHint() );
    hlayout->addWidget(execEdit, 0);

    execBrowse = new QPushButton( this, "Button_1" );
    execBrowse->setText( i18n("&Browse...") );
    execBrowse->setFixedSize(execBrowse->sizeHint());
    hlayout->addWidget(execBrowse, 0);

    hlayout = new QHBoxLayout(SEPARATION);
    mainlayout->addLayout(hlayout, 2); // double stretch, because two items

    /* instead, a label */
    hlayout->addStretch(1);

    // and the icon button, on the right of this vertical layout
    iconBox = new KIconLoaderButton( kapp->getIconLoader(), this );
    iconBox->setFixedSize( 50, 50 );
    hlayout->addWidget(iconBox, 0, AlignCenter);

    // The groupbox about swallowing

    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox( i18n("Swallowing on panel"), this, "GroupBox_1" );
    tmpQGroupBox->setFrameStyle( 49 );
    mainlayout->addWidget(tmpQGroupBox, 3); // 3 vertical items -> stretch = 3

    QVBoxLayout * grouplayout;
    grouplayout = new QVBoxLayout(tmpQGroupBox, SEPARATION);

    grouplayout->addSpacing( fontMetrics().height() ); 

    hlayout = new QHBoxLayout(SEPARATION);
    grouplayout->addLayout(hlayout, 0);

    tmpQLabel = new QLabel( tmpQGroupBox, "Label_6" );
    tmpQLabel->setText( i18n("Execute") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    hlayout->addWidget(tmpQLabel, 0);

    swallowExecEdit = new QLineEdit( tmpQGroupBox, "LineEdit_3" );
    swallowExecEdit->setText( "" );
    swallowExecEdit->setFixedHeight(fontHeight);
    swallowExecEdit->setMinimumSize( swallowExecEdit->sizeHint() );
    hlayout->addWidget(swallowExecEdit, 1);

    hlayout = new QHBoxLayout(SEPARATION);
    grouplayout->addLayout(hlayout, 0);

    tmpQLabel = new QLabel( tmpQGroupBox, "Label_7" );
    tmpQLabel->setText( i18n("Window Title") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    hlayout->addWidget(tmpQLabel, 0);

    swallowTitleEdit = new QLineEdit( tmpQGroupBox, "LineEdit_4" );
    swallowTitleEdit->setText( "" );
    swallowTitleEdit->setFixedHeight(fontHeight);
    swallowTitleEdit->setMinimumSize( swallowTitleEdit->sizeHint() );
    hlayout->addWidget(swallowTitleEdit, 1);

    // The groupbox about run in terminal

    tmpQGroupBox = new QGroupBox( this, "GroupBox_2" );
    tmpQGroupBox->setFrameStyle( 49 );
    tmpQGroupBox->setAlignment( 1 );
    mainlayout->addWidget(tmpQGroupBox, 2);  // 2 vertical items -> stretch = 2

    grouplayout = new QVBoxLayout(tmpQGroupBox, SEPARATION);

    terminalCheck = new QCheckBox( tmpQGroupBox, "RadioButton_3" );
    terminalCheck->setText( i18n("Run in terminal") );
    terminalCheck->setMinimumSize( terminalCheck->sizeHint() );
    grouplayout->addWidget(terminalCheck, 0);

    hlayout = new QHBoxLayout(SEPARATION);
    grouplayout->addLayout(hlayout, 1);

    tmpQLabel = new QLabel( tmpQGroupBox, "Label_5" );
    tmpQLabel->setText( i18n("Terminal Options") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    hlayout->addWidget(tmpQLabel, 0);

    terminalEdit = new QLineEdit( tmpQGroupBox, "LineEdit_5" );
    terminalEdit->setText( "" );
    terminalEdit->setFixedHeight(fontHeight);
    terminalEdit->setMinimumSize( terminalEdit->sizeHint() );
    hlayout->addWidget(terminalEdit, 1);

    mainlayout->activate();

    //

    QFile f( _props->getKURL()->path() );
    if ( !f.open( IO_ReadOnly ) )
	return;    
    f.close();

    KConfig config( _props->getKURL()->path() );
    config.setDollarExpansion( false );
    config.setGroup( "KDE Desktop Entry" );
    execStr = config.readEntry( "Exec" );
    iconStr = config.readEntry( "Icon" );
    swallowExecStr = config.readEntry( "SwallowExec" );
    swallowTitleStr = config.readEntry( "SwallowTitle");
    termStr = config.readEntry( "Terminal" );
    termOptionsStr = config.readEntry( "TerminalOptions" );

    if ( !swallowExecStr.isNull() )
	swallowExecEdit->setText( swallowExecStr.data() );
    if ( !swallowTitleStr.isNull() )
	swallowTitleEdit->setText( swallowTitleStr.data() );

    if ( !execStr.isNull() )
	execEdit->setText( execStr.data() );
    if ( !termOptionsStr.isNull() )
        terminalEdit->setText( termOptionsStr.data() );

    if ( !termStr.isNull() )
        terminalCheck->setChecked( termStr == "1" );
    enableCheckedEdit();

    if ( iconStr.isNull() )
        iconStr = KMimeType::find("")->icon(0L, true); // default icon
    
    iconBox->setIcon( iconStr ); 

    connect( execBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowseExec() ) );
    connect( terminalCheck, SIGNAL( clicked() ), this,  SLOT( enableCheckedEdit() ) );

}


void ExecPropsPage::enableCheckedEdit()
{
  terminalEdit->setEnabled(terminalCheck->isChecked());
}


bool ExecPropsPage::supports( KURL *_kurl, mode_t _mode )
{
/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
*/
    if (!_kurl->isLocalFile())
        return false;

    QString t( _kurl->path() );

    struct stat buff;
    stat( t, &buff );

    struct stat lbuff;
    lstat( t, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;

    FILE *f = fopen( t, "r" );
    if ( f == 0L )
      return false;

    char buffer[ 101 ];
    int n = fread( buffer, 1, 100, f );
    fclose( f );
    if ( n <= 0 )
      return false;
    if ( strncmp( buffer, "# KDE Config File", strlen( "# KDE Config File" ) ) != 0L )
      return false;

    KConfig config( t );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "Application" )
	return false;
    
    return true;
}

void ExecPropsPage::applyChanges()
{
#ifdef SVEN
// --- Sven's editable global settings changes start ---
    int i = 0;
    bool err = false;
// --- Sven's editable global settings changes end ---
#endif
    QString path = properties->getKURL()->path();

    QFile f( path );

#ifdef SVEN
    QDir lDir (kapp->localkdedir() + "/share/applnk/"); // I know it exists

    //debug (path.data());
    //debug (kapp->kde_appsdir().data());
#endif
    if ( !f.open( IO_ReadWrite ) )
    {
#ifdef SVEN
      // path = /usr/local/kde/share/applnk/network/netscape.kdelnk

      //Does path contain kde_appsdir?
      if (path.find(kapp->kde_appsdir()) == 0) // kde_appsdir on start of path
      {
	path.remove(0, strlen(kapp->kde_appsdir())); //remove kde_appsdir

	if (path[0] == '/')
	  path.remove(0, 1); // remove /

	while (path.contains('/'))
	{
	  i = path.find('/'); // find separator
	  if (!lDir.cd(path.left(i)))  // exists?
	  {
	    lDir.mkdir((path.left(i)));  // no, create
	    if (!lDir.cd((path.left(i)))) // can cd to?
	    {
	      err = true;                 // no flag it...
	      // debug ("Can't cd to  %s in %s", path.left(i).data(),
	      //	 lDir.absPath().data());
	      break;                      // and exit while
	    }
	  }
	  path.remove (0, i);           // cded to;
	  if (path[0] == '/')
	    path.remove(0, 1); // remove / from path
	}
      }
      else // path didn't contain kde_appsdir - this is an error
	err = true;

      // we created all subdirs or failed
      if (!err) // if we didn't fail try to make file just for check
      {
	path.prepend("/"); // be sure to have /netscape.kdelnk
	path.prepend(lDir.absPath());
	f.setName(path);
	//debug("path = %s", path.data());

	// we must not copy whole kdelnk to local dir
	//  because it was done in ApplicationPropsPage
	if (!f.open( IO_ReadWrite ) )
	  err = true;
      }
      if (err)
      {
#endif
      
	QMessageBox::warning( 0, i18n("KFM Error"),
			      i18n("Could not save properties\nPerhaps permissions denied") );
	return;
#ifdef SVEN
      }
#endif
    }

    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "Exec", execEdit->text() );
    config.writeEntry( "Icon", iconBox->icon() );
    config.writeEntry( "SwallowExec", swallowExecEdit->text() );
    config.writeEntry( "SwallowTitle", swallowTitleEdit->text() );

    if ( terminalCheck->isChecked() )
    {
        config.writeEntry( "Terminal", "1" );
        config.writeEntry( "TerminalOptions", terminalEdit->text() );
    }
    else
        config.writeEntry( "Terminal", "0" );

    config.sync();
}


void ExecPropsPage::slotBrowseExec()
{
    QString f = KFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( f.isNull() )
	return;

    execEdit->setText( f.data() );
}

URLPropsPage::URLPropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    QVBoxLayout * layout = new QVBoxLayout(this, SEPARATION);
    URLEdit = new QLineEdit( this, "LineEdit_1" );
    iconBox = new KIconLoaderButton( kapp->getIconLoader(), this );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setText( i18n("URL") );
    tmpQLabel->adjustSize();
    tmpQLabel->setFixedHeight( fontHeight );
    layout->addWidget(tmpQLabel);

    URLEdit->setText( "" );
    URLEdit->setMaxLength( 256 );
    URLEdit->setMinimumSize( URLEdit->sizeHint() );
    URLEdit->setFixedHeight( fontHeight );
    layout->addWidget(URLEdit);
    
    iconBox->setFixedSize( 50, 50 );
    layout->addWidget(iconBox, 0, AlignLeft);

    QString path = _props->getKURL()->path();

    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    URLStr = config.readEntry(  "URL" );
    iconStr = config.readEntry( "Icon" );

    if ( !URLStr.isNull() )
	URLEdit->setText( URLStr.data() );
    if ( iconStr.isNull() )
        iconStr = KMimeType::find("")->icon(0L, true); // default icon

    iconBox->setIcon( iconStr );

    layout->addStretch (10);
    layout->activate();

}

bool URLPropsPage::supports( KURL *_kurl, mode_t _mode )
{
/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
*/
    if (!_kurl->isLocalFile())
        return false;

    QString path = _kurl->path();

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;

    FILE *f = fopen( path, "r" );
    if ( f == 0L )
      return false;

    char buffer[ 101 ];
    int n = fread( buffer, 1, 100, f );
    fclose( f );
    if ( n <= 0 )
      return false;
    if ( strncmp( buffer, "# KDE Config File", strlen( "# KDE Config File" ) ) != 0L )
      return false;

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );

    QString URL = config.readEntry( "URL" );
    if ( URL.isNull() )
	return false;
    
    return true;    
}

void URLPropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();

    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::warning( 0, i18n("KFM Error"), 
			        i18n("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "URL", URLEdit->text() );
    config.writeEntry( "Icon", iconBox->icon() );
    config.writeEntry( "MiniIcon", iconBox->icon() );
    config.sync();
}

DirPropsPage::DirPropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    // This is strange, but if we try to use a layout in this page, it centers
    // the other pages of the dialog (!?). Ok, no layout ! DF.

    // See resize event for widgets placement
    iconBox = new KIconLoaderButton( kapp->getIconLoader(), this );

    QLabel* tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setText( i18n("Background") );
    tmpQLabel->move( 10, 90 );
    tmpQLabel->adjustSize();

    wallBox = new QComboBox( false, this, "ComboBox_1" );

    applyButton = new QPushButton( i18n("Apply") , this );
    applyButton->adjustSize();
    connect( applyButton, SIGNAL( clicked() ), this, SLOT( slotApply() ) );
    
    globalButton = new QPushButton( i18n("Apply global"), this );
    globalButton->adjustSize();
    connect( globalButton, SIGNAL( clicked() ), this, SLOT( slotApplyGlobal() ) );

    QString tmp = _props->getKURL()->path();
    
    if ( tmp.right(1) != "/" )
	tmp += "/.directory";
    else
	tmp += ".directory";

    QFile f( tmp.data() );
    if ( f.open( IO_ReadOnly ) )
    {
	f.close();

	KConfig config( tmp );
	config.setGroup( "KDE Desktop Entry" );
	wallStr = config.readEntry( "BgImage" );
	iconStr = config.readEntry( "Icon" );
    }

    if ( iconStr.isEmpty() )
    {
	QString str( KMimeType::findByURL( *properties->getKURL(), properties->getMode(), true )->icon(0L, true) );
	KURL u( str );
	iconStr = u.filename();
    }
    
    iconBox->setIcon( iconStr );

    // Load all wallpapers in the combobox
    tmp = kapp->kde_wallpaperdir().copy();
    QDir d2( tmp.data() );
    const QFileInfoList *list = d2.entryInfoList();  
    QFileInfoListIterator it2( *list );      // create list iterator
    QFileInfo *fi;                          // pointer for traversing  

    wallBox->insertItem(  i18n("(Default)"), 0 );
    
    while ( ( fi = it2.current() ) )
    {
	if ( fi->fileName() != ".." && fi->fileName() != "." )
	{
	    wallBox->insertItem( fi->fileName().data() );
	}
	++it2;                               // goto next list element
    }
    
    showSettings( wallStr );

    browseButton = new QPushButton( i18n("&Browse..."), this );
    browseButton->adjustSize();
    connect( browseButton, SIGNAL( clicked() ), SLOT( slotBrowse() ) );

    drawWallPaper();

    connect( wallBox, SIGNAL( activated( int ) ), this, SLOT( slotWallPaperChanged( int ) ) );
}

bool DirPropsPage::supports( KURL *_kurl, mode_t _mode )
{
    // Is it the trash bin ?
    QString path = _kurl->path();
    
    QString tmp = path.data();
    
    if ( tmp.right(1) != "/" )
	tmp += "/";
    if ( strcmp( _kurl->protocol(), "file" ) == 0L &&
	 tmp == KfmPaths::trashPath()) 
        return false;

/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
*/
    if (!_kurl->isLocalFile())
        return false;

    if ( !S_ISDIR( _mode ) )
      return false;
    
    return true;    
}

void DirPropsPage::applyChanges()
{
    QString tmp = properties->getKURL()->path();
    if ( tmp.right(1) != "/" )
	tmp += "/.directory";
    else
	tmp += ".directory";

    QFile f( tmp.data() );
    if ( !f.open( IO_ReadWrite ) )
    {
      QMessageBox::warning( 0, i18n("KFM Error"), 
			     i18n("Could not write to\n") + tmp );
	return;
    }
    f.close();

    KConfig config( tmp );
    config.setGroup( "KDE Desktop Entry" );

    int i = wallBox->currentItem();
    if ( i != -1 )
    {
	if ( strcmp( wallBox->text( i ),  i18n("(Default)") ) == 0 )
	    config.writeEntry( "BgImage", "" );
	else
	    config.writeEntry( "BgImage", wallBox->text( i ) );
    }

    // Get the default image
    QString str( KMimeType::findByURL( *properties->getKURL(), properties->getMode(), true )->icon(0L, "") );
    KURL u( str );
    QString str2 = u.filename();
    QString sIcon;
    // Is it another one than the default ?
    if ( str2 != iconBox->icon() )
        sIcon = iconBox->icon();
    config.writeEntry( "Icon", sIcon );
    config.writeEntry( "MiniIcon", sIcon );
    
    config.sync();

    // Send a notify to the parent directory since the
    // icon may have changed

    tmp = properties->getKURL()->url();
    
    if ( tmp.right(1) == "/" )
	tmp = tmp.left( tmp.length() - 1 );
    
    i = tmp.findRev( "/" );
    // Should never happen
    if ( i == -1 )
	return;
    tmp = tmp.left( i + 1 );
/*
    KIOServer::sendNotify( tmp.data() );
*/
}

void DirPropsPage::showSettings( QString filename )
{
  wallBox->setCurrentItem( 0 );
  for ( int i = 1; i < wallBox->count(); i++ )
    {
      if ( filename == wallBox->text( i ) )
        {
          wallBox->setCurrentItem( i );
          return;
        }
    }
 
  if ( !filename.isEmpty() )
    {
      wallBox->insertItem( filename );
      wallBox->setCurrentItem( wallBox->count()-1 );
    }
}

void DirPropsPage::slotBrowse( )
{
    QString filename = KFileDialog::getOpenFileName( kapp->kde_wallpaperdir().copy() );
    showSettings( filename );
    drawWallPaper( );
}

void DirPropsPage::slotWallPaperChanged( int )
{
    drawWallPaper();
}

void DirPropsPage::paintEvent( QPaintEvent *_ev )
{
    PropsPage::paintEvent( _ev );
    drawWallPaper();
}

void DirPropsPage::drawWallPaper()
{
    int i = wallBox->currentItem();
    if ( i == -1 )
    {
	erase( imageX, imageY, imageW, imageH );
	return;
    }
    
    QString text = wallBox->text( i );
    if ( text == i18n( "(Default)" ) )
    {
	erase( imageX, imageY, imageW, imageH );
	return;
    }

    QString file;
    if (text.left(0) == "/") { // absolute path
      file = kapp->kde_wallpaperdir().copy();
      file += "/";
      file += text;
    } else file = text;
    
    if ( file != wallFile )
    {
	// debugT("Loading WallPaper '%s'\n",file.data());
	wallFile = file;
	wallPixmap.load( file );
    }
    
    if ( wallPixmap.isNull() )
	warning("Could not load wallpaper %s\n",file.data());
    
    erase( imageX, imageY, imageW, imageH );
    QPainter painter;
    painter.begin( this );
    painter.setClipRect( imageX, imageY, imageW, imageH );
    painter.drawPixmap( QPoint( imageX, imageY ), wallPixmap );
    painter.end();
}

void DirPropsPage::resizeEvent ( QResizeEvent *)
{
    imageX = 180; // X of the image (10 + width of the combobox + 10)
    // could be calculated in the future, depending on the length of the items
    // in the list.
    imageY = 90 + fontHeight; // so that combo & image are under the label
    imageW = width() - imageX - SEPARATION;
    imageH = height() - imageY - applyButton->height() - SEPARATION*2;

    iconBox->setGeometry( 10, 20, 50, 50 );
    wallBox->setGeometry( 10, imageY, imageX-20, 30 );
    browseButton->move( 10, wallBox->y()+wallBox->height()+SEPARATION );
    applyButton->move( 10, imageY+imageH+SEPARATION );
    globalButton->move( applyButton->x() + applyButton->width() + SEPARATION, applyButton->y() );
    
}

void DirPropsPage::slotApply()
{
    applyChanges();
/*
    KIOServer::sendNotify( properties->getURL() );
*/
}

void DirPropsPage::slotApplyGlobal()
{
    KConfig *config = KApplication::getKApplication()->getConfig();
    
    config->setGroup( "KFM HTML Defaults" );

    int i = wallBox->currentItem();
    if ( i != -1 )
    {
	if ( strcmp( wallBox->text( i ),  i18n("(None)") ) == 0 )
	    config->writeEntry( "BgImage", "" );
	else
	    config->writeEntry( "BgImage", wallBox->text( i ) );
    }

    config->sync();

/*
    // Notify all opened windows
    QStrList strlist;
    QList<KfmGui>& list = KfmGui::getWindowList();
    KfmGui *win;
    for ( win = list.first(); win != 0L; win = list.next() )
	strlist.append( win->getURL() );
    
    char *s;
    for ( s = strlist.first(); s != 0L; s = strlist.next() )
	KIOServer::sendNotify( s );
*/
}

/* ----------------------------------------------------
 *
 * ApplicationPropsPage
 *
 * -------------------------------------------------- */

ApplicationPropsPage::ApplicationPropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    layout = new QBoxLayout(this, QBoxLayout::TopToBottom, SEPARATION);
    binaryPatternEdit = new QLineEdit( this, "LineEdit_1" );
    commentEdit = new QLineEdit( this, "LineEdit_2" );
    nameEdit = new QLineEdit( this, "LineEdit_3" );

    extensionsList = new QListBox( this );
    availableExtensionsList = new QListBox( this );
    addExtensionButton = new QPushButton( "<-", this );
    delExtensionButton = new QPushButton( "->", this );

    binaryPatternEdit->raise();
    binaryPatternEdit->setMinimumSize(210, fontHeight);
    binaryPatternEdit->setMaximumSize(QLayout::unlimited, fontHeight);
    binaryPatternEdit->setText( "" );
    binaryPatternEdit->setMaxLength( 512 );

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setText(  i18n("Binary Pattern (netscape;Netscape;)") );
    tmpQLabel->setFixedSize(tmpQLabel->sizeHint());
    layout->addWidget(tmpQLabel, 0, AlignLeft);

    layout->addWidget(binaryPatternEdit, 0, AlignLeft);

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setText(  i18n("Comment") );
    tmpQLabel->setFixedSize(tmpQLabel->sizeHint());
    layout->addWidget(tmpQLabel, 0, AlignLeft);

    commentEdit->raise();
    commentEdit->setMinimumSize(210, fontHeight);
    commentEdit->setMaximumSize(QLayout::unlimited, fontHeight);
    commentEdit->setMaxLength( 256 );
    layout->addWidget(commentEdit, 0, AlignLeft);

    tmpQLabel = new QLabel( this, "Label_4" );
    tmpQLabel->setText(  i18n("Name ( in your language )") );
    tmpQLabel->setFixedSize(tmpQLabel->sizeHint());
    layout->addWidget(tmpQLabel, 0, AlignLeft);

    nameEdit->raise();
    nameEdit->setMaxLength( 256 );
    nameEdit->setMinimumSize(210, fontHeight);
    nameEdit->setMaximumSize(QLayout::unlimited, fontHeight);
    layout->addWidget(nameEdit, 0, AlignLeft);

    layoutH = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addLayout(layoutH, 10);
    
    layoutH->addWidget(extensionsList, 10);

    layoutV = new QBoxLayout(QBoxLayout::TopToBottom);
    layoutH->addLayout(layoutV, 0);
    layoutV->addStretch(3);
    addExtensionButton->setFixedSize(40, 40);
    layoutV->addWidget(addExtensionButton, 0);
    layoutV->addStretch(3);
    connect( addExtensionButton, SIGNAL( pressed() ), 
	     this, SLOT( slotAddExtension() ) );
    delExtensionButton->setFixedSize(40, 40);
    layoutV->addWidget(delExtensionButton, 0);
    layoutV->addStretch(3);
    connect( delExtensionButton, SIGNAL( pressed() ), 
	     this, SLOT( slotDelExtension() ) );
    layoutH->addWidget(availableExtensionsList, 10);

    layout->activate();

    QString path = _props->getKURL()->path() ;
    //KURL::decodeURL( path );	    
    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    commentStr = config.readEntry( "Comment" );
    binaryPatternStr = config.readEntry( "BinaryPattern" );
    extensionsStr = config.readEntry( "MimeType" );
    nameStr = config.readEntry( "Name" );
    // Use the file name if no name is specified
    if ( nameStr.isEmpty() )
    {
	nameStr = _props->getKURL()->filename();
	if ( nameStr.right(7) == ".kdelnk" )
	    nameStr.truncate( nameStr.length() - 7 );
	//KURL::decodeURL( nameStr );
    }
    
    if ( !commentStr.isNull() )
	commentEdit->setText( commentStr.data() );
    if ( !nameStr.isNull() )
	nameEdit->setText( nameStr.data() );
    if ( !binaryPatternStr.isNull() )
	binaryPatternEdit->setText( binaryPatternStr.data() );
    if ( !extensionsStr.isNull() )
    {
	int pos = 0;
	int pos2 = 0;
	while ( ( pos2 = extensionsStr.find( ";", pos ) ) != -1 )
	{
	    extensionsList->inSort( extensionsStr.mid( pos, pos2 - pos ) );
	    pos = pos2 + 1;
	}
    }

    addMimeType( "all" );
    addMimeType( "alldirs" );
    addMimeType( "allfiles" );

    QDictIterator<KMimeType> it ( * KMimeType::allTypes() );
    for ( ; it.current(); ++it )
        addMimeType ( it.currentKey() );
}

void ApplicationPropsPage::addMimeType( const char * name )
{
    // Add a mimetype to the list of available mime types if not in the extensionsList

    bool insert = true;
	
    for ( uint i = 0; i < extensionsList->count(); i++ )
        if ( strcmp( name, extensionsList->text( i ) ) == 0 )
            insert = false;
	
    if ( insert )
        availableExtensionsList->inSort( name );
}

bool ApplicationPropsPage::supports( KURL *_kurl, mode_t _mode )
{
/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
*/
    if (!_kurl->isLocalFile())
        return false;

    QString path = _kurl->path();

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;

    FILE *f = fopen( path, "r" );
    if ( f == 0L )
      return false;

    char buffer[ 101 ];
    int n = fread( buffer, 1, 100, f );
    fclose( f );
    if ( n <= 0 )
      return false;
    if ( strncmp( buffer, "# KDE Config File", strlen( "# KDE Config File" ) ) != 0L )
      return false;

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "Application" )
	return false;
    
    return true;
}

void ApplicationPropsPage::applyChanges()
{
#ifdef SVEN
// --- Sven's editable global settings changes start ---
    int i = 0;
    bool err = false;
// --- Sven's editable global settings changes end ---
#endif
    
    QString path = properties->getKURL()->path();

    QFile f( path );
    
#ifdef SVEN
    QDir lDir (kapp->localkdedir() + "/share/applnk/"); // I know it exists

    //debug (path.data());
    //debug (kapp->kde_appsdir().data());
#endif
    
    if ( !f.open( IO_ReadWrite ) )
    {
#ifdef SVEN
      // path = /usr/local/kde/share/applnk/network/netscape.kdelnk

      //Does path contain kde_appsdir?
      if (path.find(kapp->kde_appsdir()) == 0) // kde_appsdir on start of path
      {
	path.remove(0, strlen(kapp->kde_appsdir())); //remove kde_appsdir

	if (path[0] == '/')
	  path.remove(0, 1); // remove /

	while (path.contains('/'))
	{
	  i = path.find('/'); // find separator
	  if (!lDir.cd(path.left(i)))  // exists?
	  {
	    lDir.mkdir((path.left(i)));  // no, create
	    if (!lDir.cd((path.left(i)))) // can cd to?
	    {
	      err = true;                 // no flag it...
	      // debug ("Can't cd to  %s in %s", path.left(i).data(),
	      //	 lDir.absPath().data());
	      break;                      // and exit while
	    }
	    // Begin copy .directory if exists here.
	    // This block can be commented out without problems
	    // in case of problems.
	    {
	      QFile tmp(kapp->kde_appsdir() +
			"/" + path.left(i) + "/.directory");
	      //debug ("---- looking for: %s", tmp.name());
	      if (tmp.open( IO_ReadOnly))
	      {
		//debug ("--- opened RO");
		char *buff = new char[tmp.size()+10];
		if (buff != 0)
		{
		  if (tmp.readBlock(buff, tmp.size()) != -1)
		  {
		    size_t tmpsize = tmp.size();
		    //debug ("--- read");
		    tmp.close();
		    tmp.setName(lDir.absPath() + "/.directory");
		    //debug ("---- copying to: %s", tmp.name());
		    if (tmp.open(IO_ReadWrite))
		    {
		      //debug ("--- opened RW");
		      if (tmp.writeBlock(buff, tmpsize) != -1)
		      {
			//debug ("--- wrote");
			tmp.close();
		      }
		      else
		      {
                        //debug ("--- removed");
			tmp.remove();
		      }
		    }                 // endif can open to write
		  }                   // endif can read
		  else     //coulnd't read
		    tmp.close();

		  delete[] buff;
		}                     // endif is alocated
	      }                       // can open to write
	    }
	    // End coping .directory file
	    
	  }
	  path.remove (0, i);           // cded to;
	  if (path[0] == '/')
	    path.remove(0, 1); // remove / from path
	}
      }
      else // path didn't contain kde_appsdir - this is an error
	err = true;

      // we created all subdirs or failed
      if (!err) // if we didn't fail try to make file just for check
      {
	path.prepend("/"); // be sure to have /netscape.kdelnk
	path.prepend(lDir.absPath());
	f.setName(path);
	//debug("path = %s", path.data());
	if ( f.open( IO_ReadWrite ) )
	{
	  // we must first copy whole kdelnk to local dir
	  // and then change it. Trust me.
	  QFile s(properties->getKURL()->path());
	  s.open(IO_ReadOnly);
	  //char *buff = (char *) malloc (s.size()+10);   CHANGE TO NEW!
	  char *buff = new char[s.size()+10];           // Done.
	  if (buff != 0)
	  {
	    //debug ("********About to copy");
	    if (s.readBlock(buff, s.size()) != -1 &&
		f.writeBlock(buff, s.size()) != -1)
	      ; // ok
	    else
	    {
	      err = true;
	      f.remove();
	    }
	    //free ((void *) buff);                      CHANGE TO DELETE!
	    delete[] buff;                            // Done.
	  }
	  else
	    err = true;
	}
	else
	  err = true;
      }
      if (err)
      {
	//debug ("************Cannot save");
#endif

	QMessageBox::warning( 0, i18n("KFM Error"),
			        i18n("Could not save properties\nPerhaps permissions denied") );
	return;
#ifdef SVEN
      }
#endif
    }
    
    f.close(); // kalle
    KConfig config( path ); // kalle
    config.setGroup( "KDE Desktop Entry" );
    config.writeEntry( "Comment", commentEdit->text(), true, false, true );

    QString tmp = binaryPatternEdit->text();
    if ( tmp.length() > 0 )
	if ( tmp.right(1) != ";" )
	    tmp += ";";
    config.writeEntry( "BinaryPattern", tmp.data() );

    extensionsStr = "";
    for ( uint i = 0; i < extensionsList->count(); i++ )
    {
	extensionsStr += extensionsList->text( i );
	extensionsStr += ";";
    }
    config.writeEntry( "MimeType", extensionsStr.data() );
    config.writeEntry( "Name", nameEdit->text(), true, false, true );
    
    config.sync();
    f.close();

    /*
    KMimeType::clearAll();
    KMimeType::init();
    if ( KRootWidget::getKRootWidget() )
	KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = KfmGui::getWindowList().first(); win != 0L; win = KfmGui::getWindowList().next() )
	win->updateView();
    */
}

void ApplicationPropsPage::slotAddExtension()
{
    int pos = availableExtensionsList->currentItem();
   
    if ( pos == -1 )
	return;
    
    extensionsList->inSort( availableExtensionsList->text( pos ) );
    availableExtensionsList->removeItem( pos );
}

void ApplicationPropsPage::slotDelExtension()
{
    int pos = extensionsList->currentItem();
   
    if ( pos == -1 )
	return;
    
    availableExtensionsList->inSort( extensionsList->text( pos ) );
    extensionsList->removeItem( pos );
}

/* ----------------------------------------------------
 *
 * BindingPropsPage
 *
 * -------------------------------------------------- */

BindingPropsPage::BindingPropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    patternEdit = new QLineEdit( this, "LineEdit_1" );
    commentEdit = new QLineEdit( this, "LineEdit_3" );
    mimeEdit = new QLineEdit( this, "LineEdit_3" );
    iconBox = new KIconLoaderButton( kapp->getIconLoader(), this );
    appBox = new QComboBox( false, this, "ComboBox_2" );

    QBoxLayout * mainlayout = new QVBoxLayout(this, SEPARATION);
    QLabel* tmpQLabel;

    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->setText(  i18n("Pattern ( example: *.html;*.HTML; )") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    mainlayout->addWidget(tmpQLabel, 1);

    patternEdit->setGeometry( 10, 40, 210, 30 );
    patternEdit->setText( "" );
    patternEdit->setMaxLength( 512 );
    patternEdit->setMinimumSize( patternEdit->sizeHint() );
    patternEdit->setFixedHeight( fontHeight );
    mainlayout->addWidget(patternEdit, 1);

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setText(  i18n("Mime Type") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    mainlayout->addWidget(tmpQLabel, 1);

    mimeEdit->setGeometry( 10, 160, 210, 30 );
    mimeEdit->setMaxLength( 256 );
    mimeEdit->setMinimumSize( mimeEdit->sizeHint() );
    mimeEdit->setFixedHeight( fontHeight );
    mainlayout->addWidget(mimeEdit, 1);

    tmpQLabel = new QLabel( this, "Label_3" );
    tmpQLabel->setText(  i18n("Comment") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    mainlayout->addWidget(tmpQLabel, 1);

    commentEdit->setGeometry( 10, 100, 210, 30 );
    commentEdit->setMaxLength( 256 );
    commentEdit->setMinimumSize( commentEdit->sizeHint() );
    commentEdit->setFixedHeight( fontHeight );
    mainlayout->addWidget(commentEdit, 1);

    QHBoxLayout * hlayout = new QHBoxLayout(SEPARATION);
    mainlayout->addLayout(hlayout, 2); // double stretch, because two items

    QVBoxLayout * vlayout; // a vertical layout for the two following items
    vlayout = new QVBoxLayout(SEPARATION);
    hlayout->addLayout(vlayout, 1);

    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->setText(  i18n("Default Application") );
    tmpQLabel->setMinimumSize(tmpQLabel->sizeHint());
    vlayout->addWidget(tmpQLabel, 1);

    appBox->setFixedHeight( fontHeight );
    vlayout->addWidget(appBox, 1);

    iconBox->setFixedSize( 50, 50 );
    hlayout->addWidget(iconBox, 0);

    mainlayout->addSpacing(fontMetrics().height());
    mainlayout->activate();

    QFile f( _props->getKURL()->path() );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( _props->getKURL()->path() );
    config.setGroup( "KDE Desktop Entry" );
    QString patternStr = config.readEntry( "Patterns" );
    QString appStr = config.readEntry( "DefaultApp" );
    QString iconStr = config.readEntry( "Icon" );
    QString commentStr = config.readEntry( "Comment" );
    QString mimeStr = config.readEntry( "MimeType" );

    if ( !patternStr.isEmpty() )
	patternEdit->setText( patternStr.data() );
    if ( !commentStr.isEmpty() )
	commentEdit->setText( commentStr.data() );
    if ( iconStr.isEmpty() )
        iconStr = KMimeType::find("")->icon(0L, true); // default icon
    if ( !mimeStr.isEmpty() )
	mimeEdit->setText( mimeStr.data() );
    
    iconBox->setIcon( iconStr );
    
    // Get list of all applications
    QStrList applist;
    QString currApp;
    appBox->insertItem( i18n("<none>") );
    kdelnklist.append( "" ); // empty item
    QListIterator<KService> it ( * KService::allServices() );
    for ( ; it.current(); ++it )
    {
	currApp = it.current()->name();

	// list every app only once
	if ( applist.find( currApp ) == -1 ) { 
	    appBox->insertItem( currApp );
	    applist.append( currApp );
            kdelnklist.append( it.current()->file());
	}
    }
    
    // Set the default app (DefaultApp=... is the kdelnk name)
    int index = kdelnklist.find( appStr );
    //debug ("appStr = %s\n\n",appStr.data());
    if ( index == -1 )
	index = 0;
    appBox->setCurrentItem( index );
}

bool BindingPropsPage::supports( KURL *_kurl, mode_t _mode )
{
/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
*/
    if (!_kurl->isLocalFile())
        return false;

    QString path = _kurl->path();

    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
	return false;

    FILE *f = fopen( path, "r" );
    if ( f == 0L )
      return false;

    char buffer[ 101 ];
    int n = fread( buffer, 1, 100, f );
    fclose( f );
    if ( n <= 0 )
      return false;
    if ( strncmp( buffer, "# KDE Config File", strlen( "# KDE Config File" ) ) != 0L )
      return false;

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "MimeType" )
	return false;
    
    return true;
}

void BindingPropsPage::applyChanges()
{
#ifdef SVEN
// --- Sven's editable global settings changes start ---
    int i = 0;
    bool err = false;
    // --- Sven's editable global settings changes end ---
#endif
    
    QString path = properties->getKURL()->path();

    QFile f( path );

#ifdef SVEN
    QDir lDir (kapp->localkdedir() + "/share/mimelnk/"); // I know it exists

    //debug (path.data());
    //debug (kapp->kde_mimedir().data());
#endif
    
    if ( !f.open( IO_ReadWrite ) )
    {
#ifdef SVEN
      // path = /usr/local/kde/share/mimelnk/image/jpeg.kdelnk
      
      //Does path contain kde_mimedir?
      if (path.find(kapp->kde_mimedir()) == 0) // kde_mimedir on start of path
      {
	path.remove(0, strlen(kapp->kde_mimedir())); //remove kde_mimedir

	if (path[0] == '/')
	  path.remove(0, 1); // remove /
	
	while (path.contains('/'))
	{
	  i = path.find('/'); // find separator
	  if (!lDir.cd(path.left(i)))  // exists?
	  {
	    lDir.mkdir((path.left(i)));  // no, create
	    if (!lDir.cd((path.left(i)))) // can cd to?
	    {
	      err = true;                 // no flag it...
	      // debug ("Can't cd to  %s in %s", path.left(i).data(),
	      //	 lDir.absPath().data());
	      break;                      // and exit while
	    }
	    // Begin copy .directory if exists here.
	    // This block can be commented out without problems
	    // in case of problems.
	    {
	      QFile tmp(kapp->kde_mimedir() +
			"/" + path.left(i) + "/.directory");
	      //debug ("---- looking for: %s", tmp.name());
	      if (tmp.open( IO_ReadOnly))
	      {
		//debug ("--- opened RO");
		char *buff = new char[tmp.size()+10];
		if (buff != 0)
		{
		  if (tmp.readBlock(buff, tmp.size()) != -1)
		  {
		    size_t tmpsize = tmp.size();
		    //debug ("--- read");
		    tmp.close();
		    tmp.setName(lDir.absPath() + "/.directory");
		    //debug ("---- copying to: %s", tmp.name());
		    if (tmp.open(IO_ReadWrite))
		    {
		      //debug ("--- opened RW");
		      if (tmp.writeBlock(buff, tmpsize) != -1)
		      {
			//debug ("--- wrote");
			tmp.close();
		      }
		      else
		      {
                        //debug ("--- removed");
			tmp.remove();
		      }
		    }                 // endif can open to write
		  }                   // endif can read
		  else     //coulnd't read
		    tmp.close();

		  delete[] buff;
		}                     // endif is alocated
	      }                       // can open to write
	    }
	    // End coping .directory file
	  }
	  path.remove (0, i);           // cded to;
	  if (path[0] == '/')
	    path.remove(0, 1); // remove / from path
	}
      }
      else // path didn't contain kde_mimdir this is an error
	err = true;
      
      // we created all subdirs or failed
      if (!err) // if we didn't fail try to make file just for check
      {
	path.prepend("/"); // be sure to have /jpeg.kdelnk
	path.prepend(lDir.absPath());
	f.setName(path);
	//debug("path = %s", path.data());
	if ( f.open( IO_ReadWrite ) )
	{
	  // we must first copy whole kdelnk to local dir
	  // and then change it. Trust me.
	  QFile s(properties->getKURL()->path());
	  s.open(IO_ReadOnly);
          char *buff = new char[s.size()+10];
	  if (buff != 0)
	  {
	    if (s.readBlock(buff, s.size()) != -1 &&
		f.writeBlock(buff, s.size()) != -1)
	      ; // ok
	    else
	    {
	      err = true;
	      f.remove();
	    }
	    delete[] buff;
	  }
	  else
	    err = true;
	}
	else
	  err = true;
      }
      if (err)
      {
#endif
	QMessageBox::warning( 0, i18n("KFM Error"),
			        i18n("Could not save properties\nPerhaps permissions denied") );
	return;
#ifdef SVEN
      }
#endif
    }
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    
    QString tmp = patternEdit->text();
    if ( tmp.length() > 1 )
	if ( tmp.right(1) != ";" )
	    tmp += ";";
    config.writeEntry( "Patterns", tmp.data() );
    config.writeEntry( "Comment", commentEdit->text(), true, false, true );
    config.writeEntry( "MimeType", mimeEdit->text() );
    config.writeEntry( "Icon", iconBox->icon() );
    config.writeEntry( "DefaultApp", kdelnklist.at( appBox->currentItem() ) );

    config.sync();

/*
    KMimeType::clearAll();
    KMimeType::init();
    if ( KRootWidget::getKRootWidget() )
	KRootWidget::getKRootWidget()->update();

    KfmGui *win;
    for ( win = KfmGui::getWindowList().first(); win != 0L; win = KfmGui::getWindowList().next() )
	win->updateView();
*/
}

/* ----------------------------------------------------
 *
 * DevicePropsPage
 *
 * -------------------------------------------------- */

DevicePropsPage::DevicePropsPage( PropertiesDialog *_props ) : PropsPage( _props )
{
    bool IamRoot = (geteuid() == 0);

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel( this, "Label_1" );
    tmpQLabel->move( 10, 10 );
    tmpQLabel->setText(  i18n("Device ( /dev/fd0 )") );
    tmpQLabel->adjustSize();
    
    device = new QLineEdit( this, "LineEdit_1" );
    device->setGeometry( 10, 40, 180, 30 );
    device->setText( "" );
    
    tmpQLabel = new QLabel( this, "Label_2" );
    tmpQLabel->move( 10, 80 );
    tmpQLabel->setText(  i18n("Mount Point ( /floppy )") );
    tmpQLabel->adjustSize();

    mountpoint = new QLineEdit( this, "LineEdit_2" );
    mountpoint->setGeometry( 10, 110, 180, 30 );
    mountpoint->setText( "" );
    if ( !IamRoot )
	mountpoint->setEnabled( false );
    
    readonly = new QCheckBox( this, "CheckBox_1" );
    readonly->setGeometry( 220, 40, 130, 30 );
    readonly->setText(  i18n("Readonly") );
    if ( !IamRoot )
	readonly->setEnabled( false );
    
    tmpQLabel = new QLabel( this, "Label_4" );
    tmpQLabel->move( 10, 150 );
    tmpQLabel->setText(  i18n("Filesystems ( iso9660,msdos,minix,default )") );
    tmpQLabel->adjustSize();

    fstype = new QLineEdit( this, "LineEdit_3" );
    fstype->setGeometry( 10, 180, 280, 30 );
    fstype->setText( "" );
    if ( !IamRoot )
	fstype->setEnabled( false );

    tmpQLabel = new QLabel( this, "Label_5" );
    tmpQLabel->move( 10, 220 );
    tmpQLabel->setText(  i18n("Mounted Icon") );
    tmpQLabel->adjustSize();

    tmpQLabel = new QLabel( this, "Label_6" );
    tmpQLabel->move( 170, 220 );
    tmpQLabel->setText(  i18n("Unmounted Icon") );
    tmpQLabel->adjustSize();
    
    mounted = new KIconLoaderButton( kapp->getIconLoader(), this );
    mounted->setGeometry( 10, 250, 50, 50 );
    
    unmounted = new KIconLoaderButton( kapp->getIconLoader(), this );
    unmounted->setGeometry( 170, 250, 50, 50 );

    QString path( _props->getKURL()->path() );
    
    QFile f( path );
    if ( !f.open( IO_ReadOnly ) )
	return;
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    deviceStr = config.readEntry( "Dev" );
    mountPointStr = config.readEntry( "MountPoint" );
    readonlyStr = config.readEntry( "ReadOnly" );
    fstypeStr = config.readEntry( "FSType" );
    mountedStr = config.readEntry( "Icon" );
    unmountedStr = config.readEntry( "UnmountIcon" );

    if ( !deviceStr.isNull() )
	device->setText( deviceStr.data() );
    if ( !mountPointStr.isNull() )
	mountpoint->setText( mountPointStr.data() );
    if ( !fstypeStr.isNull() )
	fstype->setText( fstypeStr.data() );
    if ( readonlyStr == "0" )
	readonly->setChecked( false );
    else
	readonly->setChecked( true );
    if ( mountedStr.isEmpty() )
        mountedStr = KMimeType::find("")->icon(0L, true); // default icon
    if ( unmountedStr.isEmpty() )
        unmountedStr = KMimeType::find("")->icon(0L, true); // default icon

    mounted->setIcon( mountedStr ); 
    unmounted->setIcon( unmountedStr ); 
}

bool DevicePropsPage::supports( KURL *_kurl, mode_t _mode )
{
/*
    KURL u( _kurl->url() );
    KURL u2( u.nestedURL() );
    
    if ( strcmp( u2.protocol(), "file" ) != 0 )
	return false;
*/
    if (!_kurl->isLocalFile())
        return false;

    QString path = _kurl->path();

    /*
    struct stat buff;
    stat( path, &buff );

    struct stat lbuff;
    lstat( path, &lbuff );

    if ( !S_ISREG( buff.st_mode ) || S_ISDIR( lbuff.st_mode ) )
    */
    if ( !S_ISREG( _mode ) || S_ISDIR( _mode ) )
	return false;

    FILE *f = fopen( path, "r" );
    if ( f == 0L )
      return false;

    char buffer[ 101 ];
    int n = fread( buffer, 1, 100, f );
    fclose( f );
    if ( n <= 0 )
      return false;
    if ( strncmp( buffer, "# KDE Config File", strlen( "# KDE Config File" ) ) != 0L )
      return false;

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );

    QString type = config.readEntry( "Type" );
    if ( type.isNull() )
	return false;
    if ( type != "FSDevice" )
	return false;
    
    return true;
}

void DevicePropsPage::applyChanges()
{
    QString path = properties->getKURL()->path();
    QFile f( path );
    if ( !f.open( IO_ReadWrite ) )
    {
	QMessageBox::warning( 0, i18n("KFM Error"), 
			        i18n("Could not save properties\nPerhaps permissions denied") );
	return;
    }
    f.close();

    KConfig config( path );
    config.setGroup( "KDE Desktop Entry" );
    
    config.writeEntry( "Dev", device->text() );
    config.writeEntry( "MountPoint", mountpoint->text() );
    config.writeEntry( "FSType", fstype->text() );
    
    config.writeEntry( "Icon", mounted->icon() );
    config.writeEntry( "UnmountIcon", unmounted->icon() );
    
    if ( readonly->isChecked() )
	config.writeEntry( "ReadOnly", "1" );
    else
	config.writeEntry( "ReadOnly", "0" );

    config.sync();
}

#include "kio_propsdlg.moc"
