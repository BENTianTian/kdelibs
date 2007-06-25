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
#ifndef KNEWSTUFF2_INSTALLATION_H
#define KNEWSTUFF2_INSTALLATION_H

#include <knewstuff2/knewstuff_export.h>

#include <QtCore/QString>

namespace KNS {

// FIXME: maybe it's smarter to let this class perform the installation
// so we can reduce the size of KNS::CoreEngine a bit?

/**
 * @short KNewStuff entry installation.
 *
 * The installation class stores all information related to an entry's
 * installation. However, the installation itself is performed by the
 * engine.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT Installation
{
  public:
    /**
     * Constructor.
     */
    Installation();

    /**
     * Destructor.
     */
    ~Installation();

    enum Policy
    {
      CheckNever,
      CheckIfPossible,
      CheckAlways
    };

    enum Scope
    {
      ScopeUser,
      ScopeSystem
    };

    void setUncompression(const QString& uncompression);
    void setCommand(const QString& command);

    void setStandardResourceDir(const QString& dir);
    void setTargetDir(const QString& dir);
    void setInstallPath(const QString& dir);

    void setScope(Scope scope);
    void setChecksumPolicy(Policy policy);
    void setSignaturePolicy(Policy policy);

    void setCustomName(bool customname);

    QString uncompression() const;
    QString command() const;

    QString standardResourceDir() const;
    QString targetDir() const;
    QString installPath() const;
    bool isRemote();

    Policy checksumPolicy();
    Policy signaturePolicy();
    Scope scope();

    bool customName();

  private:
    QString m_command;
    QString m_uncompression;
    QString m_standardresourcedir;
    QString m_targetdir;
    QString m_installpath;
    Policy m_checksumpolicy;
    Policy m_signaturepolicy;
    Scope m_scope;
    bool m_customname;
    class InstallationPrivate *d;
};

}

#endif
