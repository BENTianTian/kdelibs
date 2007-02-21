/*  This file is part of the KDE libraries
    Copyright (C) 2001 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdatewidget.h"

#include <qcombobox.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include "kcalendarsystem.h"
#include "kdialog.h"
#include "kglobal.h"
#include "klocale.h"


class KDateWidgetSpinBox : public QSpinBox
{
public:
  KDateWidgetSpinBox(int min, int max, QWidget *parent)
    : QSpinBox(parent)
  {
     setRange(qMin(min, max), qMax(min, max));
     setSingleStep(1);
     lineEdit()->setAlignment(Qt::AlignRight);
  }
};

class KDateWidget::KDateWidgetPrivate
{
public:
   KDateWidgetSpinBox *m_day;
   QComboBox *m_month;
   KDateWidgetSpinBox *m_year;
   QDate m_dat;
};


KDateWidget::KDateWidget( QWidget *parent )
  : QWidget( parent )
{
  init(QDate());
  setDate(QDate());
}

KDateWidget::KDateWidget( const QDate &date, QWidget *parent )
  : QWidget( parent )
{
  init(date);
  setDate(date);
}

void KDateWidget::init(const QDate& date)
{
  d = new KDateWidgetPrivate;
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(KDialog::spacingHint());
  d->m_day = new KDateWidgetSpinBox(1, 1, this);
  d->m_month = new QComboBox(this);
  const KCalendarSystem* calendar = KGlobal::locale()->calendar();
  for (int i = 1; ; ++i)
  {
    const QString str = calendar->monthName(i, calendar->year(date));
    if (str.isEmpty())
        break;
    d->m_month->addItem(str);
  }

  d->m_year = new KDateWidgetSpinBox(calendar->minValidYear(),
                                     calendar->maxValidYear(), this);
  layout->addWidget(d->m_day);
  layout->addWidget(d->m_month);
  layout->addWidget(d->m_year);
  
  connect(d->m_day, SIGNAL(valueChanged(int)), this, SLOT(slotDateChanged()));
  connect(d->m_month, SIGNAL(activated(int)), this, SLOT(slotDateChanged()));
  connect(d->m_year, SIGNAL(valueChanged(int)), this, SLOT(slotDateChanged()));
}

KDateWidget::~KDateWidget()
{
  delete d;
}

void KDateWidget::setDate( const QDate &date )
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  bool dayBlocked = d->m_day->blockSignals(true);
  bool monthBlocked = d->m_month->blockSignals(true);
  bool yearBlocked = d->m_year->blockSignals(true);

  d->m_day->setMaximum(calendar->daysInMonth(date));
  d->m_day->setValue(calendar->day(date));
  d->m_month->setCurrentIndex(calendar->month(date)-1);
  d->m_year->setValue(calendar->year(date));

  d->m_day->blockSignals(dayBlocked);
  d->m_month->blockSignals(monthBlocked);
  d->m_year->blockSignals(yearBlocked);

  d->m_dat = date;
  emit changed(d->m_dat);
}

const QDate& KDateWidget::date() const
{
  return d->m_dat;
}

void KDateWidget::slotDateChanged( )
{
  const KCalendarSystem * calendar = KGlobal::locale()->calendar();

  QDate date;
  int y,m,day;

  y = d->m_year->value();
  y = qMin(qMax(y, calendar->minValidYear()), calendar->maxValidYear());

  calendar->setYMD(date, y, 1, 1);
  m = d->m_month->currentIndex()+1;
  m = qMin(qMax(m,1), calendar->monthsInYear(date));

  calendar->setYMD(date, y, m, 1);
  day = d->m_day->value();
  day = qMin(qMax(day,1), calendar->daysInMonth(date));

  calendar->setYMD(date, y, m, day);
  setDate(date);
}


#include "kdatewidget.moc"
