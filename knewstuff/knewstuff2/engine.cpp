/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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
#include "knewstuff2/engine.h"

#include "knewstuff2/ui/downloaddialog.h"
#include "knewstuff2/ui/uploaddialog.h"
#include "knewstuff2/ui/providerdialog.h"

#include "knewstuff2/core/entryhandler.h" // tmp

#include <kapplication.h> // kapp-ptr
#include <kglobal.h>

#include <kdebug.h>

using namespace KNS;

Engine::Engine()
: DxsEngine()
{
	m_command = command_none;
	m_uploaddialog = NULL;
	m_downloaddialog = NULL;
	setDxsPolicy(DxsEngine::DxsNever); // FIXME: until KIO/cDXS gets fixed!
	m_entry = NULL;
	m_modal = false;
}

Engine::~Engine()
{
}

void Engine::workflow()
{
	if((m_command == command_upload) || (m_command == command_download))
	{
		connect(this,
			SIGNAL(signalProviderLoaded(KNS::Provider*)),
			SLOT(slotProviderLoaded(KNS::Provider*)));
		connect(this,
			SIGNAL(signalProvidersFailed()),
			SLOT(slotProvidersFailed()));
	}

	if(m_command == command_upload)
	{
		connect(this,
			SIGNAL(signalProvidersFinished()),
			SLOT(slotProvidersFinished()));

		m_entry = NULL;
	}

	if(m_command == command_download)
	{
		connect(this,
			SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
			SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
		connect(this,
			SIGNAL(signalEntriesFailed()),
			SLOT(slotEntriesFailed()));
		connect(this,
			SIGNAL(signalEntriesFinished()),
			SLOT(slotEntriesFinished()));
		connect(this,
			SIGNAL(signalEntriesFeedFinished(const KNS::Feed*)),
			SLOT(slotEntriesFeedFinished(const KNS::Feed*)));

		m_downloaddialog = new DownloadDialog(0);
		m_downloaddialog->setEngine(this);
		m_downloaddialog->show();

		// FIXME: catch dialog close event, and then clean up
		// FIXME: but dialogs don't have signals
		//connect(m_downloaddialog, SIGNAL(closed()), SLOT(slotDialogClosed()));
		//...
		//m_command = command_none;
		//delete m_downloaddialog;
	}

	start();

	if(m_modal)
	{
		while((m_command == command_upload) || (m_command == command_download))
		{
			kapp->processEvents();
		}
	}
}

KNS::Entry::List Engine::download()
{
	KNS::Entry::List entries;

	Engine *engine = new Engine();

	KComponentData component = KGlobal::mainComponent();
	QString name = component.componentName();

	bool ret = engine->init(name + ".knsrc");
	if(!ret) return entries;

	entries = engine->downloadDialogModal();
	delete engine;

	// FIXME: refcounting?
	return entries;
}

KNS::Entry::List Engine::downloadDialogModal()
{
	kDebug(550) << "Engine: downloadDialogModal" << endl;

	m_command = command_download;
	m_modal = true;

	workflow();

	return KNS::Entry::List();
}

void Engine::downloadDialog()
{
	kDebug(550) << "Engine: downloadDialog" << endl;

	if(m_command != command_none)
	{
		kError(550) << "Engine: asynchronous workflow already going on" << endl;
	}

	m_command = command_download;
	m_modal = false;

	workflow();
}

KNS::Entry *Engine::upload(QString file)
{
	KNS::Entry *entry = NULL;

	Engine *engine = new Engine();

	KComponentData component = KGlobal::mainComponent();
	QString name = component.componentName();

	bool ret = engine->init(name + ".knsrc");
	if(!ret) return entry;

	entry = engine->uploadDialogModal(file);
	delete engine;

	// FIXME: refcounting?
	return entry;
}

KNS::Entry *Engine::uploadDialogModal(QString file)
{
	kDebug(550) << "Engine: uploadDialogModal" << endl;

	m_command = command_upload;
	m_modal = true;
	m_uploadfile = file;

	workflow();

	return m_entry;
}

void Engine::uploadDialog(QString file)
{
	kDebug(550) << "Engine: uploadDialog" << endl;

	if(m_command != command_none)
	{
		kError(550) << "Engine: asynchronous workflow already going on" << endl;
	}

	m_command = command_upload;
	m_modal = false;
	m_uploadfile = file;

	workflow();
}

void Engine::slotProviderLoaded(KNS::Provider *provider)
{
	kDebug(550) << "Engine: slotProviderLoaded" << endl;

	if(m_command == command_download)
	{
		this->loadEntries(provider);
	}
	else if(m_command == command_upload)
	{
		// FIXME: inject into upload dialog
		// FIXME: dialog could do this by itself!

		// FIXME: for the modal dialog, do nothing here
		// ... and wait for slotProvidersFinished()
		m_providers.append(provider);
	}
	else
	{
		kError(550) << "Engine: invalid command" << endl;
	}
}

void Engine::slotProvidersFailed()
{
	kDebug(550) << "Engine: slotProvidersFailed" << endl;

	m_command = command_none;
}

void Engine::slotEntryLoaded(KNS::Entry *entry, const Feed *feed, const Provider *provider)
{
	kDebug(550) << "Engine: slotEntryLoaded" << endl;

	m_downloaddialog->addEntry(entry, feed, provider);
}

void Engine::slotEntriesFailed()
{
	kDebug(550) << "Engine: slotEntriesFailed" << endl;
}

void Engine::slotEntryUploaded()
{
	kDebug(550) << "Engine: slotEntryUploaded" << endl;

	m_command = command_none;

	//m_entry = ...; // FIXME: where do we get it from now?
	// FIXME: we cannot assign it earlier, probably need m_delayedentry
	// FIXME: if not modal, this must be a signal to the outside (is already the case?)
}

void Engine::slotEntryFailed()
{
	kDebug(550) << "Engine: slotEntryFailed" << endl;

	m_command = command_none;
}

void Engine::slotProvidersFinished()
{
	kDebug(550) << "Engine: slotProvidersFinished" << endl;

	int ret;

	//Provider *fakeprovider = new Provider();
	//fakeprovider->setName(QString("Fake Provider"));
	//fakeprovider->setUploadUrl(KUrl("http://localhost/dav/"));
	//fakeprovider->setUploadUrl(KUrl("webdav://localhost/uploads/"));

	ProviderDialog provdialog(0);
	for(Provider::List::Iterator it = m_providers.begin(); it != m_providers.end(); it++)
	{
		Provider *provider = (*it);
		provdialog.addProvider(provider);
	}
	//provdialog.addProvider(fakeprovider);
	ret = provdialog.exec();
	if(ret == QDialog::Rejected)
	{
		m_command = command_none;
		return;
	}

	KNS::Provider *provider = provdialog.provider();

	UploadDialog uploaddialog(0);
	uploaddialog.setPayloadFile(KUrl(m_uploadfile));
	ret = uploaddialog.exec();
	if(ret == QDialog::Rejected)
	{
		m_command = command_none;
		return;
	}

	Entry *entry = uploaddialog.entry();
	entry->setPayload(m_uploadfile);
	if(!entry)
	{
		m_command = command_none;
		return;
	}

	EntryHandler eh(*entry);
	QDomElement xml = eh.entryXML();
	QByteArray ar;
	QTextStream txt(&ar);
	txt << xml;
	kDebug(550) << "Upload: " << QString(ar) << endl;

	connect(this,
		SIGNAL(signalEntryUploaded()),
		SLOT(slotEntryUploaded()));
	connect(this,
		SIGNAL(signalEntryFailed()),
		SLOT(slotEntryFailed()));

	uploadEntry(provider, entry);
}

void Engine::slotEntriesFeedFinished(const Feed *feed)
{
	kDebug(550) << "Engine: slotEntriesFeedFinished" << endl;

	Q_UNUSED(feed);
	//m_downloaddialog->refresh();
}

void Engine::slotEntriesFinished()
{
	kDebug(550) << "Engine: slotEntriesFinished" << endl;

	m_downloaddialog->refresh();
}

#include "engine.moc"
