/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _KPDF_NEWSTUFF_H_
#define _KPDF_NEWSTUFF_H_

#include <qdialog.h>
//#include <knewstuff2/provider.h>
//#include <kio/job.h>

#include "dxs.h"

using namespace KNS;
namespace KIO { class Job; }

class AvailableItem;

class NewStuffDialog : public QDialog
{
    Q_OBJECT
    public:
        NewStuffDialog( QWidget * parent );
        ~NewStuffDialog();

	void setEngine(Dxs *engine);

        // show a message in the bottom bar
        enum MessageType { Normal, Info, Error };
        void displayMessage( const QString & msg,
            MessageType type = Normal, int timeOutMs = 3000 );

        // begin installing that item
        void installItem( AvailableItem * item );

        // remove an already installed item
        void removeItem( AvailableItem * item );

    signals:
        // tells that a file has been installed
        void installedFile( QString fileName, QString type );
        // tells that a file has been removed
        void removedFile( QString oldFileName );

    private:
        // private storage class
        class NewStuffDialogPrivate * d;

	Dxs *m_dxs;
	QMap<QString, QString> m_categorymap;

    private slots:
        void slotResetMessageColors();
        void slotNetworkTimeout();
        void slotSortingSelected( int sortType );
        // providersList loading
        void slotLoadProvidersList();
        void slotProvidersListInfoData( KIO::Job *, const QByteArray & );
        void slotProvidersListResult( KIO::Job * );
        // provider loading
        void slotLoadProvider( int provider = 0 );
        void slotProviderInfoData( KIO::Job *, const QByteArray & );
        void slotProviderInfoResult( KIO::Job * );
	// DXS
        void slotLoadProvidersListDXS();
        void slotLoadProviderDXS(int index);
	void slotCategories(QValueList<KNS::Category*> categories);
	void slotEntries(QValueList<KNS::Entry*> entries);
	void slotFault();
	void slotError();
        // file downloading
        void slotDownloadItem( AvailableItem * );
        void slotItemMessage( KIO::Job *, const QString & );
        void slotItemPercentage( KIO::Job *, unsigned long );
        void slotItemResult( KIO::Job * );
};

#endif
