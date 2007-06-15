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

#include "escpwidget.h"

#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <Qt3Support/Q3Accel>
#include <QtCore/QByteRef>
#include <kdemacros.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kdebug.h>

class EscpFactory : public KLibFactory
{
public:
	EscpFactory(QObject *parent = 0) : KLibFactory(parent) {}
protected:
	QObject* createObject(QObject *parent = 0, const char * /*classname*/ = "QObject", const QStringList& args = QStringList())
	{
		KDialog *dlg = new KDialog( static_cast<QWidget*>(parent) );
    dlg->setModal( true );
    dlg->setCaption( i18n("EPSON InkJet Printer Utilities") );
    dlg->setButtons( KDialog::Close );

		EscpWidget	*w = new EscpWidget(dlg);
		if (args.count() > 0)
			w->setDevice(args[0]);
		if (args.count() > 1)
			w->setPrinterName(args[1]);
		dlg->setMainWidget(w);
		return dlg;
	}
};

extern "C"
{
	KDE_EXPORT void* init_kdeprint_tool_escputil();
	void* init_kdeprint_tool_escputil()
	{
		return new EscpFactory;
	}
}

EscpWidget::EscpWidget(QWidget *parent)
    : QWidget(parent)
{
	m_hasoutput = false;

	connect(&m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotProcessExited()));
	connect(&m_proc, SIGNAL(readyReadStandardOutput()), SLOT(slotReceivedStdout()));
	connect(&m_proc, SIGNAL(readyReadStandardError()), SLOT(slotReceivedStderr()));

	QPushButton	*cleanbtn = new QPushButton(this);
	cleanbtn->setObjectName(QLatin1String("-c"));
	cleanbtn->setIcon(DesktopIcon("exec"));
	QPushButton	*nozzlebtn = new QPushButton(this);
	nozzlebtn->setObjectName(QLatin1String("-n"));
	nozzlebtn->setIcon(DesktopIcon("exec"));
	QPushButton	*alignbtn = new QPushButton(this);
	alignbtn->setObjectName(QLatin1String("-a"));
	alignbtn->setIcon(DesktopIcon("exec"));
	QPushButton	*inkbtn = new QPushButton(this);
	inkbtn->setObjectName(QLatin1String("-i"));
	inkbtn->setIcon(DesktopIcon("kdeprint_inklevel"));
	QPushButton	*identbtn = new QPushButton(this);
	identbtn->setObjectName(QLatin1String("-d"));
	identbtn->setIcon(DesktopIcon("exec"));

	QFont	f(font());
	f.setBold(true);
	m_printer = new QLabel(this);
	m_printer->setFont(f);
	m_device = new QLabel(this);
	m_device->setFont(f);
	m_useraw = new QCheckBox(i18n("&Use direct connection (might need root permissions)"), this);

	connect(cleanbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(nozzlebtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(alignbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(inkbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));
	connect(identbtn, SIGNAL(clicked()), SLOT(slotButtonClicked()));

	QLabel	*printerlab = new QLabel(i18n("Printer:"), this);
	printerlab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*devicelab = new QLabel(i18n("Device:"), this);
	devicelab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*cleanlab = new QLabel(i18n("Clea&n print head"), this);
	QLabel	*nozzlelab = new QLabel(i18n("&Print a nozzle test pattern"), this);
	QLabel	*alignlab = new QLabel(i18n("&Align print head"), this);
	QLabel	*inklab = new QLabel(i18n("&Ink level"), this);
	QLabel	*identlab = new QLabel(i18n("P&rinter identification"), this);

	cleanlab->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	nozzlelab->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	alignlab->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	inklab->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	identlab->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

	cleanbtn->setShortcut(Q3Accel::shortcutKey(cleanlab->text()));
	nozzlebtn->setShortcut(Q3Accel::shortcutKey(nozzlelab->text()));
	alignbtn->setShortcut(Q3Accel::shortcutKey(alignlab->text()));
	inkbtn->setShortcut(Q3Accel::shortcutKey(inklab->text()));
	identbtn->setShortcut(Q3Accel::shortcutKey(identlab->text()));

	KSeparator	*sep = new KSeparator(this);
	sep->setFixedHeight(10);

	QGridLayout	*l0 = new QGridLayout(this);
    l0->setMargin(10);
    l0->setSpacing(10);
	QGridLayout	*l1 = new QGridLayout();
    l1->setMargin(0);
    l1->setSpacing(5);
    l0->addLayout(l1, 0, 0, 1, 2);
	l1->addWidget(printerlab, 0, 0);
	l1->addWidget(devicelab, 1, 0);
	l1->addWidget(m_printer, 0, 1);
	l1->addWidget(m_device, 1, 1);
	l1->setColumnStretch(1, 1);
    l0->addWidget(sep, 1, 0, 1, 2);
	l0->addWidget(cleanbtn, 2, 0);
	l0->addWidget(nozzlebtn, 3, 0);
	l0->addWidget(alignbtn, 4, 0);
	l0->addWidget(inkbtn, 5, 0);
	l0->addWidget(identbtn, 6, 0);
	l0->addWidget(cleanlab, 2, 1);
	l0->addWidget(nozzlelab, 3, 1);
	l0->addWidget(alignlab, 4, 1);
	l0->addWidget(inklab, 5, 1);
	l0->addWidget(identlab, 6, 1);
    l0->addWidget(m_useraw, 7, 0, 1, 2);
	l0->setColumnStretch(1, 1);
}

void EscpWidget::startCommand(const QString& arg)
{
	bool	useUSB(false);

	if (m_deviceURL.isEmpty())
	{
		KMessageBox::error(this, i18n("Internal error: no device set."));
		return;
	}
	else
	{
		QString	protocol = m_deviceURL.protocol();
		if (protocol == "usb")
			useUSB = true;
		else if (protocol != "file" && protocol != "parallel" && protocol != "serial" && !protocol.isEmpty())
		{
			KMessageBox::error(this,
				i18n("Unsupported connection type: %1", protocol));
			return;
		}
	}

	if (m_proc.state() == QProcess::Running)
	{
		KMessageBox::error(this, i18n("An escputil process is still running. "
		                              "You must wait until its completion before continuing."));
		return;
	}

	QString	exestr = KStandardDirs::findExe("escputil");
	if (exestr.isEmpty())
	{
		KMessageBox::error(this, i18n("The executable escputil cannot be found in your "
		                              "PATH environment variable. Make sure gimp-print is "
		                              "installed and that escputil is in your PATH."));
		return;
	}

//	m_proc.clearArguments();
	m_proc << exestr;
	if (m_useraw->isChecked() || arg == "-i")
		m_proc << "-r" << m_deviceURL.path();
	else
		m_proc << "-P" << m_printer->text();
	if (useUSB)
		m_proc << "-u";

	m_proc << arg << "-q";
	m_errorbuffer = m_outbuffer = QString();
	m_hasoutput = ( arg == "-i" || arg == "-d" );
//	foreach ( QByteArray arg, m_proc.args() )
//		kDebug() << "ARG: " << arg << endl;

	m_proc.setOutputChannelMode(KProcess::SeparateChannels);
	m_proc.start();
	if (m_proc.waitForFinished())
		setEnabled(false);
	else
	{
		KMessageBox::error(this,
			i18n("Internal error: unable to start escputil process."));
		return;
	}
}

void EscpWidget::slotProcessExited()
{
	setEnabled(true);
	if (m_proc.exitStatus() != QProcess::NormalExit || m_proc.exitCode() != 0)
	{
		QString	msg1 = "<qt>"+i18n("Operation terminated with errors.")+"</qt>";
		QString	msg2;
		if (!m_outbuffer.isEmpty())
			msg2 += "<p><b><u>"+i18n("Output")+"</u></b></p><p>"+m_outbuffer+"</p>";
		if (!m_errorbuffer.isEmpty())
			msg2 += "<p><b><u>"+i18n("Error")+"</u></b></p><p>"+m_errorbuffer+"</p>";
		if (!msg2.isEmpty())
			KMessageBox::detailedError(this, msg1, msg2);
		else
			KMessageBox::error(this, msg1);
	}
	else if ( !m_outbuffer.isEmpty() && m_hasoutput )
	{
		KMessageBox::information( this, m_outbuffer );
	}
	m_hasoutput = false;
}

void EscpWidget::slotReceivedStdout()
{
	QString	bufstr = QString::fromLocal8Bit( m_proc.readAllStandardOutput() );
	m_outbuffer.append(bufstr);
}

void EscpWidget::slotReceivedStderr()
{
	QString	bufstr = QString::fromLocal8Bit( m_proc.readAllStandardError() );
	m_errorbuffer.append(bufstr);
}

void EscpWidget::slotButtonClicked()
{
	QString	arg = sender()->objectName();
	startCommand(arg);
}

void EscpWidget::setPrinterName(const QString& p)
{
	m_printer->setText(p);
}

void EscpWidget::setDevice(const QString& dev)
{
	m_deviceURL = dev;
	m_device->setText(dev);
}

#include "escpwidget.moc"
