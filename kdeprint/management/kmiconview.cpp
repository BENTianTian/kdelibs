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

#include "kmiconview.h"
#include "kmprinter.h"

#include <QtGui/QPainter>
#include <kiconloader.h>
#include <kdebug.h>

KMIconViewItem::KMIconViewItem(Q3IconView *parent, KMPrinter *p)
: Q3IconViewItem(parent)
{
	m_state = 0;
	m_mode = parent->itemTextPos();
	m_pixmap.clear();
	m_isclass = false;
	updatePrinter(p, m_mode);
}

void KMIconViewItem::paintItem(QPainter *p, const QColorGroup& cg)
{
	if (m_state != 0)
	{
		QFont	f(p->font());
		if (m_state & 0x1) f.setBold(true);
		if (m_state & 0x2) f.setItalic(true);
		p->setFont(f);
	}
	Q3IconViewItem::paintItem(p,cg);
}

void KMIconViewItem::updatePrinter(KMPrinter *p, int mode)
{
	bool	update(false);
	int	oldstate = m_state;
	if (p)
	{
		m_state = ((p->isHardDefault() ? 0x1 : 0x0) | (p->ownSoftDefault() ? 0x2 : 0x0) | (p->isValid() ? 0x4 : 0x0));
		update = (oldstate != m_state);
		if (p->name() != text() || update)
		{
			setText(QString());
			setText(p->name());
		}
		setKey(QString::fromLatin1("%1_%2").arg((p->isSpecial() ? "special" : (p->isClass(false) ? "class" : "printer"))).arg(p->name()));
		m_isclass = p->isClass(false);
	}
	if (mode != m_mode || ((oldstate&0x4) != (m_state&0x4)) || (p && p->pixmap() != m_pixmap))
	{
		QStringList overlays;
		if (m_state & 0x4) {
			overlays.append("lock");
                }

		if (p)
			m_pixmap = p->pixmap();
		m_mode = mode;
		if (m_mode == Qt::DockBottom)
			setPixmap(DesktopIcon(m_pixmap, 0, K3Icon::DefaultState, overlays));
		else
			setPixmap(SmallIcon(m_pixmap, 0, K3Icon::DefaultState, overlays));
	}
	//if (update)
	//	repaint();
	setDiscarded(false);
}

KMIconView::KMIconView(QWidget *parent, const char *name)
: K3IconView(parent,name)
{
	setMode(K3IconView::Select);
	setSelectionMode(Q3IconView::Single);
	setItemsMovable(false);
	setResizeMode(Q3IconView::Adjust);

	setViewMode(KMIconView::Big);

	connect(this,SIGNAL(contextMenuRequested(Q3IconViewItem*,const QPoint&)),SLOT(slotRightButtonClicked(Q3IconViewItem*,const QPoint&)));
	connect(this,SIGNAL(selectionChanged()),SLOT(slotSelectionChanged()));
}

KMIconView::~KMIconView()
{
  qDeleteAll(m_items);
}

KMIconViewItem* KMIconView::findItem(KMPrinter *p)
{
	if (p)
	{
		QListIterator<KMIconViewItem*>	it(m_items);
		while (it.hasNext()) {
      KMIconViewItem *item(it.next());
			if (item->text() == p->name()
			    && item->isClass() == p->isClass())
				return item;
    }
	}
	return 0;
}

void KMIconView::setPrinterList(QList<KMPrinter*> *list)
{
	bool	changed(false);

	QListIterator<KMIconViewItem*>	it(m_items);
	while (it.hasNext())
		it.next()->setDiscarded(true);

	if (list)
	{
		QListIterator<KMPrinter*>	it(*list);
		KMIconViewItem			*item(0);
		while (it.hasNext())
		{
			KMPrinter *printer(it.next());
			// only keep real printers (no instances)
			if (!printer->instanceName().isEmpty())
				continue;
			item = findItem(printer);
			if (!item)
			{
				item = new KMIconViewItem(this,printer);
				m_items.append(item);
				changed = true;
			}
			else
				item->updatePrinter(printer, itemTextPos());
		}
	}

	for (int i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			delete m_items.takeAt(i);
			i--;
			changed = true;
		}

	if (changed) sort();
	emit selectionChanged();
}

void KMIconView::setViewMode(ViewMode m)
{
	m_mode = m;
	bool	big = (m == KMIconView::Big);
	int	mode = (big ? Qt::DockBottom : Qt::DockRight);

	QListIterator<KMIconViewItem*>	it(m_items);
	while (it.hasNext())
		it.next()->updatePrinter(0, mode);

	setArrangement((big ? Q3IconView::LeftToRight : Q3IconView::TopToBottom));
	setItemTextPos((Q3IconView::ItemTextPos)mode);
	//setGridX((big ? 60 : -1));
	setWordWrapIconText(true);
}

void KMIconView::slotRightButtonClicked(Q3IconViewItem *item, const QPoint& p)
{
	emit rightButtonClicked(item ? item->text() : QString(), p);
}

void KMIconView::slotSelectionChanged()
{
	KMIconViewItem	*item = static_cast<KMIconViewItem*>(currentItem());
	emit printerSelected((item && !item->isDiscarded() && item->isSelected() ? item->text() : QString()));
}

void KMIconView::setPrinter(const QString& prname)
{
	QListIterator<KMIconViewItem*>	it(m_items);
	while (it.hasNext()) {
    KMIconViewItem *item(it.next());
		if (item->text() == prname)
		{
			setSelected(item, true);
			break;
		}
  }
}

void KMIconView::setPrinter(KMPrinter *p)
{
	setPrinter(p ? p->name() : QString());
}

#include "kmiconview.moc"
