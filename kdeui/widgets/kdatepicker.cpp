/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
              (C) 2007 John Layt <john@layt.net>
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

#include "kdatepicker.h"
#include "kdatepicker_p.h"
#include "kdatetable.h"

#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QFont>
#include <QtGui/QLayout>
#include <QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QToolButton>
#include <QtGui/QDoubleValidator>

#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knotification.h>

#include "kdatepicker.moc"
#include "kdatepicker_p.moc"

// Week numbers are defined by ISO 8601
// See http://www.merlyn.demon.co.uk/weekinfo.htm for details


KDatePickerPrivateYearSelector::KDatePickerPrivateYearSelector(
                                const KCalendarSystem *cal, const QDate &currentDate, QWidget* parent )
                              : QLineEdit( parent ), val( new QIntValidator( this ) ), result( 0 )
{
    calendar = cal;
    oldDate = currentDate;

    QFont font;
    font = KGlobalSettings::generalFont();
    setFont( font );

    setFrame( false );

    val->setRange( calendar->year( calendar->earliestValidDate() ),
                   calendar->year( calendar->latestValidDate() ) );
    setValidator( val );

    connect( this, SIGNAL( returnPressed() ), SLOT( yearEnteredSlot() ) );
}

void KDatePickerPrivateYearSelector::yearEnteredSlot()
{
    bool ok;
    int newYear;
    QDate newDate;

    // check if entered value is a number
    newYear = text().toInt( &ok );
    if( !ok ) {
        KNotification::beep();
        return;
    }

    // check if new year will lead to a valid date
    if ( calendar->setYMD( newDate, newYear, calendar->month( oldDate ), calendar->day( oldDate ) ) ) {
        result = newYear;
        emit( closeMe( 1 ) );
    } else {
        KNotification::beep();
    }

}

int KDatePickerPrivateYearSelector::year()
{
    return result;
}

bool KDatePickerPrivateYearSelector::setYear( int year )
{
    QString temp;
    temp.setNum( year );
    setText( temp );
}

class KDatePicker::KDatePickerPrivate
{
public:
    KDatePickerPrivate( KDatePicker *q ) :
             q( q ), closeButton( 0L ), selectWeek( 0L ), todayButton( 0 ), navigationLayout( 0 )
    {
    }

    void fillWeeksCombo();
    QDate validDateInYearMonth( int year, int month );

    QToolButton *closeButton;
    QComboBox *selectWeek;
    QToolButton *todayButton;
    QBoxLayout *navigationLayout;

    /// the year forward button
    QToolButton *yearForward;
    /// the year backward button
    QToolButton *yearBackward;
    /// the month forward button
    QToolButton *monthForward;
    /// the month backward button
    QToolButton *monthBackward;
    /// the button for selecting the month directly
    QToolButton *selectMonth;
    /// the button for selecting the year directly
    QToolButton *selectYear;
    /// the line edit to enter the date directly
    QLineEdit *line;
    /// the validator for the line edit:
    KDateValidator *val;
    /// the date table
    KDateTable *table;
    /// the date table
    KDatePicker *q;
    /// the widest month string in pixels:
    QSize maxMonthRect;

    /// the font size for the widget
    int fontsize;
};

void KDatePicker::KDatePickerPrivate::fillWeeksCombo()
{
    // every year can have a different number of weeks
    // it could be that we had 53,1..52 and now 1..53 which is the same number but different
    // so always fill with new values
    // We show all week numbers for all weeks between first day of year to last day of year
    // This of course can be a list like 53,1,2..52

    int thisYear = q->calendar()->year( q->date() );

    // Note we need this roundabout way to calculate lastDayOfYear instead of simply taking
    // 1st day of next year - 1 day, as next year may not be a valid date
    // JPL still leaves open what to do if we start or end part way through year.
    QDate lastDayOfYear;
    q->calendar()->setYMD( lastDayOfYear, thisYear, q->calendar()->monthsInYear( q->date() ), 1 );
    q->calendar()->setYMD( lastDayOfYear, thisYear, q->calendar()->monthsInYear( q->date() ),
                           q->calendar()->daysInMonth( lastDayOfYear ) );

    // JPL this won't always be a valid date!
    QDate day;
    q->calendar()->setYMD( day, thisYear, 1, 1 );

    selectWeek->clear();

    for ( ; day <= lastDayOfYear ; day = q->calendar()->addDays( day, q->calendar()->daysInWeek( day ) ) ) {
        QString week = i18n( "Week %1", q->calendar()->weekNumber( day, &thisYear ) );

        // show that this is a week from a different year
        if ( q->calendar()->year( day ) != thisYear ) {
            week += "*";
        }

        selectWeek->addItem( week );

        // make sure that the week of the lastDayOfYear is always inserted: in Chinese calendar
        // system, this is not always the case
        if ( day < lastDayOfYear &&
             day.daysTo( lastDayOfYear ) < q->calendar()->daysInWeek( day ) &&
             q->calendar()->weekNumber( lastDayOfYear ) != q->calendar()->weekNumber( day ) ) {
            day = q->calendar()->addDays( lastDayOfYear, - q->calendar()->daysInWeek( q->date() ) );
        }
    }
}

QDate KDatePicker::KDatePickerPrivate::validDateInYearMonth( int year, int month )
{
    QDate newDate;

    // Try to create a valid date in this year and month
    // First try the first of the month, then try last of month
    if ( q->calendar()->isValid( year, month, 1 ) ) {
        q->calendar()->setYMD( newDate, year, month, 1 );
    } else if ( q->calendar()->isValid( year, month + 1, 1 ) ) {
        q->calendar()->setYMD( newDate, year, month, 1 );
        q->calendar()->addDays( newDate, -1 );
    } else {
        newDate = QDate::fromJulianDay( 0 );
    }

    return newDate;
}

KDatePicker::KDatePicker( QWidget* parent ) : QFrame( parent ), d( new KDatePickerPrivate( this ) )
{
    init( QDate::currentDate() );
}

KDatePicker::KDatePicker( const QDate& date_, QWidget* parent )
            : QFrame( parent ), d( new KDatePickerPrivate( this ) )
{
    init( date_ );
}

void KDatePicker::init( const QDate &date_ )
{

    QBoxLayout * topLayout = new QVBoxLayout( this );
    topLayout->setSpacing( 0 );
    topLayout->setMargin( 0 );

    d->navigationLayout = new QHBoxLayout();
    d->navigationLayout->setSpacing( 0 );
    d->navigationLayout->setMargin( 0 );
    topLayout->addLayout( d->navigationLayout );
    d->navigationLayout->addStretch();
    d->yearBackward = new QToolButton( this );
    d->yearBackward->setAutoRaise( true );
    d->navigationLayout->addWidget( d->yearBackward );
    d->monthBackward = new QToolButton( this );
    d->monthBackward ->setAutoRaise( true );
    d->navigationLayout->addWidget( d->monthBackward );
    d->navigationLayout->addSpacing( KDialog::spacingHint() );

    d->selectMonth = new QToolButton( this );
    d->selectMonth ->setAutoRaise( true );
    d->navigationLayout->addWidget( d->selectMonth );
    d->selectYear = new QToolButton( this );
    d->selectYear->setCheckable( true );
    d->selectYear->setAutoRaise( true );
    d->navigationLayout->addWidget( d->selectYear );
    d->navigationLayout->addSpacing( KDialog::spacingHint() );

    d->monthForward = new QToolButton( this );
    d->monthForward ->setAutoRaise( true );
    d->navigationLayout->addWidget( d->monthForward );
    d->yearForward = new QToolButton( this );
    d->yearForward ->setAutoRaise( true );
    d->navigationLayout->addWidget( d->yearForward );
    d->navigationLayout->addStretch();

    d->line = new KLineEdit( this );
    d->val = new KDateValidator( this );
    d->table = new KDateTable( this );
    d->fontsize = KGlobalSettings::generalFont().pointSize();
    if ( d->fontsize == -1 ) {
        d->fontsize = QFontInfo( KGlobalSettings::generalFont() ).pointSize();
    }

    d->fontsize++; // Make a little bigger

    d->selectWeek = new QComboBox( this );  // read only week selection
    d->todayButton = new QToolButton( this );
    d->todayButton->setIcon( KIcon( "calendar-today" ) );

    d->yearForward->setToolTip( i18n( "Next year" ) );
    d->yearBackward->setToolTip( i18n( "Previous year" ) );
    d->monthForward->setToolTip( i18n( "Next month" ) );
    d->monthBackward->setToolTip( i18n( "Previous month" ) );
    d->selectWeek->setToolTip( i18n( "Select a week" ) );
    d->selectMonth->setToolTip( i18n( "Select a month" ) );
    d->selectYear->setToolTip( i18n( "Select a year" ) );
    d->todayButton->setToolTip( i18n( "Select the current day" ) );

    // -----
    setFontSize( d->fontsize );
    d->line->setValidator( d->val );
    d->line->installEventFilter( this );
    if ( QApplication::isRightToLeft() ) {
        d->yearForward->setIcon( KIcon( QLatin1String( "arrow-left-double" ) ) );
        d->yearBackward->setIcon( KIcon( QLatin1String( "arrow-right-double" ) ) );
        d->monthForward->setIcon( KIcon( QLatin1String( "arrow-left" ) ) );
        d->monthBackward->setIcon( KIcon( QLatin1String( "arrow-right" ) ) );
    } else {
        d->yearForward->setIcon( KIcon( QLatin1String( "arrow-right-double" ) ) );
        d->yearBackward->setIcon( KIcon( QLatin1String( "arrow-left-double" ) ) );
        d->monthForward->setIcon( KIcon( QLatin1String( "arrow-right" ) ) );
        d->monthBackward->setIcon( KIcon( QLatin1String( "arrow-left" ) ) );
    }

    connect( d->table, SIGNAL( dateChanged( const QDate& ) ), SLOT( dateChangedSlot( const QDate& ) ) );
    connect( d->table, SIGNAL( tableClicked() ), SLOT( tableClickedSlot() ) );
    connect( d->monthForward, SIGNAL( clicked() ), SLOT( monthForwardClicked() ) );
    connect( d->monthBackward, SIGNAL( clicked() ), SLOT( monthBackwardClicked() ) );
    connect( d->yearForward, SIGNAL( clicked() ), SLOT( yearForwardClicked() ) );
    connect( d->yearBackward, SIGNAL( clicked() ), SLOT( yearBackwardClicked() ) );
    connect( d->selectWeek, SIGNAL( activated( int ) ), SLOT( weekSelected( int ) ) );
    connect( d->todayButton, SIGNAL( clicked() ), SLOT( todayButtonClicked() ) );
    connect( d->selectMonth, SIGNAL( clicked() ), SLOT( selectMonthClicked() ) );
    connect( d->selectYear, SIGNAL( toggled( bool ) ), SLOT( selectYearClicked() ) );
    connect( d->line, SIGNAL( returnPressed() ), SLOT( lineEnterPressed() ) );
    d->table->setFocus();


    topLayout->addWidget( d->table );

    QBoxLayout * bottomLayout = new QHBoxLayout();
    bottomLayout->setMargin( 0 );
    bottomLayout->setSpacing( 0 );
    topLayout->addLayout( bottomLayout );

    bottomLayout->addWidget( d->todayButton );
    bottomLayout->addWidget( d->line );
    bottomLayout->addWidget( d->selectWeek );

    d->table->setDate( date_ );
    dateChangedSlot( date_ );  // needed because table emits changed only when newDate != oldDate
}

KDatePicker::~KDatePicker()
{
    delete d;
}

bool KDatePicker::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent * k = ( QKeyEvent * )e;

        if ( ( k->key() == Qt::Key_PageUp ) ||
             ( k->key() == Qt::Key_PageDown ) ||
             ( k->key() == Qt::Key_Up ) ||
             ( k->key() == Qt::Key_Down ) ) {
            QApplication::sendEvent( d->table, e );
            d->table->setFocus();
            return true; // eat event
        }
    }
    return QFrame::eventFilter( o, e );
}

void KDatePicker::resizeEvent( QResizeEvent* e )
{
    QWidget::resizeEvent( e );
}

void KDatePicker::dateChangedSlot( const QDate &date_ )
{
    d->line->setText( calendar()->formatDate( date_, KLocale::ShortDate ) );
    d->selectMonth->setText( calendar()->monthName( date_, KCalendarSystem::LongName ) );
    d->fillWeeksCombo();

    // calculate the item num in the week combo box; normalize selected day so as if 1.1. is the first day of the week
    QDate firstDay;
    // JPL may not always be valid!
    calendar()->setYMD( firstDay, calendar()->year( date_ ), 1, 1 );
    d->selectWeek->setCurrentIndex( ( calendar()->dayOfYear( date_ ) + calendar()->dayOfWeek( firstDay ) - 2 ) /
                                    calendar()->daysInWeek( date_ ) );
    d->selectYear->setText( calendar()->yearString( date_, KCalendarSystem::LongFormat ) );

    emit( dateChanged( date_ ) );
}

void KDatePicker::tableClickedSlot()
{
    emit( dateSelected( date() ) );
    emit( tableClicked() );
}

const QDate & KDatePicker::date() const
{
    return d->table->date();
}

bool KDatePicker::setDate( const QDate &date_ )
{
    // the table setDate does validity checking for us
    // this also emits dateChanged() which then calls our dateChangedSlot()
    return d->table->setDate( date_ );
}

const KCalendarSystem *KDatePicker::calendar() const
{
    return  d->table->calendar();
}

bool KDatePicker::setCalendar( KCalendarSystem *calendar )
{
    return  d->table->setCalendar( calendar );
}

bool KDatePicker::setCalendar( const QString &calendarType )
{
    return  d->table->setCalendar( calendarType );
}

void KDatePicker::monthForwardClicked()
{
    if ( ! setDate( calendar()->addMonths( date(), 1 ) ) ) {
        KNotification::beep();
    }
}

void KDatePicker::monthBackwardClicked()
{
    if ( ! setDate( calendar()->addMonths( date(), -1 ) ) ) {
        KNotification::beep();
    }
}

void KDatePicker::yearForwardClicked()
{
    if ( ! setDate( calendar()->addYears( d->table->date(), 1 ) ) ) {
        KNotification::beep();
    }
}

void KDatePicker::yearBackwardClicked()
{
    if ( ! setDate( calendar()->addYears( d->table->date(), -1 ) ) ) {
        KNotification::beep();
    }
}

void KDatePicker::weekSelected( int week )
{
    QDate newDate;

    // First obtain a date that has weekday number of 1, either in this week or next week, whichevers valid
    newDate = calendar()->addDays( date(), 1 - calendar()->dayOfWeek( date() ) );
    if ( ! calendar()->isValid( newDate ) ) {
        newDate = calendar()->addDays( date(),
                  1 - calendar()->dayOfWeek( date() ) + calendar()->daysInWeek( date() ) );
    }

    // If we have a valid date, then add/subtract the number of days difference between the week numbers
    if ( calendar()->isValid( newDate ) ) {
        newDate = calendar()->addDays( newDate,
                 ( week - calendar()->weekNumber( newDate ) ) * calendar()->daysInWeek( date() ) );
    }

    if ( ! setDate( newDate ) ) {
        KNotification::beep();
    }
}

void KDatePicker::selectMonthClicked()
{
    QMenu popup( d->selectMonth );

    // Populate the pick list with all the month names, this may change by year
    // JPL do we need to do somethng here for months that fall outside valid range?
    for ( int m = 1; m <= calendar()->monthsInYear( date() ); m++ ) {
        popup.addAction( calendar()->monthName( m, calendar()->year( date() ) ) )->setData( m );
    }

    QAction *item = popup.actions()[ calendar()->month( date() ) - 1 ];
    // if this happens the above should already given an assertion
    if ( item ) {
        popup.setActiveAction( item );
    }

    // cancelled
    if ( ( item = popup.exec( d->selectMonth->mapToGlobal( QPoint( 0, 0 ) ), item ) ) == 0 ) {
        return;
    }

    // We need to create a valid date in the month selected so we can find out how many days are
    // in the month.
    QDate newDate = d->validDateInYearMonth( calendar()->year( date() ), item->data().toInt() );

    // If we have succeeded in creating a date in the new month, then try to create the new date,
    // checking we don't set a day after the last day of the month
    if ( calendar()->isValid( newDate ) ) {
        calendar()->setYMD( newDate,
                            calendar()->year( date() ),
                            item->data().toInt(),
                            qMin( calendar()->day( date() ), calendar()->daysInMonth( newDate ) ) );
    }

    // Set the date, if it's invalid in any way then alert user and don't update
    if ( ! setDate( newDate ) ) {
        KNotification::beep();
    }
}

void KDatePicker::selectYearClicked()
{
    QDate newDate;

    if ( !d->selectYear->isChecked() ) {
        return;
    }

    KPopupFrame *popup = new KPopupFrame( this );
    KDatePickerPrivateYearSelector *picker = new KDatePickerPrivateYearSelector( calendar(), date(), popup );
    picker->resize( picker->sizeHint() );
    picker->setYear( calendar()->year( date() ) );
    picker->selectAll();
    popup->setMainWidget( picker );
    connect( picker, SIGNAL( closeMe( int ) ), popup, SLOT( close( int ) ) );
    picker->setFocus();

    if( popup->exec( d->selectYear->mapToGlobal( QPoint( 0, d->selectMonth->height() ) ) ) ) {
        // We need to create a valid date in the year/month selected so we can find out how many
        // days are in the month.
        newDate = d->validDateInYearMonth( picker->year(), calendar()->month( date() ) );

        // If we have succeeded in creating a date in the new month, then try to create the new
        // date, checking we don't set a day after the last day of the month
        if ( calendar()->isValid( newDate ) ) {
            calendar()->setYMD( newDate,
                                picker->year(),
                                calendar()->month( date() ),
                                qMin( calendar()->day( date() ), calendar()->daysInMonth( newDate ) ) );
        }
    }

    // Set the date, if it's invalid in any way then alert user and don't update
    if ( ! setDate( newDate ) ) {
        KNotification::beep();
    }

    d->selectYear->setChecked( false );
    delete popup;
}


// ####### KDE4: setEnabled isn't virtual anymore, so this isn't polymorphic.
// Better reimplement changeEvent() instead.
void KDatePicker::setEnabled( bool enable )
{
    QWidget * widgets[] = {
                              d->yearForward, d->yearBackward, d->monthForward, d->monthBackward,
                              d->selectMonth, d->selectYear,
                              d->line, d->table, d->selectWeek, d->todayButton
                          };
    const int Size = sizeof( widgets ) / sizeof( widgets[0] );
    int count;

    for( count = 0; count < Size; ++count ) {
        widgets[count]->setEnabled( enable );
    }
}

KDateTable *KDatePicker::dateTable() const
{
    return d->table;
}

void KDatePicker::lineEnterPressed()
{
    QDate newDate = calendar()->readDate( d->line->text() );

    if ( calendar()->isValid( newDate ) ) {
        emit( dateEntered( newDate ) );
        setDate( newDate );
    } else {
        KNotification::beep();
    }
}

void KDatePicker::todayButtonClicked()
{
    setDate( QDate::currentDate() );
}

QSize KDatePicker::sizeHint() const
{
    return QWidget::sizeHint();
}

void KDatePicker::setFontSize( int s )
{
    QWidget * buttons[] = {
                              d->selectMonth,
                              d->selectYear,
                          };
    const int NoOfButtons = sizeof( buttons ) / sizeof( buttons[0] );
    int count;
    QFont font;
    QRect r;
    // -----
    d->fontsize = s;
    for( count = 0; count < NoOfButtons; ++count ) {
        font = buttons[count]->font();
        font.setPointSize( s );
        buttons[count]->setFont( font );
    }
    QFontMetrics metrics( d->selectMonth->fontMetrics() );

    for ( int i = 1; ; ++i ) {
        QString str = calendar()->monthName( i, calendar()->year( date() ), KCalendarSystem::LongName );
        if ( str.isNull() ) {
            break;
        }
        r = metrics.boundingRect( str );
        d->maxMonthRect.setWidth( qMax( r.width(), d->maxMonthRect.width() ) );
        d->maxMonthRect.setHeight( qMax( r.height(),  d->maxMonthRect.height() ) );
    }

    QStyleOptionToolButton opt;

    // stolen from KToolBarButton
    opt.init( this );
    opt.font      = d->selectMonth->font();
    opt.icon      = d->selectMonth->icon();
    opt.text      = d->selectMonth->text();
    opt.features  = d->selectMonth->menu() ? QStyleOptionToolButton::Menu : QStyleOptionToolButton::None; //### FIXME: delay?
    opt.subControls       = QStyle::SC_All;
    opt.activeSubControls = 0; //### FIXME: !!

    QSize metricBound = style()->sizeFromContents( QStyle::CT_ToolButton, &opt,
                                                   d->maxMonthRect, d->selectMonth );
    d->selectMonth->setMinimumSize( metricBound );

    d->table->setFontSize( s );
}

int KDatePicker::fontSize() const
{
    return d->fontsize;
}


void KDatePicker::setCloseButton( bool enable )
{
    if ( enable == ( d->closeButton != 0L ) ) {
        return;
    }

    if ( enable ) {
        d->closeButton = new QToolButton( this );
        d->closeButton->setAutoRaise( true );
        d->navigationLayout->addSpacing( KDialog::spacingHint() );
        d->navigationLayout->addWidget( d->closeButton );
        d->closeButton->setToolTip( i18n( "Close" ) );
        d->closeButton->setIcon( SmallIcon( "list-remove" ) );
        connect( d->closeButton, SIGNAL( clicked() ),
                 topLevelWidget(), SLOT( close() ) );
    } else {
        delete d->closeButton;
        d->closeButton = 0L;
    }

    updateGeometry();
}

bool KDatePicker::hasCloseButton() const
{
    return ( d->closeButton );
}
