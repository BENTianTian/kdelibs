/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)
              (C) 1997,1998, 2000 Sven Radej (radej@kde.org)

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

#include <kstatusbar.h>
#include <kconfig.h>
#include <kglobal.h>

// $Id$

#define FONT_Y_DELTA 3
#define DEFAULT_BORDERWIDTH 0

KStatusBarLabel::KStatusBarLabel( const QString& text, int _id,
                                 KStatusBar *parent, const char *name) :
  QLabel( parent, name) 
{   
  id = _id;

  // Commented out - not needed? (sven)
  //int w, h;
  //QFontMetrics fm = fontMetrics();
  //w = fm.width( text )+8;
  //h = fm.height() + FONT_Y_DELTA;
  //resize( w, h );


  setText( text );

  // umm... Mosfet? Can you help here?
  
  // Warning: QStatusBar draws shaded rectangle around every item - which
  // IMHO is stupid.
  // So NoFrame|PLain is the best you get. the problem is that only in case of
  // StyledPanel|Something you get QFrame to call QStyle::drawPanel().
  
  setLineWidth  (0);
  setFrameStyle (QFrame::NoFrame | QFrame::Plain );
  
  setAlignment( AlignHCenter | AlignVCenter );

  connect (this, SIGNAL(itemPressed(int)), parent, SLOT(slotPressed(int)));
  connect (this, SIGNAL(itemReleased(int)), parent, SLOT(slotReleased(int)));
}

void KStatusBarLabel::mousePressEvent (QMouseEvent *)
{
  emit itemPressed (id);
}

void KStatusBarLabel::mouseReleaseEvent (QMouseEvent *)
{
  emit itemReleased (id);
}


KStatusBar::KStatusBar( QWidget *parent, const char *name )
  : QStatusBar( parent, name )
{
  // Don claims that this causes segfaults, because QStatusBar deletes its child
  // objects. So I won�t delete it here. (sven)

  // items.setAutoDelete(true);
  items.setAutoDelete(false);

  // make the size grip stuff configurable
  // ...but off by default (sven)
  KConfig *config = KGlobal::config();
  QString group(config->group());
  config->setGroup(QString::fromLatin1("StatusBar style"));
  bool grip_enabled = config->readBoolEntry(QString::fromLatin1("SizeGripEnabled"), false);
  setSizeGripEnabled(grip_enabled);
  config->setGroup(group);
}

KStatusBar::~KStatusBar ()
{
  // Umm... delete something;
  items.clear();
}

void KStatusBar::insertItem( const QString& text, int id, int stretch, bool permanent)
{
  KStatusBarLabel *l = new KStatusBarLabel (text, id, this);
  items.insert(id, l);
  addWidget (l, stretch, permanent);
}

void KStatusBar::removeItem (int id)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    removeWidget (l);
    items.remove(id);
    // reformat (); // needed? (sven)
  }
  else
    debug ("KStatusBar::removeItem: bad item id: %d", id);
}

void KStatusBar::changeItem( const QString& text, int id )
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    clear();
    l->setText(text);
    reformat();
  }
  else
    debug ("KStatusBar::changeItem: bad item id: %d", id);
}

void KStatusBar::setItemAlignment (int id, int align)
{
  KStatusBarLabel *l = items[id];
  if (l)
  {
    //clear();
    l->setAlignment(align);
    //reformat(); Not needed I, think (sven)
  }
  else
    debug ("KStatusBar::setItemAlignment: bad item id: %d", id);
}

void KStatusBar::setItemFixed(int id, int w)
{
   KStatusBarLabel *l = items[id];
  if (l)
  {
    //clear();
    if (w==-1)
      w=fontMetrics().boundingRect(l->text()).width()+3;

    l->setFixedSize(w, l->height());
    //reformat(); Not needed I, think (sven)
  }
  else
    debug ("KStatusBar::setItemFixed: bad item id: %d", id);
}

void KStatusBar::slotPressed(int _id)
{
  emit pressed(_id);
}

void KStatusBar::slotReleased(int _id)
{
  emit released(_id);
}


#include "kstatusbar.moc"

//Eh!!!

