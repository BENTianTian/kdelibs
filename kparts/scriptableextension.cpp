/* This file is part of the KDE project
   Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>
   Copyright (C) 2002, 2004 Koos Vriezen <koos.vriezen@gmail.com>

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
#include "scriptableextension.h"
#include "scriptableextension_p.h"
#include <kglobal.h>
#include <QDBusMetaType>

namespace KParts {

struct ScriptableExtensionPrivate {
    ScriptableExtension* hostContext;

    ScriptableExtensionPrivate(): hostContext(0)
    {}
};

ScriptableExtension::ScriptableExtension(QObject* parent):
    QObject(parent), d(new ScriptableExtensionPrivate)
{
    registerDBusTypes();
}

ScriptableExtension::~ScriptableExtension()
{
    delete d;
}

ScriptableExtension* ScriptableExtension::childObject(QObject* obj)
{
    return KGlobal::findDirectChild<KParts::ScriptableExtension*>(obj);
}

ScriptableExtension* ScriptableExtension::adapterFromLiveConnect(QObject* parentObj,
                                                                 LiveConnectExtension* oldApi)
{
    return new ScriptableLiveConnectExtension(parentObj, oldApi);
}

void ScriptableExtension::setHost(ScriptableExtension* host)
{
    d->hostContext = host;
}

ScriptableExtension* ScriptableExtension::host() const
{
    return d->hostContext;
}

QVariant ScriptableExtension::rootObject()
{
    return QVariant::fromValue(Null());
}

QVariant ScriptableExtension::enclosingObject(KParts::ReadOnlyPart* childPart)
{
    Q_UNUSED(childPart);
    return QVariant::fromValue(Null());
}

static QVariant unimplemented()
{
    ScriptableExtension::Exception except(QString::fromLatin1("[unimplemented]"));
    return QVariant::fromValue(except);
}

QVariant ScriptableExtension::callAsFunction(ScriptableExtension* callerPrincipal,
                                             quint64 objId, const ArgList& args)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(args);
    return unimplemented();
}

QVariant ScriptableExtension::callAsConstructor(ScriptableExtension* callerPrincipal,
                                                quint64 objId, const ArgList& args)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(args);
    return unimplemented();
}

bool ScriptableExtension::hasProperty(ScriptableExtension* callerPrincipal,
                                      quint64 objId, const QString& propName)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    return false;
}

QVariant ScriptableExtension::get(ScriptableExtension* callerPrincipal,
                                  quint64 objId, const QString& propName)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    return unimplemented();
}

bool ScriptableExtension::put(ScriptableExtension* callerPrincipal, quint64 objId,
                              const QString& propName, const QVariant& value)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    Q_UNUSED(value);
    return false;
}

bool ScriptableExtension::removeProperty(ScriptableExtension* callerPrincipal,
                                         quint64 objId, const QString& propName)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    return false;
}

bool ScriptableExtension::enumerateProperties(ScriptableExtension* callerPrincipal,
                                              quint64 objId, QStringList* result)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(result);
    return false;
}

bool ScriptableExtension::setException(ScriptableExtension* callerPrincipal,
                                       const QString& message)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(message);
    return false;
}

QVariant ScriptableExtension::evaluateScript(ScriptableExtension* callerPrincipal,
                                            quint64 contextObjectId,
                                            const QString& code,
                                            const QString& language)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(contextObjectId);
    Q_UNUSED(code);
    Q_UNUSED(language);
    return unimplemented();
}

void ScriptableExtension::acquire(quint64 objId)
{
    Q_UNUSED(objId);
}

void ScriptableExtension::release(quint64 objId)
{
    Q_UNUSED(objId);
}

// LiveConnectExtension -> ScriptableExtension adapter. We use
// lc object IDs as our own object IDs.
// ----------------------------------------------------------------------------
QVariant ScriptableLiveConnectExtension::rootObject()
{
    // Plugin root is always LC object #0.
    return QVariant::fromValue(ScriptableExtension::Object(this, 0));
}

bool ScriptableLiveConnectExtension::hasProperty(ScriptableExtension*, quint64 objId, const QString& propName)
{
    QVariant val = get(0, objId, propName);
    return !val.canConvert<ScriptableExtension::Exception>();
}

// Note that since we wrap around a plugin, and do not implement the browser,
// we do not perform XSS checks ourselves.
QVariant ScriptableLiveConnectExtension::callAsFunction(ScriptableExtension*,
                        quint64 objId, const ScriptableExtension::ArgList& args)
{
    QStringList qargs;
    // Convert args to strings for LC use.
    for (int i = 0; i < args.size(); ++i) {
        bool ok;
        qargs.append(toLC(args[i], &ok));
        if (!ok)
            return unimplemented();
    }

    LiveConnectExtension::Type retType;
    unsigned long              retObjId;
    QString                    retVal;    
    if (wrapee->call((unsigned long)objId, objNames[objId], qargs,
                     retType, retObjId, retVal)) {
        return fromLC(QString(), retType, retObjId, retVal);
    } else {
        return unimplemented();
    }
}

QVariant ScriptableLiveConnectExtension::get(ScriptableExtension*,
                                             quint64 objId, const QString& propName)
{
    LiveConnectExtension::Type retType;
    unsigned long              retObjId;
    QString                    retVal;
    if (wrapee->get((unsigned long)objId, propName, retType, retObjId, retVal)) {
        return fromLC(propName, retType, retObjId, retVal);
    } else {
        // exception signals failure. ### inellegant
        return unimplemented();
    }
}

bool ScriptableLiveConnectExtension::put(ScriptableExtension*, quint64 objId,
                                         const QString& propName, const QVariant& value)
{
    bool ok;
    QString val = toLC(value, &ok);
    if (!ok)
        return false;

    return wrapee->put((unsigned long)objId, propName, val);
}

QVariant ScriptableLiveConnectExtension::fromLC(const QString& name,
                                                LiveConnectExtension::Type type,
                                                unsigned long objId,
                                                const QString& value)
{
    switch (type) {
    case KParts::LiveConnectExtension::TypeBool: {
        bool ok;
        int i = value.toInt(&ok);
        if (ok)
            return QVariant(bool(i));
        return QVariant(value.toLower() == QLatin1String("true"));
    }
    case KParts::LiveConnectExtension::TypeObject:
    case KParts::LiveConnectExtension::TypeFunction:
        // if we got an object, we also have to remeber its name, as we'll need it
        // for call
        if (!refCounts.contains(objId)) {
            refCounts[objId] = 0;
            objNames [objId] = name;
        }
        return QVariant::fromValue(ScriptableExtension::Object(this, objId));
    
    case KParts::LiveConnectExtension::TypeNumber:
        return QVariant(value.toDouble());

    case KParts::LiveConnectExtension::TypeString:
        return QVariant(value);

    case KParts::LiveConnectExtension::TypeVoid:
    default:
        return QVariant::fromValue(ScriptableExtension::Undefined());
    }
}

QString ScriptableLiveConnectExtension::toLC(const QVariant& in, bool* ok)
{
    *ok = true; // most of the time.

    // Objects (or exceptions) can't be converted
    if (in.canConvert<ScriptableExtension::Object>() ||
        in.canConvert<ScriptableExtension::Exception>()) {

        *ok = false;
        return QString();
    }

    // Convert null and undefined to appropriate strings
    // ### this matches old KHTML behavior, but is this sensible?
    if (in.canConvert<ScriptableExtension::Null>())
        return QString::fromLatin1("null");

    if (in.canConvert<ScriptableExtension::Undefined>())
        return QString::fromLatin1("undefined");

    if (in.type() == QVariant::Bool)
        return in.toBool() ? QString::fromLatin1("true") : QString::fromLatin1("false");

    // Just stringify everything else, makes sense forn umms as well.
    if (in.canConvert<QString>())
        return in.toString();

    // something really icky...
    *ok = false;
    return QString();
}

void ScriptableLiveConnectExtension::acquire(quint64 objId)
{
    ++refCounts[objId];
}

void ScriptableLiveConnectExtension::release(quint64 objId)
{
    int newRC = --refCounts[objId];
    if (!newRC) {
        wrapee->unregister((unsigned long)objId);
        refCounts.remove(objId);
        objNames.remove(objId);
    }
}

} // namespace KParts


// DBus stuff
// ----------------------------------------------------------------------------
void KParts::ScriptableExtension::registerDBusTypes()
{
    qDBusRegisterMetaType<Null>();
    qDBusRegisterMetaType<Undefined>();
    qDBusRegisterMetaType<Exception>();
}

const QDBusArgument& KPARTS_EXPORT operator<<(QDBusArgument& arg,
                                              const KParts::ScriptableExtension::Null&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator>>(const QDBusArgument& arg,
                                              KParts::ScriptableExtension::Null&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator<<(QDBusArgument& arg,
                                              const KParts::ScriptableExtension::Undefined&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator>>(const QDBusArgument& arg,
                                              KParts::ScriptableExtension::Undefined&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator<<(QDBusArgument& arg,
                                              const KParts::ScriptableExtension::Exception& e)
{
    arg.beginStructure();
    arg << e.message;
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator>>(const QDBusArgument& arg,
                                              KParts::ScriptableExtension::Exception& e)
{
    arg.beginStructure();
    arg >> e.message;
    arg.endStructure();
    return arg;
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
#include "scriptableextension.moc"
#include "scriptableextension_p.moc"