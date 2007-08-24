/* This file is part of the KDE libraries
    Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>

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


#include "kautostart.h"

#include "kaboutdata.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "kdesktopfile.h"
#include "kstandarddirs.h"

#include <QtCore/QFile>
#include <QStringList>

class KAutostart::Private
{
    public:
        Private()
        {
            df = 0;
        }

        ~Private()
        {
            delete df;
        }

        QString name;
        KDesktopFile* df;
};

KAutostart::KAutostart(const QString& entryName,
           QObject* parent)
    : QObject(parent),
      d(new Private)
{
    KGlobal::dirs()->addResourceType("autostart", 0, "share/autostart");
    if (entryName.isEmpty())
    {
        // XXX sure that the mainComponent is available at this point?
        d->name = KGlobal::mainComponent().aboutData()->appName();
    }
    else
    {
        d->name = entryName;
    }

    if (d->name.right(8) != ".desktop")
    {
        d->name.append(".desktop");
    }

    d->df = new KDesktopFile( "autostart", d->name);
}

KAutostart::~KAutostart()
{
    delete d;
}

void KAutostart::setAutostarts(bool autostart)
{
    d->df->desktopGroup().writeEntry("Hidden", !autostart);
}

bool KAutostart::autostarts(const QString& environment,
                            Conditions check) const
{
    // check if this desktop file actually exists
    bool starts = d->df->hasGroup("Desktop Entry");

    // check the hidden field
    starts &= !d->df->desktopGroup().readEntry("Hidden", false);

    if (!environment.isEmpty())
    {
        starts &= (allowedEnvironments().indexOf(environment) != -1);
    }

    if (check == CheckCommand)
    {
        starts &= d->df->tryExec();
    }

    return starts;
}

QString KAutostart::command() const
{
    return d->df->desktopGroup().readEntry( "Exec", QString() );
}

void KAutostart::setCommand(const QString& command)
{
    return d->df->desktopGroup().writeEntry( "Exec", command );
}

QString KAutostart::visibleName() const
{
    return d->df->readName();
}

void KAutostart::setVisibleName(const QString& name)
{
    d->df->desktopGroup().writeEntry( "Name", name );
}

bool KAutostart::isServiceRegistered(const QString& entryName)
{
    return QFile::exists(KStandardDirs::locate("autostart", entryName + ".desktop"));
}

QString KAutostart::commandToCheck() const
{
    return d->df->desktopGroup().readPathEntry( "TryExec" );
}

void KAutostart::setCommandToCheck(const QString& exec)
{
    d->df->desktopGroup().writePathEntry( "TryExec", exec );
}

KAutostart::StartPhase KAutostart::startPhase() const
{
    int phase = d->df->desktopGroup().readEntry("X-KDE-autostart-phase",
                                                (int)KAutostart::Applications);
    if (phase < KAutostart::BaseDesktop ||
        phase > KAutostart::Applications)
    {
        phase = KAutostart::Applications;
    }

    return static_cast<StartPhase>(phase);
}

void KAutostart::setStartPhase(KAutostart::StartPhase phase)
{
    d->df->desktopGroup().writeEntry( "X-KDE-autostart-phase", (int)phase );
}

QStringList KAutostart::allowedEnvironments() const
{
    return d->df->desktopGroup().readEntry( "OnlyShowIn", QStringList(), ';' );
}

void KAutostart::setAllowedEnvironments(const QStringList& environments)
{
    d->df->desktopGroup().writeEntry( "OnlyShowIn", environments, ';' );
}

void KAutostart::addToAllowedEnvironments(const QString& environment)
{
    QStringList envs = allowedEnvironments();

    if (envs.contains(environment))
    {
        return;
    }

    envs.append(environment);
    d->df->desktopGroup().writeEntry( "OnlyShowIn", envs, ';' );
}

void KAutostart::removeFromAllowedEnvironments(const QString& environment)
{
    QStringList envs = allowedEnvironments();
    int index = envs.indexOf(environment);

    if (index < 0)
    {
        return;
    }

    envs.removeAt(index);
    d->df->desktopGroup().writeEntry("OnlyShowIn", envs, ';');
}

QStringList KAutostart::excludedEnvironments() const
{
    return d->df->desktopGroup().readEntry("NotShowIn", QStringList(), ';');
}

void KAutostart::setExcludedEnvironments(const QStringList& environments)
{
    d->df->desktopGroup().writeEntry("NotShowIn", environments, ';');
}

void KAutostart::addToExcludedEnvironments(const QString& environment)
{
    QStringList envs = excludedEnvironments();

    if (envs.contains(environment))
    {
        return;
    }

    envs.append(environment);
    setExcludedEnvironments(envs);
}

void KAutostart::removeFromExcludedEnvironments(const QString& environment)
{
    QStringList envs = excludedEnvironments();
    int index = envs.indexOf(environment);

    if (index < 0)
    {
        return;
    }

    envs.removeAt(index);
    setExcludedEnvironments(envs);
}

#include "kautostart.moc"
