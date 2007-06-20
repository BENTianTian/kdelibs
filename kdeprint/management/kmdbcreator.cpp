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

#include "kmdbcreator.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <QtCore/QByteRef>
#include <QtGui/QProgressDialog>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

KMDBCreator::KMDBCreator(QObject *parent)
    : QObject(parent)
{
	m_dlg = 0;
	m_status = true;
	m_proc.setOutputChannelMode(KProcess::SeparateChannels);
	m_proc.closeReadChannel(KProcess::StandardError); //ignore stderr for now

	connect(&m_proc,SIGNAL(readyReadStdout()),SLOT(slotReceivedStdout()));
	connect(&m_proc,SIGNAL(finished(int,KProcess::ExitStatus)),
                SLOT(slotProcessExited(int,KProcess::ExitStatus)));
}

KMDBCreator::~KMDBCreator()
{
	if (m_proc.state() == KProcess::Running)
		m_proc.kill();
	// do not delete the progress dialog anymore: it's persistent and owned by
	// it's parent. It will be destroyed along with its parent.
}

bool KMDBCreator::checkDriverDB(const QString& dirname, const QDateTime& d)
{
	// don't block GUI
	qApp->processEvents();

	// first check current directory
	QFileInfo	dfi(dirname);
	if (dfi.lastModified() > d)
		return false;

	// then check most recent file in current directory
	QDir	dir(dirname);
	const QFileInfoList	list = dir.entryInfoList(QDir::Files,QDir::Time);
	if (list.count() > 0 && list.first().lastModified() > d)
		return false;

	// then loop into subdirs
	QStringList	slist = dir.entryList(QDir::Dirs,QDir::Time);
	for (QStringList::ConstIterator it=slist.begin(); it!=slist.end(); ++it)
		if ((*it) != "." && (*it) != ".." && !checkDriverDB(dir.absoluteFilePath(*it),d))
			return false;

	// everything is OK
	return true;
}

bool KMDBCreator::createDriverDB(const QString& dirname, const QString& filename, QWidget *parent)
{
	bool	started(true);

        m_dbfilename = filename;

	// initialize status
	m_status = false;
	m_firstflag = true;

	// start the child process
	m_proc.clearProgram();
	QString	exestr = KMFactory::self()->manager()->driverDbCreationProgram();
	QString	msg;
	if (exestr.isEmpty())
		msg = i18n("No executable defined for the creation of the "
		           "driver database. This operation is not implemented.");
	else {
		exestr = KStandardDirs::findExe(exestr);
		m_proc << exestr << dirname << filename;
		kDebug() << "executing : " << exestr << " " << dirname << " " << filename << endl;
		if (exestr.isEmpty())
			msg = i18n("The executable %1 could not be found in your "
			           "PATH. Check that this program exists and is "
				   "accessible in your PATH variable.", exestr);
		else {
			m_proc.start();
			if (!m_proc.waitForStarted())
				msg = i18n("Unable to start the creation of the "
					"driver database. The execution of %1 "
					"failed.", exestr);
		}
	}
	if (!msg.isEmpty())
	{
		KMManager::self()->setErrorMsg(msg);
		started = false;
	}

	// Create the dialog if the process is running and if needed
	if (started)
	{
		if (!m_dlg)
		{
			m_dlg = new QProgressDialog(parent->topLevelWidget());
      m_dlg->setObjectName("progress-dialog");
			m_dlg->setLabelText(i18n("Please wait while KDE rebuilds a driver database."));
			m_dlg->setWindowTitle(i18n("Driver Database"));
			connect(m_dlg,SIGNAL(canceled()),SLOT(slotCancelled()));
		}
		m_dlg->setMinimumDuration(0);	// always show the dialog
		m_dlg->setValue(0);		// to force showing
	}
	else
		// be sure to emit this signal otherwise the DB widget won't never be notified
		emit dbCreated();

	return started;
}

void KMDBCreator::slotReceivedStdout()
{
	// save buffer
	QString	str( m_proc.readAllStandardOutput() );

	// get the number, cut the string at the first '\n' otherwise
	// the toInt() will return 0. If that occurs for the first number,
	// then the number of steps will be also 0.
	bool	ok;
	int	p = str.indexOf('\n');
	int	n = str.mid(0, p).toInt(&ok);

	// process the number received
	if (ok && m_dlg)
	{
		if (m_firstflag)
		{
			m_dlg->setRange(0, n);
			m_firstflag = false;
		}
		else
		{
			m_dlg->setValue(n);
		}
	}
}

void KMDBCreator::slotProcessExited(int, KProcess::ExitStatus exitStatus)
{
	// delete the progress dialog
	if (m_dlg)
	{
		m_dlg->reset();
	}

	// set exit status
	m_status = (exitStatus == KProcess::NormalExit);
	if (!m_status)
	{
		KMFactory::self()->manager()->setErrorMsg(i18n("Error while creating driver database: abnormal child-process termination."));
		// remove the incomplete driver DB file so that it will be
		// reconstructed on next check
		QFile::remove(m_dbfilename);
	}
	//else
		emit dbCreated();
}

void KMDBCreator::slotCancelled()
{
	if (m_proc.state() == KProcess::Running)
		m_proc.kill();
	else
		emit dbCreated();
}
#include "kmdbcreator.moc"
