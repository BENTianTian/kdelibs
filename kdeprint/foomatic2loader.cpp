/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2003 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "foomatic2loader.h"
#include "driver.h"

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QBuffer>
#include <QtCore/QStringList>
#include <kdebug.h>
#include <klocale.h>

void kdeprint_foomatic2scanner_init(QIODevice*);
void kdeprint_foomatic2scanner_terminate();

Foomatic2Loader::Foomatic2Loader()
{
}

Foomatic2Loader::~Foomatic2Loader()
{
}

bool Foomatic2Loader::read(QIODevice *d)
{
    bool result = true;
    m_foodata.clear();
    kdeprint_foomatic2scanner_init(d);
    if (kdeprint_foomatic2parse(this) != 0)
        result = false;
    kdeprint_foomatic2scanner_terminate();
    return result;
}

bool Foomatic2Loader::readFromFile(const QString& filename)
{
    QFile f(filename);
    m_foodata.clear();
    if (f.open(QIODevice::ReadOnly))
        return read(&f);
    return false;
}

bool Foomatic2Loader::readFromBuffer(const QString& buffer)
{
    QByteArray buf = buffer.toUtf8();
    QBuffer d(&buf);
    m_foodata.clear();
    if (d.open(QIODevice::ReadOnly))
        return read(&d);
    return false;
}

DrBase* Foomatic2Loader::createValue(const QString& name, const QMap<QString, QVariant>& m) const
{
    DrBase *choice = new DrBase;
    choice->setName(name);
    choice->set("text", m.operator[]("comment").toString());
    return choice;
}

DrBase* Foomatic2Loader::createOption(const QMap<QString, QVariant>& m) const
{
    QString type = m.operator[]("type").toString();
    DrBase *opt = NULL;
    if (type == "enum") {
        DrListOption *lopt = new DrListOption;
        QVariant a = m.operator[]("vals_byname");
        QMap<QString, QVariant>::ConstIterator it = a.toMap().begin();
        for (; it != a.toMap().end(); ++it) {
            if (it.value().type() != QVariant::Map)
                continue;
            DrBase *ch = createValue(it.key(), it.value().toMap());
            if (ch)
                lopt->addChoice(ch);
        }
        opt = lopt;
    } else if (type == "int" || type == "float") {
        if (type == "int")
            opt = new DrIntegerOption;
        else
            opt = new DrFloatOption;
        opt->set("minval", m.operator[]("min").toString());
        opt->set("maxval", m.operator[]("max").toString());
    } else if (type == "bool") {
        DrBooleanOption *bopt = new DrBooleanOption;
        DrBase *choice;
        // choice 1
        choice = new DrBase;
        choice->setName("0");
        choice->set("text", m.operator[]("name_false").toString());
        bopt->addChoice(choice);
        choice = new DrBase;
        choice->setName("1");
        choice->set("text", m.operator[]("name_true").toString());
        bopt->addChoice(choice);
        opt = bopt;
    } else if (type == "string") {
        opt = new DrStringOption;
    }
    if (opt) {
        opt->setName(m.operator[]("name").toString());
        opt->set("text", m.operator[]("comment").toString());
        QString defval = m.operator[]("default").toString();
        if (!defval.isEmpty()) {
            opt->setValueText(defval);
            opt->set("default", defval);
        }
    }
    return opt;
}

DrMain* Foomatic2Loader::buildDriver() const
{
    if (m_foodata.isEmpty())
        return NULL;

    QVariant v = m_foodata.value("VAR");
    if (!v.isNull() && v.type() == QVariant::Map) {
        DrMain *driver = new DrMain;
        QMap<QString, DrGroup*> groups;
        driver->set("manufacturer", v.toMap().value("make").toString());
        driver->set("model", v.toMap().value("model").toString());
        driver->set("matic_printer", v.toMap().value("id").toString());
        driver->set("matic_driver", v.toMap().value("driver").toString());
        driver->set("text", QString("%1 %2 (%3)").arg(driver->get("manufacturer")).arg(driver->get("model")).arg(driver->get("matic_driver")));
        if (m_foodata.contains("POSTPIPE"))
            driver->set("postpipe", m_foodata.value("POSTPIPE").toString());
        v = v.toMap().value("args");
        if (!v.isNull() && v.type() == QVariant::List) {
            QList<QVariant>::ConstIterator it = v.toList().begin();
            for (; it != v.toList().end(); ++it) {
                if ((*it).type() != QVariant::Map)
                    continue;
                DrBase *opt = createOption((*it).toMap());
                if (opt) {
                    QString group = DrGroup::groupForOption(opt->name());
                    DrGroup *grp = NULL;
                    if (!groups.contains(group)) {
                        grp = new DrGroup;
                        grp->set("text", group);
                        driver->addGroup(grp);
                        groups.insert(group, grp);
                    } else
                        grp = groups[ group ];
                    grp->addOption(opt);
                    if (opt->name() == "PageSize") {
                        // try to add the corresponding page sizes
                        QVariant choices = (*it).toMap().value("vals_byname");
                        QRegExp re("(\\d+) +(\\d+)");
                        if (choices.type() == QVariant::Map) {
                            QMap<QString, QVariant>::ConstIterator it = choices.toMap().begin();
                            for (; it != choices.toMap().end(); ++it) {
                                QString driverval = (*it).toMap().value("driverval").toString();
                                if (re.exactMatch(driverval)) {
                                    driver->addPageSize(new DrPageSize(it.key(), re.cap(1).toInt(), re.cap(2).toInt(), 36, 24, 36, 24));
                                }
                            }
                        }
                    }
                } else
                    kWarning(500) << "Failed to create option: " << (*it).toMap()[ "name" ].toString();
            }
        }
        return driver;
    }
    return NULL;
}

DrMain* Foomatic2Loader::modifyDriver(DrMain *driver) const
{
    if (!m_foodata.isEmpty()) {
        QList<DrBase*> optList;
        DrGroup *grp = NULL;

        QVariant V = m_foodata.value("VAR");
        if (!V.isNull() && V.type() == QVariant::Map) {
            QVariant v = V.toMap().value("args");
            if (!v.isNull() && v.type() == QVariant::List) {
                QList<QVariant>::ConstIterator it = v.toList().begin();
                for (; it != v.toList().end(); ++it) {
                    if ((*it).type() != QVariant::Map)
                        continue;
                    DrBase *opt = createOption((*it).toMap());
                    if (opt)
                        optList.append(opt);
                    else
                        kWarning(500) << "Failed to create option: " << (*it).toMap()[ "name" ].toString();
                }
            } else {
                v = V.toMap().value("args_byname");
                if (!v.isNull() && v.type() == QVariant::Map) {
                    QMap<QString, QVariant>::ConstIterator it = v.toMap().begin();
                    for (; it != v.toMap().end(); ++it) {
                        if ((*it).type() != QVariant::Map)
                            continue;
                        DrBase *opt = createOption((*it).toMap());
                        if (opt)
                            optList.append(opt);
                        else
                            kWarning(500) << "Failed to create option: " << (*it).toMap()[ "name" ].toString();
                    }
                }
            }
        }

        foreach(DrBase* opt, optList) {
            if (opt) {
                switch (opt->type()) {
                case DrBase::List:
                case DrBase::Boolean:
                    delete opt;
                    break;
                default: {
                    if (!grp) {
                        grp = new DrGroup;
                        grp->set("text", i18n("Adjustments"));
                        driver->addGroup(grp);
                    }
                    DrBase *oldOpt = driver->findOption(opt->name());
                    if (oldOpt && oldOpt->type() == DrBase::List) {
                        QStringList fixedvals;
                        foreach(DrBase* choice, static_cast<DrListOption*>(oldOpt)->choices())
                        fixedvals.append(choice->name());
                        opt->set("fixedvals", fixedvals.join("|"));
                    }
                    driver->removeOptionGlobally(opt->name());
                    grp->addOption(opt);
                    break;
                }
                }
            }
        }
    }
    return driver;
}

DrMain* Foomatic2Loader::loadDriver(const QString& filename)
{
    Foomatic2Loader loader;
    if (loader.readFromFile(filename))
        return loader.buildDriver();
    else
        return NULL;
}
