/* This file is part of the KDE libraries
   Copyright 2009 by Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstatusnotifieritem.h"
#include "kstatusnotifieritemprivate_p.h"
#include "kstatusnotifieritemdbus_p.h"

#include <QDBusConnection>
#include <QPixmap>
#include <QImage>
#include <QApplication>
#include <QMovie>
#include <QPainter>


#include <kdebug.h>
#include <ksystemtrayicon.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kmenu.h>
#include <kaction.h>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <netinet/in.h>

#include "statusnotifieritemadaptor.h"

KStatusNotifierItem::KStatusNotifierItem(QObject *parent)
      : QObject(parent),
        d(new KStatusNotifierItemPrivate(this))
{
    d->init(QString());
}


KStatusNotifierItem::KStatusNotifierItem(const QString &id, QObject *parent)
      : QObject(parent),
        d(new KStatusNotifierItemPrivate(this))
{
    d->init(id);
}

KStatusNotifierItem::~KStatusNotifierItem()
{
    delete d->statusNotifierWatcher;
    delete d->notificationsClient;
    delete d->systemTrayIcon;
    delete d->menu;
    delete d;
    KGlobal::deref();
}

QString KStatusNotifierItem::id() const
{
    //kDebug(299) << "id requested" << d->id;
    return d->id;
}

void KStatusNotifierItem::setCategory(const ItemCategory category)
{
    d->category = category;
}

KStatusNotifierItem::ItemStatus KStatusNotifierItem::status() const
{
    return d->status;
}

KStatusNotifierItem::ItemCategory KStatusNotifierItem::category() const
{
    return d->category;
}

void KStatusNotifierItem::setTitle(const QString &title)
{
    d->title = title;
}

void KStatusNotifierItem::setStatus(const ItemStatus status)
{
    d->status = status;
    emit d->statusNotifierItemDBus->NewStatus(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status));

    if (d->systemTrayIcon) {
        d->syncLegacySystemTrayIcon();
    }
}



//normal icon

void KStatusNotifierItem::setIconByName(const QString &name)
{
    d->serializedIcon = KDbusImageVector();
    d->iconName = name;
    emit d->statusNotifierItemDBus->NewIcon();
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(KIcon(name));
    }
}

QString KStatusNotifierItem::iconName() const
{
    return d->iconName;
}

void KStatusNotifierItem::setIconByPixmap(const QIcon &icon)
{
    d->iconName.clear();
    d->serializedIcon = d->iconToVector(icon);
    emit d->statusNotifierItemDBus->NewIcon();

    d->icon = icon;
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setIcon(icon);
    }
}

QIcon KStatusNotifierItem::iconPixmap() const
{
    return d->icon;
}

void KStatusNotifierItem::setOverlayIconByName(const QString &name)
{
    d->overlayIconName = name;
    emit d->statusNotifierItemDBus->NewOverlayIcon();
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = KIcon(d->iconName).pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        if (!name.isEmpty()) {
            QPixmap overlayPixmap = KIcon(d->overlayIconName).pixmap(KIconLoader::SizeSmallMedium/2, KIconLoader::SizeSmallMedium/2);
            QPainter p(&iconPixmap);
            p.drawPixmap(iconPixmap.width()-overlayPixmap.width(), iconPixmap.height()-overlayPixmap.height(), overlayPixmap);
            p.end();
        }
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QString KStatusNotifierItem::overlayIconName() const
{
    return d->overlayIconName;
}

void KStatusNotifierItem::setOverlayIconByPixmap(const QIcon &icon)
{
    d->serializedOverlayIcon = d->iconToVector(icon);
    emit d->statusNotifierItemDBus->NewOverlayIcon();

    d->overlayIcon = icon;
    if (d->systemTrayIcon) {
        QPixmap iconPixmap = d->icon.pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        QPixmap overlayPixmap = d->overlayIcon.pixmap(KIconLoader::SizeSmallMedium/2, KIconLoader::SizeSmallMedium/2);

        QPainter p(&iconPixmap);
        p.drawPixmap(iconPixmap.width()-overlayPixmap.width(), iconPixmap.height()-overlayPixmap.height(), overlayPixmap);
        p.end();
        d->systemTrayIcon->setIcon(iconPixmap);
    }
}

QIcon KStatusNotifierItem::overlayIconPixmap() const
{
    return d->overlayIcon;
}

//Icons and movie for requesting attention state

void KStatusNotifierItem::setAttentionIconByName(const QString &name)
{
    d->serializedAttentionIcon = KDbusImageVector();
    d->attentionIconName = name;
    emit d->statusNotifierItemDBus->NewAttentionIcon();
}

QString KStatusNotifierItem::attentionIconName() const
{
    return d->attentionIconName;
}

void KStatusNotifierItem::setAttentionIconByPixmap(const QIcon &icon)
{
    d->attentionIconName.clear();
    d->serializedAttentionIcon = d->iconToVector(icon);
    d->attentionIcon = icon;
    emit d->statusNotifierItemDBus->NewAttentionIcon();
}

QIcon KStatusNotifierItem::attentionIconPixmap() const
{
    return d->attentionIcon;
}

void KStatusNotifierItem::setAttentionMovieByName(const QString &name)
{
    if (d->movieName == name) {
        return;
    }

    d->movieName = name;

    delete d->movie;
    d->movie = 0;

    emit d->statusNotifierItemDBus->NewAttentionIcon();

    if (d->systemTrayIcon) {
        d->movie = new QMovie(d->movieName);
        d->systemTrayIcon->setMovie(d->movie);
    }
}

QString KStatusNotifierItem::attentionMovieName() const
{
    return d->movieName;
}

//ToolTip

void KStatusNotifierItem::setToolTip(const QString &iconName, const QString &title, const QString &subTitle)
{
    setToolTipIconByName(iconName);
    setToolTipTitle(title);
    setToolTipSubTitle(subTitle);
    emit d->statusNotifierItemDBus->NewToolTip();
}

void KStatusNotifierItem::setToolTip(const QIcon &icon, const QString &title, const QString &subTitle)
{
    setToolTipIconByPixmap(icon);
    setToolTipTitle(title);
    setToolTipSubTitle(subTitle);
    emit d->statusNotifierItemDBus->NewToolTip();
}

void KStatusNotifierItem::setToolTipIconByName(const QString &name)
{
    d->serializedToolTipIcon = KDbusImageVector();
    d->toolTipIconName = name;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QString KStatusNotifierItem::toolTipIconName() const
{
    return d->toolTipIconName;
}

void KStatusNotifierItem::setToolTipIconByPixmap(const QIcon &icon)
{
    d->toolTipIconName.clear();
    d->serializedToolTipIcon = d->iconToVector(icon);
    d->toolTipIcon = icon;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QIcon KStatusNotifierItem::toolTipIconPixmap() const
{
    return d->toolTipIcon;
}

void KStatusNotifierItem::setToolTipTitle(const QString &title)
{
    d->toolTipTitle = title;
    emit d->statusNotifierItemDBus->NewToolTip();
    if (d->systemTrayIcon) {
        d->systemTrayIcon->setToolTip(title);
    }
}

QString KStatusNotifierItem::toolTipTitle() const
{
    return d->toolTipTitle;
}

void KStatusNotifierItem::setToolTipSubTitle(const QString &subTitle)
{
    d->toolTipSubTitle = subTitle;
    emit d->statusNotifierItemDBus->NewToolTip();
}

QString KStatusNotifierItem::toolTipSubTitle() const
{
    return d->toolTipSubTitle;
}


void KStatusNotifierItem::setContextMenu(KMenu *menu)
{
    if (d->menu != menu) {
        d->menu->removeEventFilter(this);
        delete d->menu;
    }

    d->menu = menu;

    if (d->menu) {
        d->menu->installEventFilter(this);
        connect(d->menu, SIGNAL(aboutToShow()), this, SLOT(contextMenuAboutToShow()));
    }

    if (d->systemTrayIcon) {
        d->systemTrayIcon->setContextMenu(menu);
    }
}

KMenu *KStatusNotifierItem::contextMenu() const
{
    return d->menu;
}

void KStatusNotifierItem::setAssociatedWidget(QWidget *associatedWidget)
{
    if (associatedWidget) {
        d->associatedWidget = associatedWidget->window();
    } else {
        d->associatedWidget = 0;
    }

    if (d->systemTrayIcon) {
        delete d->systemTrayIcon;
        d->systemTrayIcon = 0;
        d->setLegacySystemTrayEnabled(true);
    }

    if (d->associatedWidget) {
        QAction *action = d->actionCollection->action("minimizeRestore");

        if (!action) {
            action = d->actionCollection->addAction("minimizeRestore");
            action->setText(i18n("&Minimize"));
            connect(action, SIGNAL(triggered(bool)), this, SLOT(minimizeRestore()));
        }

#ifdef Q_WS_X11
        KWindowInfo info = KWindowSystem::windowInfo(d->associatedWidget->winId(), NET::WMDesktop);
        d->onAllDesktops = info.onAllDesktops();
#else
        d->onAllDesktops = false;
#endif
    } else {
        if (d->menu && d->hasQuit) {
            QAction *action = d->actionCollection->action("minimizeRestore");
            if (action) {
                d->menu->removeAction(action);
            }
        }

        d->onAllDesktops = false;
    }
}

QWidget *KStatusNotifierItem::associatedWidget() const
{
    return d->associatedWidget;
}

KActionCollection *KStatusNotifierItem::actionCollection() const
{
    return d->actionCollection;
}

void KStatusNotifierItem::setStandardActionsEnabled(bool enabled)
{
    if (d->standardActionsEnabled == enabled) {
        return;
    }

    d->standardActionsEnabled = enabled;

    if (d->menu && !enabled && d->hasQuit) {
        QAction *action = d->actionCollection->action("minimizeRestore");
        if (action) {
            d->menu->removeAction(action);
        }

        action = d->actionCollection->action(KStandardAction::name(KStandardAction::Quit));
        if (action) {
            d->menu->removeAction(action);
        }


        d->hasQuit = false;
    }
}

bool KStatusNotifierItem::standardActionsEnabled() const
{
    return d->standardActionsEnabled;
}

void KStatusNotifierItem::showMessage(const QString & title, const QString & message, const QString &icon, int timeout)
{
    if (!d->notificationsClient) {
        d->notificationsClient = new org::freedesktop::Notifications("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                                                QDBusConnection::sessionBus());
    }

    uint id = 0;
    d->notificationsClient->Notify(d->title, id, icon, title, message, QStringList(), QVariantMap(), timeout);
}

QString KStatusNotifierItem::title() const
{
    return d->title;
}



void KStatusNotifierItem::activate(const QPoint &pos)
{
    //if the user activated the icon the NeedsAttention state is no longer necessary
    //FIXME: always true?
    if (d->status == NeedsAttention) {
        d->status = Active;
        emit d->statusNotifierItemDBus->NewStatus(metaObject()->enumerator(metaObject()->indexOfEnumerator("ItemStatus")).valueToKey(d->status));
    }

    if (d->associatedWidget == d->menu) {
        d->statusNotifierItemDBus->ContextMenu(pos.x(), pos.y());
        return;
    }
    if (d->menu->isVisible()) {
        d->menu->hide();
    }

    if (!d->associatedWidget) {
        emit activateRequested(true, pos);
        return;
    }

    d->checkVisibility(pos);
}

bool KStatusNotifierItemPrivate::checkVisibility(QPoint pos, bool perform)
{
#ifdef Q_WS_WIN
#if 0
    // the problem is that we lose focus when the systray icon is activated
    // and we don't know the former active window
    // therefore we watch for activation event and use our stopwatch :)
    if(GetTickCount() - dwTickCount < 300) {
        // we were active in the last 300ms -> hide it
        minimizeRestore(false);
        emit activateRequested(false, pos);
    } else {
        minimizeRestore(true);
        emit activateRequested(true, pos);
    }
#endif
#elif defined(Q_WS_X11)
    KWindowInfo info1 = KWindowSystem::windowInfo(associatedWidget->winId(), NET::XAWMState | NET::WMState | NET::WMDesktop);
    // mapped = visible (but possibly obscured)
    bool mapped = (info1.mappingState() == NET::Visible) && !info1.isMinimized();

//    - not mapped -> show, raise, focus
//    - mapped
//        - obscured -> raise, focus
//        - not obscured -> hide
    //info1.mappingState() != NET::Visible -> window on another desktop?
    if (!mapped) {
        if (perform) {
            minimizeRestore(true);
            emit q->activateRequested(true, pos);
        }

        return true;
    } else {
        QListIterator< WId > it (KWindowSystem::stackingOrder());
        it.toBack();
        while (it.hasPrevious()) {
            WId id = it.previous();
            if (id == associatedWidget->winId()) {
                break;
            }

            KWindowInfo info2 = KWindowSystem::windowInfo(id,
                NET::WMDesktop | NET::WMGeometry | NET::XAWMState | NET::WMState | NET::WMWindowType);

            if (info2.mappingState() != NET::Visible) {
                continue; // not visible on current desktop -> ignore
            }

            if (!info2.geometry().intersects(associatedWidget->geometry())) {
                continue; // not obscuring the window -> ignore
            }

            if (!info1.hasState(NET::KeepAbove) && info2.hasState(NET::KeepAbove)) {
                continue; // obscured by window kept above -> ignore
            }

            NET::WindowType type = info2.windowType(NET::NormalMask | NET::DesktopMask
                | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
                | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask);

            if (type == NET::Dock || type == NET::TopMenu) {
                continue; // obscured by dock or topmenu -> ignore
            }

            if (perform) {
                KWindowSystem::raiseWindow(associatedWidget->winId());
                KWindowSystem::activateWindow(associatedWidget->winId());
                emit q->activateRequested(true, pos);
            }

            return true;
        }

        //not on current desktop?
        if (!info1.isOnCurrentDesktop()) {
            if (perform) {
                KWindowSystem::activateWindow(associatedWidget->winId());
                emit q->activateRequested(true, pos);
            }

            return true;
        }

        if (perform) {
            minimizeRestore(false); // hide
            emit q->activateRequested(false, pos);
        }

        return false;
    }
#endif

    return true;
}

bool KStatusNotifierItem::eventFilter(QObject *watched, QEvent *event)
{
    if (d->systemTrayIcon == 0) {
        //FIXME: ugly ugly workaround to weird QMenu's focus problems
        if (watched == d->menu &&
            (event->type() == QEvent::WindowDeactivate || (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton))) {
            //put at the back of even queue to let the action activate anyways
            QTimer::singleShot(0, this, SLOT(hideMenu()));
        }
    }
    return false;
}


//KStatusNotifierItemPrivate

const int KStatusNotifierItemPrivate::s_protocolVersion = 0;

KStatusNotifierItemPrivate::KStatusNotifierItemPrivate(KStatusNotifierItem *item)
    : q(item),
      category(KStatusNotifierItem::ApplicationStatus),
      status(KStatusNotifierItem::Passive),
      movie(0),
      menu(0),
      titleAction(0),
      statusNotifierWatcher(0),
      notificationsClient(0),
      systemTrayIcon(0),
      hasQuit(false),
      onAllDesktops(false),
      standardActionsEnabled(true)
{
}

void KStatusNotifierItemPrivate::init(const QString &extraId)
{
    // Ensure that closing the last KMainWindow doesn't exit the application
    // if a system tray icon is still present.
    KGlobal::ref();

    qDBusRegisterMetaType<KDbusImageStruct>();
    qDBusRegisterMetaType<KDbusImageVector>();
    qDBusRegisterMetaType<KDbusToolTipStruct>();

    actionCollection = new KActionCollection(q);
    statusNotifierItemDBus = new KStatusNotifierItemDBus(q);
    q->setAssociatedWidget(qobject_cast<QWidget*>(q->parent()));

    QObject::connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     q, SLOT(serviceChange(QString,QString,QString)));

    //create a default menu, just like in KSystemtrayIcon
    menu = new KMenu(associatedWidget);
    titleAction = menu->addTitle(qApp->windowIcon(), KGlobal::caption());
    menu->setTitle(KGlobal::mainComponent().aboutData()->programName());
    q->setContextMenu(menu);

    KStandardAction::quit(q, SLOT(maybeQuit()), actionCollection);

    id = title = KGlobal::mainComponent().aboutData()->programName();

    if (!extraId.isEmpty()) {
        id.append('_').append(extraId);
    }

    // Init iconThemePath to the app folder for now
    QStringList dirs = KGlobal::dirs()->findDirs("appdata", "icons");
    if (!dirs.isEmpty()) {
        iconThemePath = dirs.first();
    }

    registerToDaemon();
}

void KStatusNotifierItemPrivate::registerToDaemon()
{
    kDebug(299) << "Registering a client interface to the KStatusNotifierWatcher";
    if (!statusNotifierWatcher) {
        QString interface("org.kde.StatusNotifierWatcher");
        statusNotifierWatcher = new org::kde::StatusNotifierWatcher(interface, "/StatusNotifierWatcher",
                                                                        QDBusConnection::sessionBus());
    }

    if (statusNotifierWatcher->isValid() &&
        statusNotifierWatcher->property("ProtocolVersion").toInt() == s_protocolVersion) {

        statusNotifierWatcher->RegisterStatusNotifierItem(statusNotifierItemDBus->service());
            setLegacySystemTrayEnabled(false);
    } else {
        kDebug(299)<<"KStatusNotifierWatcher not reachable";
        setLegacySystemTrayEnabled(true);
    }
}

void KStatusNotifierItemPrivate::serviceChange(const QString& name, const QString& oldOwner, const QString& newOwner)
{
    bool legacy = false;
    if (name == "org.kde.StatusNotifierWatcher") {
        if (newOwner.isEmpty()) {
            //unregistered
            kDebug(299) << "Connection to the KStatusNotifierWatcher lost";
            legacy = true;
        } else if (oldOwner.isEmpty()) {
            //registered
            legacy = false;
        }
    } else if (name.startsWith(QLatin1String("org.kde.StatusNotifierHost-"))) {
        if (newOwner.isEmpty() && (!statusNotifierWatcher ||
                                   !statusNotifierWatcher->property("IsStatusNotifierHostRegistered").toBool())) {
            kDebug(299)<<"Connection to the last KStatusNotifierHost lost";
            legacy = true;
        } else if (oldOwner.isEmpty()) {
            kDebug(299)<<"New KStatusNotifierHost";
            legacy = false;
        }
    } else {
        return;
    }

    kDebug(299) << "Service " << name << "status change, old owner:" << oldOwner << "new:" << newOwner;

    if (legacy == (systemTrayIcon != 0)) {
        return;
    }

    if (legacy) {
        //unregistered
        setLegacySystemTrayEnabled(true);
    } else {
        //registered
        registerToDaemon();
    }
}

void KStatusNotifierItemPrivate::legacyWheelEvent(int delta)
{
    statusNotifierItemDBus->Scroll(delta, "vertical");
}

void KStatusNotifierItemPrivate::legacyActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::MiddleClick) {
        emit q->secondaryActivateRequested(systemTrayIcon->geometry().topLeft());
    }
}

void KStatusNotifierItemPrivate::setLegacySystemTrayEnabled(bool enabled)
{
    if (enabled) {
        delete statusNotifierWatcher;
        statusNotifierWatcher = 0;

        if (!systemTrayIcon) {
            systemTrayIcon = new KStatusNotifierLegacyIcon(associatedWidget);
            systemTrayIcon->setContextMenu(menu);
            syncLegacySystemTrayIcon();
            systemTrayIcon->setToolTip(toolTipTitle);
            systemTrayIcon->show();
            QObject::connect(systemTrayIcon, SIGNAL(wheel(int)), q, SLOT(legacyWheelEvent(int)));
            QObject::connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), q, SLOT(legacyActivated(QSystemTrayIcon::ActivationReason)));
        }

        if (menu) {
            menu->setWindowFlags(Qt::Popup);
        }
    } else {
        delete systemTrayIcon;
        systemTrayIcon = 0;

        if (menu) {
            menu->setWindowFlags(Qt::Window);
        }
    }
}

void KStatusNotifierItemPrivate::syncLegacySystemTrayIcon()
{
    if (status == KStatusNotifierItem::NeedsAttention) {
        if (!movieName.isNull()) {
            if (!movie) {
                movie = new QMovie(movieName);
            }
            systemTrayIcon->setMovie(movie);
        } else if (!attentionIconName.isNull()) {
            systemTrayIcon->setIcon(KIcon(attentionIconName));
        } else {
            systemTrayIcon->setIcon(attentionIcon);
        }
    } else {
        if (!iconName.isNull()) {
            systemTrayIcon->setIcon(KIcon(iconName));
        } else {
            systemTrayIcon->setIcon(icon);
        }
    }

    systemTrayIcon->setToolTip(toolTipTitle);
}

void KStatusNotifierItemPrivate::contextMenuAboutToShow()
{
    if (!hasQuit && standardActionsEnabled) {
        // we need to add the actions to the menu afterwards so that these items
        // appear at the _END_ of the menu
        menu->addSeparator();
        if (associatedWidget) {
            QAction *action = actionCollection->action("minimizeRestore");

            if (action) {
                menu->addAction(action);
            }
        }

        QAction *action = actionCollection->action(KStandardAction::name(KStandardAction::Quit));

        if (action) {
            menu->addAction(action);
        }

        hasQuit = true;
    }

    if (associatedWidget) {
        QAction* action = actionCollection->action("minimizeRestore");
        if (checkVisibility(QPoint(0, 0), false)) {
            action->setText(i18n("&Restore"));
        } else {
            action->setText(i18n("&Minimize"));
        }
    }
}

void KStatusNotifierItemPrivate::maybeQuit()
{
    QString caption = KGlobal::caption();
    QString query = i18n("<qt>Are you sure you want to quit <b>%1</b>?</qt>", caption);

    if (KMessageBox::warningContinueCancel(associatedWidget, query,
                                     i18n("Confirm Quit From System Tray"),
                                     KStandardGuiItem::quit(),
                                     KStandardGuiItem::cancel(),
                                     QString("systemtrayquit%1")
                                            .arg(caption)) == KMessageBox::Continue) {
        qApp->quit();
    }

}

void KStatusNotifierItemPrivate::minimizeRestore()
{
    q->activate(QPoint(0, 0));
}

void KStatusNotifierItemPrivate::hideMenu()
{
    menu->hide();
}

void KStatusNotifierItemPrivate::minimizeRestore(bool show)
{
#ifdef Q_WS_X11
    KWindowInfo info = KWindowSystem::windowInfo(associatedWidget->winId(), NET::WMDesktop | NET::WMFrameExtents);
    if (show) {
        if (onAllDesktops) {
            KWindowSystem::setOnAllDesktops(associatedWidget->winId(), true);
        } else {
            KWindowSystem::setCurrentDesktop(info.desktop());
        }

        associatedWidget->move(info.frameGeometry().topLeft()); // avoid placement policies
        associatedWidget->show();
        associatedWidget->raise();
        KWindowSystem::raiseWindow(associatedWidget->winId());
        KWindowSystem::activateWindow(associatedWidget->winId());
    } else {
        onAllDesktops = info.onAllDesktops();
        associatedWidget->hide();
    }
#else
    if (show) {
        associatedWidget->show();
        associatedWidget->raise();
        KWindowSystem::forceActiveWindow(associatedWidget->winId());
    } else {
        associatedWidget->hide();
    }
#endif
}

KDbusImageStruct KStatusNotifierItemPrivate::imageToStruct(const QImage &image)
{
    KDbusImageStruct icon;
    icon.width = image.size().width();
    icon.height = image.size().height();
    if (image.format() == QImage::Format_ARGB32) {
        icon.data = QByteArray((char*)image.bits(), image.numBytes());
    } else {
        QImage image32 = image.convertToFormat(QImage::Format_ARGB32);
        icon.data = QByteArray((char*)image32.bits(), image32.numBytes());
    }

    //swap to network byte order if we are little endian
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        quint32 *uintBuf = (quint32 *) icon.data.data();
        for (uint i = 0; i < icon.data.size()/sizeof(quint32); ++i) {
            *uintBuf = htonl(*uintBuf);
            ++uintBuf;
        }
    }

    return icon;
}

KDbusImageVector KStatusNotifierItemPrivate::iconToVector(const QIcon &icon)
{
    KDbusImageVector iconVector;

    QPixmap iconPixmap;

    //if an icon exactly that size wasn't found don't add it to the vector
    foreach (const QSize &size, icon.availableSizes()) {
        //hopefully huge and enormous not necessary right now, since it's quite costly
        if (size.width() <= KIconLoader::SizeLarge) {
            iconPixmap = icon.pixmap(size);
            iconVector.append(imageToStruct(iconPixmap.toImage()));
        }
    }

    return iconVector;
}

#include "kstatusnotifieritem.moc"
#include "kstatusnotifieritemprivate_p.moc"
