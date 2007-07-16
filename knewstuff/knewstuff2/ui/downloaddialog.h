/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *   Copyright (C) 2007 Dirk Mueller <mueller@kde.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 ***************************************************************************/

#ifndef KNEWSTUFF2_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF2_UI_DOWNLOADDIALOG_H

#include <kdialog.h>
#include <ktitlewidget.h>

#include <QtGui/QScrollArea>

#include <knewstuff2/dxs/dxsengine.h>
#include <knewstuff2/core/category.h>

class KJob;
class KLineEdit;
class QComboBox;
class QLabel;

namespace KNS
{

    class DownloadDialog;

/** GUI/CORE: HTML Widget to operate on AvailableItem::List */
class ItemsView : public QScrollArea
{
public:
    ItemsView( DownloadDialog * newStuffDialog, QWidget * parentWidget );
    ~ItemsView();

    void setEngine(DxsEngine *engine);

    void setItems( /*const Entry::List & itemList*/ QMap<const Feed*, KNS::Entry::List> itemList );
    void setProviders( QMap<Entry*, const Provider*> providers );
    void setSorting( int sortType );
    void updateItem( Entry *entry );

private:
    void buildContents();

    void clear();

    DownloadDialog * m_newStuffDialog;
    QMap<const Feed*, Entry::List> m_entries;
    QMap<Entry*, const Provider*> m_providers;
    QWidget *m_root;
    int m_sorting;
    QMap<QPixmap*, QWidget*> m_pixmaps;
    DxsEngine *m_engine;
};



/**
 * KNewStuff download dialog.
 *
 * The download dialog will present several categories of items to the user
 * for installation, deinstallation and updates.
 * Preview images as well as other meta information can be seen.
 * If the GHNS provider supports DXS, then the dialog will offer an extended
 * popup menu with collaborative interactions such as ratings, removal
 * requests, comments and translation suggestions.
 *
 * Please remember when changing this class that it has to render
 * untrusted data. Do not let scripts run, make sure that data is properly escaped etc.
 *
 * This class is used internally by the Engine class as part of the download
 * workflow.
 *
 * @internal
 */
class DownloadDialog : public KDialog
{
    Q_OBJECT

public:
    DownloadDialog( DxsEngine* engine, QWidget * parent );
    ~DownloadDialog();

    void addEntry(Entry *entry, const Feed *feed, const Provider *provider);
    void refresh();

    // show a message in the bottom bar
    void displayMessage( const QString & msg,
            KTitleWidget::MessageType type = KTitleWidget::PlainMessage,
            int timeOutMs = 0 );

    // begin installing that item
    void installItem( Entry * entry );

    // remove an already installed item
    void removeItem( Entry * entry );

private Q_SLOTS:
    void slotResetMessage();
    void slotNetworkTimeout();
    void slotSortingSelected( int sortType );
    // DXS
    void slotLoadProvidersListDXS();
    void slotLoadProviderDXS(int index);
    void slotCategories(QList<Category*> categories);
    void slotEntries(QList<Entry*> entries);
    void slotFault();
    void slotError();
    // file downloading
    void slotDownloadItem( Entry * );
    //void slotItemMessage( KJob *, const QString & );
    //void slotItemPercentage( KJob *, unsigned long );
    //void slotItemResult( KJob * );

private:
    // Contents
    // gui related vars
    KLineEdit * searchLine;
    QComboBox * typeCombo;
    QComboBox * sortCombo;
    QLabel    * statusLabel;
    ItemsView * itemsView;

    // other classes
    QTimer * messageTimer;
    QTimer * networkTimer;
    KTitleWidget* titleWidget;

    DxsEngine *m_engine;
    QMap<QString, QString> categorymap;

    //QList<Entry*> m_entries;
    QMap<const Feed*, Entry::List> entries;
    QMap<Entry*, const Provider*> providers;
};

}

#endif
