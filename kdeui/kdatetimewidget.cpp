#include "kdatetimewidget.h"

#include <qdatetime.h>
#include <qlayout.h>
#include <QTimeEdit>

#include <kdebug.h>

#include "kdatewidget.h"
#include "kdialog.h"

class KDateTimeWidget::KDateTimeWidgetPrivate
{
public:
  KDateWidget * dateWidget;
  QTimeEdit * timeWidget;
};

KDateTimeWidget::KDateTimeWidget(QWidget * parent)
  : QWidget(parent)
{
  init();
}

KDateTimeWidget::KDateTimeWidget(const QDateTime & datetime,
                                 QWidget * parent)
  : QWidget(parent)
{
  init();

  setDateTime(datetime);
}

KDateTimeWidget::~KDateTimeWidget()
{
  delete d;
}

void KDateTimeWidget::init()
{
  d = new KDateTimeWidgetPrivate;

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(KDialog::spacingHint());

  d->dateWidget = new KDateWidget(this);
  d->timeWidget = new QTimeEdit(this);
  layout->addWidget( d->dateWidget );
  layout->addWidget( d->timeWidget );

  connect(d->dateWidget, SIGNAL(changed(QDate)),
          SLOT(slotValueChanged()));
  connect(d->timeWidget, SIGNAL(timeChanged(const QTime &)),
          SLOT(slotValueChanged()));
}

void KDateTimeWidget::setDateTime(const QDateTime & datetime)
{
  d->dateWidget->setDate(datetime.date());
  d->timeWidget->setTime(datetime.time());
}

QDateTime KDateTimeWidget::dateTime() const
{
  return QDateTime(d->dateWidget->date(), d->timeWidget->time());
}

void KDateTimeWidget::slotValueChanged()
{
  QDateTime datetime(d->dateWidget->date(),
                     d->timeWidget->time());

  kDebug() << "slotValueChanged(): " << datetime << "\n";

  emit valueChanged(datetime);
}

#include "kdatetimewidget.moc"
