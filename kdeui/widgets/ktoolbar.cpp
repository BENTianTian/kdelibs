/* This file is part of the KDE libraries
    Copyright
    (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999 Chris Schlaeger (cs@kde.org)
    (C) 1999 Kurt Granroth (granroth@kde.org)
    (C) 2005-2006 Hamish Rodda (rodda@kde.org)

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

#include <config.h>

#include <QtGui/QFrame>
#include <QtGui/QLayout>
#include <QtGui/QMouseEvent>
#include <QtXml/QDomElement>
#include <QtCore/QPointer>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kxmlguifactory.h>

#include "ktoolbar.h"

class KToolBar::Private
{
  public:
    Private( KToolBar *_parent )
      : parent( _parent ),
        honorStyle( false ),
        enableContext( true ),
        modified( false ),
        unlockedMovable( true ),
        xmlguiClient( 0 ),
        contextLockAction( 0 ),
        HiddenDefault( false ),
        IconSizeDefault( 22 ),
        ToolButtonStyleDefault( Qt::ToolButtonTextUnderIcon ),
        NewLineDefault( false ),
        OffsetDefault( 0 ),
        PositionDefault( "Top" ),
        dropIndicatorAction( 0 ),
        context( 0 ),
        dragAction( 0 )
    {
    }

    void slotReadConfig();
    void slotAppearanceChanged();
    void slotContextAboutToShow();
    void slotContextAboutToHide();
    void slotContextLeft();
    void slotContextRight();
    void slotContextTop();
    void slotContextBottom();
    void slotContextIcons();
    void slotContextText();
    void slotContextTextRight();
    void slotContextTextUnder();
    void slotContextIconSize();
    void slotLockToolBars(bool lock);

    void init( bool readConfig = true, bool honorStyle = false );
    void getAttributes( QString &position, Qt::ToolButtonStyle &toolButtonStyle, int &index ) const;
    int dockWindowIndex() const;
    KMenu *contextMenu();
    bool isMainToolBar() const;
    void setLocked( bool locked );
    void adjustSeparatorVisibility();

    static Qt::ToolButtonStyle toolButtonStyleFromString( const QString& style );
    static QString toolButtonStyleToString( Qt::ToolButtonStyle );


    KToolBar *parent;
    bool honorStyle : 1;
    bool enableContext : 1;
    bool modified : 1;
    bool unlockedMovable : 1;
    static bool s_editable;
    static bool s_locked;

    KXMLGUIClient *xmlguiClient;

    struct ToolBarInfo
    {
      ToolBarInfo()
        : index( -1 ), offset( -1 ), newline( false ), area( Qt::TopToolBarArea )
      {
      }

      ToolBarInfo( Qt::ToolBarArea a, int i, bool n, int o )
        : index( i ), offset( o ), newline( n ), area( a )
      {
      }

      int index, offset;
      bool newline;
      Qt::ToolBarArea area;
    };

    ToolBarInfo toolBarInfo;
    QList<int> iconSizes;

    QMenu* contextOrient;
    QMenu* contextMode;
    QMenu* contextSize;

    QAction* contextTop;
    QAction* contextLeft;
    QAction* contextRight;
    QAction* contextBottom;
    QAction* contextIcons;
    QAction* contextTextRight;
    QAction* contextText;
    QAction* contextTextUnder;
    KToggleAction* contextLockAction;
    QMap<QAction*,int> contextIconSizes;

    // Default Values.
    bool HiddenDefault;
    int IconSizeDefault;
    Qt::ToolButtonStyle ToolButtonStyleDefault;
    bool NewLineDefault;
    int OffsetDefault;
    QString PositionDefault;

    QList<QAction*> actionsBeingDragged;
    QAction* dropIndicatorAction;

    KMenu* context;
    KAction* dragAction;
    QPoint dragStartPosition;
};

bool KToolBar::Private::s_editable = false;
bool KToolBar::Private::s_locked = false;

void KToolBar::Private::init( bool readConfig, bool _honorStyle )
{
  honorStyle = _honorStyle;

  // finally, read in our configurable settings
  if ( readConfig )
    slotReadConfig();

  if ( parent->mainWindow() ) {
    // Get notified when settings change
    connect( parent, SIGNAL( allowedAreasChanged( Qt::ToolBarAreas ) ),
             parent->mainWindow(), SLOT( setSettingsDirty() ) );
    connect( parent, SIGNAL( iconSizeChanged( const QSize& ) ),
             parent->mainWindow(), SLOT( setSettingsDirty() ) );
    connect( parent, SIGNAL( toolButtonStyleChanged( Qt::ToolButtonStyle ) ),
             parent->mainWindow(), SLOT( setSettingsDirty() ) );
    connect( parent, SIGNAL( movableChanged( bool ) ),
             parent->mainWindow(), SLOT( setSettingsDirty() ) );
    connect( parent, SIGNAL( orientationChanged( Qt::Orientation ) ),
             parent->mainWindow(), SLOT( setSettingsDirty() ) );
  }

  if ( !KAuthorized::authorize( "movable_toolbars" ) )
    parent->setMovable( false );

  connect( parent, SIGNAL( movableChanged( bool ) ),
           parent, SLOT( slotMovableChanged( bool ) ) );

  parent->setAcceptDrops( true );

  parent->setFont( KGlobalSettings::toolBarFont() );
}

void KToolBar::Private::getAttributes( QString &position, Qt::ToolButtonStyle &toolButtonStyle, int &index ) const
{
  // get all of the stuff to save
  switch ( parent->mainWindow()->toolBarArea( const_cast<KToolBar*>( parent ) ) ) {
    case Qt::BottomToolBarArea:
      position = "Bottom";
      break;
    case Qt::LeftToolBarArea:
      position = "Left";
      break;
    case Qt::RightToolBarArea:
      position = "Right";
      break;
    case Qt::TopToolBarArea:
    default:
      position = "Top";
      break;
  }

  toolButtonStyle = parent->KToolBar::toolButtonStyle();

  index = dockWindowIndex();
}

int KToolBar::Private::dockWindowIndex() const
{
  Q_ASSERT( parent->mainWindow() );

  return parent->mainWindow()->layout()->indexOf( const_cast<KToolBar*>( parent ) );
}

KMenu *KToolBar::Private::contextMenu()
{
  if ( !context) {
    context = new KMenu( parent );
    context->addTitle( i18n( "Toolbar Menu" ) );

    contextOrient = new KMenu( i18n( "Orientation" ), context );
    context->addMenu( contextOrient );

    contextTop = contextOrient->addAction( i18nc( "toolbar position string", "Top" ), parent, SLOT( slotContextTop() ) );
    contextTop->setChecked( true );
    contextLeft = contextOrient->addAction( i18nc( "toolbar position string", "Left" ), parent, SLOT( slotContextLeft() ) );
    contextRight = contextOrient->addAction( i18nc( "toolbar position string", "Right" ), parent, SLOT( slotContextRight() ) );
    contextBottom = contextOrient->addAction( i18nc( "toolbar position string", "Bottom" ), parent, SLOT( slotContextBottom() ) );

    QActionGroup* positionGroup = new QActionGroup( contextOrient );
    foreach ( QAction* action, contextOrient->actions() ) {
      action->setActionGroup( positionGroup );
      action->setCheckable( true );
    }

    contextMode = new KMenu( i18n( "Text Position" ), context );
    context->addMenu( contextMode );

    contextIcons = contextMode->addAction( i18n( "Icons Only" ), parent, SLOT( slotContextIcons() ) );
    contextIcons->setChecked( true );
    contextText = contextMode->addAction( i18n( "Text Only" ), parent, SLOT( slotContextText() ) );
    contextTextRight = contextMode->addAction( i18n( "Text Alongside Icons" ), parent, SLOT( slotContextTextRight() ) );
    contextTextUnder = contextMode->addAction( i18n( "Text Under Icons" ), parent, SLOT( slotContextTextUnder() ) );

    QActionGroup* textGroup = new QActionGroup( contextMode );
    foreach ( QAction* action, contextMode->actions() ) {
      action->setActionGroup( textGroup );
      action->setCheckable( true );
    }

    contextSize = new KMenu( i18n( "Icon Size" ), context );
    context->addMenu( contextSize );

    contextIconSizes.insert( contextSize->addAction( i18n( "Default" ), parent, SLOT( slotContextIconSize() ) ), 0 );

    // Query the current theme for available sizes
    KIconTheme *theme = kapp->iconLoader()->theme();
    QList<int> avSizes;
    if ( theme ) {
      if ( isMainToolBar() )
        avSizes = theme->querySizes( K3Icon::MainToolbar );
      else
        avSizes = theme->querySizes( K3Icon::Toolbar );
    }

    iconSizes = avSizes;
    qSort( avSizes );

    if ( avSizes.count() < 10 ) {
      // Fixed or threshold type icons
      foreach ( int it, avSizes ) {
        QString text;
        if ( it < 19 )
          text = i18n( "Small (%1x%2)", it, it );
        else if (it < 25)
          text = i18n( "Medium (%1x%2)", it, it );
        else if (it < 35)
          text = i18n( "Large (%1x%2)", it, it );
        else
          text = i18n( "Huge (%1x%2)", it, it );

        // save the size in the contextIconSizes map
        contextIconSizes.insert( contextSize->addAction( text, parent, SLOT( slotContextIconSize() ) ), it );
      }
    } else {
      // Scalable icons.
      const int progression[] = { 16, 22, 32, 48, 64, 96, 128, 192, 256 };

      for ( uint i = 0; i < 9; i++ ) {
        foreach ( int it, avSizes ) {
          if ( it >= progression[ i ] ) {
            QString text;
            if ( it < 19 )
              text = i18n( "Small (%1x%2)", it, it );
            else if (it < 25)
              text = i18n( "Medium (%1x%2)", it, it );
            else if (it < 35)
              text = i18n( "Large (%1x%2)", it, it );
            else
              text = i18n( "Huge (%1x%2)", it, it );

            // save the size in the contextIconSizes map
            contextIconSizes.insert( contextSize->addAction( text, parent, SLOT( slotContextIconSize() ) ), it );
            break;
          }
        }
      }
    }

    QActionGroup* sizeGroup = new QActionGroup( contextSize );
    foreach ( QAction* action, contextSize->actions() ) {
      action->setActionGroup( sizeGroup );
      action->setCheckable( true );
    }

    if ( !parent->toolBarsLocked() && !parent->isMovable() )
      unlockedMovable = false;

    delete contextLockAction;
    contextLockAction = new KToggleAction( KIcon( "lock" ), i18n( "Lock Toolbars" ), 0L, 0L );
    context->addAction( contextLockAction );
    contextLockAction->setChecked( parent->toolBarsLocked() );
    contextLockAction->setCheckedState( KGuiItem( i18n( "Unlock Toolbars" ), KIcon( "unlock" ) ) );
    connect( contextLockAction, SIGNAL( toggled( bool ) ), parent, SLOT( slotLockToolBars( bool ) ) );

    connect( context, SIGNAL( aboutToShow() ), parent, SLOT( slotContextAboutToShow() ) );
  }

  contextOrient->menuAction()->setVisible( !parent->toolBarsLocked() );
  contextMode->menuAction()->setVisible( !parent->toolBarsLocked() );
  contextSize->menuAction()->setVisible( !parent->toolBarsLocked() );

  // Unplugging a submenu from abouttohide leads to the popupmenu floating around
  // So better simply call that code from after exec() returns (DF)
  //connect( context, SIGNAL( aboutToHide() ), this, SLOT( slotContextAboutToHide() ) );

  return context;
}

bool KToolBar::Private::isMainToolBar( ) const
{
  return parent->objectName() == QLatin1String( "mainToolBar" );
}

void KToolBar::Private::setLocked( bool locked )
{
  if ( unlockedMovable )
    parent->setMovable( !locked );
}

void KToolBar::Private::adjustSeparatorVisibility()
{
  bool visibleNonSeparator = false;
  int separatorToShow = -1;

  for ( int index = 0; index < parent->actions().count(); ++index ) {
    QAction* action = parent->actions()[ index ];
    if ( action->isSeparator() ) {
      if ( visibleNonSeparator ) {
        separatorToShow = index;
        visibleNonSeparator = false;
      } else {
        action->setVisible( false );
      }
    } else if ( !visibleNonSeparator ) {
      if ( action->isVisible() ) {
        visibleNonSeparator = true;
        if ( separatorToShow != -1 ) {
          parent->actions()[ separatorToShow ]->setVisible( true );
          separatorToShow = -1;
        }
      }
    }
  }

  if ( separatorToShow != -1 )
    parent->actions()[ separatorToShow ]->setVisible( false );
}

Qt::ToolButtonStyle KToolBar::Private::toolButtonStyleFromString( const QString & _style )
{
  QString style = _style.toLower();
  if ( style == "textbesideicon" || style == "icontextright" )
    return Qt::ToolButtonTextBesideIcon;
  else if ( style == "textundericon" || style == "icontextbottom" )
    return Qt::ToolButtonTextUnderIcon;
  else if ( style == "textonly" )
    return Qt::ToolButtonTextOnly;
  else
    return Qt::ToolButtonIconOnly;
}

QString KToolBar::Private::toolButtonStyleToString( Qt::ToolButtonStyle style )
{
  switch( style )
  {
    case Qt::ToolButtonIconOnly:
    default:
      return "IconOnly";
    case Qt::ToolButtonTextBesideIcon:
      return "TextBesideIcon";
    case Qt::ToolButtonTextOnly:
      return "TextOnly";
    case Qt::ToolButtonTextUnderIcon:
      return "TextUnderIcon";
  }
}

void KToolBar::Private::slotReadConfig()
{
  /**
   * Read appearance settings (hmm, we used to do both here,
   * but a well behaved application will call applyMainWindowSettings
   * anyway, right ?)
   */
  parent->applyAppearanceSettings( KGlobal::config(), QString() );
}

void KToolBar::Private::slotAppearanceChanged()
{
  // Read appearance settings from global file.
  parent->applyAppearanceSettings( KGlobal::config(), QString(), true /* lose local settings */ );

  // And remember to save the new look later
  KMainWindow *kmw = qobject_cast<KMainWindow *>( parent->mainWindow() );
  if ( kmw )
    kmw->setSettingsDirty();
}

void KToolBar::Private::slotContextAboutToShow()
{
  /**
   * The idea here is to reuse the "static" part of the menu to save time.
   * But the "Toolbars" action is dynamic (can be a single action or a submenu)
   * and ToolBarHandler::setupActions() deletes it, so better not keep it around.
   * So we currently plug/unplug the last two actions of the menu.
   * Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
   */
  KMainWindow *kmw = qobject_cast<KMainWindow *>( parent->mainWindow() );
  if ( kmw ) {
    kmw->setupToolbarMenuActions();
    // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
    QAction *tbAction = kmw->toolBarMenuAction();
    if ( !parent->toolBarsLocked() && tbAction && tbAction->associatedWidgets().count() > 0 )
      contextMenu()->addAction( tbAction );
  }

  // try to find "configure toolbars" action
  QAction *configureAction = 0;
  const char* actionName = KStandardAction::name( KStandardAction::ConfigureToolbars );
  if ( xmlguiClient )
    configureAction = xmlguiClient->actionCollection()->action( actionName );

  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action( actionName );

  if ( configureAction )
    context->addAction( configureAction );

  KEditToolbar::setDefaultToolbar( parent->QObject::objectName().toLatin1().constData() );

  // Check the actions that should be checked
  switch ( parent->toolButtonStyle() ) {
    case Qt::ToolButtonIconOnly:
    default:
      contextIcons->setChecked( true );
      break;
    case Qt::ToolButtonTextBesideIcon:
      contextTextRight->setChecked( true );
      break;
    case Qt::ToolButtonTextOnly:
      contextText->setChecked( true );
      break;
    case Qt::ToolButtonTextUnderIcon:
      contextTextUnder->setChecked( true );
      break;
  }

  QMapIterator< QAction*, int > it = contextIconSizes;
  while ( it.hasNext() ) {
    it.next();
    if ( it.value() == parent->iconSize().width() ) {
      it.key()->setChecked( true );
      break;
    }
  }

  switch ( parent->mainWindow()->toolBarArea( parent ) ) {
    case Qt::BottomToolBarArea:
      contextBottom->setChecked( true );
      break;
    case Qt::LeftToolBarArea:
      contextLeft->setChecked( true );
      break;
    case Qt::RightToolBarArea:
      contextRight->setChecked( true );
      break;
    default:
    case Qt::TopToolBarArea:
      contextTop->setChecked( true );
      break;
  }
}

void KToolBar::Private::slotContextAboutToHide()
{
  // We have to unplug whatever slotContextAboutToShow plugged into the menu.
  // Unplug the toolbar menu action
  KMainWindow *kmw = qobject_cast<KMainWindow *>( parent->mainWindow() );
  if ( kmw && kmw->toolBarMenuAction() )
    if ( kmw->toolBarMenuAction()->associatedWidgets().count() > 1 )
      contextMenu()->removeAction( kmw->toolBarMenuAction() );

  // Unplug the configure toolbars action too, since it's afterwards anyway
  QAction *configureAction = 0;
  const char* actionName = KStandardAction::name( KStandardAction::ConfigureToolbars );
  if ( xmlguiClient )
    configureAction = xmlguiClient->actionCollection()->action( actionName );

  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action( actionName );

  if ( configureAction )
    context->removeAction( configureAction );
}

void KToolBar::Private::slotContextLeft()
{
  parent->mainWindow()->addToolBar( Qt::LeftToolBarArea, parent );
}

void KToolBar::Private::slotContextRight()
{
  parent->mainWindow()->addToolBar( Qt::RightToolBarArea, parent );
}

void KToolBar::Private::slotContextTop()
{
  parent->mainWindow()->addToolBar( Qt::TopToolBarArea, parent );
}

void KToolBar::Private::slotContextBottom()
{
  parent->mainWindow()->addToolBar( Qt::BottomToolBarArea, parent );
}

void KToolBar::Private::slotContextIcons()
{
  parent->setToolButtonStyle( Qt::ToolButtonIconOnly );
}

void KToolBar::Private::slotContextText()
{
  parent->setToolButtonStyle( Qt::ToolButtonTextOnly );
}

void KToolBar::Private::slotContextTextUnder()
{
  parent->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
}

void KToolBar::Private::slotContextTextRight()
{
  parent->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
}

void KToolBar::Private::slotContextIconSize()
{
  QAction* action = qobject_cast<QAction*>( parent->sender() );
  if ( action && contextIconSizes.contains( action ) ) {
    parent->setIconDimensions( contextIconSizes.value( action ) );
  }
}

void KToolBar::Private::slotLockToolBars( bool lock )
{
  parent->setToolBarsLocked( lock );
}



KToolBar::KToolBar( QWidget *parent, bool honorStyle, bool readConfig )
  : QToolBar( parent ),
    d( new Private( this ) )
{
  d->init( readConfig, honorStyle );

  // KToolBar is auto-added to the top area of the main window if parent is a QMainWindow
  if ( QMainWindow* mw = qobject_cast<QMainWindow*>( parent ) )
    mw->addToolBar( this );
}

KToolBar::KToolBar( const QString& objectName, QMainWindow* parent, Qt::ToolBarArea area,
                    bool newLine, bool honorStyle, bool readConfig )
  : QToolBar( parent ),
    d( new Private( this ) )
{
  setObjectName( objectName );
  d->init( readConfig, honorStyle );

  if ( newLine )
    mainWindow()->addToolBarBreak( area );

  mainWindow()->addToolBar( area, this );

  if ( newLine )
    mainWindow()->addToolBarBreak( area );
}

KToolBar::~KToolBar()
{
  delete d->contextLockAction;
  delete d;
}

void KToolBar::setContextMenuEnabled( bool enable )
{
  d->enableContext = enable;
}

bool KToolBar::contextMenuEnabled() const
{
  return d->enableContext;
}

QString KToolBar::settingsGroup() const
{
  QString configGroup;
  if ( objectName().isEmpty() || d->isMainToolBar() )
    configGroup = "Toolbar style";
  else
    configGroup = QString( objectName() ) + " Toolbar style";

  if ( mainWindow() ) {
    configGroup.prepend(" ");
    configGroup.prepend( mainWindow()->objectName() );
  }

  return configGroup;
}

void KToolBar::saveSettings( KConfig *config, const QString &_configGroup )
{
  QString configGroup = _configGroup;

  if ( configGroup.isEmpty() )
    configGroup = settingsGroup();

  QString position;
  Qt::ToolButtonStyle ToolButtonStyle;
  int index;
  d->getAttributes( position, ToolButtonStyle, index );

  KConfigGroup cg( config, configGroup );

  if ( !cg.hasDefault( "Position" ) && position == d->PositionDefault )
    cg.revertToDefault( "Position" );
  else
    cg.writeEntry( "Position", position );

  if ( d->honorStyle && ToolButtonStyle == d->ToolButtonStyleDefault && !cg.hasDefault( "ToolButtonStyle" ) )
    cg.revertToDefault( "ToolButtonStyle" );
  else
    cg.writeEntry( "ToolButtonStyle", d->toolButtonStyleToString( ToolButtonStyle ) );

  if ( !cg.hasDefault( "IconSize" ) && iconSize().width() == iconSizeDefault() )
    cg.revertToDefault( "IconSize" );
  else
    cg.writeEntry( "IconSize", iconSize().width() );

  if ( !cg.hasDefault( "Hidden" ) && isHidden() == d->HiddenDefault )
    cg.revertToDefault( "Hidden" );
  else
    cg.writeEntry( "Hidden", isHidden() );

    // Note that index, unlike the other settings, depends on the other toolbars
    // So on the first run with a clean local config file, even the usual
    // hasDefault/==IndexDefault test would save the toolbar indexes
    // (IndexDefault was 0, whereas index is the real index in the GUI)
    //
    // Saving the whole set of indexes is necessary though. When moving only
    // one toolbar, if we only saved the changed indexes, the toolbars wouldn't
    // reappear at the same position the next time.
    // The whole set of indexes has to be saved.
    //kDebug(220) << name() << "                writing index " << index << endl;

    // don't save if there's only one toolbar

    // Don't use kmw->toolBarIterator() because you might
    // mess up someone else's iterator.  Make the list on your own

    /* FIXME KMainWindow port - no replacement
    QList<KToolBar*> toolbarList = mainWindow()->findChildren<KToolBar*>();

    if ( !kmw || toolbarList.count() > 1 )
        cg.writeEntry("Index", index);
    else
        cg.revertToDefault("Index");*/

    /* FIXME KMainWindow port - no replacement
    if(!cg.hasDefault("Offset") && offset() == d->OffsetDefault )
      cg.revertToDefault("Offset");
    else
      cg.writeEntry("Offset", offset());*/

    /* FIXME KToolBar port - need to implement
    if(!cg.hasDefault("NewLine") && newLine() == d->NewLineDefault )
      cg.revertToDefault("NewLine");
    else
      cg.writeEntry("NewLine", newLine());*/
}

void KToolBar::setXMLGUIClient( KXMLGUIClient *client )
{
  d->xmlguiClient = client;
}

void KToolBar::contextMenuEvent( QContextMenuEvent* event )
{
  if ( mainWindow() && d->enableContext ) {
    QPointer<KToolBar> guard( this );
    d->contextMenu()->exec( event->globalPos() );

    // "Configure Toolbars" recreates toolbars, so we might not exist anymore.
    if ( guard )
      d->slotContextAboutToHide();
    return;
  }

  QToolBar::contextMenuEvent( event );
}

Qt::ToolButtonStyle KToolBar::toolButtonStyleSetting()
{
  KConfigGroup saver( KGlobal::config(), "Toolbar style" );

  return KToolBar::Private::toolButtonStyleFromString( saver.readEntry( "ToolButtonStyle", "TextUnderIcon") );
}

void KToolBar::loadState( const QDomElement &element )
{
  QMainWindow *mw = mainWindow();

  if ( !mw )
    return;

  {
    QByteArray text = element.namedItem( "text" ).toElement().text().toUtf8();
    if ( text.isEmpty() )
      text = element.namedItem( "Text" ).toElement().text().toUtf8();

    if ( !text.isEmpty() )
      setWindowTitle( i18n( text ) );
  }

  /*
    This method is called in order to load toolbar settings from XML.
    However this can be used in two rather different cases:
    - for the initial loading of the app's XML. In that case the settings
      are only the defaults, the user's KConfig settings will override them
      (KDE4 TODO: how about saving those user settings into the local XML file instead?
      Then this whole thing would be simpler, no KConfig settings to apply afterwards.
      OTOH we'd have to migrate those settings when the .rc version increases,
      like we do for shortcuts)

    - for later re-loading when switching between parts in KXMLGUIFactory.
      In that case the XML contains the final settings, not the defaults.
      We do need the defaults, and the toolbar might have been completely
      deleted and recreated meanwhile. So we store the app-default settings
      into the XML.
   */
  bool loadingAppDefaults = true;
  if ( element.hasAttribute( "offsetDefault" ) ) {
    // this isn't the first time, so the defaults have been saved into the (in-memory) XML
    loadingAppDefaults = false;
    d->OffsetDefault = element.attribute( "offsetDefault" ).toInt();
    d->NewLineDefault = element.attribute( "newlineDefault" ) == "true";
    d->HiddenDefault = element.attribute( "hiddenDefault" ) == "true";
    d->IconSizeDefault = element.attribute( "iconSizeDefault" ).toInt();
    d->PositionDefault = element.attribute( "positionDefault" );
    d->ToolButtonStyleDefault = d->toolButtonStyleFromString( element.attribute( "toolButtonStyleDefault" ) );
  }

  {
    QString attrIconText = element.attribute( "iconText" ).toLower().toLatin1();
    if ( !attrIconText.isEmpty() ) {
      setToolButtonStyle( d->toolButtonStyleFromString( attrIconText ) );
    } else {
      if ( d->honorStyle )
        setToolButtonStyle( d->ToolButtonStyleDefault );
    }
  }

  QString attrIconSize = element.attribute( "iconSize" ).toLower().trimmed();
  int iconSize = d->IconSizeDefault;

  {
    bool ok;
    int newIconSize = attrIconSize.toInt( &ok );
    if ( ok )
      iconSize = newIconSize;
  }

  setIconDimensions( iconSize );

  int index = -1; // append by default. This is very important, otherwise

  // with all 0 indexes, we keep reversing the toolbars.
  {
    QString attrIndex = element.attribute( "index" ).toLower();
    if ( !attrIndex.isEmpty() )
      index = attrIndex.toInt();
  }

  int offset = d->OffsetDefault;
  bool newLine = d->NewLineDefault;
  bool hidden = d->HiddenDefault;

  {
    QString attrOffset = element.attribute( "offset" );
    if ( !attrOffset.isEmpty() )
      offset = attrOffset.toInt();
  }

  {
    QString attrNewLine = element.attribute( "newline" ).toLower();
    if ( !attrNewLine.isEmpty() )
      newLine = attrNewLine == "true";

    if ( newLine && mainWindow() )
      mainWindow()->insertToolBarBreak( this );
  }

  {
    QString attrHidden = element.attribute( "hidden" ).toLower();
    if ( !attrHidden.isEmpty() )
      hidden = attrHidden  == "true";
  }

  if ( hidden )
    hide();
  else
    show();

  if ( loadingAppDefaults ) {
    d->getAttributes( d->PositionDefault, d->ToolButtonStyleDefault, index );

    d->OffsetDefault = offset;
    d->NewLineDefault = newLine;
    d->HiddenDefault = hidden;
    d->IconSizeDefault = iconSize;
  }
}

void KToolBar::saveState( QDomElement &current ) const
{
  Q_ASSERT( !current.isNull() );

  QString position;
  Qt::ToolButtonStyle ToolButtonStyle;
  int index = -1;
  d->getAttributes( position, ToolButtonStyle, index );

  current.setAttribute( "noMerge", "1" );
  current.setAttribute( "position", position );
  current.setAttribute( "toolButtonStyle", d->toolButtonStyleToString( ToolButtonStyle ) );
  current.setAttribute( "index", index );
  // FIXME KAction port
  //current.setAttribute( "offset", offset() );
  //current.setAttribute( "newline", newLine() );
  if ( isHidden() )
    current.setAttribute( "hidden", "true" );
  d->modified = true;

  // TODO if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
  // then the stuff below shouldn't always be done.
  current.setAttribute( "offsetDefault", d->OffsetDefault );
  current.setAttribute( "newlineDefault", d->NewLineDefault );
  current.setAttribute( "hiddenDefault", d->HiddenDefault ? "true" : "false" );
  current.setAttribute( "iconSizeDefault", d->IconSizeDefault );
  current.setAttribute( "positionDefault", d->PositionDefault );
  current.setAttribute( "toolButtonStyleDefault", d->toolButtonStyleToString( d->ToolButtonStyleDefault ) );
}

void KToolBar::applySettings( KConfig *config, const QString &_configGroup, bool force )
{
  QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;

  /*
    Let's explain this a bit more in details.
    The order in which we apply settings is :
     Global config / <appnamerc> user settings                        if no XMLGUI is used
     Global config / App-XML attributes / <appnamerc> user settings   if XMLGUI is used

    So in the first case, we simply read everything from KConfig as below,
    but in the second case we don't do anything here if there is no app-specific config,
    and the XMLGUI-related code (loadState()) uses the static methods of this class
    to get the global defaults.

    Global config doesn't include position (index, offset, newline and hidden/shown).
  */

  // First the appearance stuff - the one which has a global config
  applyAppearanceSettings( config, configGroup );

  // ...and now the position stuff
  if ( config->hasGroup( configGroup ) || force ) {
    KConfigGroup cg(config, configGroup);

    QString position = cg.readEntry( "Position", d->PositionDefault );
    int index = cg.readEntry( "Index", int(-1) );
    int offset = cg.readEntry( "Offset", int(d->OffsetDefault) );
    bool newLine = cg.readEntry( "NewLine", d->NewLineDefault );
    bool hidden = cg.readEntry( "Hidden", d->HiddenDefault );

    Qt::ToolBarArea pos = Qt::TopToolBarArea;
    if ( position == "Top" )
      pos = Qt::TopToolBarArea;
    else if ( position == "Bottom" )
      pos = Qt::BottomToolBarArea;
    else if ( position == "Left" )
      pos = Qt::LeftToolBarArea;
    else if ( position == "Right" )
      pos = Qt::RightToolBarArea;

    if (hidden)
      hide();
    else
      show();

    if ( mainWindow() )
      d->toolBarInfo = KToolBar::Private::ToolBarInfo( pos, index, newLine, offset );
  }
}

void KToolBar::applyAppearanceSettings( KConfig *config, const QString &_configGroup, bool forceGlobal )
{
  QString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;

  // If we have application-specific settings in the XML file,
  // and nothing in the application's config file, then
  // we don't apply the global defaults, the XML ones are preferred
  // (see applySettings for a full explanation)
  // This is the reason for the xmlgui tests below.
  bool xmlgui = d->xmlguiClient && !d->xmlguiClient->xmlFile().isEmpty();

  KConfig *gconfig = KGlobal::config();

  // we actually do this in two steps.
  // First, we read in the global styles [Toolbar style] (from the KControl module).
  // Then, if the toolbar is NOT 'mainToolBar', we will also try to read in [barname Toolbar style]
  bool applyToolButtonStyle = !xmlgui; // if xmlgui is used, global defaults won't apply
  bool applyIconSize = !xmlgui;

  int iconSize = d->IconSizeDefault;
  Qt::ToolButtonStyle ToolButtonStyle = d->ToolButtonStyleDefault;

  // this is the first iteration

  { // start block for KConfigGroup
      KConfigGroup cg(gconfig, "Toolbar style");

    // we read in the ToolButtonStyle property *only* if we intend on actually
    // honoring it
    if ( d->honorStyle )
        d->ToolButtonStyleDefault = d->toolButtonStyleFromString( cg.readEntry( "ToolButtonStyle",
                                        d->toolButtonStyleToString( d->ToolButtonStyleDefault ) ) );
    else
      d->ToolButtonStyleDefault = Qt::ToolButtonTextUnderIcon;

    // Use the default icon size for toolbar icons.
    d->IconSizeDefault = cg.readEntry( "IconSize", int(d->IconSizeDefault) );

    iconSize = d->IconSizeDefault;
    ToolButtonStyle = d->ToolButtonStyleDefault;

    if ( !forceGlobal && config->hasGroup( configGroup ) ) {
      config->setGroup( configGroup );

      // read in the ToolButtonStyle property
      if ( config->hasKey( "ToolButtonStyle" ) ) {
          ToolButtonStyle = d->toolButtonStyleFromString( config->readEntry( "ToolButtonStyle", QString() ) );
        applyToolButtonStyle = true;
      }

      // now get the size
      if ( config->hasKey( "IconSize" ) ) {
          iconSize = config->readEntry( "IconSize", 0 );
          applyIconSize = true;
      }
    }
  } // end block for KConfigGroup

  // check if the icon/text has changed
  if ( ToolButtonStyle != toolButtonStyle() && applyToolButtonStyle )
    setToolButtonStyle( ToolButtonStyle );

  // ...and check if the icon size has changed
  if ( iconSize != KToolBar::iconSize().width() && applyIconSize )
    setIconDimensions( iconSize );
}

KMainWindow * KToolBar::mainWindow() const
{
  return qobject_cast<KMainWindow*>( const_cast<QObject*>( parent() ) );
}

void KToolBar::setIconDimensions( int size )
{
  QToolBar::setIconSize( QSize( size, size ) );
}

int KToolBar::iconSizeDefault() const
{
  if ( QObject::objectName() == "mainToolBar" )
    return kapp->iconLoader()->currentSize( K3Icon::MainToolbar );

  return kapp->iconLoader()->currentSize( K3Icon::Toolbar );
}

void KToolBar::slotMovableChanged( bool movable )
{
  if ( movable && !KAuthorized::authorize( "movable_toolbars" ) )
    setMovable( false );
}

void KToolBar::dragEnterEvent( QDragEnterEvent *event )
{
  if ( toolBarsEditable() && event->proposedAction() & (Qt::CopyAction | Qt::MoveAction) &&
       event->mimeData()->hasFormat( "application/x-kde-action-list" ) ) {
    QByteArray data = event->mimeData()->data( "application/x-kde-action-list" );

    QDataStream stream( data );

    QStringList actionNames;

    stream >> actionNames;

    foreach ( const QString& actionName, actionNames ) {
      foreach ( KActionCollection* ac, KActionCollection::allCollections() ) {
        QAction* newAction = ac->action( actionName.toAscii().constData() );
        if ( newAction ) {
          d->actionsBeingDragged.append( newAction );
          break;
        }
      }
    }

    if ( d->actionsBeingDragged.count() ) {
      QAction* overAction = actionAt( event->pos() );

      QFrame* dropIndicatorWidget = new QFrame( this );
      dropIndicatorWidget->resize( 8, height() - 4 );
      dropIndicatorWidget->setFrameShape( QFrame::VLine );
      dropIndicatorWidget->setLineWidth( 3 );

      d->dropIndicatorAction = insertWidget( overAction, dropIndicatorWidget );

      insertAction( overAction, d->dropIndicatorAction );

      event->acceptProposedAction();
      return;
    }
  }

  QToolBar::dragEnterEvent( event );
}

void KToolBar::dragMoveEvent( QDragMoveEvent *event )
{
  if ( toolBarsEditable() )
    forever {
      if ( d->dropIndicatorAction ) {
        QAction* overAction = 0L;
        foreach ( QAction* action, actions() ) {
          // want to make it feel that half way across an action you're dropping on the other side of it
          QWidget* widget = widgetForAction( action );
          if ( event->pos().x() < widget->pos().x() + (widget->width() / 2) ) {
            overAction = action;
            break;
          }
        }

        if ( overAction != d->dropIndicatorAction ) {
          // Check to see if the indicator is already in the right spot
          int dropIndicatorIndex = actions().indexOf( d->dropIndicatorAction );
          if ( dropIndicatorIndex + 1 < actions().count() ) {
            if ( actions()[ dropIndicatorIndex + 1 ] == overAction )
              break;
          } else if ( !overAction ) {
            break;
          }

          insertAction( overAction, d->dropIndicatorAction );
        }

        event->accept();
        return;
      }
      break;
    }

  QToolBar::dragMoveEvent( event );
}

void KToolBar::dragLeaveEvent( QDragLeaveEvent *event )
{
  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  delete d->dropIndicatorAction;
  d->dropIndicatorAction = 0L;
  d->actionsBeingDragged.clear();

  if ( toolBarsEditable() ) {
    event->accept();
    return;
  }

  QToolBar::dragLeaveEvent( event );
}

void KToolBar::dropEvent( QDropEvent *event )
{
  if ( toolBarsEditable() ) {
    foreach ( QAction* action, d->actionsBeingDragged ) {
      if ( actions().contains( action ) )
        removeAction( action );
      insertAction( d->dropIndicatorAction, action );
    }
  }

  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  delete d->dropIndicatorAction;
  d->dropIndicatorAction = 0L;
  d->actionsBeingDragged.clear();

  if ( toolBarsEditable() ) {
    event->accept();
    return;
  }

  QToolBar::dropEvent( event );
}

void KToolBar::mousePressEvent( QMouseEvent *event )
{
  if ( toolBarsEditable() && event->button() == Qt::LeftButton ) {
    if ( KAction* action = qobject_cast<KAction*>( actionAt( event->pos() ) ) ) {
      d->dragAction = action;
      d->dragStartPosition = event->pos();
      event->accept();
      return;
    }
  }

  QToolBar::mousePressEvent( event );
}

void KToolBar::mouseMoveEvent( QMouseEvent *event )
{
  if ( !toolBarsEditable() || !d->dragAction )
    return QToolBar::mouseMoveEvent( event );

  if ( (event->pos() - d->dragStartPosition).manhattanLength() < QApplication::startDragDistance() ) {
    event->accept();
    return;
  }

  QDrag *drag = new QDrag( this );
  QMimeData *mimeData = new QMimeData;

  QByteArray data;
  {
    QDataStream stream( &data, QIODevice::WriteOnly );

    QStringList actionNames;
    actionNames << d->dragAction->objectName();

    stream << actionNames;
  }

  mimeData->setData( "application/x-kde-action-list", data );

  drag->setMimeData( mimeData );

  Qt::DropAction dropAction = drag->start( Qt::MoveAction );

  if ( dropAction == Qt::MoveAction )
    // Only remove from this toolbar if it was moved to another toolbar
    // Otherwise the receiver moves it.
    if ( drag->target() != this )
      removeAction( d->dragAction );

  d->dragAction = 0L;
  event->accept();
}

void KToolBar::mouseReleaseEvent( QMouseEvent *event )
{
  // Want to clear this even if toolBarsEditable was changed mid-drag (unlikey)
  if ( d->dragAction ) {
    d->dragAction = 0L;
    event->accept();
    return;
  }

  QToolBar::mouseReleaseEvent( event );
}

bool KToolBar::eventFilter( QObject * watched, QEvent * event )
{
  // Generate context menu events for disabled buttons too...
  if ( event->type() == QEvent::MouseButtonPress ) {
    QMouseEvent* me = static_cast<QMouseEvent*>( event );
    if ( me->buttons() & Qt::RightButton )
      if ( QWidget* ww = qobject_cast<QWidget*>( watched ) )
        if ( ww->parent() == this )
          if ( !ww->isEnabled() )
            QCoreApplication::postEvent( this, new QContextMenuEvent( QContextMenuEvent::Mouse, me->pos(), me->globalPos() ) );

  } else if ( event->type() == QEvent::ParentChange ) {
    // Make sure we're not leaving stale event filters around
    if ( QWidget* ww = qobject_cast<QWidget*>( watched ) ) {
      while ( ww ) {
        if ( ww == this )
          goto found;
      }
      // New parent is not a subwidget - remove event filter
      ww->removeEventFilter( this );
      foreach ( QWidget* child, ww->findChildren<QWidget*>() )
        child->removeEventFilter( this );
    }
  }

  found:

  // Redirect mouse events to the toolbar when drag + drop editing is enabled
  if ( toolBarsEditable() ) {
    if ( QWidget* ww = qobject_cast<QWidget*>( watched ) ) {
      switch ( event->type() ) {
        case QEvent::MouseButtonPress: {
          QMouseEvent* me = static_cast<QMouseEvent*>( event );
          QMouseEvent newEvent( me->type(), mapFromGlobal( ww->mapToGlobal( me->pos() ) ), me->globalPos(),
                                me->button(), me->buttons(), me->modifiers() );
          mousePressEvent( &newEvent );
          return true;
        }
        case QEvent::MouseMove: {
          QMouseEvent* me = static_cast<QMouseEvent*>( event );
          QMouseEvent newEvent( me->type(), mapFromGlobal( ww->mapToGlobal( me->pos() ) ), me->globalPos(),
                                me->button(), me->buttons(), me->modifiers() );
          mouseMoveEvent( &newEvent );
          return true;
        }
        case QEvent::MouseButtonRelease: {
          QMouseEvent* me = static_cast<QMouseEvent*>( event );
          QMouseEvent newEvent( me->type(), mapFromGlobal( ww->mapToGlobal( me->pos() ) ), me->globalPos(),
                                me->button(), me->buttons(), me->modifiers() );
          mouseReleaseEvent( &newEvent );
          return true;
        }
        default:
          break;
      }
    }
  }

  return QToolBar::eventFilter( watched, event );
}

void KToolBar::actionEvent( QActionEvent * event )
{
  if ( event->type() == QEvent::ActionRemoved ) {
    QWidget* widget = widgetForAction( event->action() );
    widget->removeEventFilter( this );

    foreach ( QWidget* child, widget->findChildren<QWidget*>() )
      child->removeEventFilter( this );
  }

  QToolBar::actionEvent( event );

  if ( event->type() == QEvent::ActionAdded ) {
    QWidget* widget = widgetForAction( event->action() );
    widget->installEventFilter( this );

    foreach ( QWidget* child, widget->findChildren<QWidget*>() )
      child->installEventFilter( this );
  }

  d->adjustSeparatorVisibility();
}

bool KToolBar::toolBarsEditable( )
{
  return KToolBar::Private::s_editable;
}

void KToolBar::setToolBarsEditable( bool editable )
{
  if ( KToolBar::Private::s_editable != editable )
    KToolBar::Private::s_editable = editable;
}

void KToolBar::setToolBarsLocked( bool locked )
{
  if ( KToolBar::Private::s_locked != locked ) {
    KToolBar::Private::s_locked = locked;

    foreach ( KMainWindow* mw, KMainWindow::memberList() )
      foreach ( KToolBar* toolbar, mw->findChildren<KToolBar*>() )
        toolbar->d->setLocked( locked );
  }
}

bool KToolBar::toolBarsLocked()
{
  return KToolBar::Private::s_locked;
}

#include "ktoolbar.moc"

