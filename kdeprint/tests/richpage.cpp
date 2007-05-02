#include "richpage.h"

#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QFontDatabase>
#include <QtGui/QLayout>

RichPage::RichPage(QWidget *parent)
: KPrintDialogPage(parent)
{
	m_title = "Rich Text Options";

	margin_ = new QSpinBox(this);
	margin_->setRange(1,999);
	margin_->setValue(72);

	fontsize_ = new QSpinBox(this);
	fontsize_->setRange(4,100);
	fontsize_->setValue(10);

	fontname_ = new QComboBox(this);
	QFontDatabase	db;
	QStringList	fonts = db.families();
	fontname_->addItems(fonts);
	fontname_->setCurrentIndex(fonts.indexOf(QLatin1String("times")));
	if (fontname_->currentIndex() < 0) fontname_->setCurrentIndex(0);

	QLabel	*l1 = new QLabel("Margin:",this);
	QLabel	*l2 = new QLabel("Font name:",this);
	QLabel	*l3 = new QLabel("Font size:",this);

	QHBoxLayout	*s1 = new QHBoxLayout();
	QHBoxLayout	*s2 = new QHBoxLayout();
	QVBoxLayout	*main_ = new QVBoxLayout(this);

	main_->addLayout(s1,0);
	main_->addSpacing(20);
	main_->addLayout(s2,0);
	main_->addStretch(1);

	s1->addWidget(l1,0);
	s1->addWidget(margin_,0);
	s1->addStretch(1);
        s1->setSpacing(10);

	s2->addWidget(l2,0);
	s2->addWidget(fontname_,0);
	s2->addSpacing(20);
	s2->addWidget(l3,0);
	s2->addWidget(fontsize_,0);
	s2->addStretch(1);
}

RichPage::~RichPage()
{
}

void RichPage::setOptions(const QMap<QString,QString>& opts)
{
	QString	value;

	value = opts["app-rich-margin"];
	if (!value.isEmpty())
		margin_->setValue(value.toInt());

	value = opts["app-rich-fontname"];
	if (!value.isEmpty())
		for (int i=0;i<fontname_->count();i++)
			if (fontname_->itemText(i) == value)
			{
				fontname_->setCurrentIndex(i);
				break;
			}

	value = opts["app-rich-fontsize"];
	if (!value.isEmpty())
		fontsize_->setValue(value.toInt());
}

void RichPage::getOptions(QMap<QString,QString>& opts, bool)
{
	opts["app-rich-margin"] = margin_->text();
	opts["app-rich-fontname"] = fontname_->currentText();
	opts["app-rich-fontsize"] = fontsize_->text();
}
