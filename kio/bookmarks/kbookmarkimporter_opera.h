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

#ifndef __kbookmarkimporter_opera_h
#define __kbookmarkimporter_opera_h

#include <qdom.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <ksimpleconfig.h>

#include <kbookmarkimporter.h>

/**
 * A class for importing Opera bookmarks
 * @deprecated
 */
class KOperaBookmarkImporter : public QObject
{
    Q_OBJECT
public:
    KOperaBookmarkImporter( const QString & fileName ) : m_fileName(fileName) {}
    ~KOperaBookmarkImporter() {}

    void parseOperaBookmarks();

    // Usual place for Opera bookmarks
    static QString operaBookmarksFile();

signals:
    void newBookmark( const QString & text, const QCString & url, const QString & additionalInfo );
    void newFolder( const QString & text, bool open, const QString & additionalInfo );
    void newSeparator();
    void endFolder();

protected:
    QString m_fileName;

};

/**
 * A class for importing Opera bookmarks
 * @since 3.2
 */
class KOperaBookmarkImporterImpl : public KBookmarkImporterBase
{
public:
    KOperaBookmarkImporterImpl() { }
    virtual void parse();
    virtual QString findDefaultLocation(bool forSaving = false) const;
};

class KOperaBookmarkExporterImpl : public KBookmarkExporterBase
{
public:
    KOperaBookmarkExporterImpl(KBookmarkManager* mgr, const QString & filename)
      : KBookmarkExporterBase(mgr, filename) 
    { ; }
    virtual ~KOperaBookmarkExporterImpl() {}
    virtual void write(KBookmarkGroup);
};

#endif
