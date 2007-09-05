/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#include "droptionview.h"
#include "driver.h"
#include "driveritem.h"

#include <math.h>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QLabel>
#include <klistwidget.h>
#include <kbuttongroup.h>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QLayout>
#include <QtGui/QApplication>

#include <kcursor.h>
#include <kdialog.h>
#include <klocale.h>

OptionBaseView::OptionBaseView(QWidget *parent)
    : QWidget(parent)
{
	blockSS = false;
}

void OptionBaseView::setOption(DrBase*)
{
}

void OptionBaseView::setValue(const QString&)
{
}

//******************************************************************************************************

OptionNumericView::OptionNumericView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_edit = new QLineEdit(this);
	m_slider = new QSlider(Qt::Horizontal,this);
	m_slider->setTickPosition(QSlider::TicksBelow);
	QLabel	*lab = new QLabel(i18n("Value:"),this);
	m_minval = new QLabel(this);
	m_maxval = new QLabel(this);

	m_integer = true;

	QVBoxLayout	*main_ = new QVBoxLayout(this);
  main_->setMargin(0);
  main_->setSpacing(10);
	QHBoxLayout	*sub_ = new QHBoxLayout();
  sub_->setMargin(0);
  sub_->setSpacing(10);
	QHBoxLayout	*sub2_ = new QHBoxLayout();
  sub2_->setMargin(0);
  sub2_->setSpacing(5);
	main_->addStretch(1);
	main_->addLayout(sub_,0);
	main_->addLayout(sub2_,0);
	main_->addStretch(1);
	sub_->addWidget(lab,0);
	sub_->addWidget(m_edit,0);
	sub_->addStretch(1);
	sub2_->addWidget(m_minval,0);
	sub2_->addWidget(m_slider,1);
	sub2_->addWidget(m_maxval,0);

	connect(m_slider,SIGNAL(valueChanged(int)),SLOT(slotSliderChanged(int)));
	connect(m_edit,SIGNAL(textChanged(const QString&)),SLOT(slotEditChanged(const QString&)));
}

void OptionNumericView::setOption(DrBase *opt)
{
	if (opt->type() != DrBase::Integer && opt->type() != DrBase::Float)
		return;

	blockSS = true;
	if (opt->type() == DrBase::Integer)
	{
		m_integer = true;
		int	min_ = opt->get("minval").toInt();
		int	max_ = opt->get("maxval").toInt();
		m_slider->setRange(min_,max_);
    m_slider->setSingleStep(1);
    m_slider->setPageStep(qMax((max_-min_)/20,1));
		m_minval->setText(QString::number(min_));
		m_maxval->setText(QString::number(max_));
	}
	else
	{
		m_integer = false;
		int	min_ = (int)rint(opt->get("minval").toFloat()*1000);
		int	max_ = (int)rint(opt->get("maxval").toFloat()*1000);
		m_slider->setRange(min_,max_);
		m_slider->setSingleStep(1);
    m_slider->setPageStep(qMax((max_-min_)/20,1));
		m_minval->setText(opt->get("minval"));
		m_maxval->setText(opt->get("maxval"));
	}
	m_slider->update();
	blockSS = false;

	setValue(opt->valueText());
}

void OptionNumericView::setValue(const QString& val)
{
	m_edit->setText(val);
}

void OptionNumericView::slotSliderChanged(int value)
{
	if (blockSS) return;

	QString	txt;
	if (m_integer)
		txt = QString::number(value);
	else
		txt = QString::number(float(value)/1000.0,'f',3);
	blockSS = true;
	m_edit->setText(txt);
	blockSS = false;
	emit valueChanged(txt);
}

void OptionNumericView::slotEditChanged(const QString& txt)
{
	if (blockSS) return;

	bool	ok(false);
	int	val(0);
	if (m_integer)
		val = txt.toInt(&ok);
	else
		val = (int)rint(txt.toFloat(&ok)*1000);
	if (ok)
	{
		blockSS = true;
		m_slider->setValue(val);
		blockSS = false;
		emit valueChanged(txt);
	}
	else
	{
		m_edit->selectAll();
		QApplication::beep();
	}
}

//******************************************************************************************************

OptionStringView::OptionStringView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_edit = new QLineEdit(this);
	QLabel	*lab = new QLabel(i18n("String value:"),this);

	QVBoxLayout	*main_ = new QVBoxLayout(this);
	main_->setMargin(0);
	main_->setSpacing(5);
	main_->addStretch(1);
	main_->addWidget(lab,0);
	main_->addWidget(m_edit,0);
	main_->addStretch(1);

	connect(m_edit,SIGNAL(textChanged(const QString&)),SIGNAL(valueChanged(const QString&)));
}

void OptionStringView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::String)
		m_edit->setText(opt->valueText());
}

void OptionStringView::setValue(const QString& val)
{
	m_edit->setText(val);
}

//******************************************************************************************************

OptionListView::OptionListView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_list = new KListWidget(this);

	QVBoxLayout	*main_ = new QVBoxLayout(this);
	main_->setMargin(0);
	main_->setSpacing(10);
	main_->addWidget(m_list);

	connect(m_list,SIGNAL(itemSelectionChanged()),SLOT(slotSelectionChanged()));
}

void OptionListView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::List)
	{
		blockSS = true;
		m_list->clear();
		m_choices.clear();
                foreach (DrBase* choice, static_cast<DrListOption*>(opt)->choices())
		{
			m_list->addItem(choice->get("text"));
			m_choices.append(choice->name());
		}
		blockSS = false;
		setValue(opt->valueText());
	}
}

void OptionListView::setValue(const QString& val)
{
	m_list->setCurrentRow(m_choices.indexOf(val));
}

void OptionListView::slotSelectionChanged()
{
	if (blockSS) return;

	QString	s = m_choices[m_list->currentRow()];
	emit valueChanged(s);
}

//******************************************************************************************************

OptionBooleanView::OptionBooleanView(QWidget *parent)
    : OptionBaseView(parent)
{
	m_group = new KButtonGroup(this);
	QVBoxLayout * layout = new QVBoxLayout( m_group );
	//m_group->setFrameStyle(QFrame::NoFrame);

	m_btn1 = new QRadioButton(m_group);
	m_btn1->setCursor( QCursor( Qt::OpenHandCursor ) );
	m_btn2 = new QRadioButton(m_group);
	m_btn2->setCursor( QCursor( Qt::OpenHandCursor ) );
	
	layout->addWidget( m_btn1 );
	layout->addWidget( m_btn2 );

	QVBoxLayout	*main_ = new QVBoxLayout(this);
	main_->setMargin(0);
	main_->setSpacing(10);
	main_->addWidget(m_group);

	connect(m_group,SIGNAL(clicked(int)),SLOT(slotSelected(int)));
}

void OptionBooleanView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::Boolean)
	{
                const QList<DrBase*>& choices = static_cast<DrBooleanOption*>(opt)->choices();
		m_choices.clear();
		m_btn1->setText(choices.first()->get("text"));
		m_choices.append(choices.first()->name());
		m_btn2->setText(choices.last()->get("text"));
		m_choices.append(choices.last()->name());
		setValue(opt->valueText());
	}
}

void OptionBooleanView::setValue(const QString& val)
{
	int	ID = m_choices.indexOf(val);
	m_group->setSelected(ID);
}

void OptionBooleanView::slotSelected(int ID)
{
	QString	s = m_choices[ID];
	emit valueChanged(s);
}

//******************************************************************************************************

DrOptionView::DrOptionView(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(KDialog::spacingHint());
	layout->setMargin(0);

	m_title = new QLabel(this);
	layout->addWidget(m_title);
	setTitle(i18n("No Option Selected"));

	m_stack = new QStackedWidget(this);
	layout->addWidget(m_stack);
	layout->setStretchFactor(m_stack, 10);
	layout->addStretch(1);

	OptionBaseView	*w = new OptionBaseView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_optionBaseID[ m_stack->addWidget(w) ] = 0;	// empty widget
	
	w = new OptionListView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_optionBaseID[ m_stack->addWidget(w) ] = DrBase::List;

	w = new OptionStringView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_optionBaseID[ m_stack->addWidget(w) ] = DrBase::String;

	w = new OptionNumericView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_optionBaseID[ m_stack->addWidget(w) ] = DrBase::Integer;

	w = new OptionBooleanView(m_stack);
	connect(w,SIGNAL(valueChanged(const QString&)),SLOT(slotValueChanged(const QString&)));
	m_optionBaseID[ m_stack->addWidget(w) ] = DrBase::Boolean;

	m_stack->setCurrentIndex(0);

	m_item = 0;
	m_block = false;
	m_allowfixed = true;
}

void DrOptionView::slotItemSelected(QTreeWidgetItem *i)
{
	m_item = (DriverItem*)i;
	if (m_item && !m_item->drItem()->isOption())
		m_item = 0;
	int	ID(0);
	if (m_item)
	{
		if (m_item->drItem()->type() == DrBase::Float) ID = DrBase::Integer;
		else ID = m_item->drItem()->type();
	}
	
	OptionBaseView	*w = optionBaseView(ID);
	if (w)
	{
		m_block = true;
		bool 	enabled(true);
		if (m_item)
		{
			w->setOption((m_item ? m_item->drItem() : 0));
			setTitle(m_item->drItem()->get("text"));
			enabled = ((m_item->drItem()->get("fixed") != "1") || m_allowfixed);
		}
		else
		{
			setTitle(i18n("No Option Selected"));
		}
		m_stack->setCurrentWidget(w);
		w->setEnabled(enabled);
		m_block = false;
	}
}

void DrOptionView::setTitle(const QString& title)
{
	m_title->setText("<b>" + title + "<b>");
}

OptionBaseView *DrOptionView::optionBaseView( int id )
{
	for ( int i = 0; i < 5; ++i )
	{
		if ( m_optionBaseID[i] == id )
			return (OptionBaseView*)m_stack->widget(i);
	}
	return 0;
}

void DrOptionView::slotValueChanged(const QString& val)
{
	if (m_item && m_item->drItem() && !m_block)
	{
		m_item->drItem()->setValueText(val);
		m_item->updateText();
		emit changed();
	}
}

#include "droptionview.moc"
