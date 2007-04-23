/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef KPRINTPREVIEW_H
#define KPRINTPREVIEW_H

#include <kdialog.h>
#include <QtCore/QString>
#include <k3process.h>

class KLibFactory;

class KPrintPreview : public KDialog
{
	Q_OBJECT
public:
	explicit KPrintPreview(QWidget *parent = 0, bool previewOnly = false);
	~KPrintPreview();

	void openFile(const QString& file);
	bool isValid() const;

	static bool preview(const QString& file, bool previewOnly = false, WId parentId = 0);

private:
	void initView(KLibFactory*);

private:
	class KPrintPreviewPrivate;
	KPrintPreviewPrivate* const d;
};

class KPreviewProc : public K3Process
{
	Q_OBJECT
public:
	KPreviewProc();
	virtual ~KPreviewProc();

	bool startPreview();

Q_SIGNALS:
  void finished();

protected Q_SLOTS:
	void slotProcessExited(K3Process*);
private:
	bool m_bOk;
};

#endif
