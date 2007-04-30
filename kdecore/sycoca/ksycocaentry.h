/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#ifndef KSYCOCAENTRY_H
#define KSYCOCAENTRY_H

#include <kdecore_export.h>
#include <ksycocatype.h>
#include <ksharedptr.h>

#include <QtCore/QDataStream>
#include <QtCore/QList>

class QString;
class QStringList;

/**
 * Base class for all Sycoca entries.
 *
 * You can't create an instance of KSycocaEntry, but it provides
 * the common functionality for servicetypes and services.
 *
 * @internal
 * @see http://techbase.kde.org/Development/Architecture/KDE3/System_Configuration_Cache
 */
class KDECORE_EXPORT KSycocaEntry : public KShared
{

public:
   virtual ~KSycocaEntry();

   virtual bool isType(KSycocaType t) const;
   virtual KSycocaType sycocaType() const;

   typedef KSharedPtr<KSycocaEntry> Ptr;
   typedef QList<Ptr> List;

   /**
    * Default constructor
    */
   explicit KSycocaEntry(const QString &path);

   /**
    * Safe demarshalling functions.
    */
   static void read( QDataStream &s, QString &str );
   static void read( QDataStream &s, QStringList &list );

   /**
    * @internal
    * Restores itself from a stream.
    */
   KSycocaEntry( QDataStream &_str, int iOffset );

   /**
    * @return the name of this entry
    */
   virtual QString name() const = 0;

   /**
    * @return the path of this entry
    * The path can be absolute or relative.
    * The corresponding factory should know relative to what.
    */
   QString entryPath() const;

   /**
    * @return true if valid
    */
   virtual bool isValid() const = 0;

   /**
    * @return true if deleted
    */
   bool isDeleted() const;

   /**
    * Sets whether or not this service is deleted
    */
   void setDeleted( bool deleted );

   /**
    * @internal
    * @return the position of the entry in the sycoca file
    */
   int offset() const;

   /**
    * @internal
    * Save ourselves to the database. Don't forget to call the parent class
    * first if you override this function.
    */
   virtual void save(QDataStream &s);

   /**
    * @internal
    * Load ourselves from the database. Don't call the parent class!
    */
   virtual void load(QDataStream &) = 0;

protected:
   /** Virtual hook, used to add new "virtual" functions while maintaining
       binary compatibility. Unused in this class.
   */
   virtual void virtual_hook( int id, void* data );

private:
   Q_DISABLE_COPY(KSycocaEntry)

   class Private;
   Private* const d;
};

#endif
