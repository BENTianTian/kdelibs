/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kparts_factory_h__
#define __kparts_factory_h__

#include <klibloader.h>

class QWidget;

namespace KParts
{

class Part;

/**
 * A generic factory object to create a Part. @see_also KLibFactory.
 */
class Factory : public KLibFactory
{
  Q_OBJECT
public:
  Factory( QObject *parent = 0, const char *name = 0 );
  virtual ~Factory();

    /**
     * reimplement this method in your implementation to create the Part.
     *
     * The QStrinList can be used to pass additional arguments to the part.
     * If the part needs additional arguments, it should take them as
     * name="value" pairs. This is the way additional arguments will get passed
     * to the part from eg. khtml. You can for example emebed the part into HTML
     * by using the following code:
     * <pre>
     *    &lt;object type="my_mimetype" data="url_to_my_data"&gt;
     *        &lt;param name="name1" value="value1"&gt;
     *        &lt;param name="name2" value="value2"&gt;
     *    &lt;/object&gt;
     * </pre>
     * This could result in a call to 
     * <pre>
     *     createPart( parentWidget, name, parentObject, parentName, "Kparts::Part",
     *                 QStrinList("name1=\"value1\"", "name2=\"value2\"") );
     * </pre>
     * 
     * @returns the newly created part.
     */
    virtual Part *createPart( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KParts::Part", const QStringList &args = QStringList() ) = 0;

  virtual QObject *create( QObject *parent = 0, const char *name = 0, const char *classname = "QObject", const QStringList &args = QStringList() );

};

};

#endif
