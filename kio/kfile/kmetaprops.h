/* This file is part of the KDE libraries
 * Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#ifndef __KMETAPROPS_H__
#define __KMETAPROPS_H__
#include <kpropertiesdialog.h>

 
class KFileMetaInfoItem;

/**
 * 'MetaProps plugin
 * In this plugin you can modify meta information like id3 tags of mp3 files
 * @internal
 */
class KFileMetaPropsPlugin : public KPropsDlgPlugin
{
  Q_OBJECT
public:
  /**
   * Constructor
   */
  KFileMetaPropsPlugin( KPropertiesDialog *_props );
  virtual ~KFileMetaPropsPlugin();

  virtual void applyChanges();

  /**
   * Tests whether the file specified by _items has a 'MetaInfo' plugin.
   */
  static bool supports( KFileItemList _items );
  
signals:
    void changed();

private:
    void createLayout();

    QWidget* makeBoolWidget(const KFileMetaInfoItem& item, QWidget* parent);
    QWidget* makeIntWidget(const KFileMetaInfoItem& item, QWidget* parent,
                                                    QString& valClass);
    QWidget* makeStringWidget(const KFileMetaInfoItem& item, QWidget* parent,
                                                       QString& valClass);
    QWidget* makeDateTimeWidget(const KFileMetaInfoItem& item, QWidget* parent,
                                                     QString& valClass);

private slots:
    // Code disabled until the "Add" button is implemented
//    void slotAdd();
    
private:
    
  class KFileMetaPropsPluginPrivate;
  KFileMetaPropsPluginPrivate *d;
};

#endif
