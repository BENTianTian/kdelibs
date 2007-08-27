/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "kpluginfactory.h"
#include "kpluginfactory_p.h"


KPluginFactory::KPluginFactory(const char *componentName, const char *catalogName, QObject *parent)
    : QObject(parent), d_ptr(new KPluginFactoryPrivate)
{
    Q_D(KPluginFactory);
    d->q_ptr = this;
    d->componentData = KComponentData(componentName, catalogName);
}

KPluginFactory::KPluginFactory(const KAboutData *aboutData, QObject *parent)
    : QObject(parent), d_ptr(new KPluginFactoryPrivate)
{
    Q_D(KPluginFactory);
    d->q_ptr = this;
    d->aboutData = aboutData;
    d->componentData = KComponentData(d->aboutData);
}

KPluginFactory::KPluginFactory(QObject *parent)
    : QObject(parent), d_ptr(new KPluginFactoryPrivate())
{
    Q_D(KPluginFactory);
    d->q_ptr = this;
}

KPluginFactory::KPluginFactory(KPluginFactoryPrivate &d, QObject *parent)
    : QObject(parent), d_ptr(&d)
{
}

KPluginFactory::~KPluginFactory()
{
    Q_D(KPluginFactory);

    if (d->catalogInitialized && d->componentData.isValid()) {
        KGlobal::locale()->removeCatalog(d->componentData.catalogName());
    }

    delete d_ptr;
}

KComponentData KPluginFactory::componentData() const
{
    Q_D(const KPluginFactory);
    return d->componentData;
}

void KPluginFactory::registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction)
{
    Q_D(KPluginFactory);

    if (!metaObject) {
        kFatal(152) << "The plugin you are trying to register is missing the Q_OBJECT macro!";
    }

    // we allow different interfaces to be registered without keyword
    if (!keyword.isEmpty()) {
        if (d->createInstanceHash.contains(keyword)) {
            kFatal(152) << "A plugin with the keyword" << keyword << "was already registered. A keyword must be unique!";
        }
        d->createInstanceHash.insert(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    } else {
        QList<KPluginFactoryPrivate::Plugin> clashes(d->createInstanceHash.values(keyword));
        const QMetaObject *superClass = metaObject->superClass();
        if (superClass) {
            foreach (const KPluginFactoryPrivate::Plugin &plugin, clashes) {
                for (const QMetaObject *otherSuper = plugin.first->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                        kFatal(152) << "Two plugins with the same interface(" << superClass->className() << ") were registered. Use keywords to identify the plugins.";
                    }
                }
            }
        }
        foreach (const KPluginFactoryPrivate::Plugin &plugin, clashes) {
            superClass = plugin.first->superClass();
            if (superClass) {
                for (const QMetaObject *otherSuper = metaObject->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                        kFatal(152) << "Two plugins with the same interface(" << superClass->className() << ") were registered. Use keywords to identify the plugins.";
                    }
                }
            }
        }
        d->createInstanceHash.insertMulti(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    }
}

QObject *KPluginFactory::createObject(QObject *parent, const char *className, const QStringList &args)
{
    Q_UNUSED(parent);
    Q_UNUSED(className);
    Q_UNUSED(args);
    return 0;
}

QObject *KPluginFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    Q_D(KPluginFactory);

    QObject *obj = 0;

    if (!d->catalogInitialized) {
        d->catalogInitialized = true;
        setupTranslations();
    }

    if (keyword.isEmpty() && (obj = createObject(parent, iface, variantListToStringList(args)))) {
        objectCreated(obj);
        return obj;
    }

    QList<KPluginFactoryPrivate::Plugin> canidates(d->createInstanceHash.values(keyword));
    // for !keyword.isEmpty() canidates.count() is 0 or 1

    foreach (const KPluginFactoryPrivate::Plugin &plugin, canidates) {
        for (const QMetaObject *current = plugin.first; current; current = current->superClass()) {
            if (0 == qstrcmp(iface, current->className())) {
                if (obj) {
                    kFatal(152) << "ambigious interface requested from a DSO containing more than one plugin";
                }
                obj = plugin.second(parentWidget, parent, args);
                break;
            }
        }
    }

    if (obj) {
        emit objectCreated(obj);
    }
    return obj;
}

void KPluginFactory::setupTranslations()
{
    Q_D(KPluginFactory);

    if (!d->componentData.isValid())
        return;

    KGlobal::locale()->insertCatalog(d->componentData.catalogName());
}

void KPluginFactory::setComponentData(const KComponentData &kcd)
{
    Q_D(KPluginFactory);
    d->componentData = kcd;
}

QStringList KPluginFactory::variantListToStringList(const QVariantList &list)
{
    QVariantList copy(list);
    QStringList stringlist;

    while (!copy.isEmpty()) 
        stringlist << copy.takeFirst().toString();

    return stringlist;
}

QVariantList KPluginFactory::stringListToVariantList(const QStringList &list)
{
    QStringList copy(list);
    QVariantList variantlist;

    while (!copy.isEmpty()) 
        variantlist << QVariant(copy.takeFirst());

    return variantlist;
}

//#include "kpluginfactory.moc"
