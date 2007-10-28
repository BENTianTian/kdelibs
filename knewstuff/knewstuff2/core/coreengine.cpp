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

#include "coreengine.h"

#include "entryhandler.h"
#include "providerhandler.h"
#include "entryloader.h"
#include "providerloader.h"
#include "installation.h"
#include "security.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kcodecs.h>
#include <kprocess.h>
#include <kshell.h>

#include <kio/job.h>
#include <krandom.h>
#include <ktar.h>
#include <kzip.h>

#include <QtCore/QDir>
#include <QtXml/qdom.h>
#include <QtCore/Q_PID>

using namespace KNS;

CoreEngine::CoreEngine(QObject* parent)
    : QObject(parent)
{
    m_initialized = false;
    m_cachepolicy = CacheNever;
    m_automationpolicy = AutomationOff;

    m_uploadedentry = NULL;
    m_uploadprovider = NULL;

    m_installation = NULL;

    m_activefeeds = 0;
}

CoreEngine::~CoreEngine()
{
	shutdown();
}

bool CoreEngine::init(const QString &configfile)
{
	kDebug(550) << "Initializing KNS::CoreEngine from '" << configfile << "'";

	KConfig conf(configfile);
	if(conf.accessMode() == KConfig::NoAccess)
	{
		kError(550) << "No knsrc file named '" << configfile << "' was found." << endl;
		return false;
	}
	// FIXME: accessMode() doesn't return NoAccess for non-existing files
	// - bug in kdecore?
	// - this needs to be looked at again until KConfig backend changes for KDE 4
	// the check below is a workaround
	if(KStandardDirs::locate("config", configfile).isEmpty())
	{
		kError(550) << "No knsrc file named '" << configfile << "' was found." << endl;
		return false;
	}

	if(!conf.hasGroup("KNewStuff2"))
	{
		kError(550) << "A knsrc file was found but it doesn't contain a KNewStuff2 section." << endl;
		return false;
	}

	KConfigGroup group = conf.group("KNewStuff2");
	m_providersurl = group.readEntry("ProvidersUrl", QString());
	//m_componentname = group.readEntry("ComponentName", QString());
	m_componentname = configfile.section(".", 0, 0);

	// FIXME: add support for several categories later on
	// FIXME: read out only when actually installing as a performance improvement?
	m_installation = new Installation();
	m_installation->setUncompression(group.readEntry("Uncompress", QString()));
	m_installation->setCommand(group.readEntry("InstallationCommand", QString()));
	m_installation->setStandardResourceDir(group.readEntry("StandardResource", QString()));
	m_installation->setTargetDir(group.readEntry("TargetDir", QString()));
	m_installation->setInstallPath(group.readEntry("InstallPath", QString()));
	m_installation->setCustomName(group.readEntry("CustomName", false));

	QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
	QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
	QString scope = group.readEntry("Scope", QString());
	if(!checksumpolicy.isEmpty())
	{
		if(checksumpolicy == "never")
			m_installation->setChecksumPolicy(Installation::CheckNever);
		else if(checksumpolicy == "ifpossible")
			m_installation->setChecksumPolicy(Installation::CheckIfPossible);
		else if(checksumpolicy == "always")
			m_installation->setChecksumPolicy(Installation::CheckAlways);
		else
		{
			kError(550) << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
			return false;
		}
	}
	if(!signaturepolicy.isEmpty())
	{
		if(signaturepolicy == "never")
			m_installation->setSignaturePolicy(Installation::CheckNever);
		else if(signaturepolicy == "ifpossible")
			m_installation->setSignaturePolicy(Installation::CheckIfPossible);
		else if(signaturepolicy == "always")
			m_installation->setSignaturePolicy(Installation::CheckAlways);
		else
		{
			kError(550) << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
			return false;
		}
	}
	if(!scope.isEmpty())
	{
		if(scope == "user")
			m_installation->setScope(Installation::ScopeUser);
		else if(scope == "system")
			m_installation->setScope(Installation::ScopeSystem);
		else
		{
			kError(550) << "The scope '" + scope + "' is unknown." << endl;
			return false;
		}

		if(m_installation->scope() == Installation::ScopeSystem)
		{
			if(!m_installation->installPath().isEmpty())
			{
				kError(550) << "System installation cannot be mixed with InstallPath." << endl;
				return false;
			}
		}
	}

	m_initialized = true;

	return true;
}

void CoreEngine::start()
{
	if(!m_initialized)
	{
		kError(550) << "Must call KNS::CoreEngine::init() first." << endl;
		return;
	}

	if(m_cachepolicy != CacheNever)
	{
		loadProvidersCache();
#if 0
		loadEntriesCache();
#endif
	}

	loadRegistry();

	// FIXME: also return if CacheResident and its conditions fulfilled
	if(m_cachepolicy == CacheOnly)
	{
		emit signalEntriesFinished();
		return;
	}

	ProviderLoader *provider_loader = new ProviderLoader(this);
	provider_loader->load(m_providersurl);

	connect(provider_loader,
		SIGNAL(signalProvidersLoaded(KNS::Provider::List)),
		SLOT(slotProvidersLoaded(KNS::Provider::List)));
	connect(provider_loader,
		SIGNAL(signalProvidersFailed()),
		SLOT(slotProvidersFailed()));
}

void CoreEngine::loadEntries(Provider *provider)
{
	if(m_cachepolicy == CacheOnly)
	{
		return;
	}

	QStringList feeds = provider->feeds();
	for(int i = 0; i < feeds.count(); i++)
	{
		Feed *feed = provider->downloadUrlFeed(feeds.at(i));
		if(feed)
		{
			EntryLoader *entry_loader = new EntryLoader(this);
			entry_loader->load(provider, feed);

			connect(entry_loader,
				SIGNAL(signalEntriesLoaded(KNS::Entry::List)),
				SLOT(slotEntriesLoaded(KNS::Entry::List)));
			connect(entry_loader,
				SIGNAL(signalEntriesFailed()),
				SLOT(slotEntriesFailed()));

			m_activefeeds++;
		}
	}
}

void CoreEngine::downloadPreview(Entry *entry)
{
	if(m_previewfiles.contains(entry))
	{
		// FIXME: ensure somewhere else that preview file even exists
		kDebug(550) << "Reusing preview from '" << m_previewfiles[entry] << "'";
		emit signalPreviewLoaded(KUrl::fromPath(m_previewfiles[entry]));
		return;
	}

	KUrl source = KUrl(entry->preview().representation());

	if(!source.isValid())
	{
		kError(550) << "The entry doesn't have a preview." << endl;
		return;
	}

	KUrl destination = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10);
	kDebug(550) << "Downloading preview '" << source << "' to '" << destination << "'";

	// FIXME: check for validity
	KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
	connect(job,
		SIGNAL(result(KJob*)),
		SLOT(slotPreviewResult(KJob*)));

	m_entry_jobs[job] = entry;
}

void CoreEngine::downloadPayload(Entry *entry)
{
	KUrl source = KUrl(entry->payload().representation());

	if(!source.isValid())
	{
		kError(550) << "The entry doesn't have a payload." << endl;
		return;
	}

	if(m_installation->isRemote())
	{
		// Remote resource
		kDebug(550) << "Relaying remote payload '" << source << "'";
		entry->setStatus(Entry::Installed);
		emit signalPayloadLoaded(source);
		// FIXME: we still need registration for eventual deletion
		return;
	}

	KUrl destination = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10);
	kDebug(550) << "Downloading payload '" << source << "' to '" << destination << "'";

	// FIXME: check for validity
	KIO::FileCopyJob *job = KIO::file_copy(source, destination, -1, KIO::Overwrite | KIO::HideProgressInfo);
	connect(job,
		SIGNAL(result(KJob*)),
		SLOT(slotPayloadResult(KJob*)));
	connect(job,
		SIGNAL(percent(KJob*, unsigned long)),
		SLOT(slotPayloadProgress(KJob*, unsigned long)));

	m_entry_jobs[job] = entry;
}

bool CoreEngine::uploadEntry(Provider *provider, Entry *entry)
{
	kDebug(550) << "Uploading " << entry->name().representation() << "...";

	if(m_uploadedentry)
	{
		kError(550) << "Another upload is in progress!" << endl;
		return false;
	}

	if(!provider->uploadUrl().isValid())
	{
		kError(550) << "The provider doesn't support uploads." << endl;
		return false;

		// FIXME: support for <noupload> will go here (file bundle creation etc.)
	}

	// FIXME: validate files etc.

	m_uploadedentry = entry;

	KUrl sourcepayload = KUrl(entry->payload().representation());
	KUrl destfolder = provider->uploadUrl();

	destfolder.setFileName(sourcepayload.fileName());

	KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepayload, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
	connect(fcjob,
		SIGNAL(result(KJob*)),
		SLOT(slotUploadPayloadResult(KJob*)));

	return true;
}

void CoreEngine::slotProvidersLoaded(KNS::Provider::List list)
{
	ProviderLoader *loader = dynamic_cast<ProviderLoader*>(sender());
	delete loader;

	mergeProviders(list);
}

void CoreEngine::slotProvidersFailed()
{
	ProviderLoader *loader = dynamic_cast<ProviderLoader*>(sender());
	delete loader;

	emit signalProvidersFailed();
}

void CoreEngine::slotEntriesLoaded(KNS::Entry::List list)
{
    EntryLoader *loader = dynamic_cast<EntryLoader*>(sender());
    if (!loader) return;
    const Provider *provider = loader->provider();
    const Feed *feed = loader->feed();
    delete loader;
    m_activefeeds--;

    kDebug(550) << "Provider source " << provider->name().representation();
    kDebug(550) << "Feed source " << feed->name().representation();

    mergeEntries(list, feed, provider);
}

void CoreEngine::slotEntriesFailed()
{
	EntryLoader *loader = dynamic_cast<EntryLoader*>(sender());
	delete loader;
	m_activefeeds--;

	emit signalEntriesFailed();
}

void CoreEngine::slotPayloadProgress(KJob *job, unsigned long percent)
{
	emit signalPayloadProgress((qobject_cast<KIO::FileCopyJob*>(job))->srcUrl(), percent);
}

void CoreEngine::slotPayloadResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot load payload file." << endl;
		kError(550) << job->errorString() << endl;

		m_entry_jobs.remove(job);
		emit signalPayloadFailed();
	}
	else
	{
		KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

		if(m_entry_jobs.contains(job))
		{
			// FIXME: this is only so exposing the KUrl suffices for downloaded entries
			Entry *entry = m_entry_jobs[job];
			entry->setStatus(Entry::Installed);
			m_entry_jobs.remove(job);
			m_payloadfiles[entry] = fcjob->destUrl().path();
		}
		// FIXME: ignore if not? shouldn't happen...

		emit signalPayloadLoaded(fcjob->destUrl());
	}
}

// FIXME: this should be handled more internally to return a (cached) preview image
void CoreEngine::slotPreviewResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot load preview file." << endl;
		kError(550) << job->errorString() << endl;

		m_entry_jobs.remove(job);
		emit signalPreviewFailed();
	}
	else
	{
		KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);

		if(m_entry_jobs.contains(job))
		{
			// now, assign temporary filename to entry and update entry cache
			Entry *entry = m_entry_jobs[job];
			m_entry_jobs.remove(job);
			m_previewfiles[entry] = fcjob->destUrl().path();
			cacheEntry(entry);
		}
		// FIXME: ignore if not? shouldn't happen...

		emit signalPreviewLoaded(fcjob->destUrl());
	}
}

void CoreEngine::slotUploadPayloadResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot upload payload file." << endl;
		kError(550) << job->errorString() << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}

	if(m_uploadedentry->preview().isEmpty())
	{
		// FIXME: we abuse 'job' here for the shortcut if there's no preview
		slotUploadPreviewResult(job);
		return;
	}

	KUrl sourcepreview = KUrl(m_uploadedentry->preview().representation());
	KUrl destfolder = m_uploadprovider->uploadUrl();

	KIO::FileCopyJob *fcjob = KIO::file_copy(sourcepreview, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
	connect(fcjob,
		SIGNAL(result(KJob*)),
		SLOT(slotUploadPreviewResult(KJob*)));
}

void CoreEngine::slotUploadPreviewResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot upload preview file." << endl;
		kError(550) << job->errorString() << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}

	// FIXME: the following save code is also in cacheEntry()
	// when we upload, the entry should probably be cached!

	// FIXME: adhere to meta naming rules as discussed
	KUrl sourcemeta = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".meta";
	KUrl destfolder = m_uploadprovider->uploadUrl();

	EntryHandler eh(*m_uploadedentry);
	QDomElement exml = eh.entryXML();

	QFile f(sourcemeta.path());
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << sourcemeta << "'." << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}
	QTextStream metastream(&f);
	metastream << exml;
	f.close();

	KIO::FileCopyJob *fcjob = KIO::file_copy(sourcemeta, destfolder, -1, KIO::Overwrite | KIO::HideProgressInfo);
	connect(fcjob,
		SIGNAL(result(KJob*)),
		SLOT(slotUploadMetaResult(KJob*)));
}

void CoreEngine::slotUploadMetaResult(KJob *job)
{
	if(job->error())
	{
		kError(550) << "Cannot upload meta file." << endl;
		kError(550) << job->errorString() << endl;

		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		emit signalEntryFailed();
		return;
	}
	else
	{
		m_uploadedentry = NULL;
		m_uploadprovider = NULL;

		//KIO::FileCopyJob *fcjob = static_cast<KIO::FileCopyJob*>(job);
		emit signalEntryUploaded();
	}
}

void CoreEngine::loadRegistry()
{
	KStandardDirs d;

	kDebug(550) << "Loading registry in all directories named 'knewstuff2-entries.registry'.";

	// this must be same as in registerEntry()
	QStringList dirs = d.findDirs("data", "knewstuff2-entries.registry");
	for(QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it)
	{
		kDebug(550) << " + Load from directory '" + (*it) + "'.";
		QDir dir((*it));
		QStringList files = dir.entryList(QDir::Files | QDir::Readable);
		for(QStringList::iterator fit = files.begin(); fit != files.end(); ++fit)
		{
			QString filepath = (*it) + '/' + (*fit);
			kDebug(550) << "  + Load from file '" + filepath + "'.";

			bool ret;
			QFile f(filepath);
			ret = f.open(QIODevice::ReadOnly);
			if(!ret)
			{
				kWarning(550) << "The file could not be opened.";
				continue;
			}

			QDomDocument doc;
			ret = doc.setContent(&f);
			if(!ret)
			{
				kWarning(550) << "The file could not be parsed.";
				continue;
			}

			QDomElement root = doc.documentElement();
			if(root.tagName() != "ghnsinstall")
			{
				kWarning(550) << "The file doesn't seem to be of interest.";
				continue;
			}

			QDomElement stuff = root.firstChildElement("stuff");
			if(stuff.isNull())
			{
				kWarning(550) << "Missing GHNS installation metadata.";
				continue;
			}

			EntryHandler handler(stuff);
			if(!handler.isValid())
			{
				kWarning(550) << "Invalid GHNS installation metadata.";
				continue;
			}

			Entry *e = handler.entryptr();
			e->setStatus(Entry::Installed);
			m_entry_cache.append(e);
			m_entry_index[id(e)] = e;

			// FIXME: we must overwrite cache entries with registered entries
			// and not just append the latter ones
			// (m_entry_index is correct here but m_entry_cache not yet)
		}
	}
}

void CoreEngine::loadProvidersCache()
{
	KStandardDirs d;

	kDebug(550) << "Loading provider cache.";

	// FIXME: one file per ProvidersUrl - put this into the filename
	// FIXME: e.g. http_foo_providers.xml.cache?
	QString cachefile = d.findResource("cache", "knewstuff2-providers.cache.xml");
	if(cachefile.isEmpty())
	{
		kDebug(550) << "Cache not present, skip loading.";
		return;
	}

	kDebug(550) << " + Load from file '" + cachefile + "'.";

	bool ret;
	QFile f(cachefile);
	ret = f.open(QIODevice::ReadOnly);
	if(!ret)
	{
		kWarning(550) << "The file could not be opened.";
		return;
	}

	QDomDocument doc;
	ret = doc.setContent(&f);
	if(!ret)
	{
		kWarning(550) << "The file could not be parsed.";
		return;
	}

	QDomElement root = doc.documentElement();
	if(root.tagName() != "ghnsproviders")
	{
		kWarning(550) << "The file doesn't seem to be of interest.";
		return;
	}

	QDomElement provider = root.firstChildElement("provider");
	if(provider.isNull())
	{
		kWarning(550) << "Missing provider entries in the cache.";
		return;
	}

	while(!provider.isNull())
	{
		ProviderHandler handler(provider);
		if(!handler.isValid())
		{
			kWarning(550) << "Invalid provider metadata.";
			continue;
		}

		Provider *p = handler.providerptr();
		m_provider_cache.append(p);
		m_provider_index[pid(p)] = p;

		emit signalProviderLoaded(p);

		loadFeedCache(p);

		if(m_automationpolicy == AutomationOn)
		{
			loadEntries(p);
		}

		provider = provider.nextSiblingElement("provider");
	}
}

void CoreEngine::loadFeedCache(Provider *provider)
{
	KStandardDirs d;

	kDebug(550) << "Loading feed cache.";

	QStringList cachedirs = d.findDirs("cache", "knewstuff2-feeds.cache");
	if(cachedirs.size() == 0)
	{
		kDebug(550) << "Cache directory not present, skip loading.";
		return;
	}
	QString cachedir = cachedirs.first();

	QStringList entrycachedirs = d.findDirs("cache", "knewstuff2-entries.cache/" + m_componentname);
	if(entrycachedirs.size() == 0)
	{
		kDebug(550) << "Cache directory not present, skip loading.";
		return;
	}
	QString entrycachedir = entrycachedirs.first();

	kDebug(550) << " + Load from directory '" + cachedir + "'.";

	QStringList feeds = provider->feeds();
	for(int i = 0; i < feeds.count(); i++)
	{
		Feed *feed = provider->downloadUrlFeed(feeds.at(i));
		QString feedname = feeds.at(i);

		// FIXME: hack because we saved with wrong name!
		feedname = "???";
		QString idbase64 = QString(pid(provider).toUtf8().toBase64() + '-' + feedname);
		QString cachefile = cachedir + '/' + idbase64 + ".xml";

		kDebug(550) << "  + Load from file '" + cachefile + "'.";

		bool ret;
		QFile f(cachefile);
		ret = f.open(QIODevice::ReadOnly);
		if(!ret)
		{
			kWarning(550) << "The file could not be opened.";
			return;
		}

		QDomDocument doc;
		ret = doc.setContent(&f);
		if(!ret)
		{
			kWarning(550) << "The file could not be parsed.";
			return;
		}

		QDomElement root = doc.documentElement();
		if(root.tagName() != "ghnsfeeds")
		{
			kWarning(550) << "The file doesn't seem to be of interest.";
			return;
		}

		QDomElement entryel = root.firstChildElement("entry-id");
		if(entryel.isNull())
		{
			kWarning(550) << "Missing entries in the cache.";
			return;
		}

		while(!entryel.isNull())
		{
			QString idbase64 = entryel.text();
			QString filepath = entrycachedir + '/' + idbase64 + ".meta";

			kDebug(550) << "   + Load entry from file '" + filepath + "'.";

			// FIXME: pass feed and make loadEntryCache return void for consistency?
			Entry *entry = loadEntryCache(filepath);
			if(entry)
			{
				feed->addEntry(entry);
				emit signalEntryLoaded(entry, feed, provider);
			}

			entryel = entryel.nextSiblingElement("entry-id");
		}
	}
}

KNS::Entry *CoreEngine::loadEntryCache(const QString& filepath)
{
	bool ret;
	QFile f(filepath);
	ret = f.open(QIODevice::ReadOnly);
	if(!ret)
	{
		kWarning(550) << "The file could not be opened.";
		return NULL;
	}

	QDomDocument doc;
	ret = doc.setContent(&f);
	if(!ret)
	{
		kWarning(550) << "The file could not be parsed.";
		return NULL;
	}

	QDomElement root = doc.documentElement();
	if(root.tagName() != "ghnscache")
	{
		kWarning(550) << "The file doesn't seem to be of interest.";
		return NULL;
	}

	QDomElement stuff = root.firstChildElement("stuff");
	if(stuff.isNull())
	{
		kWarning(550) << "Missing GHNS cache metadata.";
		return NULL;
	}

	EntryHandler handler(stuff);
	if(!handler.isValid())
	{
		kWarning(550) << "Invalid GHNS installation metadata.";
		return NULL;
	}

	Entry *e = handler.entryptr();
	e->setStatus(Entry::Downloadable);
	m_entry_cache.append(e);
	m_entry_index[id(e)] = e;

	if(root.hasAttribute("previewfile"))
	{
		m_previewfiles[e] = root.attribute("previewfile");
		// FIXME: check here for a [ -f previewfile ]
	}
	if(root.hasAttribute("payloadfile"))
	{
		m_payloadfiles[e] = root.attribute("payloadfile");
		// FIXME: check here for a [ -f payloadfile ]
	}

	return e;
}

// FIXME: not needed anymore?
#if 0
void CoreEngine::loadEntriesCache()
{
	KStandardDirs d;

	kDebug(550) << "Loading entry cache.";

	QStringList cachedirs = d.findDirs("cache", "knewstuff2-entries.cache/" + m_componentname);
	if(cachedirs.size() == 0)
	{
		kDebug(550) << "Cache directory not present, skip loading.";
		return;
	}
	QString cachedir = cachedirs.first();

	kDebug(550) << " + Load from directory '" + cachedir + "'.";

	QDir dir(cachedir);
	QStringList files = dir.entryList(QDir::Files | QDir::Readable);
	for(QStringList::iterator fit = files.begin(); fit != files.end(); ++fit)
	{
		QString filepath = cachedir + '/' + (*fit);
		kDebug(550) << "  + Load from file '" + filepath + "'.";

		Entry *e = loadEntryCache(filepath);

		if(e)
		{
			// FIXME: load provider/feed information first
			emit signalEntryLoaded(e, NULL, NULL);
		}
	}
}
#endif

void CoreEngine::shutdown()
{
	m_entry_index.clear();
	m_provider_index.clear();

	qDeleteAll(m_entry_cache);
	qDeleteAll(m_provider_cache);

	m_entry_cache.clear();
	m_provider_cache.clear();

	delete m_installation;
}

bool CoreEngine::providerCached(Provider *provider)
{
	if(m_cachepolicy == CacheNever) return false;

	if(m_provider_index.contains(pid(provider)))
		return true;
	return false;
}

bool CoreEngine::providerChanged(Provider *oldprovider, Provider *provider)
{
	QStringList oldfeeds = oldprovider->feeds();
	QStringList feeds = provider->feeds();
	if(oldfeeds.count() != feeds.count())
		return true;
	for(int i = 0; i < feeds.count(); i++)
	{
		Feed *oldfeed = oldprovider->downloadUrlFeed(feeds.at(i));
		Feed *feed = provider->downloadUrlFeed(feeds.at(i));
		if(!oldfeed)
			return true;
		if(feed->feedUrl() != oldfeed->feedUrl())
			return true;
	}
	return false;
}

void CoreEngine::mergeProviders(Provider::List providers)
{
	for(Provider::List::Iterator it = providers.begin(); it != providers.end(); ++it)
	{
		Provider *p = (*it);

		if(providerCached(p))
		{
			kDebug(550) << "CACHE: hit provider " << p->name().representation();
			Provider *oldprovider = m_provider_index[pid(p)];
			if(providerChanged(oldprovider, p))
			{
				kDebug(550) << "CACHE: update provider";
				cacheProvider(p);
				emit signalProviderChanged(p);
				// FIXME: oldprovider can now be deleted, see entry hit case
			}
		}
		else
		{
			if(m_cachepolicy != CacheNever)
			{
				kDebug(550) << "CACHE: miss provider " << p->name().representation();
				cacheProvider(p);
			}
			emit signalProviderLoaded(p);

			if(m_automationpolicy == AutomationOn)
			{
				loadEntries(p);
			}
		}

		m_provider_cache.append(p);
		m_provider_index[pid(p)] = p;
	}

	emit signalProvidersFinished();
}

bool CoreEngine::entryCached(Entry *entry)
{
	if(m_cachepolicy == CacheNever) return false;

	// Direct cache lookup first
	// FIXME: probably better use URL (changes less frequently) and do iteration
	if(m_entry_index.contains(id(entry))) return true;

	// If entry wasn't found, either
	// - a translation was added which matches our locale better, or
	// - our locale preferences changed, or both.
	// In that case we've got to find the old name in the new entry,
	// since we assume that translations are always added but never removed.
	for(int i = 0; i < m_entry_cache.count(); i++)
	{
		Entry *oldentry = m_entry_cache.at(i);
		QString lang = id(oldentry).section(":", 0, 0);
		QString oldname = oldentry->name().translated(lang);
		QString name = entry->name().translated(lang);
		kDebug(550) << "CACHE: compare entry names " << oldname << "/" << name;
		if(name == oldname) return true;
	}

	return false;
}

bool CoreEngine::entryChanged(Entry *oldentry, Entry *entry)
{
	if((!oldentry) || (entry->releaseDate() > oldentry->releaseDate()))
		return true;
	return false;
}

void CoreEngine::mergeEntries(Entry::List entries, const Feed *feed, const Provider *provider)
{
	for(Entry::List::Iterator it = entries.begin(); it != entries.end(); ++it)
	{
		// TODO: find entry in entrycache, replace if needed
		// don't forget marking as 'updateable'
		Entry *e = (*it);
		// set it to Installed if it's in the registry
		if (m_entry_index.contains(id(e)))
		{
			e->setStatus(Entry::Installed);
			emit signalEntryLoaded(e, feed, provider);
		}
		else
		{
			e->setStatus(Entry::Downloadable);

			if(entryCached(e))
			{
				kDebug(550) << "CACHE: hit entry " << e->name().representation();
				// FIXME: separate version updates from server-side translation updates?
				Entry *oldentry = m_entry_index[id(e)];
				if(entryChanged(oldentry, e))
				{
					kDebug(550) << "CACHE: update entry";
					e->setStatus(Entry::Updateable);
					// entry has changed
					// FIXME: important: for cache filename, whole-content comparison
					// is harmful, still needs id-based one!
					cacheEntry(e);
					emit signalEntryChanged(e);
					// FIXME: oldentry can now be deleted, but it's still in the list!
					// FIXME: better: assigne all values to 'e', keeps refs intact
				}
			}
			else
			{
				if(m_cachepolicy != CacheNever)
				{
					kDebug(550) << "CACHE: miss entry " << e->name().representation();
					cacheEntry(e);
				}
				emit signalEntryLoaded(e, feed, provider);
			}

			m_entry_cache.append(e);
			m_entry_index[id(e)] = e;
		}
	}

	if(m_cachepolicy != CacheNever)
	{
		// FIXME: feed name?
		cacheFeed(provider, "???", feed, entries);
	}

	emit signalEntriesFeedFinished(feed);
	if(m_activefeeds == 0)
	{
		emit signalEntriesFinished();
	}
}

void CoreEngine::cacheProvider(Provider *provider)
{
	KStandardDirs d;

	kDebug(550) << "Caching provider.";

	QString cachedir = d.saveLocation("cache");
	QString cachefile = cachedir + "knewstuff2-providers.cache.xml";

	kDebug(550) << " + Save to file '" + cachefile + "'.";

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnsproviders");

	for(Provider::List::Iterator it = m_provider_cache.begin(); it != m_provider_cache.end(); ++it)
	{
		Provider *p = (*it);
		ProviderHandler ph(*p);
		QDomElement pxml = ph.providerXML();
		root.appendChild(pxml);
	}
	ProviderHandler ph(*provider);
	QDomElement pxml = ph.providerXML();
	root.appendChild(pxml);

	QFile f(cachefile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << cachedir << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();

	/*QStringList feeds = p->feeds();
	for(int i = 0; i < feeds.count(); i++)
	{
		Feed *feed = p->downloadUrlFeed(feeds.at(i));
		cacheFeed(p, feeds.at(i), feed);
	}*/
}

void CoreEngine::cacheFeed(const Provider *provider, QString feedname, const Feed *feed, Entry::List entries)
{
	KStandardDirs d;

	Q_UNUSED(feed);

	kDebug(550) << "Caching feed.";

	QString cachedir = d.saveLocation("cache", "knewstuff2-feeds.cache");

	kDebug(550) << " + Save to directory '" + cachedir + "'.";

	QString idbase64 = QString(pid(provider).toUtf8().toBase64() + '-' + feedname);
	QString cachefile = idbase64 + ".xml";

	kDebug(550) << " + Save to file '" + cachefile + "'.";

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnsfeeds");
	for(int i = 0; i < entries.count(); i++)
	{
		QString idbase64 = id(entries.at(i)).toUtf8().toBase64();
		QDomElement entryel = doc.createElement("entry-id");
		root.appendChild(entryel);
		QDomText entrytext = doc.createTextNode(idbase64);
		entryel.appendChild(entrytext);
	}

	QFile f(cachedir + cachefile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << cachedir + cachefile << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();
}

void CoreEngine::cacheEntry(Entry *entry)
{
	KStandardDirs d;

	kDebug(550) << "Caching entry.";

	QString cachedir = d.saveLocation("cache", "knewstuff2-entries.cache/" + m_componentname);

	kDebug(550) << " + Save to directory '" + cachedir + "'.";

	//QString cachefile = KRandom::randomString(10) + ".meta";
	//FIXME: this must be deterministic, but it could also be an OOB random string
	//which gets stored into <ghnscache> just like preview...
	QString idbase64 = QString(id(entry).toUtf8().toBase64());
	QString cachefile = idbase64 + ".meta";

	kDebug(550) << " + Save to file '" + cachefile + "'.";

	// FIXME: adhere to meta naming rules as discussed
	// FIXME: maybe related filename to base64-encoded id(), or the reverse?

	EntryHandler eh(*entry);
	QDomElement exml = eh.entryXML();

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnscache");
	root.appendChild(exml);

	if(m_previewfiles.contains(entry))
	{
		root.setAttribute("previewfile", m_previewfiles[entry]);
	}
	/*if(m_payloadfiles.contains(entry))
	{
		root.setAttribute("payloadfile", m_payloadfiles[entry]);
	}*/

	QFile f(cachedir + cachefile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << cachedir + cachefile << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();
}

void CoreEngine::registerEntry(Entry *entry)
{
	KStandardDirs d;

	kDebug(550) << "Registering entry.";

	// NOTE: this directory must match loadRegistry
	QString registrydir = d.saveLocation("data", "knewstuff2-entries.registry");

	kDebug(550) << " + Save to directory '" + registrydir + "'.";

	// FIXME: see cacheEntry() for naming-related discussion
	QString registryfile = QString(id(entry).toUtf8().toBase64()) + ".meta";

	kDebug(550) << " + Save to file '" + registryfile + "'.";

	EntryHandler eh(*entry);
	QDomElement exml = eh.entryXML();

	QDomDocument doc;
	QDomElement root = doc.createElement("ghnsinstall");
	root.appendChild(exml);

	if(m_payloadfiles.contains(entry))
	{
		root.setAttribute("payloadfile", m_payloadfiles[entry]);
	}

	QFile f(registrydir + registryfile);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		kError(550) << "Cannot write meta information to '" << registrydir + registryfile << "'." << endl;
		// FIXME: ignore?
		return;
	}
	QTextStream metastream(&f);
	metastream << root;
	f.close();
}

QString CoreEngine::id(Entry *e)
{
	// This is the primary key of an entry:
	// A lookup on the name, which must exist but might be translated
	// This requires some care for comparison since translations might be added
	return e->name().language() + ':' + e->name().representation();
}

QString CoreEngine::pid(const Provider *p)
{
	// This is the primary key of a provider:
	// The download URL, which is never translated
	// If no download URL exists, a feed or web service URL must exist
	//if(p->downloadUrl().isValid())
	//	return p->downloadUrl().url();
	QStringList feeds = p->feeds();
	for(int i = 0; i < feeds.count(); i++)
	{
		QString feedtype = feeds.at(i);
		Feed *f = p->downloadUrlFeed(feedtype);
		if(f->feedUrl().isValid())
			return f->feedUrl().url();
	}
	if(p->webService().isValid())
		return p->webService().url();
	return QString();
}

bool CoreEngine::install(const QString &payloadfile)
{
	QList<Entry*> entries = m_payloadfiles.keys(payloadfile);
	if(entries.size() != 1)
	{
		// FIXME: shouldn't ever happen - make this an assertion?
		kError(550) << "ASSERT: payloadfile is not associated" << endl;
		return false;
	}

	Entry *entry = entries.first();

	// FIXME: first of all, do the security stuff here
	// this means check sum comparison and signature verification
	// signature verification might take a long time - make async?!

	if(m_installation->checksumPolicy() != Installation::CheckNever)
	{
		if(entry->checksum().isEmpty())
		{
			if(m_installation->checksumPolicy() == Installation::CheckIfPossible)
			{
				kDebug(550) << "Skip checksum verification";
			}
			else
			{
				kError(550) << "Checksum verification not possible" << endl;
				return false;
			}
		}
		else
		{
			kDebug(550) << "Verify checksum...";
		}
	}

	if(m_installation->signaturePolicy() != Installation::CheckNever)
	{
		if(entry->signature().isEmpty())
		{
			if(m_installation->signaturePolicy() == Installation::CheckIfPossible)
			{
				kDebug(550) << "Skip signature verification";
			}
			else
			{
				kError(550) << "Signature verification not possible" << endl;
				return false;
			}
		}
		else
		{
			kDebug(550) << "Verify signature...";
		}
	}

	kDebug(550) << "INSTALL resourceDir " << m_installation->standardResourceDir();
	kDebug(550) << "INSTALL targetDir " << m_installation->targetDir();
	kDebug(550) << "INSTALL installPath " << m_installation->installPath();
	kDebug(550) << "INSTALL + scope " << m_installation->scope();
	kDebug(550) << "INSTALL + customName" << m_installation->customName();
	kDebug(550) << "INSTALL + uncompression " << m_installation->uncompression();
	kDebug(550) << "INSTALL + command " << m_installation->command();

	// FIXME: make naming convention configurable through *.knsrc? e.g. for kde-look.org image names
	KUrl source = KUrl(entry->payload().representation());
	QString installfile;
	QString ext = source.fileName().section('.', -1);
	if(m_installation->customName())
	{
		installfile = entry->name().representation();
		installfile += '-' + entry->version();
		if(!ext.isEmpty()) installfile += '.' + ext;
	}
	else
	{
		installfile = source.fileName();
	}

	QString installpath, installdir;
	int pathcounter = 0;
	if(!m_installation->standardResourceDir().isEmpty())
	{
		if(m_installation->scope() == Installation::ScopeUser)
			installdir = KStandardDirs::locateLocal(m_installation->standardResourceDir().toUtf8(), "/");
		else
			installdir = KStandardDirs::locate(m_installation->standardResourceDir().toUtf8(), "/");
		pathcounter++;
	}
	if(!m_installation->targetDir().isEmpty())
	{
		if(m_installation->scope() == Installation::ScopeUser)
			installdir = KStandardDirs::locateLocal("data", m_installation->targetDir() + '/');
		else
			installdir = KStandardDirs::locate("data", m_installation->targetDir() + '/');
		pathcounter++;
	}
	if(!m_installation->installPath().isEmpty())
	{
		installdir = QDir::home().path() + '/' + m_installation->installPath() + '/';
		pathcounter++;
	}
	installpath = installdir + '/' + installfile;

	if(pathcounter != 1)
	{
		kError(550) << "Wrong number of installation directories given." << endl;
		return false;
	}

	kDebug(550) << "Install to file " << installpath;
	// FIXME: copy goes here (including overwrite checking)
	// FIXME: what must be done now is to update the cache *again*
	//        in order to set the new payload filename (on root tag only)
	//        - this might or might not need to take uncompression into account
	// FIXME: for updates, we might need to force an overwrite (that is, deleting before)
	QFile file(payloadfile);
	bool success = file.rename(installpath);
	if(!success)
	{
		kError(550) << "Cannot move file to destination" << endl;
		return false;
	}

	m_payloadfiles[entry] = installpath;
	registerEntry(entry);
	// FIXME: hm, do we need to update the cache really?
	// only registration is probably be needed here

	if(!m_installation->uncompression().isEmpty())
	{
		kDebug(550) << "Postinstallation: uncompress the file";

		// FIXME: check for overwriting, malicious archive entries (../foo) etc.
		// FIXME: KArchive should provide "safe mode" for this!
		// FIXME: value for uncompression was application/x-gzip etc. - and now?

		if(ext == "zip")
		{
			KZip zip(installpath);
			bool success = zip.open(QIODevice::ReadOnly);
			if(!success)
			{
				kError(550) << "Cannot open archive file '" << installpath << "'" << endl;
				return false;
			}
			const KArchiveDirectory *dir = zip.directory();
			dir->copyTo(installdir);
			zip.close();
			QFile::remove(installpath);
		}
		else if((ext == "tar") || (ext == "gz") || (ext == "bz2"))
		{
			KTar tar(installpath);
			bool success = tar.open(QIODevice::ReadOnly);
			if(!success)
			{
				kError(550) << "Cannot open archive file '" << installpath << "'" << endl;
				return false;
			}
			const KArchiveDirectory *dir = tar.directory();
			dir->copyTo(installdir);
			tar.close();
			QFile::remove(installpath);
		}
		else
		{
			kError(550) << "Unknown uncompression method " << ext << endl;
			return false;
		}
	}

	if(!m_installation->command().isEmpty())
	{
		KProcess process;
		QString command(m_installation->command());
		QString fileArg(KShell::quoteArg(installpath));
		command.replace("%f", fileArg);

		kDebug(550) << "Postinstallation: execute command";
		kDebug(550) << "Command is: " << command;

		process.setShellCommand(command);
		int exitcode = process.execute();

		if(exitcode)
		{
			kError(550) << "Command failed" << endl;
		}
		else
		{
			kDebug(550) << "Command executed successfully";
		}
	}

	// ==== FIXME: security code below must go above, when async handling is complete ====

	// FIXME: security object lifecycle - it is a singleton!
	Security *sec = Security::ref();

	connect(sec,
		SIGNAL(validityResult(int)),
		SLOT(slotInstallationVerification(int)));

	// FIXME: change to accept filename + signature
	sec->checkValidity(QString());

	return true;
}

bool CoreEngine::uninstall(KNS::Entry *entry)
{
	entry->setStatus(Entry::Deleted);
	// FIXME: remove payload file, and maybe unpacked files
	// FIXME: also remove registry for this file?
	return true;
}

void CoreEngine::slotInstallationVerification(int result)
{
	kDebug(550) << "SECURITY result " << result;

	if(result & Security::SIGNED_OK)
		emit signalInstallationFinished();
	else
		emit signalInstallationFailed();
}

void CoreEngine::setAutomationPolicy(AutomationPolicy policy)
{
	m_automationpolicy = policy;
}

void CoreEngine::setCachePolicy(CachePolicy policy)
{
	m_cachepolicy = policy;
}

#include "coreengine.moc"
