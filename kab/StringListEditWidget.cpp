/* -*- C++ -*-
 * �Header�
 */

#include "StringListEditWidget.h" 
#include <qsize.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <kiconloader.h>
#include "debug.h"
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>

#ifndef BITMAPS_INCLUDED
#ifdef KDE_USE_FINAL
#include "arrow_left.xbm"
#include "arrow_right.xbm"
#endif
#include "arrow_up.xbm"
#include "arrow_down.xbm"
#include "trash_open.xbm"
#define BITMAPS_INCLUDED
#endif

StringListEditWidget::StringListEditWidget(QWidget* parent, const char* name)
  : StringListEditWidgetData(parent, name)
{
  // ############################################################################
  // ----- load bitmaps:
  QBitmap up(16, 16, (unsigned char*)uparrow_bits, true);
  QBitmap down(16, 16, (unsigned char*)arrow_down_bits, true);
  QBitmap erase(16, 16, (unsigned char*)trashcan_bits, true);
  // ----- set button texts, tooltips and bitmaps
  buttonUp->setPixmap(up);
  buttonDown->setPixmap(down);
  buttonDelete->setPixmap(erase);
  // TODO: loadApplicationMiniIcon is a very bad hack in this situation
  // as application icons like x.xpm are not what should be used in widgets
  buttonNew->setPixmap(KGlobal::iconLoader()->
		       loadApplicationIcon("x", KIconLoader::Small));
  QToolTip::add(buttonNew, i18n("Add a new string"));
  QToolTip::add(buttonUp, i18n("Move this string up"));
  QToolTip::add(buttonDown, i18n("Move this string down"));
  QToolTip::add(buttonDelete, i18n("Delete this string"));
  leLine->setEnabled(false);
  // -----
  connect(kapp, SIGNAL(appearanceChanged()), SLOT(initializeGeometry()));
  initializeGeometry();
  enableButtons();
  // ############################################################################
}


StringListEditWidget::~StringListEditWidget()
{
  // ############################################################################
  // ############################################################################
}

void StringListEditWidget::initializeGeometry()
{
  // ############################################################################
  setMinimumSize(sizeHint());
  // ############################################################################
}

QSize StringListEditWidget::sizeHint() const
{
  // ############################################################################
  const int Grid=3,
    ButtonSize=24;
  int cx, cy; 
  // -----
  cx=QMAX((int)1.5*leLine->sizeHint().width()+Grid+ButtonSize, // le, new-button
	  lbStrings->sizeHint().width()+Grid+ButtonSize);
  cy=QMAX(leLine->sizeHint().height(), ButtonSize)
    +Grid
    +QMAX(lbStrings->sizeHint().height(),
	  3*ButtonSize+2*Grid+10);
  return QSize(cx, cy);
  // ############################################################################
}

void StringListEditWidget::resizeEvent(QResizeEvent*)
{
  // ############################################################################
  const int Grid=3,
    ButtonSize=24;
  QPushButton* buttons[]= {
    buttonUp, 
    buttonDown,
    buttonDelete };
  const int Size=sizeof(buttons)/sizeof(buttons[0]);
  int i, tempx, tempy;
  // -----
  tempy=QMAX(ButtonSize, leLine->sizeHint().height());
  leLine->setGeometry(0, 0, width()-Grid-ButtonSize, tempy);
  buttonNew->setGeometry(width()-ButtonSize, 0, ButtonSize, ButtonSize);
  tempy+=Grid;
  lbStrings->setGeometry(0, tempy, width()-Grid-ButtonSize, height()-tempy);
  tempx=lbStrings->width()+Grid;
  tempy=leLine->height()+Grid;
  for(i=0; i<Size; i++)
    {
      if(i==Size-1) // last button at bottom
	{
	  buttons[i]->setGeometry(tempx, tempy+lbStrings->height()-ButtonSize,
				  ButtonSize, ButtonSize);
	} else {
	  buttons[i]->setGeometry(tempx, tempy+i*(ButtonSize+Grid), 
				  ButtonSize, ButtonSize);
	}	
    }
  // ############################################################################
}

void StringListEditWidget::setStrings(const list<string>& strings)
{
  // ############################################################################
  list<string>::const_iterator pos;
  // -----
  lbStrings->clear();
  for(pos=strings.begin(); pos!=strings.end(); pos++)
    {
      lbStrings->insertItem((*pos).c_str());
    }
  CHECK(lbStrings->count()==strings.size());
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::setStrings(const QStrList& strings)
{
  // ############################################################################
  lbStrings->clear();
  lbStrings->insertStrList(&strings);
  CHECK(lbStrings->count()==strings.count());
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::getStrings(list<string>& result)
{
  // ############################################################################
  int count;
  // -----
  result.erase(result.begin(), result.end());
  CHECK(result.empty());
  for(count=0; (unsigned int)count<lbStrings->count(); count++)
    {
      result.push_back(lbStrings->text(count).ascii());
    }
  CHECK(result.size()==lbStrings->count());
  // ############################################################################
}

void StringListEditWidget::getStrings(QStrList& result)
{
  // ############################################################################
  int count;
  // -----
  result.clear();
  CHECK(result.isEmpty());
  for(count=0; (unsigned int)count<lbStrings->count(); count++)
    {
      result.append(lbStrings->text(count));
    }
  CHECK(result.count()==lbStrings->count());  
  // ############################################################################
}

void StringListEditWidget::upPressed()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  int index=lbStrings->currentItem();
  string text;
  // -----
  LG(GUARD, "StringListDialog::upPressed: moving item %i up.\n", index);
  if(index==-1 || lbStrings->count()<2)
    {
      LG(GUARD, "StringListDialog::upPressed: nothing selected.\n");
      qApp->beep();
    } else {
      if(index!=0)
	{
	  text=lbStrings->text(index);
	  lbStrings->removeItem(index);
	  lbStrings->insertItem(text.c_str(), index-1);
	  lbStrings->setCurrentItem(index-1);
	  lbStrings->centerCurrentItem();
	} else {
	  qApp->beep();
	}
    }	  
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::downPressed()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  int index=lbStrings->currentItem();
  string text;
  // -----
  LG(GUARD, "StringListDialog::downPressed: moving item %i down.\n", index);
  if(index==-1 || lbStrings->count()<2)
    {
      LG(GUARD, "StringListDialog::downPressed: nothing selected.\n");
      qApp->beep();
    } else {
      if((unsigned int)index+1!=lbStrings->count())
	{
	  text=lbStrings->text(index);
	  lbStrings->removeItem(index);
	  lbStrings->insertItem(text.c_str(), index+1);
	  lbStrings->setCurrentItem(index+1);
	  lbStrings->centerCurrentItem();
	} else {
	  qApp->beep();
	}
    }
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::deletePressed()
{
  // ############################################################################
  int index=lbStrings->currentItem();
  // -----
  if(index==-1)
    {
      qApp->beep();
    } else {
      lbStrings->removeItem(index);
      if(lbStrings->count()==0)
 	{
 	  leLine->setText("");
 	  leLine->setEnabled(false);
 	}
    }
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::itemSelected(int index)
{
  // ############################################################################
  leLine->setEnabled(true);
  leLine->setText(lbStrings->text(index));
  leLine->setFocus();
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::newItem()
{
  // ############################################################################
  int index;
  // -----
  index=lbStrings->currentItem();
  lbStrings->insertItem("", index);
  switch(index)
    {
    case -1: // nothing selected:
      {
	lbStrings->setCurrentItem(lbStrings->count()-1);
	break;
      }
    case  0: // list contains one element
      {
	lbStrings->setCurrentItem(0);
	break;
      }
    default: 
      {
	lbStrings->setCurrentItem(index);
      }
    }
  leLine->setFocus();
  enableButtons();
  // ############################################################################
}

void StringListEditWidget::itemChanged(const char* text)
{
  REQUIRE(lbStrings->currentItem()!=-1 || lbStrings->count()==0);
  // ############################################################################
  if(!lbStrings->count()==0)
    {
      lbStrings->changeItem(text, lbStrings->currentItem());
    }
  // ############################################################################
}

void StringListEditWidget::enableButtons()
{
  // ############################################################################
  int index;
  // -----
  index=lbStrings->currentItem();
  buttonUp->setEnabled(index>0);
  buttonDown->setEnabled((unsigned int)index < (lbStrings->count()-1) && index!=-1);
  buttonDelete->setEnabled(index!=-1);
  // ############################################################################
}

// ##############################################################################
// MOC OUTPUT FILES:
#include "StringListEditWidget.moc"
#include "StringListEditWidgetData.moc"
// ##############################################################################

