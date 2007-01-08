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

#include "kprintaction.h"

#include <kprinter.h>
#include <kdeprint/kmmanager.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kactioncollection.h>
#include <klocale.h>

class KPrintAction::KPrintActionPrivate
{
public:
	KPrintActionPrivate()
	{
		type = All;
		parentWidget = 0;
	}

	PrinterType	type;
	QStringList	printers;
	QWidget *parentWidget;
};

KPrintAction::KPrintAction(const QString& text, PrinterType type, QWidget *parentWidget)
: KActionMenu(text, parentWidget),d(new KPrintActionPrivate())
{
	initialize(type, parentWidget);
}

KPrintAction::KPrintAction(const KIcon& icon, const QString& text, PrinterType type, QWidget *parentWidget)
: KActionMenu(icon, text, parentWidget),d(new KPrintActionPrivate())
{
	initialize(type, parentWidget);
}

KPrintAction::~KPrintAction()
{
	delete d;
}

void KPrintAction::initialize(PrinterType type, QWidget *parentWidget)
{
	connect(menu(), SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
	connect(menu(), SIGNAL(triggered(QAction*)), SLOT(slotActivated(QAction*)));

	d->type = type;
	d->parentWidget = parentWidget;
}

void KPrintAction::slotAboutToShow()
{
	menu()->clear();
	d->printers.clear();
	QList<KMPrinter*>	prts = KMManager::self()->printerList();
	if (!prts.isEmpty())
	{
		QListIterator<KMPrinter*>	it(prts);
		bool	first(false);
		int	ID(0);
		while (it.hasNext())
		{
			KMPrinter *printer(it.next());
			if (d->type == All || (d->type == Specials && printer->isSpecial()) || (d->type == Regular && !printer->isSpecial()))
			{
				if (d->type == All && !first && printer->isSpecial())
				{
					if (menu()->actions().count() > 0)
						menu()->addSeparator();
					first = true;
				}
				QAction *action = menu()->addAction(KIcon(printer->pixmap()), printer->name());
				action->setData(ID++);
				d->printers.append(printer->name());
			}
		}
	}
}

void KPrintAction::slotActivated(QAction *action)
{
  int ID = action->data().toInt();
	KPrinter	printer(false);
	KMPrinter	*mprt = KMManager::self()->findPrinter(d->printers[ID]);
	if (mprt && mprt->autoConfigure(&printer, d->parentWidget))
	{
		// emit the signal
		emit print(&printer);
	}
}

KPrintAction* KPrintAction::exportAll(QWidget *parentWidget, KActionCollection *parent, const char *name)
{
	KPrintAction *a = new KPrintAction(i18n("&Export..."), All, parentWidget);
	if ( parent!=0 )
		parent->addAction(name ? name : "export_all", a);
	return a;
}

KPrintAction* KPrintAction::exportRegular(QWidget *parentWidget, KActionCollection *parent, const char *name)
{
	KPrintAction *a = new KPrintAction(i18n("&Export..."), Regular, parentWidget);
	if ( parent!=0 )
		parent->addAction(name ? name : "export_regular", a);
	return a;
}

KPrintAction* KPrintAction::exportSpecial(QWidget *parentWidget, KActionCollection *parent, const char *name)
{
	KPrintAction *a = new KPrintAction(i18n("&Export..."), Specials, parentWidget);
	if ( parent!=0 )
		parent->addAction(name ? name : "export_special", a);
	return a;
}

#include "kprintaction.moc"
