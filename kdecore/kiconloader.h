/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __KIconLoader_h_Included__
#define __KIconLoader_h_Included__

#include <qstring.h>
#include <qlist.h>
#include <qmap.h>
#include <qpixmap.h>

#include <kglobal.h>
#include <kinstance.h>
#include <kicontheme.h>

class KIconGroup;
class KIconThemeNode;
class KConfig;
class KIconLoaderPrivate;
class KStandardDirs;
class KIconEffect;


/**
 * Iconloader for KDE.
 *
 * KIconLoader will load the current icon theme and all its base themes. 
 * Icons will be searched in any of these themes. Additionally, it caches 
 * icons and applies effects according the the user's preferences.
 *
 * In KDE, it is encouraged to load icons by "Group". An icon group is a 
 * location on the screen where icons are being used. Standard groups are: 
 * Desktop, Toolbar, MainToolbar and Small. Each group has some centrally
 * configured properties bound to it, including the icon size and effects. 
 * This makes it possible to offer a consistent icon look in all KDE 
 * applications.
 *
 * The standard groups are defined below.
 *
 * @li KIcon::Desktop: Icons in the iconview of konqueror, kdesktop and similar apps.
 * @li KIcon::Toolbar: Icons in toolbars.
 * @li KIcon::MainToolbar: Icons in the main toolbars.
 * @li KIcon::Small: Various small (typical 16x16) places: titlebars, listviews and
 * menu entries.
 *
 * The icons are stored on disk in an icon theme or in a standalone
 * directory. The icon theme directories contain multiple sizes and/or
 * depths for the same icon. The iconloader will load the correct one based
 * on the icon group and the current theme. Icon themes are stored globally 
 * in share/icons, or, application specific in share/apps/$appdir/icons.
 *
 * The standalone directories contain just one version of an icon. The 
 * directories that are searched are: $appdir/pics and $appdir/toolbar.
 * Icons in these directories can be loaded by using the special group
 * "User".
 *
 */
class KIconLoader
{
public:

    /** Provided for source compatibility. Do not use. */
    enum Size { Small=16, Medium=32, Large=48, Default=0 };

    /**
     * Construct the iconloader.
     * @param appname Add the data directories of this application to the
     * icon search path for the "User" group. The default argument adds the 
     * directories of the current application.
     */
    KIconLoader(QString appname=QString::null);

    /** Cleanup */
    ~KIconLoader();

    /** 
     * Add @em appname to the list of application specific directories.
     * @param appname The application name.
     */ 
    void addAppDir(QString appname);

    /**
     * Load an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or 
     * the "unknown" pixmap is returned, depending on the value of the
     * @em canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to 
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @em group.
     * @param state The icon state: @em DefaultState, @em ActiveState or 
     * @em DisabledState. Depending on the user's preferences, the iconloader 
     * may apply a visual effect to hint about its state.
     * @param path_store If not null, the path of the icon is stored here.
     * @param canReturnNull Can return a null pixmap? If false, the
     * "unknown" pixmap is returned when no appropriate icon has been found.
     */
    QPixmap loadIcon(QString name, int group, int size=0, 
	    int state=KIcon::DefaultState, QString *path_store=0L, 
	    bool canReturnNull=false);

    /**
     * Returns the path of an icon.
     * @param name The name of the icon, without extension.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @em group_or_size. If negative, search
     * icons whose size is - @em group_or_size.
     * @param canReturnNull Can return a null pixamp?
     */
    QString iconPath(QString name, int group_or_size, 
	    bool canReturnNull=false);

    /**
     * Query all available icons for a specific group, having a specific
     * context.
     * @param group_or_size The icon group or size. See @ref #iconPath.
     * @param context The icon context.
     */
    QStringList queryIcons(int group_or_size, int context=KIcon::Any);

    /**
     * Return the current size for an icon group.
     */
    int currentSize(int group);

    /**
     * Returns a pointer to the current theme. Can be used to query
     * available and default sizes for groups.
     */
    KIconTheme *theme();


private:
    KIcon iconPath2(QString name, int size);
    KIcon iconPath2(QString name, int size, int match, KIconThemeNode *node);

    void addIconTheme(KIconTheme *theme, KIconThemeNode *node);
    void addAppThemes(QString appname);
    void addIcons(QStringList *lst, int size, int context, KIconThemeNode *node);
    void printThemeTree(KIconThemeNode *node);

    QString mTheme;
    QStringList mThemeList, mThemeTree;
    KIconGroup *mpGroups;
    KIconThemeNode *mpThemeRoot;
    KStandardDirs *mpDirs;
    KIconEffect *mpEffect;
    KIconLoaderPrivate *d;
};

/** Load a desktop icon.  */
QPixmap DesktopIcon(QString name, int size=0, int state=KIcon::DefaultState, 
	KInstance *instance=KGlobal::instance());

/** Load a desktop icon. */
QPixmap DesktopIcon(QString name, KInstance *instance);

/** Load a toolbar icon.  */
QPixmap BarIcon(QString name, int size=0, int state=KIcon::DefaultState, 
	KInstance *instance=KGlobal::instance());

/** Load a toolbar icon.  */
QPixmap BarIcon(QString name, KInstance *instance);

/** Load a small icon.  */
QPixmap SmallIcon(QString name, int size=0, int state=KIcon::DefaultState, 
	KInstance *instance=KGlobal::instance());

/** Load a small icon.  */
QPixmap SmallIcon(QString name, KInstance *instance);

/** Load a main toolbar icon.  */
QPixmap MainBarIcon(QString name, int size=0, int state=KIcon::DefaultState, 
	KInstance *instance=KGlobal::instance());

/** Load a main toolbar icon.  */
QPixmap MainBarIcon(QString name, KInstance *instance);

/** Load a user icon. */
QPixmap UserIcon(QString name, int state=KIcon::DefaultState, 
	KInstance *instance=KGlobal::instance());

/** Load a user icon. */
QPixmap UserIcon(QString name, KInstance *instance);

/** Returns the current icon size for a specific group.  */
int IconSize(int group, KInstance *instance=KGlobal::instance());

#endif // __KIconLoader_h_Included__
