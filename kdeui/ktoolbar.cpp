/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              (C) 1999 Chris Schlaeger (cs@kde.org)
              (C) 1999 Kurt Granroth (granroth@kde.org)

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

#include <qpainter.h>
#include <qtooltip.h>
#include <qdrawutil.h>
#include <qstring.h>
#include <qrect.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ktoolbar.h"
#include "klined.h"
#include "kseparator.h"
#include <ktmainwindow.h>
#include <klocale.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kwm.h>
#include <ktoolboxmgr.h>
#include <kstyle.h>

#include "ktoolbarbutton.h"
#include "ktoolbaritem.h"

template class QList<KToolBarItem>;

// Use enums instead of defines. We are C++ and NOT C !
enum {
    CONTEXT_LEFT = 0,
    CONTEXT_RIGHT = 1,
    CONTEXT_TOP = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4,
    CONTEXT_FLAT = 5,
    CONTEXT_ICONS = 6,
    CONTEXT_TEXT = 7,
    CONTEXT_TEXTRIGHT = 8,
    CONTEXT_TEXTUNDER = 9
};

class KToolBarPrivate
{
public:
  KToolBarPrivate()
  {
    m_iconSize     = "Normal";
    m_iconText     = KToolBar::IconOnly;
    m_position     = KToolBar::Top;
    m_highlight    = true;
    m_transparent  = true;
    m_honorStyle   = false;
    m_isHorizontal = true;

    m_items = new KToolBarItemList;
    m_items->setAutoDelete(true);

    m_maxItemWidth  = 0;
    m_maxItemHeight = 0;

    m_maxHorWidth   = -1;
    m_maxVerHeight  = -1;

    m_approxItemSize = 26;
  }
  ~KToolBarPrivate()
  {
    delete m_items; m_items = 0;
  }
  QString m_iconSize;
  QString m_title;
  KToolBar::IconText    m_iconText;
  KToolBar::BarPosition m_position;
  bool m_highlight;
  bool m_transparent;
  bool m_honorStyle;
  bool m_isHorizontal;

  KToolBarItemList *m_items;
  QWidget          *m_parent;

  int m_maxItemWidth;
  int m_maxItemHeight;
  int m_maxHorWidth;
  int m_maxVerHeight;

  int m_approxItemSize;
};

// this should be adjustable (in faar future... )
#define MIN_AUTOSIZE 150

/****************************** Tolbar **************************************/

KToolBar::KToolBar(QWidget *parent, const char *name, bool _honor_mode)
  : QFrame( parent, name )
{
  d = new KToolBarPrivate;
  d->m_honorStyle = _honor_mode;
  d->m_parent     = parent;        // our father

  init();
}

KToolBar::~KToolBar()
{
  delete d;

  // I would never guess that (sven)
  if (!QApplication::closingDown())
     delete context;
}

void KToolBar::init()
{
  // construct our context popup menu
  context = new QPopupMenu( 0, "context" );
  context->insertItem( i18n("Left"), CONTEXT_LEFT );
  context->insertItem( i18n("Top"),  CONTEXT_TOP );
  context->insertItem( i18n("Right"), CONTEXT_RIGHT );
  context->insertItem( i18n("Bottom"), CONTEXT_BOTTOM );
  context->insertItem( i18n("Floating"), CONTEXT_FLOAT );
  context->insertItem( i18n("Flat"), CONTEXT_FLAT );
  context->insertSeparator(-1);

  QPopupMenu *mode = new QPopupMenu( context, "mode" );
  mode->insertItem( i18n("Icons only"), CONTEXT_ICONS );
  mode->insertItem( i18n("Text only"), CONTEXT_TEXT );
  mode->insertItem( i18n("Text aside icons"), CONTEXT_TEXTRIGHT );
  mode->insertItem( i18n("Text under icons"), CONTEXT_TEXTUNDER );

  context->insertItem( i18n("Mode"), mode );

  // set some more defaults
  fullSizeMode  = true;
  mouseEntered  = false;
  localResize   = false;
  buttonDownOnHandle = FALSE;
  moving        = true;
  min_width     = -1;
  min_height    = -1;
  haveAutoSized = false;
  mgr           = 0L;

  setFrameStyle(NoFrame);
  setLineWidth( 1 );
  updateGeometry();
  enableFloating(true);
  setMouseTracking(true);

  connect(kapp, SIGNAL(appearanceChanged()), this, SLOT(slotReadConfig()));

  // finally, read in our configurable settings
  slotReadConfig();
}

void KToolBar::slotReadConfig()
{
  // read in the global ('kdeglobals') config file
  KConfig *config = KGlobal::config();
  QString group = config->group();

  static QString grpToolbar = QString::fromLatin1("Toolbar style");
  static QString grpKDE     = QString::fromLatin1("KDE");

  static QString attrIconText  = QString::fromLatin1("IconText");
  static QString attrHighlight = QString::fromLatin1("Highlighting");
  static QString attrTrans     = QString::fromLatin1("TransparentMoving");
  static QString attrIconStyle = QString::fromLatin1("KDEIconStyle");

  // first, get the toolbar specific settings
  config->setGroup(grpToolbar);
  bool highlight  = (bool)config->readNumEntry(attrHighlight, 1);
  int transparent = config->readBoolEntry(attrTrans, true);

  // we read in the IconText property *only* if we intend on actually
  // honoring it
  IconText icontext;
  if (d->m_honorStyle)
    icontext = (IconText)config->readNumEntry(attrIconText, IconOnly);
  else
    icontext = IconOnly;

  // now get the other global settings
  config->setGroup(grpKDE);
  QString iconstyle = config->readEntry(attrIconStyle, "Normal");

  // the actual size of the toolbar will be dependent on the buttons..
  // but we can make some guesses now
  if (iconstyle == "Large")
    d->m_approxItemSize = 36;
  else if (iconstyle == "Small")
    d->m_approxItemSize = 20;
  else
    d->m_approxItemSize = 26;

  // revert back to the old group
  config->setGroup(group);

  bool doUpdate = false;

  // check if the icon/text has changed
  if (icontext != d->m_iconText)
  {
    d->m_iconText = icontext;
    doUpdate = true;
  }

  // ...and check if the icon size has changed
  if (iconstyle != d->m_iconSize)
  {
    d->m_iconSize = iconstyle;
    doUpdate = true;
  }

  // ...and if we should highlight
  if (highlight != d->m_highlight)
  {
    d->m_highlight = highlight;
    doUpdate = true;
  }

  // ...and if we should move transparently
  if (transparent != d->m_transparent)
  {
    d->m_transparent = transparent;
    doUpdate = false;
  }

  if (doUpdate)
    emit modechange(); // tell buttons what happened
  if (isVisible ())
    updateRects(true);
}

void KToolBar::drawContents ( QPainter *)
{
}

void KToolBar::setMaxHeight(int h)
{
  d->m_maxVerHeight = h;
  updateRects(true);
}

int KToolBar::maxHeight()
{
  return d->m_maxVerHeight;
}

void KToolBar::setMaxWidth (int w)
{
  d->m_maxHorWidth = w;
  updateRects(true);
}

int KToolBar::maxWidth()
{
  return d->m_maxHorWidth;
}

void KToolBar::setTitle(const QString& title)
{
  d->m_title = title;
}

void KToolBar::layoutHorizontal(int w)
{
  int xOffset = 4 + 9 + 3;
  int yOffset = 1;
  int widest  = 0;
  int tallest = 0;

  /* We are, obviously, a horizontal toolbar */
  d->m_isHorizontal = true;

  /* For the horizontal layout we have to iterate three times through
   * the toolbar items.  During the first iteration, we find out the
   * size of the largest (non-autosized and non-right-aligned) button */
  QListIterator<KToolBarItem> qli(*d->m_items);
  if (d->m_honorStyle)
  {
    for (; *qli; ++qli)
    {
      /* make sure this is a button */
      if ((*qli)->itemType() != KToolBarItem::Button)
        continue;

      if ((*qli)->width() > d->m_maxItemWidth)
        d->m_maxItemWidth = (*qli)->width();

      if ((*qli)->height() > d->m_maxItemHeight)
        d->m_maxItemHeight = (*qli)->height();
    }
  }

  /* During the second iteration we resize and position the left
   * aligned items, find the auto-size item and accumulate the total
   * width for the left aligned widgets. */
  KToolBarItem* autoSizeItem = 0;

  /* This variable is used to accumulate the horizontal space the
   * left aligned items need. This includes the 3 pixel space
   * between the items. */
  int totalRightItemWidth = 0;

  /* Second iteration */
  for (qli.toFirst(); *qli; ++qli)
  {
    /* check for the line */
    Item *item = (*qli)->getItem();

    /* handle vertical separator lines */
    if (item->inherits("QFrame"))
    {
      QFrame *frame = (QFrame*)item;
      if (frame->frameShape() == QFrame::HLine)
      {
        frame->setFrameShape(QFrame::VLine);
        frame->resize(5, tallest - 2);
        continue;
      }
    }

    // now handle this item if it is NOT right aligned
    if ((*qli)->isRight() == false)
    {
      int itemWidth  = (*qli)->width();
      int itemHeight = (*qli)->height();
      if ((*qli)->isAuto())
      {
        itemWidth = MIN_AUTOSIZE;
        autoSizeItem = *qli;
      }
      else
      {
        /* if this isn't autosized, then make sure that it is the same
         * size as the maximum width IF it is a button */
        if ((itemWidth < d->m_maxItemWidth) &&
            ((*qli)->itemType() == KToolBarItem::Button))
        {
          (*qli)->resize(d->m_maxItemWidth, d->m_maxItemHeight);
          itemWidth  = d->m_maxItemWidth;
          itemHeight = d->m_maxItemHeight;
        }
      }

      /* make sure that it is the standard height */
      if ((itemHeight < d->m_maxItemHeight) &&
          ((*qli)->itemType() == KToolBarItem::Button))
        (*qli)->resize(d->m_maxItemWidth, d->m_maxItemHeight);

      if (xOffset + 3 + itemWidth > w)
      {
        /* The current line is full. We need to wrap-around and start
         * a new line. */
        xOffset = 4 + 9 + 3;
        yOffset += tallest + 3;
        tallest = 0;
      }

      /* position the button (or item.. whatever) */
      (*qli)->move(xOffset, yOffset);
      xOffset += 3 + itemWidth;

      /* We need to save the tallest height and the widest width. */
      if (itemWidth > widest)
        widest = itemWidth;
      if ((*qli)->height() > tallest)
        tallest = (*qli)->height();
    }
    else
    {
      totalRightItemWidth += (*qli)->width() + 3;
    }
  }

  int newXOffset = w - (3 + (totalRightItemWidth + 3) % w);
  if (newXOffset < xOffset)
  {
    /* right aligned items do not fit in the current line, so we start
     * a new line */
    if (autoSizeItem)
    {
      /* The auto-sized widget extends from the last normal left-alined
       * item to the right edge of the widget */
      autoSizeItem->resize(w - xOffset - 3 + MIN_AUTOSIZE,
                           autoSizeItem->height());
    }
    yOffset += tallest + 3;
    tallest = 0;
  }
  else
  {
    /* Right aligned items do fit in the current line. The auto-space
     * item may fill the space between left and right aligned items. */
    if (autoSizeItem)
      autoSizeItem->resize(newXOffset - xOffset - 3 + MIN_AUTOSIZE,
                           autoSizeItem->height());
  }
  xOffset = newXOffset;

  /* During the last iteration we position the right aligned items. */
  for (qli.toFirst(); *qli; ++qli)
  {
    if ((*qli)->isRight())
    {
      if (xOffset + 3 + (*qli)->width() > w)
      {
        xOffset = 4 + 9 + 3;
        yOffset += tallest + 3;
        tallest = 0;
      }

      (*qli)->move(xOffset, yOffset);
      xOffset += 3 + (*qli)->width();

      /* We need to save the tallest height and the widest width. */
      if ((*qli)->width() > widest)
        widest = (*qli)->width();
      if ((*qli)->height() > tallest)
        tallest = (*qli)->height();
    }
  }

  toolbarWidth = w;
  toolbarHeight = yOffset + tallest + 1;
  min_width = 4 + 9 + 3 + widest + 3;
  min_height = toolbarHeight;
  updateGeometry();
}

int
KToolBar::heightForWidth(int w) const
{
  if (!w) return min_height;

  /* This function only works for Top, Bottom or Floating tool
   * bars. For other positions it should never be called. To be save
   * on the save side the current minimum height is returned. */
  if (d->m_position != Top && d->m_position != Bottom && d->m_position != Floating)
    return (min_height);

  int xOffset = 4 + 9 + 3;
  int yOffset = 1;
  int tallest = 0;

  /* This variable is used to accumulate the horizontal space the
   * left aligned items need. This includes the 3 pixel space
   * between the items. */
  int totalRightItemWidth = 0;
  QListIterator<KToolBarItem> qli(*d->m_items);
  for (; *qli; ++qli)
  {
    if (!(*qli)->isRight())
    {
      if (xOffset + 3 + (*qli)->width() > w)
      {
        xOffset = 4 + 9 + 3;
        yOffset += tallest + 3;
        tallest = 0;
      }

      xOffset += 3 + (*qli)->width();

      /* We need to save the tallest height. */
      if ((*qli)->height() > tallest)
        tallest = (*qli)->height();
    }
    else
       totalRightItemWidth += (*qli)->width() + 3;
  }

  int newXOffset = w - (3 + (totalRightItemWidth + 3) % w);
  if (newXOffset < xOffset)
  {
    xOffset = 4 + 9 + 3;
    yOffset += tallest + 3;
    tallest = 0;
  }
  else
    xOffset = newXOffset;

  /* During the second iteration we position the left aligned items. */
  for (qli.toFirst(); (*qli); ++qli)
  {
    if ((*qli)->isRight())
    {
      if (xOffset + 3 + (*qli)->width() > w)
      {
        xOffset = 4 + 9 + 3;
        yOffset += tallest + 3;
        tallest = 0;
      }

      xOffset += 3 + (*qli)->width();

      /* We need to save the tallest height. */
      if ((*qli)->height() > tallest)
        tallest = (*qli)->height();
    }
  }

  return (yOffset + tallest + 1);
}

void
KToolBar::layoutVertical(int h)
{
  int xOffset = 3;
  int yOffset = 3 + 9 + 4;
  int widest = 0;
  int tallest = 0;

  /* we are, of course, not a horizontal toolbar */
  d->m_isHorizontal = false;

  /* For vertical toolbars, we have to iterate twice.. once to get the
   * sizes and another time to resize and position things */
  QListIterator<KToolBarItem> qli(*d->m_items);
  if (d->m_honorStyle)
  {
    for (; *qli; ++qli)
    {
      /* make sure this is a button */
      if ((*qli)->itemType() != KToolBarItem::Button)
        continue;

      if ((*qli)->width() > d->m_maxItemWidth)
        d->m_maxItemWidth = (*qli)->width();

      if ((*qli)->height() > d->m_maxItemHeight)
        d->m_maxItemHeight = (*qli)->height();
    }
  }

  /* Second iteration */
  for (qli.toFirst(); *qli; ++qli)
  {
    /* check for the line */
    Item *item = (*qli)->getItem();
    if (item->inherits("QFrame"))
    {
      QFrame *frame = (QFrame*)item;
      if (frame->frameShape() == QFrame::VLine)
      {
        frame->setFrameShape(QFrame::HLine);
        frame->resize(d->m_approxItemSize - 2, 5);
      }
    }

    /* resize the buttons if necessary */
    if ((*qli)->itemType() == KToolBarItem::Button)
    {
      int itemWidth  = (*qli)->width();
      int itemHeight = (*qli)->height();
      if ((itemWidth < d->m_maxItemWidth) || (itemHeight  < d->m_maxItemHeight))
      {
        (*qli)->resize(d->m_maxItemWidth, d->m_maxItemHeight);
        itemWidth  = d->m_maxItemWidth;
        itemHeight = d->m_maxItemHeight;
      }
    }

    if (yOffset + (*qli)->height() + 3 > h)
    {
      /* A column has been filled. We need to start a new column */
      yOffset = 4 + 9 + 3;
      xOffset += widest + 3;
      widest = 0;
    }

    /* arrange the toolbar item */
    (*qli)->move(xOffset, yOffset);

    /* auto-size items are set to the minimum auto-size or the width of
     * the widest widget so far. Wider widgets that follow have no
     * impact on the auto-size widgets that are above in the column. We
     * might want to improve this later. */
    if ((*qli)->isAuto())
      (*qli)->resize((widest > MIN_AUTOSIZE) ?
            widest : MIN_AUTOSIZE, (*qli)->height());

    /* adjust yOffset */
    yOffset += (*qli)->height() + 3;
    /* keep track of the maximum with of the column */
    if ((*qli)->width() > widest)
      widest = (*qli)->width();
    /* keep track of the tallest overall widget */
    if ((*qli)->height() > tallest)
      tallest = (*qli)->height();
  }

  toolbarHeight = h;
  toolbarWidth = min_width = xOffset + widest + 3;
  min_height = 4 + 9 + 3 + tallest + 3;
  updateGeometry();
}

int
KToolBar::widthForHeight(int h) const
{
  /* This function only works for Top, Bottom or Floating tool
   * bars. For other positions it should never be called. To be on
   * the save side the current minimum height is returned. */
  if (d->m_position != Left && d->m_position != Right && d->m_position != Floating)
    return (min_height);

  int xOffset = 3;
  int yOffset = 3 + 9 + 4;
  int widest = 0;
  int tallest = 0;

  QListIterator<KToolBarItem> qli(*d->m_items);
  for (; *qli; ++qli)
  {
    if (yOffset + (*qli)->height() + 3 > h)
    {
      /* A column has been filled. We need to start a new column */
      yOffset = 4 + 9 + 3;
      xOffset += widest + 3;
      widest = 0;
    }

    int itemWidth = (*qli)->width();
    /* auto-size items are set to the minimum auto-size or the width of
     * the widest widget so far. Wider widgets that follow have no
     * impact on the auto-size widgets that are above in the column. We
     * might want to improve this later. */
    if ((*qli)->isAuto())
      itemWidth = (widest > MIN_AUTOSIZE) ? widest : MIN_AUTOSIZE;

    /* adjust yOffset */
    yOffset += (*qli)->height() + 3;
    /* keep track of the maximum with of the column */
    if (itemWidth > widest)
      widest = itemWidth;
    /* keep track of the tallest overall widget */
    if ((*qli)->height() > tallest)
      tallest = (*qli)->height();
  }

  return(xOffset + widest + 3);
}

void
KToolBar::updateRects(bool res)
{
  switch (d->m_position)
  {
  case Flat:
    min_width = 30;
    min_height = 10;
    updateGeometry();
    break;

  case Top:
  case Bottom:
  {
    int mw = width();
    if (!fullSizeMode)
    {
      /* If we are not in full size mode and the user has requested a
       * certain width, this will be used. If no size has been requested
       * and the parent width is larger than the maximum width, we use
       * the maximum width. */
      if (d->m_maxHorWidth != -1)
        mw = d->m_maxHorWidth;
      else if (width() > maximumSizeHint().width())
        mw = maximumSizeHint().width();
    }  
    layoutHorizontal(mw);
    break;
  }

  case Left:
  case Right:
  {
    int mh = height();
    if (!fullSizeMode)
    {
      /* If we are not in fullSize mode and the user has requested a
       * certain height, this will be used. If no size has been requested
       * and the parent height is larger than the maximum height, we use
       * the maximum height. */
      if (d->m_maxVerHeight != -1)
        mh = d->m_maxVerHeight;
      else if (height() > maximumSizeHint().height())
        mh = maximumSizeHint().height();
    }
    layoutVertical(mh);
    break;
  }
  default:
    return;
  }

  if (res == true)
  {
    localResize = true;
    resize(toolbarWidth, toolbarHeight);
    localResize = false;
  }
}

QSize
KToolBar::sizeHint() const
{
  switch (d->m_position)
  {
  case Floating:
    /* Floating bars are under direct control of the WM. sizeHint() is
     * ignored. */
    break;

  case Top:
  case Bottom:
    if (!fullSizeMode && (d->m_maxHorWidth != -1))
    {
      /* If fullSize mode is disabled and the user has requested a
       * specific width, then we use this value. */
      return (QSize(d->m_maxHorWidth, min_height));
    }
    break;
  case Right:
  case Left:
    if (!fullSizeMode && (d->m_maxVerHeight != -1))
    {
      /* If fullSize mode is disabled and the user has requested a
       * specific height, then we use this value. */
      return (QSize(min_width, d->m_maxVerHeight));
    }
    break;
  default:
    break;
  }
  
  return (QSize(min_width, min_height));
}

QSize
KToolBar::maximumSizeHint() const
{
  /* This function returns the maximum size the bar can have. All toolbar
   * items are placed in a single line. */
  int prefWidth = -1;
  int prefHeight = -1;

  QListIterator<KToolBarItem> qli(*d->m_items);

  switch (d->m_position)
  {
  case Flat:
    prefWidth = 30;
    prefHeight = 10;
    break;

  case Floating:
  case Top:
  case Bottom:
    prefWidth = 4 + 9 + 3;
    prefHeight = 0;

    for (; *qli; ++qli)
    {
      int itemWidth = (*qli)->width();
      if ((*qli)->isAuto())
        itemWidth = MIN_AUTOSIZE;

      prefWidth += 3 + itemWidth;
      if ((*qli)->height() > prefHeight)
        prefHeight = (*qli)->height();
    }
    prefWidth += 3;    /* 3 more pixels to the right */
    prefHeight += 2;  /* one more pixels above and below */
    break;

  case Left:
  case Right:  
    prefWidth = 0;
    prefHeight = 4 + 9 + 3;

    for (; *qli; ++qli)
    {
      prefHeight += (*qli)->height() + 3;
      /* keep track of the maximum with of the column */
      if ((*qli)->isAuto())
      {
        if (MIN_AUTOSIZE > prefWidth)
          prefWidth = MIN_AUTOSIZE;
      }
      else
      {
        if ((*qli)->width() > prefWidth)
          prefWidth = (*qli)->width();
      }
    }
    prefWidth += 2;    /* one more pixels to the left and right */
    prefHeight += 3;  /* 3 more pixels below */
    break;
  }
  return (QSize(prefWidth, prefHeight));
}

QSize
KToolBar::minimumSizeHint() const
{
  return (sizeHint());
}

QSizePolicy
KToolBar::sizePolicy() const
{
  switch (d->m_position)
  {
  case Floating:
    /* Floating bars are under direct control of the WM. sizePolicy() is
     * ignored. */
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  case Top:
  case Bottom:
    return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

  case Left:
  case Right:
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);  

  default:
    return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  }
}

void KToolBar::mouseMoveEvent ( QMouseEvent *mev)
{
    if (!moving)
  return;
    
  /* The toolbar handles are hightlighted when the mouse moves over
     * the handle. */
  if ((d->m_isHorizontal && (mev->x() < 0 || mev->x() > 9)) ||
    (!d->m_isHorizontal && (mev->y() < 0 || mev->y() > 9)))
  {
    /* Mouse is outside of the handle. If it's still hightlighed we have
     * to de-d->m_highlight it. */
    if (mouseEntered)
    {
      mouseEntered = false;
      repaint(false);
    }
    return;
  }
  else
  {
    /* Mouse is over the handle. If the handle is not yet hightlighted we
     * have to to it now. */
    if (!mouseEntered)
    {
      mouseEntered = true;
      repaint(false);
    }
  }
    
  if (!buttonDownOnHandle)
    return;
  buttonDownOnHandle = FALSE;

  if (d->m_position != Flat)
  {
    int ox, oy, ow, oh;

    QRect rr(d->m_parent->geometry());
    ox = rr.x();
    oy = rr.y();
    ow = rr.width();
    oh = rr.height();
    if (d->m_parent->inherits("KTMainWindow"))
    {
      QRect mainView = ((KTMainWindow*) d->m_parent)->mainViewGeometry();

      ox += mainView.left();
      oy += mainView.top();
      ow = mainView.width();
      oh = mainView.height();
    }

    int fat = 25; //ness

    mgr = new KToolBoxManager(this, d->m_transparent);

    //Firt of all discover _your_ position

    if (d->m_position == Top )
      mgr->addHotSpot(geometry(), true);             // I'm on top
    else
      mgr->addHotSpot(rr.x(), oy, rr.width(), fat); // top

    if (d->m_position == Bottom)
      mgr->addHotSpot(geometry(), true);           // I'm on bottom
    else
      mgr->addHotSpot(rr.x(), oy+oh-fat, rr.width(), fat); // bottom

    if (d->m_position == Left)
      mgr->addHotSpot(geometry(), true);           // I'm on left
    else
      mgr->addHotSpot(ox, oy, fat, oh); // left

    if (d->m_position == Right)
      mgr->addHotSpot(geometry(), true);           // I'm on right
    else
      mgr->addHotSpot(ox+ow-fat, oy, fat, oh); //right

    movePos = d->m_position;
    connect (mgr, SIGNAL(onHotSpot(int)), SLOT(slotHotSpot(int)));
    if (d->m_transparent)
      mgr->doMove(true, false, true);
    else
    {
      /*
        QList<KToolBarItem> ons;
        for (KToolBarItem *b = d->m_items->first(); b; b = d->m_items->next())
        {
        if (b->isEnabled())
        ons.append(b);
        b->setEnabled(false);
        }
      */
      mgr->doMove(true, false, false);
      /*
        for (KToolBarItem *b = ons.first(); b; b=ons.next())
        b->setEnabled(true);
      */
    }
    if (d->m_transparent)
    {
      setBarPos (movePos);

      if (movePos == Floating)
        move (mgr->x(), mgr->y());
      if (!isVisible())
        show();
    }
    mouseEntered = false;
    delete mgr;
    mgr=0;
    repaint (false);
  }
}

void KToolBar::mouseReleaseEvent ( QMouseEvent *m)
{
    buttonDownOnHandle = FALSE;
    if (!moving)
  return;
    
    if (mgr)
  mgr->stop();
    if ( d->m_position != Floating &&
   ((d->m_isHorizontal && m->x()<9) || (!d->m_isHorizontal && m->y()<9)) ) {
  setFlat (d->m_position != Flat);
    }
}

void KToolBar::mousePressEvent ( QMouseEvent *m )
{
    buttonDownOnHandle |=   ((d->m_isHorizontal && m->x()<9) || (!d->m_isHorizontal && m->y()<9));

  if (moving)
      if (m->button() == RightButton)
  {
      context->popup( mapToGlobal( m->pos() ), 0 );
      buttonDownOnHandle = FALSE;
      ContextCallback(0);
        }
}

void KToolBar::slotHotSpot(int hs)
{
  if (mgr == 0)
    return;
  if (!d->m_transparent) // opaque
  {
    switch (hs)
    {
      case 0: //top
        setBarPos(Top);
        break;

      case 1: //bottom
        setBarPos(Bottom);
        break;

      case 2: //left
        setBarPos(Left);
        break;

      case 3: //right
        setBarPos(Right);
        break;

      case -1: // left all
        setBarPos(Floating);
        break;
    }
    if (d->m_position != Floating)
    {
      QPoint p(d->m_parent->mapToGlobal(pos())); // OH GOOOOODDDD!!!!!
      mgr->setGeometry(p.x(), p.y(), width(), height());
    }
    if (!isVisible())
      show();
  }
  else // transparent
  {
    switch (hs)
    {
      case 0: //top
        mgr->setGeometry(0);
        movePos=Top;
        break;

      case 1: //bottom
        mgr->setGeometry(1);
        movePos=Bottom;
        break;

      case 2: //left
        mgr->setGeometry(2);
        movePos=Left;
        break;

      case 3: //right
        mgr->setGeometry(3);
        movePos=Right;
        break;

      case -1: // left all
        mgr->setGeometry(mgr->mouseX(), mgr->mouseY(), width(), height());
        movePos=Floating;
        break;
    }
  }
}

void KToolBar::resizeEvent(QResizeEvent*)
{
  /*
   * The resize can affect the arrangement of the toolbar items so
   * we have to call updateRects(). But we need not trigger another
   * resizeEvent!  */
  updateRects();

  if (d->m_position == Floating)
  {
    /* It's flicker time again. If the size is under direct control of
     * the WM we have to force the height to make the heightForWidth
     * feature work. */
    if (d->m_isHorizontal)
    {
      /* horizontal bar */
      if (height() != heightForWidth(width()))
        resize(width(), heightForWidth(width()));
    }
    else
    {
      /* vertical bar */
      if (width() != widthForHeight(height()))
        resize(height(), widthForHeight(height()));
    }
  }
}

void KToolBar::paintEvent(QPaintEvent *)
{
  if (mgr)
    return;

  // we don't want to paint anything if we have no items
  if (d->m_items->count() == 0)
  {
    hide();
    return;
  }

  toolbarHeight = height ();
  if (d->m_position == Flat)
    toolbarWidth = min_width;
  else
    toolbarWidth = width ();

  int stipple_height;

  // Moved around a little to make variables available for KStyle (mosfet).

  QColorGroup g = QWidget::colorGroup();
  // Took higlighting handle from kmenubar - sven 040198
  QBrush b;
  if (mouseEntered && d->m_highlight)
      b = colorGroup().highlight(); // this is much more logical then
  // the hardwired value used before!!
  else
      b = QWidget::backgroundColor();

  QPainter *paint = new QPainter();
  paint->begin( this );

  if(kapp->kstyle()){
      kapp->kstyle()->drawKToolBar(paint, 0, 0, toolbarWidth, toolbarHeight,
                                   colorGroup(), d->m_position == Floating);
      if(moving){
          if(d->m_isHorizontal)
              kapp->kstyle()->drawKBarHandle(paint, 0, 0, 9, toolbarHeight,
                                             colorGroup(), true,  &b);
          else
              kapp->kstyle()->drawKBarHandle(paint, 0, 0, toolbarWidth, 9,
                                             colorGroup(), false, &b);
      }
      paint->end();
      delete paint;
      return;
  }
  if (moving)
  {
    // Handle point
    if (d->m_isHorizontal)
    {
      qDrawShadePanel( paint, 0, 0, 9, toolbarHeight,
                       g , false, 1, &b);
      paint->setPen( g.light() );
      paint->drawLine( 9, 0, 9, toolbarHeight);
      stipple_height = 3;
      while ( stipple_height < toolbarHeight-4 ) {
        paint->drawPoint( 1, stipple_height+1);
        paint->drawPoint( 4, stipple_height);
        stipple_height+=3;
      }
      paint->setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < toolbarHeight-4 ) {
        paint->drawPoint( 2, stipple_height+1);
        paint->drawPoint( 5, stipple_height);
        stipple_height+=3;
      }
    }
    else // vertical
    {
      qDrawShadePanel( paint, 0, 0, toolbarWidth, 9,
                       g , false, 1, &b);

      paint->setPen( g.light() );
      paint->drawLine( 0, 9, toolbarWidth, 9);
      stipple_height = 3;
      while ( stipple_height < toolbarWidth-4 ) {
        paint->drawPoint( stipple_height+1, 1);
        paint->drawPoint( stipple_height, 4 );
        stipple_height+=3;
      }
      paint->setPen( g.dark() );
      stipple_height = 4;
      while ( stipple_height < toolbarWidth-4 ) {
        paint->drawPoint( stipple_height+1, 2 );
        paint->drawPoint( stipple_height, 5);
        stipple_height+=3;
      }
    }
  } //endif moving

  qDrawShadePanel(paint, 0, 0, width(), height(), g , false, 1);

  paint->end();
  delete paint;
}

void KToolBar::closeEvent (QCloseEvent *e)
{
  if (d->m_position == Floating)
   {
     setBarPos(lastPosition);
     e->ignore();
     return;
   }
  e->accept();
}


void KToolBar::ButtonClicked( int id )
{
  emit clicked( id );
}

void KToolBar::ButtonDblClicked( int id )
{
  emit doubleClicked( id );
}

void KToolBar::ButtonPressed( int id )
{
  emit pressed( id );
}

void KToolBar::ButtonReleased( int id )
{
  emit released( id );
}

void KToolBar::ButtonToggled( int id )
{
  emit toggled( id );
}

void KToolBar::ButtonHighlighted(int id, bool on )
{
  emit highlighted(id, on);
}


 /********************\
 *                    *
 * I N T E R F A C E  *
 *                    *
 \********************/

/***** BUTTONS *****/

// insert a button
int KToolBar::insertButton( const QString& icon, int id, bool enabled,
          const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( icon, id, this, 0L, _text);
  KToolBarItem *item = new KToolBarItem(button, KToolBarItem::Button, id, true);
  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  item->setEnabled( enabled );
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

/// Inserts a button.
int KToolBar::insertButton( const QPixmap& pixmap, int id, bool enabled,
          const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0L, _text);
  KToolBarItem *item = new KToolBarItem(button, KToolBarItem::Button, id,
                                        true);
  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  item->setEnabled( enabled );
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

/// Inserts a button with popup.
int KToolBar::insertButton( const QPixmap& pixmap, int id, QPopupMenu *_popup,
                            bool enabled, const QString&_text, int index)
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0L, _text);
  KToolBarItem *item = new KToolBarItem(button, KToolBarItem::Button, id,
                                        true);
  button->setPopup(_popup);

  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );

  item->setEnabled( enabled );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}


/// Inserts a button with connection.
int KToolBar::insertButton( const QString& icon, int id, const char *signal,
          const QObject *receiver, const char *slot, bool enabled,
          const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( icon, id, this, 0L, _text);
  KToolBarItem *item = new KToolBarItem(button, KToolBarItem::Button, id, true);

  /* now do checks to make sure that this is the right size */
  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  connect( button, signal, receiver, slot );
  item->setEnabled( enabled );
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

int KToolBar::insertButton( const QPixmap& pixmap, int id, const char *signal,
          const QObject *receiver, const char *slot, bool enabled,
          const QString&_text, int index )
{
  KToolBarButton *button = new KToolBarButton( pixmap, id, this, 0L, _text);
  KToolBarItem *item = new KToolBarItem(button, KToolBarItem::Button, id,
                                        true);

  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );

  connect(button, SIGNAL(clicked(int)), this, SLOT(ButtonClicked(int)));
  connect(button, SIGNAL(doubleClicked(int)), this, SLOT(ButtonDblClicked(int)));
  connect(button, SIGNAL(released(int)), this, SLOT(ButtonReleased(int)));
  connect(button, SIGNAL(pressed(int)), this, SLOT(ButtonPressed(int)));
  connect(button, SIGNAL(highlighted(int, bool)), this,
          SLOT(ButtonHighlighted(int, bool)));

  connect( button, signal, receiver, slot );
  item->setEnabled( enabled );
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

/********* SEPARATOR *********/
/// Inserts separator

int KToolBar::insertSeparator( int index )
{
  KToolBarButton *separ = new KToolBarButton( this );
  KToolBarItem *item = new KToolBarItem(separ, KToolBarItem::Separator, -1,
                                        true);

  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );
  
  if (d->m_position == Flat)
    item->hide();
  updateRects(true);
  return d->m_items->at();
}


/********* LINESEPARATOR *********/
/// Inserts line separator

int KToolBar::insertLineSeparator( int index )
{

  KSeparator *separ = new KSeparator(QFrame::VLine, this);

  KToolBarItem *item = new KToolBarItem(separ, KToolBarItem::Frame, -1, true);
  item->resize( 5, d->m_approxItemSize - 2 );

  if ( index == -1 )
    d->m_items->append( item );
  else
    d->m_items->insert( index, item );
  
  if (d->m_position == Flat)
    item->hide();
  updateRects(true);
  return d->m_items->at();
}


/********* Frame **********/
/// inserts QFrame

int KToolBar::insertFrame (int _id, int _size, int _index)
{
  debug ("insertFrame is deprecated. use insertWidget");

  QFrame *frame;
  bool mine = false;

  // ok I'll do it for you;
  frame = new QFrame (this);
  mine = true;

  KToolBarItem *item = new KToolBarItem(frame, KToolBarItem::Frame, _id, mine);

  if (_index == -1)
    d->m_items->append (item);
  else
    d->m_items->insert(_index, item);
  item-> resize (_size, d->m_approxItemSize-2);
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}
/* A poem all in G-s! No, any widget */

int KToolBar::insertWidget(int _id, int _size, QWidget *_widget,
    int _index )
{
  KToolBarItem *item = new KToolBarItem(_widget, KToolBarItem::Frame, _id, false);

  if (_index == -1)
    d->m_items->append (item);
  else
    d->m_items->insert(_index, item);
  item-> resize (_size, d->m_approxItemSize-2);
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

/************** LINE EDITOR **************/
// Inserts a KLineEdit. KLineEdit is derived from QLineEdit and has
//  another signal, tabPressed, for completions.

int KToolBar::insertLined(const QString& text, int id, const char *signal,
        const QObject *receiver, const char *slot,
        bool enabled, const QString& tooltiptext, int size, int index)
{
  KLineEdit *lined = new KLineEdit (this, 0);
  KToolBarItem *item = new KToolBarItem(lined, KToolBarItem::Lined, id,
                                        true);


  if (index == -1)
    d->m_items->append (item);
  else
    d->m_items->insert(index, item);
  if (!tooltiptext.isNull())
    QToolTip::add( lined, tooltiptext );
  connect( lined, signal, receiver, slot );
  lined->setText(text);
  item->resize(size, d->m_approxItemSize-2);
  item->setEnabled(enabled);
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

/************** COMBO BOX **************/
/// Inserts comboBox with QStrList

int KToolBar::insertCombo (QStrList *list, int id, bool writable,
                           const char *signal, const QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext,
                           int size, int index,
                           QComboBox::Policy policy)
{
  QComboBox *combo = new QComboBox (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, KToolBarItem::Combo, id,
                                        true);

  if (index == -1)
    d->m_items->append (item);
  else
    d->m_items->insert (index, item);
  combo->insertStrList (list);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->setAutoResize(false);
  item->resize(size, 24);
  item->setEnabled(enabled);
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

/// Inserts comboBox with QStringList

int KToolBar::insertCombo (const QStringList &list, int id, bool writable,
                           const char *signal, const QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext,
                           int size, int index,
                           QComboBox::Policy policy)
{
  QComboBox *combo = new QComboBox (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, KToolBarItem::Combo, id,
                                        true);

  if (index == -1)
    d->m_items->append (item);
  else
    d->m_items->insert (index, item);
  combo->insertStringList (list);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
    QToolTip::add( combo, tooltiptext );
  connect ( combo, signal, receiver, slot );
  combo->setAutoResize(false);
  item->resize(size, 24);
  item->setEnabled(enabled);
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}


/// Inserts combo with text

int KToolBar::insertCombo (const QString& text, int id, bool writable,
                           const char *signal, QObject *receiver,
                           const char *slot, bool enabled,
                           const QString& tooltiptext, int size, int index,
                           QComboBox::Policy policy)
{
  QComboBox *combo = new QComboBox (writable, this);
  KToolBarItem *item = new KToolBarItem(combo, KToolBarItem::Combo, id,
                                        true);

  if (index == -1)
    d->m_items->append (item);
  else
    d->m_items->insert (index, item);
  combo->insertItem (text);
  combo->setInsertionPolicy(policy);
  if (!tooltiptext.isNull())
    QToolTip::add( combo, tooltiptext );
  connect (combo, signal, receiver, slot);
  combo->setAutoResize(false);
  item->resize(size, 24);
  item->setEnabled(enabled);
  if (d->m_position != Flat)
    item->show();
  updateRects(true);
  return d->m_items->at();
}

void KToolBar::clear ()
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
  {
      if(b->isAuto())
        haveAutoSized=false;
      d->m_items->remove();
  }
  updateRects(true);
}

/// Removes item by ID

void KToolBar::removeItem (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      d->m_items->remove();
    }
  updateRects(true);
}

void KToolBar::showItem (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      b->show();
    }
  updateRects(true);
}

void KToolBar::hideItem (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if(b->isAuto())
        haveAutoSized=false;
      b->hide();
    }
  updateRects(true);
}
/// ******** Tools

/// misc
void KToolBar::addConnection (int id, const char *signal,
                              const QObject *receiver, const char *slot)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
           connect (b->getItem(), signal, receiver, slot);
}

/// Common
void KToolBar::setItemEnabled( int id, bool enabled )
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      b->setEnabled(enabled);
}

void KToolBar::setItemAutoSized ( int id, bool enabled )
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      b->autoSize(enabled);
      haveAutoSized = true;
    updateRects(true);
    }
}

void KToolBar::alignItemRight(int id, bool yes)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      b->alignRight (yes);
    updateRects();
    }
}

void KToolBar::setItemNoStyle(int id, bool no_style)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      ((KToolBarButton *)b->getItem())->setNoStyle(no_style);
      break;
    }
}

/// Butoons
void KToolBar::setButtonPixmap( int id, const QPixmap& _pixmap )
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setPixmap( _pixmap );
}


void KToolBar::setDelayedPopup (int id , QPopupMenu *_popup, bool toggle )
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setDelayedPopup(_popup, toggle);
}


/// Toggle buttons
void KToolBar::setToggle ( int id, bool yes )
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      ((KToolBarButton *) b->getItem())->setToggle(yes);
      connect (b->getItem(), SIGNAL(toggled(int)),
               this, SLOT(ButtonToggled(int)));
    }
}

void KToolBar::toggleButton (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if (((KToolBarButton *) b->getItem())->isToggleButton() == true)
        ((KToolBarButton *) b->getItem())->toggle();
    }
}

void KToolBar::setButton (int id, bool on)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->on(on);
}

//Autorepeat buttons
void KToolBar::setAutoRepeat (int id, bool flag /*, int delay, int repeat */)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      ((KToolBarButton *) b->getItem())->setAutoRepeat(flag);
}


bool KToolBar::isButtonOn (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if (((KToolBarButton *) b->getItem())->isToggleButton() == true)
        return ((KToolBarButton *) b->getItem())->isOn();
    }
  return false;
}

/// Lined
void KToolBar::setLinedText (int id, const QString& text)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      ((KLineEdit *) b->getItem())->setText(text);
    }
}

QString KToolBar::getLinedText (int id )
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      return ((KLineEdit *) b->getItem())->text();
  return QString::null;
}

/// Combos
void KToolBar::insertComboItem (int id, const QString& text, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      ((QComboBox *) b->getItem())->insertItem(text, index);
    }
}

void KToolBar::insertComboList (int id, QStrList *list, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
  ((QComboBox *) b->getItem())->insertStrList(list, index);
}

void KToolBar::insertComboList (int id, const QStringList &list, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
  ((QComboBox *) b->getItem())->insertStringList(list, index);
}

void KToolBar::setCurrentComboItem (int id, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      ((QComboBox *) b->getItem())->setCurrentItem(index);
    }
}

void KToolBar::removeComboItem (int id, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      ((QComboBox *) b->getItem())->removeItem(index);
}

void KToolBar::changeComboItem  (int id, const QString& text, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if (index == -1)
      {
        index = ((QComboBox *) b->getItem())->currentItem();
        ((QComboBox *) b->getItem())->changeItem(text, index);
      }
      else
      {
        ((QComboBox *) b->getItem())->changeItem(text, index);
      }
    }
}

void KToolBar::clearCombo (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
  ((QComboBox *) b->getItem())->clear();
}

QString KToolBar::getComboItem (int id, int index)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
    {
      if (index == -1)
        index = ((QComboBox *) b->getItem())->currentItem();
      return ((QComboBox *) b->getItem())->text(index);
    }
  return QString::null;
}

QComboBox *KToolBar::getCombo (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b=d->m_items->next())
    if (b->ID() == id )
      return ((QComboBox *) b->getItem());
  return 0;
}

KLineEdit *KToolBar::getLined (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b!=NULL; b=d->m_items->next())
    if (b->ID() == id )
      return ((KLineEdit *) b->getItem());
  return 0;
}


KToolBarButton* KToolBar::getButton( int id )
{
  for( KToolBarItem* b = d->m_items->first(); b != NULL; b = d->m_items->next() )
    if(b->ID() == id )
      return ((KToolBarButton *) b->getItem());
  return 0;
}

QFrame *KToolBar::getFrame (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b = d->m_items->next())
    if (b->ID() == id )
      return ((QFrame *) b->getItem());
  return 0;
}

QWidget *KToolBar::getWidget (int id)
{
  for (KToolBarItem *b = d->m_items->first(); b; b = d->m_items->next())
    if (b->ID() == id )
      return (b->getItem());
  return 0;
}


/// Toolbar itself

void KToolBar::setFullWidth(bool flag)
{
  fullSizeMode = flag;
}

bool KToolBar::fullSize() const
{
  return (fullSizeMode);
}

void KToolBar::enableMoving(bool flag)
{
  moving = flag;
}

void KToolBar::setBarPos(BarPosition bpos)
{
  if (d->m_position != bpos)
  {
    if (bpos == Floating)
    {
      lastPosition = d->m_position;
      d->m_position = bpos;
      oldX = x();
      oldY = y();
      oldWFlags = getWFlags();
      QPoint p = mapToGlobal(QPoint(0,0));
      hide();
      emit moved (bpos);  // this sets up KTW but not toolbar which floats
      updateRects(false); // we need this to set us up
      recreate(0, 0, p, false);
      XSetTransientForHint( qt_xdisplay(), winId(), d->m_parent->topLevelWidget()->winId());
      KWM::setDecoration(winId(), 2);
      KWM::moveToDesktop(winId(), KWM::desktop(d->m_parent->winId()));
      setCaption(QString::null); // this triggers a qt bug
      if (!d->m_title)
        setCaption(d->m_title);
      else
      {
        QString s = d->m_parent->caption();
        s.append(QString::fromLatin1(" [tools]"));
        setCaption(s);
      }
      context->changeItem (i18n("UnFloat"), CONTEXT_FLOAT);
      context->setItemEnabled (CONTEXT_FLAT, FALSE);
      setMouseTracking(true);
      mouseEntered=false;
      return;
    }
    else if (d->m_position == Floating) // was floating
    {
      d->m_position = bpos;
      hide();
      recreate(d->m_parent, oldWFlags, QPoint(oldX, oldY), true);
      emit moved (bpos); // another bar::updateRects (damn) No! It's ok.
      context->changeItem (i18n("Float"), CONTEXT_FLOAT);
      context->setItemEnabled (CONTEXT_FLAT, TRUE);
      setMouseTracking(true);
      mouseEntered = false;
      updateRects ();
      return;
    }
    else
    {
      if (bpos == Flat)
      {
        setFlat (true);
        return;
      }
      d->m_position = bpos;
      enableFloating (true);
      emit moved ( bpos );
      updateRects();
      return;
    }
  }
}

KToolBar::BarPosition KToolBar::barPos()
{
  return d->m_position;
}

void KToolBar::enableFloating (bool arrrrrrgh)
{
    context->setItemEnabled (CONTEXT_FLOAT, arrrrrrgh);
}

void KToolBar::setIconText(IconText icontext)
{
  bool doUpdate=false;

  if (icontext != d->m_iconText)
  {
    d->m_maxItemWidth  = 0;
    d->m_maxItemHeight = 0;

    d->m_iconText = icontext;
    doUpdate=true;
  }

  if (doUpdate)
    emit modechange(); // tell buttons what happened

  if (isVisible())
    updateRects(true);
}

KToolBar::IconText KToolBar::iconText() const
{
  return d->m_iconText;
}

bool KToolBar::enable(BarStatus stat)
{
  bool mystat = isVisible();

  if ( (stat == Toggle && mystat) || stat == Hide )
    hide();
  else
    show();

  emit moved (d->m_position); // force KTM::updateRects (David)
  return ( isVisible() == mystat );
}

/*************************************************************

Mouse move and drag routines

*************************************************************/


void KToolBar::leaveEvent (QEvent *)
{
  if (mgr)
    return;
  if ( mouseEntered ) {
      mouseEntered = false;
      repaint( false );
  }
}


void KToolBar::setFlat (bool flag)
{

#define also

  if (d->m_position == Floating )
    return;
  if ( flag == (d->m_position == Flat))
    also return;


  if (flag) //flat
  {
    context->changeItem (i18n("UnFlat"), CONTEXT_FLAT);
    lastPosition = d->m_position; // test float. I did and it works by miracle!?
    //debug ("Flat");
    d->m_position = Flat;
    d->m_isHorizontal = false;
    for (KToolBarItem *b = d->m_items->first(); b; b = d->m_items->next())
      b->hide();       // move items out of sight
    enableFloating(false);
  }
  else //unflat
  {
    context->changeItem (i18n("Flat"), CONTEXT_FLAT);
    //debug ("Unflat");
    setBarPos(lastPosition);
    for (KToolBarItem *b = d->m_items->first(); b; b = d->m_items->next())
    {
      if (b->itemType() != KToolBarItem::Separator)
        b->show();
    }
    enableFloating(true);
  }
  emit moved(Flat); // KTM will block this->updateRects
  updateRects();
}

int KToolBar::count()
{
  return d->m_items->count();
}

void KToolBar::ContextCallback( int )
{
  int i = context->exec();
  switch ( i )
    {
    case CONTEXT_LEFT:
      setBarPos( Left );
      break;
    case CONTEXT_RIGHT:
      setBarPos( Right );
      break;
    case CONTEXT_TOP:
      setBarPos( Top );
      break;
    case CONTEXT_BOTTOM:
      setBarPos( Bottom );
      break;
    case CONTEXT_FLOAT:
      if (d->m_position == Floating)
        setBarPos (lastPosition);
      else
      {
        setBarPos( Floating );
        move(QCursor::pos());
        show();
      }
      break;
    case CONTEXT_FLAT:
      setFlat (d->m_position != Flat);
      break;
    case CONTEXT_ICONS:
      setIconText( IconOnly );
      break;
    case CONTEXT_TEXTRIGHT:
      setIconText( IconTextRight );
      break;
    case CONTEXT_TEXT:
      setIconText( TextOnly );
      break;
    case CONTEXT_TEXTUNDER:
      setIconText( IconTextBottom );
      break;
    }

  mouseEntered=false;
  repaint(false);
}

KToolBarItemList *KToolBar::items()
{
  return d->m_items;
}

bool KToolBar::highlight() const
{
  return d->m_highlight;
}

#include "ktoolbar.moc"
