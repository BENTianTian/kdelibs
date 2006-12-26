/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
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

/////////////////// KDateTable widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998-2001 Mirko Boehm
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal:
//      dateSelected(QDate)

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>
#include <knotification.h>
#include <kcalendarsystem.h>
#include <kshortcut.h>
#include <kstandardshortcut.h>
#include "kdatepicker.h"
#include "kdatetable.h"
#include "kmenu.h"
#include "kactioncollection.h"
#include "kaction.h"

#include <qdatetime.h>
#include <qstring.h>
#include <qpen.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qevent.h>
#include <qhash.h>
#include <qapplication.h>
#include <assert.h>


class KDateTable::KDateTablePrivate
{
public:
   KDateTablePrivate()
   {
      popupMenuEnabled=false;
      useCustomColors=false;
   }

   ~KDateTablePrivate()
   {
   }

   bool popupMenuEnabled;
   bool useCustomColors;

   struct DatePaintingMode
   {
     QColor fgColor;
     QColor bgColor;
     BackgroundMode bgMode;
   };
   QHash <QString,DatePaintingMode*> customPaintingModes;

};


KDateValidator::KDateValidator(QWidget* parent)
    : QValidator(parent)
{
}

QValidator::State
KDateValidator::validate(QString& text, int&) const
{
  QDate temp;
  // ----- everything is tested in date():
  return date(text, temp);
}

QValidator::State
KDateValidator::date(const QString& text, QDate& d) const
{
  QDate tmp = KGlobal::locale()->readDate(text);
  if (!tmp.isNull())
    {
      d = tmp;
      return Acceptable;
    } else
      return QValidator::Intermediate;
}

void
KDateValidator::fixup( QString& ) const
{

}

KDateTable::KDateTable(const QDate& date_, QWidget* parent)
  : Q3GridView(parent)
{
  d = new KDateTablePrivate;
  setFontSize(10);
  setFocusPolicy( Qt::StrongFocus );
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  QPalette palette;
  palette.setColor(viewport()->backgroundRole(), KGlobalSettings::baseColor());
  viewport()->setPalette(palette);

  if(!date_.isValid())
  {
    kDebug() << "KDateTable ctor: WARNING: Given date is invalid, using current date." << endl;
    setDate(QDate::currentDate()); // this initializes firstday, numdays, numDaysPrevMonth
  }
  else
    setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth
  initAccels();
}

KDateTable::KDateTable(QWidget *parent)
  : Q3GridView(parent)
{
  d = new KDateTablePrivate;
  setFontSize(10);
  setFocusPolicy( Qt::StrongFocus );
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  QPalette palette;
  palette.setColor(viewport()->backgroundRole(), KGlobalSettings::baseColor());
  viewport()->setPalette(palette);
  setDate(QDate::currentDate()); // this initializes firstday, numdays, numDaysPrevMonth
  initAccels();
}

KDateTable::~KDateTable()
{
  delete d;
}

void KDateTable::initAccels()
{
  KActionCollection* localCollection = new KActionCollection(this);
  localCollection->setAssociatedWidget(this);

  KAction* next = new KAction(localCollection, 0);
  next->setShortcut(KStandardShortcut::next());
  connect(next, SIGNAL(triggered(bool)), SLOT(nextMonth()));

  KAction* prior = new KAction(localCollection, 0);
  prior->setShortcut(KStandardShortcut::prior());
  connect(prior, SIGNAL(triggered(bool)), SLOT(previousMonth()));

  KAction* beginMonth = new KAction(localCollection, 0);
  beginMonth->setShortcut(KStandardShortcut::home());
  connect(beginMonth, SIGNAL(triggered(bool)), SLOT(beginningOfMonth()));

  KAction* endMonth = new KAction(localCollection, 0);
  endMonth->setShortcut(KStandardShortcut::end());
  connect(endMonth, SIGNAL(triggered(bool)), SLOT(endOfMonth()));

  KAction* beginWeek = new KAction(localCollection, 0);
  beginWeek->setShortcut(KStandardShortcut::beginningOfLine());
  connect(beginWeek, SIGNAL(triggered(bool)), SLOT(beginningOfWeek()));

  KAction* endWeek = new KAction(localCollection, 0);
  endWeek->setShortcut(KStandardShortcut::endOfLine());
  connect(endWeek, SIGNAL(triggered(bool)), SLOT(endOfWeek()));

  localCollection->readSettings();
}

int KDateTable::posFromDate( const QDate &dt )
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  const int firstWeekDay = KGlobal::locale()->weekStartDay();
  int pos = calendar->day( dt );
  int offset = (firstday - firstWeekDay + 7) % 7;
  // make sure at least one day of the previous month is visible.
  // adjust this <1 if more days should be forced visible:
  if ( offset < 1 ) offset += 7;
  return pos + offset;
}

QDate KDateTable::dateFromPos( int pos )
{
  QDate pCellDate;
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  calendar->setYMD(pCellDate, calendar->year(mDate), calendar->month(mDate), 1);

  int firstWeekDay = KGlobal::locale()->weekStartDay();
  int offset = (firstday - firstWeekDay + 7) % 7;
  // make sure at least one day of the previous month is visible.
  // adjust this <1 if more days should be forced visible:
  if ( offset < 1 ) offset += 7;
  pCellDate = calendar->addDays( pCellDate, pos - offset );
  return pCellDate;
}

void
KDateTable::paintCell(QPainter *painter, int row, int col)
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  QRect rect;
  QString text;
  QPen pen;
  int w=cellWidth();
  int h=cellHeight();
  QFont font=KGlobalSettings::generalFont();
  // -----

  if(row == 0)
    { // we are drawing the headline
      font.setBold(true);
      painter->setFont(font);
      bool normalday = true;
      int firstWeekDay = KGlobal::locale()->weekStartDay();
      int daynum = ( col+firstWeekDay < 8 ) ? col+firstWeekDay :
                                              col+firstWeekDay-7;
      if ( daynum == calendar->weekDayOfPray() ||
         ( daynum == 6 && calendar->calendarName() == "gregorian" ) )
          normalday=false;

      QBrush brushInvertTitle(palette().base());
      QColor titleColor(isEnabled()?( KGlobalSettings::activeTitleColor() ):( KGlobalSettings::inactiveTitleColor() ) );
      QColor textColor(isEnabled()?( KGlobalSettings::activeTextColor() ):( KGlobalSettings::inactiveTextColor() ) );

      if (!normalday)
        {
          painter->setPen(textColor);
          painter->setBrush(textColor);
          painter->drawRect(0, 0, w, h);
          painter->setPen(titleColor);
        } else {
          painter->setPen(titleColor);
          painter->setBrush(titleColor);
          painter->drawRect(0, 0, w, h);
          painter->setPen(textColor);
        }
      painter->drawText(0, 0, w, h-1, Qt::AlignCenter,
                        calendar->weekDayName(daynum, true), &rect);
      painter->setPen(palette().color(QPalette::Text));
      painter->drawLine(0, h-1, w-1, h-1);
      // ----- draw the weekday:
    } else {
      bool paintRect=true;
      painter->setFont(font);
      int pos=7*(row-1)+col;

      QDate pCellDate = dateFromPos( pos );
      // First day of month
      text = calendar->dayString(pCellDate, true);
      if( calendar->month(pCellDate) != calendar->month(mDate) )
        { // we are either
          // ° painting a day of the previous month or
          // ° painting a day of the following month
          // TODO: don't hardcode gray here! Use a color with less contrast to the background than normal text.
          painter->setPen( palette().color(QPalette::Mid) );
//          painter->setPen(gray);
        } else { // paint a day of the current month
          if ( d->useCustomColors )
          {
            KDateTablePrivate::DatePaintingMode *mode=d->customPaintingModes[pCellDate.toString()];
            if (mode)
            {
              if (mode->bgMode != NoBgMode)
              {
                QBrush oldbrush=painter->brush();
                painter->setBrush( mode->bgColor );
                switch(mode->bgMode)
                {
                  case(CircleMode) : painter->drawEllipse(0,0,w,h);break;
                  case(RectangleMode) : painter->drawRect(0,0,w,h);break;
                  case(NoBgMode) : // Should never be here, but just to get one
                                   // less warning when compiling
                  default: break;
                }
                painter->setBrush( oldbrush );
                paintRect=false;
              }
              painter->setPen( mode->fgColor );
            } else
              painter->setPen(palette().color(QPalette::Text));
          } else //if ( firstWeekDay < 4 ) // <- this doesn' make sense at all!
          painter->setPen(palette().color(QPalette::Text));
        }

      pen=painter->pen();
      int firstWeekDay=KGlobal::locale()->weekStartDay();
      int offset=firstday-firstWeekDay;
      if(offset<1)
        offset+=7;
      int d = calendar->day(mDate);
           if( (offset+d) == (pos+1))
        {
           // draw the currently selected date
	   if (isEnabled())
	   {
           painter->setPen(palette().color(QPalette::Highlight));
           painter->setBrush(palette().color(QPalette::Highlight));
	   }
	   else
	   {
	   painter->setPen(palette().color(QPalette::Text));
           painter->setBrush(palette().color(QPalette::Text));
	   }
           pen=palette().color(QPalette::HighlightedText);
        } else {
          painter->setBrush(palette().color(QPalette::Background));
          painter->setPen(palette().color(QPalette::Background));
//          painter->setBrush(palette().base());
//          painter->setPen(palette().base());
        }

      if ( pCellDate == QDate::currentDate() )
      {
         painter->setPen(palette().color(QPalette::Text));
      }

      if ( paintRect ) painter->drawRect(0, 0, w, h);
      painter->setPen(pen);
      painter->drawText(0, 0, w, h, Qt::AlignCenter, text, &rect);
    }
  if(rect.width()>maxCell.width()) maxCell.setWidth(rect.width());
  if(rect.height()>maxCell.height()) maxCell.setHeight(rect.height());
}

void KDateTable::nextMonth()
{
    const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  setDate(calendar->addMonths( mDate, 1 ));
}

void KDateTable::previousMonth()
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();
  setDate(calendar->addMonths( mDate, -1 ));
}

void KDateTable::beginningOfMonth()
{
  setDate(mDate.addDays(1 - mDate.day()));
}

void KDateTable::endOfMonth()
{
  setDate(mDate.addDays(mDate.daysInMonth() - mDate.day()));
}

void KDateTable::beginningOfWeek()
{
  setDate(mDate.addDays(1 - mDate.dayOfWeek()));
}

void KDateTable::endOfWeek()
{
  setDate(mDate.addDays(7 - mDate.dayOfWeek()));
}

void
KDateTable::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() ) {
    case Qt::Key_Up:
            setDate(mDate.addDays(-7));
        break;
    case Qt::Key_Down:
            setDate(mDate.addDays(7));
        break;
    case Qt::Key_Left:
            setDate(mDate.addDays(-1));
        break;
    case Qt::Key_Right:
            setDate(mDate.addDays(1));
        break;
    case Qt::Key_Minus:
        setDate(mDate.addDays(-1));
	break;
    case Qt::Key_Plus:
        setDate(mDate.addDays(1));
	break;
    case Qt::Key_N:
        setDate(QDate::currentDate());
	break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        emit tableClicked();
        break;
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Shift:
      // Don't beep for modifiers
      break;
    default:
      if (!e->modifiers()) { // hm
    KNotification::beep();
}
    }
}

void
KDateTable::viewportResizeEvent(QResizeEvent * e)
{
  Q3GridView::viewportResizeEvent(e);

  setCellWidth(viewport()->width()/7);
  setCellHeight(viewport()->height()/7);
}

void
KDateTable::setFontSize(int size)
{
  int count;
  QFontMetrics metrics(fontMetrics());
  QRect rect;
  // ----- store rectangles:
  fontsize=size;
  // ----- find largest day name:
  maxCell.setWidth(0);
  maxCell.setHeight(0);
  for(count=0; count<7; ++count)
    {
      rect=metrics.boundingRect(KGlobal::locale()->calendar()
                                ->weekDayName(count+1, true));
      maxCell.setWidth(qMax(maxCell.width(), rect.width()));
      maxCell.setHeight(qMax(maxCell.height(), rect.height()));
    }
  // ----- compare with a real wide number and add some space:
  rect=metrics.boundingRect(QLatin1String("88"));
  maxCell.setWidth(qMax(maxCell.width()+2, rect.width()));
  maxCell.setHeight(qMax(maxCell.height()+4, rect.height()));
}

void
KDateTable::wheelEvent ( QWheelEvent * e )
{
    setDate(mDate.addMonths( -(int)(e->delta()/120)) );
    e->accept();
}

void
KDateTable::contentsMousePressEvent(QMouseEvent *e)
{

  if(e->type()!=QEvent::MouseButtonPress)
    { // the KDatePicker only reacts on mouse press events:
      return;
    }
  if(!isEnabled())
    {
      KNotification::beep();
      return;
    }

  // -----
  int row, col, pos, temp;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());
  if(row<1 || col<0)
    { // the user clicked on the frame of the table
      return;
    }

  // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
  // the row with the days of the week in the calculation.

  // old selected date:
  temp = posFromDate( mDate );
  // new position and date
  pos = (7 * (row - 1)) + col;
  QDate clickedDate = dateFromPos( pos );

  // set the new date. If it is in the previous or next month, the month will
  // automatically be changed, no need to do that manually...
  setDate( clickedDate );

  // call updateCell on the old and new selection. If setDate switched to a different
  // month, these cells will be painted twice, but that's no problem.
  updateCell( temp/7+1, temp%7 );
  updateCell( row, col );

  emit tableClicked();

  if (  e->button() == Qt::RightButton && d->popupMenuEnabled )
  {
        KMenu *menu = new KMenu();
        menu->addTitle( KGlobal::locale()->formatDate(clickedDate) );
        emit aboutToShowContextMenu( menu, clickedDate );
        menu->popup(e->globalPos());
  }
}

bool
KDateTable::setDate(const QDate& date_)
{
  bool changed=false;
  QDate temp;
  // -----
  if(!date_.isValid())
    {
      kDebug() << "KDateTable::setDate: refusing to set invalid date." << endl;
      return false;
    }
  if(mDate!=date_)
    {
      emit(dateChanged(mDate, date_));
      mDate=date_;
      emit(dateChanged(mDate));
      changed=true;
    }
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  calendar->setYMD(temp, calendar->year(mDate), calendar->month(mDate), 1);
  //temp.setYMD(mDate.year(), mDate.month(), 1);
  //kDebug() << "firstDayInWeek: " << temp.toString() << endl;
  firstday=temp.dayOfWeek();
  numdays=calendar->daysInMonth(mDate);

  temp = calendar->addMonths(temp, -1);
  numDaysPrevMonth=calendar->daysInMonth(temp);
  if(changed)
    {
      repaintContents(false);
    }
  return true;
}

const QDate&
KDateTable::date() const
{
  return mDate;
}

// what are those repaintContents() good for? (pfeiffer)
void KDateTable::focusInEvent( QFocusEvent *e )
{
//    repaintContents(false);
    Q3GridView::focusInEvent( e );
}

void KDateTable::focusOutEvent( QFocusEvent *e )
{
//    repaintContents(false);
    Q3GridView::focusOutEvent( e );
}

QSize
KDateTable::sizeHint() const
{
  if(maxCell.height()>0 && maxCell.width()>0)
    {
      return QSize(maxCell.width()*numCols()+2*frameWidth(),
             (maxCell.height()+2)*numRows()+2*frameWidth());
    } else {
      kDebug() << "KDateTable::sizeHint: obscure failure - " << endl;
      return QSize(-1, -1);
    }
}

void KDateTable::setPopupMenuEnabled( bool enable )
{
   d->popupMenuEnabled=enable;
}

bool KDateTable::popupMenuEnabled() const
{
   return d->popupMenuEnabled;
}

void KDateTable::setCustomDatePainting(const QDate &date, const QColor &fgColor, BackgroundMode bgMode, const QColor &bgColor)
{
    if (!fgColor.isValid())
    {
        unsetCustomDatePainting( date );
        return;
    }

    KDateTablePrivate::DatePaintingMode *mode=new KDateTablePrivate::DatePaintingMode;
    mode->bgMode=bgMode;
    mode->fgColor=fgColor;
    mode->bgColor=bgColor;

    d->customPaintingModes.insert( date.toString(), mode );
    d->useCustomColors=true;
    update();
}

void KDateTable::unsetCustomDatePainting( const QDate &date )
{
    d->customPaintingModes.remove( date.toString() );
}

KDateInternalWeekSelector::KDateInternalWeekSelector
(QWidget* parent)
  : QLineEdit(parent),
    val(new QIntValidator(this)),
    result(0)
{
  QFont font;
  // -----
  font=KGlobalSettings::generalFont();
  setFont(font);
  setFrame(false);
  setValidator(val);
  connect(this, SIGNAL(returnPressed()), SLOT(weekEnteredSlot()));
}

void
KDateInternalWeekSelector::weekEnteredSlot()
{
  bool ok;
  int week;
  // ----- check if this is a valid week:
  week=text().toInt(&ok);
  if(!ok)
    {
      KNotification::beep();
      return;
    }
  result=week;
  emit(closeMe(1));
}

int
KDateInternalWeekSelector::getWeek()
{
  return result;
}

void
KDateInternalWeekSelector::setWeek(int week)
{
  QString temp;
  // -----
  temp.setNum(week);
  setText(temp);
}

void
KDateInternalWeekSelector::setMaxWeek(int max)
{
  val->setRange(1, max);
}

// ### CFM To avoid binary incompatibility.
//     In future releases, remove this and replace by  a QDate
//     private member, needed in KDateInternalMonthPicker::paintCell
class KDateInternalMonthPicker::KDateInternalMonthPrivate {
public:
        KDateInternalMonthPrivate (int y, int m, int d)
        : year(y), month(m), day(d)
        {};
        int year;
        int month;
        int day;
};

KDateInternalMonthPicker::~KDateInternalMonthPicker() {
   delete d;
}

KDateInternalMonthPicker::KDateInternalMonthPicker
(const QDate & date, QWidget* parent)
  : Q3GridView(parent),
    result(0) // invalid
{
  QRect rect;
  QFont font;
  // -----
  activeCol = -1;
  activeRow = -1;
  font=KGlobalSettings::generalFont();
  setFont(font);
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  setFrameStyle(QFrame::NoFrame);
  setNumCols(3);
  d = new KDateInternalMonthPrivate(date.year(), date.month(), date.day());
  // For monthsInYear != 12
  setNumRows( (KGlobal::locale()->calendar()->monthsInYear(date) + 2) / 3);
  // enable to find drawing failures:
  // setTableFlags(Tbl_clipCellPainting);
  QPalette palette;
  palette.setColor(viewport()->backgroundRole(), KGlobalSettings::baseColor());
  viewport()->setPalette(palette);
  // ----- find the preferred size
  //       (this is slow, possibly, but unfortunately it is needed here):
  QFontMetrics metrics(font);
  for(int i = 1; ; ++i)
    {
      QString str = KGlobal::locale()->calendar()->monthName(i,
         KGlobal::locale()->calendar()->year(date), false);
      if (str.isNull()) break;
      rect=metrics.boundingRect(str);
      if(max.width()<rect.width()) max.setWidth(rect.width());
      if(max.height()<rect.height()) max.setHeight(rect.height());
    }
}

QSize
KDateInternalMonthPicker::sizeHint() const
{
  return QSize((max.width()+6)*numCols()+2*frameWidth(),
         (max.height()+6)*numRows()+2*frameWidth());
}

int
KDateInternalMonthPicker::getResult() const
{
  return result;
}

void
KDateInternalMonthPicker::setupPainter(QPainter *p)
{
  p->setPen(KGlobalSettings::textColor());
}

void
KDateInternalMonthPicker::viewportResizeEvent(QResizeEvent*)
{
  setCellWidth(width() / numCols());
  setCellHeight(height() / numRows());
}

void
KDateInternalMonthPicker::paintCell(QPainter* painter, int row, int col)
{
  int index;
  QString text;
  // ----- find the number of the cell:
  index=3*row+col+1;
  text=KGlobal::locale()->calendar()->monthName(index,
    KGlobal::locale()->calendar()->year(QDate(d->year, d->month,
    d->day)), false);
  painter->drawText(0, 0, cellWidth(), cellHeight(), Qt::AlignCenter, text);
  if ( activeCol == col && activeRow == row )
      painter->drawRect( 0, 0, cellWidth(), cellHeight() );
}

void
KDateInternalMonthPicker::contentsMousePressEvent(QMouseEvent *e)
{
  if(!isEnabled() || e->button() != Qt::LeftButton)
    {
      KNotification::beep();
      return;
    }
  // -----
  int row, col;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());

  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      activeCol = -1;
      activeRow = -1;
    } else {
      activeCol = col;
      activeRow = row;
      updateCell( row, col /*, false */ );
  }
}

void
KDateInternalMonthPicker::contentsMouseMoveEvent(QMouseEvent *e)
{
  if (e->modifiers() & Qt::LeftButton)
    {
      int row, col;
      QPoint mouseCoord;
      // -----
      mouseCoord = e->pos();
      row=rowAt(mouseCoord.y());
      col=columnAt(mouseCoord.x());
      int tmpRow = -1, tmpCol = -1;
      if(row<0 || col<0)
        { // the user clicked on the frame of the table
          if ( activeCol > -1 )
            {
              tmpRow = activeRow;
              tmpCol = activeCol;
            }
          activeCol = -1;
          activeRow = -1;
        } else {
          bool differentCell = (activeRow != row || activeCol != col);
          if ( activeCol > -1 && differentCell)
            {
              tmpRow = activeRow;
              tmpCol = activeCol;
            }
          if ( differentCell)
            {
              activeRow = row;
              activeCol = col;
              updateCell( row, col /*, false */ ); // mark the new active cell
            }
        }
      if ( tmpRow > -1 ) // repaint the former active cell
          updateCell( tmpRow, tmpCol /*, true */ );
    }
}

void
KDateInternalMonthPicker::contentsMouseReleaseEvent(QMouseEvent *e)
{
  if(!isEnabled())
    {
      return;
    }
  // -----
  int row, col, pos;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());
  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      emit(closeMe(0));
    }

  pos=3*row+col+1;
  result=pos;
  emit(closeMe(1));
}



KDateInternalYearSelector::KDateInternalYearSelector
(QWidget* parent)
  : QLineEdit(parent),
    val(new QIntValidator(this)),
    result(0)
{
  QFont font;
  // -----
  font=KGlobalSettings::generalFont();
  setFont(font);
  setFrame(false);
  // we have to respect the limits of QDate here, I fear:
  val->setRange(0, 8000);
  setValidator(val);
  connect(this, SIGNAL(returnPressed()), SLOT(yearEnteredSlot()));
}

void
KDateInternalYearSelector::yearEnteredSlot()
{
  bool ok;
  int year;
  QDate date;
  // ----- check if this is a valid year:
  year=text().toInt(&ok);
  if(!ok)
    {
      KNotification::beep();
      return;
    }
  //date.setYMD(year, 1, 1);
  KGlobal::locale()->calendar()->setYMD(date, year, 1, 1);
  if(!date.isValid())
    {
      KNotification::beep();
      return;
    }
  result=year;
  emit(closeMe(1));
}

int
KDateInternalYearSelector::getYear()
{
  return result;
}

void
KDateInternalYearSelector::setYear(int year)
{
  QString temp;
  // -----
  temp.setNum(year);
  setText(temp);
}

KPopupFrame::KPopupFrame(QWidget* parent)
  : QFrame(parent, Qt::Popup),
    result(0), // rejected
    main(0)
{
  setFrameStyle(QFrame::Box|QFrame::Raised);
  setMidLineWidth(2);
}

void
KPopupFrame::keyPressEvent(QKeyEvent* e)
{
  if(e->key()==Qt::Key_Escape)
    {
      result=0; // rejected
      emit leaveModality();
      //qApp->exit_loop();
    }
}

void
KPopupFrame::close(int r)
{
  result=r;
  emit leaveModality();
  //qApp->exit_loop();
}

void
KPopupFrame::setMainWidget(QWidget* m)
{
  main=m;
  if(main)
    {
      resize(main->width()+2*frameWidth(), main->height()+2*frameWidth());
    }
}

void
KPopupFrame::resizeEvent(QResizeEvent*)
{
  if(main)
    {
      main->setGeometry(frameWidth(), frameWidth(),
          width()-2*frameWidth(), height()-2*frameWidth());
    }
}

void
KPopupFrame::popup(const QPoint &pos)
{
  // Make sure the whole popup is visible.
  QRect d = KGlobalSettings::desktopGeometry(pos);

  int x = pos.x();
  int y = pos.y();
  int w = width();
  int h = height();
  if (x+w > d.x()+d.width())
    x = d.width() - w;
  if (y+h > d.y()+d.height())
    y = d.height() - h;
  if (x < d.x())
    x = 0;
  if (y < d.y())
    y = 0;

  // Pop the thingy up.
  move(x, y);
  show();
}

int
KPopupFrame::exec(const QPoint &pos)
{
  popup(pos);
  repaint();
  QEventLoop eventLoop;
  connect(this, SIGNAL(leaveModality()),
          &eventLoop, SLOT(quit()));
  eventLoop.exec();

  hide();
  return result;
}

int
KPopupFrame::exec(int x, int y)
{
  return exec(QPoint(x, y));
}

#include "kdatetable.moc"
