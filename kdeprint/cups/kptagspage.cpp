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

#include "kptagspage.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <qlayout.h>
#include <qlabel.h>
#include <qregexp.h>

#include <klocale.h>

KPTagsPage::KPTagsPage(bool ro, QWidget *parent)
: KPrintDialogPage(parent)
{

	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisAdditionalTagsTable = i18n( 
						" <ul>"
						" <li>Use any current or future standard CUPS job option not supported by the "
						" KDEPrint GUI. </li>"
						" <li>Control any custom job option you may want to support in custom CUPS filters "
						" and backends plugged into the CUPS filtering chain.</li> "
						" <li>Send short messages to the operators of your production printers in your "
						" <em>Central Repro Department</em>."
						" </ul> "
						" <p><b>Standard CUPS job options:</b> A complete list of standard CUPS job "
						" options is in the <a href=\"http://localhost:631/sum.html\">CUPS User Manual</a>. "
						" Mappings of the kprinter user interface widgets to respective CUPS job option "
						" names are named in the various <em>What\'s This</em> help items.</p>"
						" <p><b>Custom CUPS job options:</b> CUPS print servers may be customized with additional "
						" print filters and backends which understand custom job options. You can specify such "
						" custom job options here. If in doubt, ask your system administrator.</p>"
						" <p><b> </b></p>"
						" <p><b>Operator Messages:</b> You may send additional messages to the operator(s) of your"
						" production printers (e.g. in your <em>Central Repro Department</p>)."
						" Messages can be read by the operator(s) (or yourself) by viewing"
						" the <em>\"Job IPP Report\"</em> for the job.</p>"
						" <b>Examples:</b><br>"
						" <pre>"
						" A standard CUPS job option:<br> "
						"   <em>(Name) number-up</em>                -- <em>(Value) 9</em>                     <br>"
						" <br>"
						" A job option for custom CUPS filters or backends:<br> "
						"   <em>(Name) DANKA_watermark</em>          -- <em>(Value) Company_Confidential</em>   <br>"
						" <br>"
						" A message to the operator(s):<br> "
						"   <em>(Name) Deliver_after_completion</em> -- <em>(Value) to_Marketing_Departm.</em><br>"
						" </pre>"
						" <p><b>Note:</b> the fields must not include spaces, tabs or quotes. You may need to "
						" double-click on a field to edit it."
						" <p><b>Warning:</b> Do not use such standard CUPS option names which also can be used "
						" through the KDEPrint GUI. Results may be  unpredictable if they conflict, "
						" or if they are sent multiple times. For all options supported by the GUI, please do use "
						" the GUI. (Each GUI element's  'What\'s This' names the related CUPS option name.) </p> "
						" </qt>" );
	setTitle(i18n("Additional Tags"));
	setOnlyRealPrinters(true);

	QStringList lst;
	lst << i18n("Name")<<i18n("Value");
	m_tags = new QTableWidget(10, 2, this);
	m_tags->horizontalHeader()->resizeSections ( QHeaderView::Stretch );
	m_tags->setVerticalHeaderLabels(lst);

#warning "kde4: port m_tags->setReadOnly(ro);!!!!!\n";
	//m_tags->setReadOnly(ro);
	m_tags->setWhatsThis(whatsThisAdditionalTagsTable);

	QVBoxLayout	*l0 = new QVBoxLayout(this);
	l0->setMargin(0);
	l0->setSpacing(5);
	l0->addWidget(m_tags);

	if (ro)
	{
		QLabel	*lab = new QLabel(i18n("Read-Only"), this);
		QFont	f = lab->font();
		f.setBold(true);
		lab->setFont(f);
		lab->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
		l0->addWidget(lab);
	}
}

KPTagsPage::~KPTagsPage()
{
}

bool KPTagsPage::isValid(QString& msg)
{
	QRegExp	re("\\s");
	for (int r=0; r<m_tags->columnCount(); r++)
	{
		QString	tag(m_tags->itemAt( 0, r)->text());
		if (tag.isEmpty())
			continue;
		else if (tag.indexOf(re) != -1)
		{
			msg = i18n("The tag name must not contain any spaces, tabs or quotes: <b>%1</b>.", tag);
			return false;
		}
	}
	return true;
}

void KPTagsPage::setOptions(const QMap<QString,QString>& opts)
{
	int	r(0);
	QRegExp	re("^\"|\"$");
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end() && r<m_tags->rowCount (); ++it)
	{
		if (it.key().startsWith("KDEPrint-"))
		{
			if (!m_tags->itemAt( 0, r))  m_tags->setItem( 0, r, new QTableWidgetItem());
			if (!m_tags->itemAt( 1, r))  m_tags->setItem( 1, r, new QTableWidgetItem());
			m_tags->itemAt( 0, r)->setText(it.key().mid(9));
			QString	data = it.value();
			m_tags->itemAt(1, r )-> setText(data.replace(re, ""));
			r++;
		}
	}
	for (; r<m_tags->rowCount(); r++)
	{
		if (!m_tags->itemAt( 0, r))  m_tags->setItem( 0, r, new QTableWidgetItem());
		if (!m_tags->itemAt( 1, r))  m_tags->setItem( 0, r, new QTableWidgetItem());
		m_tags->itemAt(0,r)->setText(QString());
		m_tags->itemAt(1,r)->setText(QString());
	}
}

void KPTagsPage::getOptions(QMap<QString,QString>& opts, bool)
{
	for (int r=0; r<m_tags->rowCount(); r++)
	{
		QString	tag(m_tags->itemAt(0,r)->text()), val(m_tags->itemAt(1,r)->text());
		if (!tag.isEmpty())
		{
			tag.prepend("KDEPrint-");
			opts[tag] = val.prepend("\"").append("\"");
		}
	}
}

QSize KPTagsPage::sizeHint() const
{
	return QSize(-1, -1);
}

QSize KPTagsPage::minimumSizeHint() const
{
	return QSize(-1, -1);
}
