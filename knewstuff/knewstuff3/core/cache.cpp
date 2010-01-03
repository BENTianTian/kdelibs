/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cache.h"

#include <QtCore/QFile>
#include <kstandarddirs.h>
#include <kdebug.h>

using namespace KNS3;

Cache::Cache(QObject* parent): QObject(parent)
{
}

void Cache::setRegistryFileName(const QString& file)
{
    registryFile = KStandardDirs::locateLocal("data", "knewstuff3/" + file + ".knsregistry");
    kDebug() << "Using registry file: " << registryFile;
}

void Cache::readRegistry()
{
    QFile f(registryFile);
    if (!f.open(QIODevice::ReadOnly)) {
        kWarning() << "The file " << registryFile << " could not be opened.";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f)) {
        kWarning() << "The file could not be parsed.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "hotnewstuffregistry") {
        kWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    QDomElement stuff = root.firstChildElement("stuff");
    while (!stuff.isNull()) {
        EntryInternal e;
        e.setEntryXML(stuff);
        e.setSource(EntryInternal::Cache);
        cache.insert(e);
        stuff = stuff.nextSiblingElement("stuff");
    }
    /*
    if (root.hasAttribute("previewfile")) {
        d->previewfiles[e] = root.attribute("previewfile");
        // FIXME: check here for a [ -f previewfile ]
    }
    */

    if (root.hasAttribute("payloadfile")) {
        // FIXME d->payloadfiles[e] = root.attribute("payloadfile");
        // FIXME: check here for a [ -f payloadfile ]
    }

    kDebug() << "Cache read... entries: " << cache.size();
}


EntryInternal::List Cache::registryForProvider(const QString& providerId)
{
    EntryInternal::List entries;
    foreach (const EntryInternal& e, cache) {
        if (e.providerId() == providerId) {
            entries.append(e);
        }
    }
    return entries;
}


void Cache::writeRegistry()
{
    kDebug() << "Write registry";

    QFile f(registryFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        kWarning() << "Cannot write meta information to '" << registryFile << "'." << endl;
        return;
    }

    QDomDocument doc;
    QDomElement root = doc.createElement("hotnewstuffregistry");

    foreach (const EntryInternal& entry, cache) {
        // Write the entry, unless the policy is CacheNever and the entry is not installed.
        if (entry.status() == EntryInternal::Installed || entry.status() == EntryInternal::Updateable) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << root;
    f.close();
}

void Cache::registerChangedEntry(const KNS3::EntryInternal& entry)
{
    cache.insert(entry);
}

void Cache::insertRequest(const KNS3::Provider::SearchRequest& request, const KNS3::EntryInternal::List& entries)
{
    requestCache[request.hashForRequest()] = entries;
    kDebug() << request.hashForRequest() << " keys: " << requestCache.keys();
}

EntryInternal::List Cache::requestFromCache(const KNS3::Provider::SearchRequest& request)
{
    kDebug() << request.hashForRequest();
    return requestCache.value(request.hashForRequest());
}

#include "cache.moc"
