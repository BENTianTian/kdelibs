/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KSETTINGS_DIALOG_H
#define KSETTINGS_DIALOG_H

#include "../kutils_export.h"
#include "../kcmultidialog.h"

#include <kservice.h>

template<class T> class QList;
class KPluginInfo;
class KCModuleInfo;

namespace KSettings
{
    class DialogPrivate;

/**
 * @short Generic configuration dialog that works over component boundaries
 *
 * For more information see \ref KSettings.
 *
 * This class aims to standardize the use of configuration dialogs in KDE
 * applications. Especially when using KParts and/or Plugins you face problems
 * creating a consistent config dialog.
 *
 * To show a configuration dialog you only have to call the show method and be
 * done with it. A code example:
 *
 * You initialize \p m_cfgdlg with
 * \code
 * m_cfgdlg = new Dialog( Dialog::Static, this );
 * \endcode
 * If you use a KPart that was not especially designed for your app you can use
 * the second constructor:
 * \code
 * QStringList kpartslist;
 * for( all my kparts )
 *   kpartslist += m_mypart->componentData().componentName();
 * m_cfgdlg = new Dialog( kpartslist, this );
 * \endcode
 * and the action for the config dialog is connected to the show slot:
 * \code
 * KStdAction::preferences( m_cfgdlg, SLOT( show() ), actionCollection() );
 * \endcode
 *
 * If you need to be informed when the config was changed and applied in the
 * dialog you might want to take a look at Dispatcher.
 *
 * For more information see \ref KSettings.
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
class KUTILS_EXPORT Dialog : public KCMultiDialog
{
    friend class PageNode;
    Q_DECLARE_PRIVATE(Dialog)
    Q_OBJECT
    public:
        /**
         * Tells the dialog whether the entries in the listview are all static
         * or whether it should add a Configure... button to select which parts
         * of the optional functionality should be active or not.
         */
        enum ContentInListView
        {
            /**
             * Static listview, while running no entries are added or deleted
             */
            Static,
            /**
             * Configurable listview. The user can select what functionality he
             * wants.
             */
            Configurable
        };

        /**
         * Construct a new Preferences Dialog for the application. It uses all
         * KCMs with X-KDE-ParentApp set to KGlobal::mainComponent().componentName().
         *
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param arguments    A list of arguments that are passed to all
         *                     KCModules when adding them to the dialog
         */
        explicit Dialog( ContentInListView content = Static, QWidget * parent = 0,
                         const QStringList& arguments = QStringList() );

        /**
         * Construct a new Preferences Dialog with the pages for the selected
         * instance names. For example if you want to have the configuration
         * pages for the kviewviewer KPart you would pass a
         * QStringList consisting of only the name of the part "kviewviewer".
         *
         * @param components   A list of the names of the components that your
         *                     config dialog should merge the config pages in.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param arguments    A list of arguments that are passed to all
         *                     KCModules when adding them to the dialog
         */
         explicit Dialog( const QStringList & components, QWidget * parent = 0,
                          const QStringList& arguments = QStringList() );

        /**
         * Construct a new Preferences Dialog with the pages for the selected
         * instance names. For example if you want to have the configuration
         * pages for the kviewviewer KPart you would pass a
         * QStringList consisting of only the name of the part "kviewviewer".
         *
         * @param components   A list of the names of the components that your
         *                     config dialog should merge the config pages in.
         * @param content      Select whether you want a static or configurable
         *                     config dialog.
         * @param parent       The parent is only used as the parent for the
         *                     dialog - centering the dialog over the parent
         *                     widget.
         * @param arguments    A list of arguments that are passed to all
         *                     KCModules when adding them to the dialog
         */
        Dialog( const QStringList & components, ContentInListView
                content, QWidget * parent = 0,
                const QStringList& arguments = QStringList() );

        ~Dialog();

        /**
         * If you use a Configurable dialog you need to pass KPluginInfo
         * objects that the dialog should configure.
         */
        void addPluginInfos( const QList<KPluginInfo*> & plugininfos );

    public Q_SLOTS:
        /**
         * Show the config dialog. The slot immediately returns since the dialog
         * is non-modal.
         */
        void show();

    Q_SIGNALS:
        /**
         * If you use the dialog in Configurable mode and want to be notified
         * when the user changes the plugin selections use this signal. It's
         * emitted if the selection has changed and the user pressed Apply or
         * Ok. In the slot you would then load and unload the plugins as
         * requested.
         */
        void pluginSelectionChanged();

    protected:
        DialogPrivate *const d_ptr;

    private:
        Q_PRIVATE_SLOT(d_func(), void _k_configureTree())
        Q_PRIVATE_SLOT(d_func(), void _k_updateTreeList())
        Q_PRIVATE_SLOT(d_func(), void _k_syncConfiguration())
        Q_PRIVATE_SLOT(d_func(), void _k_reparseConfiguration(const QByteArray &))
};

}

#endif // KSETTINGS_DIALOG_H
