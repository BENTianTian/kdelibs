/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kurlbutton_p.h"

#include "kurlnavigator.h"

#include <kglobalsettings.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

KUrlButton::KUrlButton(KUrlNavigator* parent) :
    QPushButton(parent),
    m_displayHint(0),
    m_urlNavigator(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMinimumHeight(parent->minimumHeight());

    connect(this, SIGNAL(clicked()), parent, SLOT(requestActivation()));
}

KUrlButton::~KUrlButton()
{
}

void KUrlButton::setDisplayHintEnabled(DisplayHint hint,
                                       bool enable)
{
    if (enable) {
        m_displayHint = m_displayHint | hint;
    } else {
        m_displayHint = m_displayHint & ~hint;
    }
    update();
}

bool KUrlButton::isDisplayHintEnabled(DisplayHint hint) const
{
    return (m_displayHint & hint) > 0;
}

void KUrlButton::enterEvent(QEvent* event)
{
    QPushButton::enterEvent(event);
    setDisplayHintEnabled(EnteredHint, true);
    update();
}

void KUrlButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    setDisplayHintEnabled(EnteredHint, false);
    update();
}

void KUrlButton::contextMenuEvent(QContextMenuEvent* event)
{
    KMenu popup(this);

    // provide 'Copy' action, which copies the current URL of
    // the URL navigator into the clipboard
    QAction* copyAction = popup.addAction(KIcon("edit-copy"), i18n("Copy"));

    // provide 'Paste' action, which copies the current clipboard text
    // into the URL navigator
    QAction* pasteAction = popup.addAction(KIcon("edit-paste"), i18n("Paste"));
    QClipboard* clipboard = QApplication::clipboard();
    pasteAction->setEnabled(!clipboard->text().isEmpty());

    popup.addSeparator();

    // provide radiobuttons for toggling between the edit and the navigation mode
    QAction* editAction = popup.addAction(i18n("Edit"));
    editAction->setCheckable(true);

    QAction* navigateAction = popup.addAction(i18n("Navigate"));
    navigateAction->setCheckable(true);

    QActionGroup* modeGroup = new QActionGroup(&popup);
    modeGroup->addAction(editAction);
    modeGroup->addAction(navigateAction);
    if (m_urlNavigator->isUrlEditable()) {
        editAction->setChecked(true);
    } else {
        navigateAction->setChecked(true);
    }

    QAction* activatedAction = popup.exec(QCursor::pos());
    if (activatedAction == copyAction) {
        QMimeData* mimeData = new QMimeData();
        mimeData->setText(m_urlNavigator->url().prettyUrl());
        clipboard->setMimeData(mimeData);
    } else if (activatedAction == pasteAction) {
        m_urlNavigator->setUrl(KUrl(clipboard->text()));
    } else if (activatedAction == editAction) {
        m_urlNavigator->setUrlEditable(true);
    } else if (activatedAction == navigateAction) {
        m_urlNavigator->setUrlEditable(false);
    }
}

QColor KUrlButton::foregroundColor() const
{
    const bool isHighlighted = isDisplayHintEnabled(EnteredHint) ||
                               isDisplayHintEnabled(DraggedHint) ||
                               isDisplayHintEnabled(PopupActiveHint);

    QColor foregroundColor = isHighlighted ? KGlobalSettings::highlightedTextColor() :
                                             KGlobalSettings::buttonTextColor();

    if (!urlNavigator()->isActive()) {
        QColor dimmColor(palette().brush(QPalette::Background).color());
        foregroundColor = mixColors(foregroundColor, dimmColor);
    }

    return foregroundColor;
}

QColor KUrlButton::backgroundColor() const
{
    const bool isHighlighted = isDisplayHintEnabled(EnteredHint) ||
                               isDisplayHintEnabled(DraggedHint) ||
                               isDisplayHintEnabled(PopupActiveHint);

    QColor backgroundColor = isHighlighted ? KGlobalSettings::highlightColor() :
                                             palette().brush(QPalette::Background).color();
    if (!urlNavigator()->isActive() && isHighlighted) {
        QColor dimmColor(palette().brush(QPalette::Background).color());
        backgroundColor = mixColors(backgroundColor, dimmColor);
    }

    return backgroundColor;
}

QColor KUrlButton::mixColors(const QColor& c1,
                             const QColor& c2) const
{
    const int red   = (c1.red()   + c2.red())   / 2;
    const int green = (c1.green() + c2.green()) / 2;
    const int blue  = (c1.blue()  + c2.blue())  / 2;
    return QColor(red, green, blue);
}

#include "kurlbutton_p.moc"
