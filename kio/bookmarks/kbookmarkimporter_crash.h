/* This file is part of the KDE libraries
   Copyright (C) 2002 Alexander Kellett <lypanov@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kbookmarkimporter_crash_h
#define __kbookmarkimporter_crash_h

#include <qdom.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <ksimpleconfig.h>
#include <kdemacros.h>

#include "kbookmarkimporter.h"

/**
 * A class for importing all crash sessions as bookmarks
 * @deprecated
 */
class KDE_DEPRECATED KCrashBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KCrashBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KCrashBookmarkImporter() {}
    void parseCrashBookmarks( bool del = true );
    static QString crashBookmarksDir( );
    static QStringList getCrashLogs(); // EMPTY!
signals:
    void newBookmark( const QString & text, const QCString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();
protected:
    QString m_fileName;
    void parseCrashLog( QString filename, bool del ); // EMPTY!
};

/**
 * A class for importing all crash sessions as bookmarks
 * @since 3.2
 */
class KCrashBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KCrashBookmarkImporterImpl() : m_shouldDelete(false) { }
    void setShouldDelete(bool);
    virtual void parse();
    virtual QString findDefaultLocation(bool forSaving = false) const;
    static QStringList getCrashLogs();
private:
    bool m_shouldDelete;
    QMap<QString, QString> parseCrashLog_noemit( const QString & filename, bool del );
};

#endif
