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

#include "kurlnavigatorbutton_p.h"

#include <assert.h>

#include "kurlnavigator.h"

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kglobalsettings.h>
#include <kmenu.h>

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtGui/QStyleOption>

KUrlNavigatorButton::KUrlNavigatorButton(int index, KUrlNavigator* parent) :
    KUrlButton(parent),
    m_index(-1),
    m_popupDelay(0),
    m_listJob(0)
{
    setAcceptDrops(true);
    setIndex(index);
    connect(this, SIGNAL(clicked()), this, SLOT(updateNavigatorUrl()));

    m_popupDelay = new QTimer(this);
    m_popupDelay->setSingleShot(true);
    connect(m_popupDelay, SIGNAL(timeout()), this, SLOT(startListJob()));
    connect(this, SIGNAL(pressed()), this, SLOT(startPopupDelay()));
}

KUrlNavigatorButton::~KUrlNavigatorButton()
{
}

void KUrlNavigatorButton::setIndex(int index)
{
    m_index = index;

    if (m_index < 0) {
        return;
    }

    QString path(urlNavigator()->url().pathOrUrl());
    const QString buttonText = path.section('/', index, index);
    setText(buttonText);

    // Check whether the button indicates the full path of the URL. If
    // this is the case, the button is marked as 'active'.
    ++index;
    QFont adjustedFont(font());
    if (path.section('/', index, index).isEmpty()) {
        setDisplayHintEnabled(ActivatedHint, true);
        adjustedFont.setBold(true);
    } else {
        setDisplayHintEnabled(ActivatedHint, false);
        adjustedFont.setBold(false);
    }

    setFont(adjustedFont);

    QFontMetrics fontMetrics(adjustedFont);
    int minWidth = fontMetrics.width(buttonText) + arrowWidth() + 2 * BorderWidth;
    if (minWidth < 50) {
        minWidth = 50;
    }
    else if (minWidth > 150) {
        // don't let an overlong path name waste all the URL navigator space
        minWidth = 150;
    }
    setMinimumWidth(minWidth);

    update();
}

QSize KUrlNavigatorButton::sizeHint() const
{
    // the minimum size is textWidth + arrowWidth() + 2 * BorderWidth; for the
    // preferred size we add the BorderWidth 2 times again for having an uncluttered look
    const int width = fontMetrics().width(text()) + arrowWidth() + 4 * BorderWidth;
    return QSize(width, KUrlButton::sizeHint().height());
}

void KUrlNavigatorButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());
    const int buttonWidth  = width();
    const int buttonHeight = height();

    const QColor bgColor = backgroundColor();
    const QColor fgColor = foregroundColor();

    // draw button background
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRect(0, 0, buttonWidth, buttonHeight);

    int textLeft = 0;
    int textWidth = buttonWidth;
    painter.setPen(fgColor);

    const bool leftToRight = (layoutDirection() == Qt::LeftToRight);

    if (!isDisplayHintEnabled(ActivatedHint)) {
        // draw arrow
        const int arrowSize = arrowWidth();
        const int arrowX = leftToRight ? (buttonWidth - arrowSize) - BorderWidth : BorderWidth;
        const int arrowY = (buttonHeight - arrowSize) / 2;

        QStyleOption option;
        option.rect = QRect(arrowX, arrowY, arrowSize, arrowSize);
        option.palette = palette();
        option.palette.setColor(QPalette::Text, fgColor);
        option.palette.setColor(QPalette::WindowText, fgColor);
        option.palette.setColor(QPalette::ButtonText, fgColor);

        if (leftToRight) {
            style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &option, &painter, this);
        } else {
            style()->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &option, &painter, this);
            textLeft += arrowSize + 2 * BorderWidth;
        }
        textWidth -= arrowSize + 2 * BorderWidth;
    }

    const bool clipped = isTextClipped();
    const int align = clipped ? Qt::AlignVCenter : Qt::AlignCenter;
    const QRect textRect(textLeft, 0, textWidth, buttonHeight);
    if (clipped) {
        QLinearGradient gradient(textRect.topLeft(), textRect.topRight());
        if (leftToRight) {
            gradient.setColorAt(0.8, fgColor);
            gradient.setColorAt(1.0, bgColor);
        } else {
            gradient.setColorAt(0.0, bgColor);
            gradient.setColorAt(0.2, fgColor);
        }

        QPen pen;
        pen.setBrush(QBrush(gradient));
        painter.setPen(pen);
        painter.drawText(textRect, align, text());
    } else {
        painter.drawText(textRect, align, text());
    }
}

void KUrlNavigatorButton::enterEvent(QEvent* event)
{
    KUrlButton::enterEvent(event);

    // if the text is clipped due to a small window width, the text should
    // be shown as tooltip
    if (isTextClipped()) {
        setToolTip(text());
    }
}

void KUrlNavigatorButton::leaveEvent(QEvent* event)
{
    KUrlButton::leaveEvent(event);
    setToolTip(QString());
}

void KUrlNavigatorButton::dropEvent(QDropEvent* event)
{
    if (m_index < 0) {
        return;
    }

    const KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
    if (!urls.isEmpty()) {
        event->acceptProposedAction();

        setDisplayHintEnabled(DraggedHint, true);

        QString path(urlNavigator()->url().prettyUrl());
        path = path.section('/', 0, m_index + 2);

        emit urlsDropped(urls, KUrl(path));

        setDisplayHintEnabled(DraggedHint, false);
        update();
    }
}

void KUrlNavigatorButton::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        setDisplayHintEnabled(DraggedHint, true);
        event->acceptProposedAction();

        update();
    }
}

void KUrlNavigatorButton::dragLeaveEvent(QDragLeaveEvent* event)
{
    KUrlButton::dragLeaveEvent(event);

    setDisplayHintEnabled(DraggedHint, false);
    update();
}


void KUrlNavigatorButton::updateNavigatorUrl()
{
    stopPopupDelay();

    if (m_index < 0) {
        return;
    }

    urlNavigator()->setUrl(urlNavigator()->url(m_index));
}

void KUrlNavigatorButton::startPopupDelay()
{
    if (m_popupDelay->isActive() || (m_listJob != 0) || (m_index < 0)) {
        return;
    }

    m_popupDelay->start(300);
}

void KUrlNavigatorButton::stopPopupDelay()
{
    m_popupDelay->stop();
    if (m_listJob != 0) {
        m_listJob->kill();
        m_listJob = 0;
    }
}

void KUrlNavigatorButton::startListJob()
{
    if (m_listJob != 0) {
        return;
    }

    const KUrl& url = urlNavigator()->url(m_index);
    m_listJob = KIO::listDir(url, false, false);
    m_subdirs.clear(); // just to be ++safe

    connect(m_listJob, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList &)),
            this, SLOT(entriesList(KIO::Job*, const KIO::UDSEntryList&)));
    connect(m_listJob, SIGNAL(result(KJob*)), this, SLOT(listJobFinished(KJob*)));
}

void KUrlNavigatorButton::entriesList(KIO::Job* job, const KIO::UDSEntryList& entries)
{
    if (job != m_listJob) {
        return;
    }

    KIO::UDSEntryList::const_iterator it = entries.constBegin();
    const KIO::UDSEntryList::const_iterator itEnd = entries.constEnd();

    while (it != itEnd) {
        const KIO::UDSEntry entry = *it;
        if (entry.isDir()) {
            const QString name = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            if ((name != ".") && (name != "..")) {
                m_subdirs.append(name);
            }
        }

        ++it;
    }

    m_subdirs.sort();
}

void KUrlNavigatorButton::listJobFinished(KJob* job)
{
    if (job != m_listJob) {
        return;
    }

    m_listJob = 0;
    if (job->error() || m_subdirs.isEmpty()) {
        // clear listing
        return;
    }

    setDisplayHintEnabled(PopupActiveHint, true);
    update(); // ensure the button is drawn highlighted

    KMenu* dirsMenu = new KMenu(this);
    QStringList::const_iterator it = m_subdirs.constBegin();
    QStringList::const_iterator itEnd = m_subdirs.constEnd();
    int i = 0;
    while (it != itEnd) {
        QAction* action = new QAction(*it, this);
        action->setData(i);
        dirsMenu->addAction(action);
        ++it;
        ++i;
    }

    const QAction* action = dirsMenu->exec(urlNavigator()->mapToGlobal(geometry().bottomLeft()));
    if (action != 0) {
        const int result = action->data().toInt();
        KUrl url = urlNavigator()->url(m_index);
        url.addPath(m_subdirs[result]);
        urlNavigator()->setUrl(url);
    }

    m_subdirs.clear();
    delete dirsMenu;
    dirsMenu = 0;

    setDisplayHintEnabled(PopupActiveHint, false);
}

int KUrlNavigatorButton::arrowWidth() const
{
    int width = height() / 2;
    if (width < 4) {
        width = 4;
    }
    return width;
}

bool KUrlNavigatorButton::isTextClipped() const
{
    int availableWidth = width() - 2 * BorderWidth;
    if (!isDisplayHintEnabled(ActivatedHint)) {
        availableWidth -= arrowWidth() - BorderWidth;
    }

    QFontMetrics fontMetrics(font());
    return fontMetrics.width(text()) >= availableWidth;
}

#include "kurlnavigatorbutton_p.moc"
