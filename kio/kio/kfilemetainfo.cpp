/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2007 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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
#include "kfilemetainfo_private.h"
#include <strigi/bufferedstream.h>
#include <strigi/indexwriter.h>
#include <strigi/analysisresult.h>
#include <strigi/fieldtypes.h>
#include "kurl.h"
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QStringList>

using namespace std;
using namespace Strigi;

class KFileMetaInfoGroupPrivate : public QSharedData {
public:
    QString name;
};
KFileMetaInfoGroup::~KFileMetaInfoGroup() {
}
KFileMetaInfoGroup::KFileMetaInfoGroup(KFileMetaInfoGroup const& g) {
    p = g.p;
}

QDataStream& operator >>(QDataStream& s, KFileMetaInfo& ) {
    return s;
}
QDataStream& operator <<(QDataStream& s, const KFileMetaInfo&) {
    return s;
}

class QIODeviceInputStream : public jstreams::BufferedInputStream<char> {
private:
    QIODevice& in;
    int32_t fillBuffer(char* start, int32_t space);
public:
    QIODeviceInputStream(QIODevice& i) :in(i) {}
};
int32_t
QIODeviceInputStream::fillBuffer(char* start, int32_t space) {
    if (!in.isOpen() || !in.isReadable()) return -1;
    // read into the buffer
    int32_t nwritten = in.read(start, space);
    // check the file stream status
    if (nwritten < 0) {
        error = "Could not read from QIODevice.";
        in.close();
        return -1;
    }
    if (nwritten == 0 || in.atEnd()) {
        in.close();
    }
    return nwritten;
}
class KMetaInfoWriter : public IndexWriter {
public:
    void startAnalysis(AnalysisResult*) {
    }
    void addText(const AnalysisResult*, const char* s, int32_t n) {
        // we do not store text as metainfo
        QString text = QString::fromUtf8(s, n);
        //qDebug() << text;
    }
    void addField(const AnalysisResult* idx, const RegisteredField* fieldname,
            const string& value) {
        if (idx->writerData()) {
            QString val = QString::fromUtf8(value.c_str(), value.size());
            addField(idx, fieldname, val);
        }
    }
    void addField(const AnalysisResult* idx, const RegisteredField* fieldname,
        const unsigned char* data, uint32_t size) {
        if (idx->writerData()) {
            QByteArray d((const char*)data, size);
            addField(idx, fieldname, QVariant(d));
        }
    }
    void addField(const AnalysisResult* idx, const RegisteredField* fieldname,
            uint32_t value) {
        if (idx->writerData()) {
            addField(idx, fieldname, QVariant(value));
        }
    }
    void addField(const AnalysisResult* idx, const RegisteredField* fieldname,
            int32_t value) {
        if (idx->writerData()) {
            addField(idx, fieldname, QVariant(value));
        }
    }
    void addField(const AnalysisResult* idx, const RegisteredField* fieldname,
            double value) {
        if (idx->writerData()) {
            addField(idx, fieldname, QVariant(value));
        }
    }
    void addField(const AnalysisResult* idx,
            const RegisteredField* fieldname, const QVariant& value) {
        QHash<QString, KFileMetaInfoItem>* info
            = static_cast<QHash<QString, KFileMetaInfoItem>*>(
            idx->writerData());
        if (info) {
            string name(fieldname->getKey());
            QString key = QString::fromUtf8(name.c_str(), name.size());
            QHash<QString, KFileMetaInfoItem>::iterator i = info->find(key);
            if (i == info->end()) {
                PredicateProperties pp(key);
                info->insert(key, KFileMetaInfoItem(pp, value, 0, true));
            } else {
                i.value().addValue(value);
            }
        }
    }
    void addTriplet(const std::string& subject,
        const std::string& predicate, const std::string& object) {}
    void finishAnalysis(const AnalysisResult*) {}
    void deleteEntries(const vector<string>&) {}
    void deleteAllEntries() {}
};
class KFileMetaInfoPrivate : public QSharedData {
public:
    QHash<QString, KFileMetaInfoItem> items;
    KUrl kurl;

    //Private() :QSharedData() {qDebug() <<"ok: " << this;}
    void init(QIODevice& stream, const KUrl& url, time_t mtime);
    void initWriters(QIODevice& /*file*/);
};
static const KFileMetaInfoItem nullitem;

//const KFileMetaInfoItem KFileMetaInfoPrivate::null;
void
KFileMetaInfoPrivate::init(QIODevice& stream, const KUrl& url, time_t mtime) {
    // get data from Strigi
    kurl = url;
    StreamAnalyzer& indexer = PredicatePropertyProvider::self()->indexer();
    KMetaInfoWriter writer;
    QIODeviceInputStream strigiStream(stream);
    qDebug() << "'" << url.url().toUtf8().data() << "'";
    AnalysisResult idx(url.url().toUtf8().data(), mtime, writer, indexer);

    idx.setWriterData(&items);
    indexer.analyze(idx, &strigiStream);

    // TODO: get data from Nepomuk
}
void
KFileMetaInfoPrivate::initWriters(QIODevice& file) {
    QStringList mimetypes;
    QHash<QString, KFileMetaInfoItem>::iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
        KFileWritePlugin *w = 
            KFileWriterProvider::self()->loadPlugin(i.key());
        i.value().p->writer = w;
        if (w) {
            w->canWrite(file, i.key());
        }
    }
}

KFileMetaInfo::KFileMetaInfo(const QString& path, const QString& /*mimetype*/,
        KFileMetaInfo::WhatFlags /*w*/) :p(new KFileMetaInfoPrivate()) {
    QFileInfo fileinfo(path);
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    KUrl u;
    u.setPath(path);
    p->init(file, u, fileinfo.lastModified().toTime_t());
    if (fileinfo.isWritable()) {
        p->initWriters(file);
    }
}
KFileMetaInfo::KFileMetaInfo(const KUrl& url) :p(new KFileMetaInfoPrivate()) {
    QFileInfo fileinfo(url.path());
    QFile file(url.path());
    file.open(QIODevice::ReadOnly);
    p->init(file, url, fileinfo.lastModified().toTime_t());
    if (fileinfo.isWritable()) {
        p->initWriters(file);
    }
}
KFileMetaInfo::KFileMetaInfo() :p(new KFileMetaInfoPrivate()) {
}
KFileMetaInfo::KFileMetaInfo(const KFileMetaInfo&) {
}
const KFileMetaInfo&
KFileMetaInfo::operator=(KFileMetaInfo const& kfmi) {
    p->items = kfmi.p->items;
    p->kurl = kfmi.p->kurl;
    return kfmi;
}
KFileMetaInfo::~KFileMetaInfo() {
}
bool
KFileMetaInfo::applyChanges() {
    // go through all editable fields and group them by writer
    QHash<KFileWritePlugin*, QVariantMap> data;
    QHash<QString, KFileMetaInfoItem>::const_iterator i;
    for (i = p->items.begin(); i != p->items.end(); ++i) {
        if (i.value().isModified()) {
            data[i.value().p->writer][i.key()] = i.value().value();
        }
    }

    // call the writers on the data they can write
    bool ok = true;
    QHash<KFileWritePlugin*, QVariantMap>::const_iterator j;
    QFile file(p->kurl.path());
    file.open(QIODevice::ReadOnly);
    for (j = data.begin(); j != data.end(); ++j) {
        ok &= j.key()->write(file, j.value());
    }
    return ok;
}
const KUrl&
KFileMetaInfo::url() const {
    return p->kurl;
}
const QHash<QString, KFileMetaInfoItem>&
KFileMetaInfo::items() const {
    return p->items;
}
const KFileMetaInfoItem&
KFileMetaInfo::item(const QString& key) const {
    QHash<QString, KFileMetaInfoItem>::const_iterator i = p->items.find(key);
    return (i == p->items.end()) ?nullitem :i.value();
}
QStringList
KFileMetaInfo::keys() const {
    return p->items.keys();
}
KFileMetaInfoItem&
KFileMetaInfo::item(const QString& key) {
    return p->items[key];
}
bool
KFileMetaInfo::isValid() const {
    return true;
}
QStringList KFileMetaInfo::preferredKeys() const { return QStringList(); }
QStringList KFileMetaInfo::supportedKeys() const { return QStringList(); }

KFileMetaInfoGroupList
KFileMetaInfo::preferredGroups() const {
    return KFileMetaInfoGroupList();
}
KFileMetaInfoItemList
KFileMetaInfoGroup::items() const {
    return KFileMetaInfoItemList();
}
const QString&
KFileMetaInfoGroup::name() const {
    return p->name;
}

