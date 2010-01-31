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

#include "kurlnavigator.h"
#include "kurlnavigatormenu_p.h"
#include "kdirsortfilterproxymodel.h"

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstringhandler.h>

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtGui/QStyleOption>

QPointer<KUrlNavigatorMenu> KUrlNavigatorButton::m_dirsMenu;

KUrlNavigatorButton::KUrlNavigatorButton(const KUrl& url, KUrlNavigator* parent) :
    KUrlButton(parent),
    m_hoverArrow(false),
    m_pendingTextChange(false),
    m_url(url),
    m_subDir(),
    m_popupDelay(0),
    m_listJob(0)
{
    setAcceptDrops(true);
    setUrl(url);
    setMouseTracking(true);

    m_popupDelay = new QTimer(this);
    m_popupDelay->setSingleShot(true);
    connect(m_popupDelay, SIGNAL(timeout()), this, SLOT(startListJob()));
    connect(this, SIGNAL(pressed()), this, SLOT(startPopupDelay()));
    connect(this, SIGNAL(clicked()), this, SLOT(stopPopupDelay()));
}

KUrlNavigatorButton::~KUrlNavigatorButton()
{
}

void KUrlNavigatorButton::setUrl(const KUrl& url)
{
    m_url = url;

    if (m_url.isLocalFile()) {
        setText(m_url.fileName());
    } else {
        m_pendingTextChange = true;
        KIO::StatJob* job = KIO::stat(m_url, KIO::HideProgressInfo);
        connect(job, SIGNAL(result(KJob*)),
                this, SLOT(statFinished(KJob*)));
    }
}

KUrl KUrlNavigatorButton::url() const
{
    return m_url;
}

void KUrlNavigatorButton::setText(const QString& text)
{
    QString adjustedText = text;
    if (adjustedText.isEmpty()) {
        adjustedText = m_url.protocol();
    }
    KUrlButton::setText(adjustedText);
    updateMinimumWidth();

    // assure that statFinished() does not overwrite a text that has been
    // set by a client of the URL navigator button
    m_pendingTextChange = false;
}

void KUrlNavigatorButton::setActiveSubDirectory(const QString& subDir)
{
    m_subDir = subDir;

    QFont adjustedFont(font());
    adjustedFont.setBold(m_subDir.isEmpty());
    setFont(adjustedFont);

    update();
}

QString KUrlNavigatorButton::activeSubDirectory() const
{
    return m_subDir;
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
    Q_UNUSED(event);

    QPainter painter(this);

    int buttonWidth  = width();
    int preferredWidth = sizeHint().width();
    if (preferredWidth < minimumWidth()) {
        preferredWidth = minimumWidth();
    }
    if (buttonWidth > preferredWidth) {
        buttonWidth = preferredWidth;
    }
    const int buttonHeight = height();

    const QColor fgColor = foregroundColor();
    drawHoverBackground(&painter);

    int textLeft = 0;
    int textWidth = buttonWidth;

    const bool leftToRight = (layoutDirection() == Qt::LeftToRight);

    if (!m_subDir.isEmpty()) {
        // draw arrow
        const int arrowSize = arrowWidth();
        const int arrowX = leftToRight ? (buttonWidth - arrowSize) - BorderWidth : BorderWidth;
        const int arrowY = (buttonHeight - arrowSize) / 2;

        QStyleOption option;
        option.initFrom(this);
        option.rect = QRect(arrowX, arrowY, arrowSize, arrowSize);
        option.palette = palette();
        option.palette.setColor(QPalette::Text, fgColor);
        option.palette.setColor(QPalette::WindowText, fgColor);
        option.palette.setColor(QPalette::ButtonText, fgColor);

        if (m_hoverArrow) {
            // highlight the background of the arrow to indicate that the directories
            // popup can be opened by a mouse click
            QColor hoverColor = palette().color(QPalette::HighlightedText);
            hoverColor.setAlpha(96);
            painter.setPen(Qt::NoPen);
            painter.setBrush(hoverColor);

            int hoverX = arrowX;
            if (!leftToRight) {
                hoverX -= BorderWidth;
            }
            painter.drawRect(QRect(hoverX, 0, arrowSize + BorderWidth, buttonHeight));
        }

        if (leftToRight) {
            style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &option, &painter, this);
        } else {
            style()->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &option, &painter, this);
            textLeft += arrowSize + 2 * BorderWidth;
        }

        textWidth -= arrowSize + 2 * BorderWidth;
    }

    painter.setPen(fgColor);
    const bool clipped = isTextClipped();
    const int align = clipped ? Qt::AlignVCenter : Qt::AlignCenter;
    const QRect textRect(textLeft, 0, textWidth, buttonHeight);
    if (clipped) {
        QColor bgColor = fgColor;
        bgColor.setAlpha(0);
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
    }
    painter.drawText(textRect, align, text());
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

    if (m_hoverArrow) {
        m_hoverArrow = false;
        update();
    }
}

void KUrlNavigatorButton::dropEvent(QDropEvent* event)
{
    const KUrl::List urls = KUrl::List::fromMimeData(event->mimeData());
    if (!urls.isEmpty()) {
        setDisplayHintEnabled(DraggedHint, true);

        emit urlsDropped(m_url, event);

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

void KUrlNavigatorButton::dragMoveEvent(QDragMoveEvent* event)
{
    QRect rect = event->answerRect();
    if (isAboveArrow(rect.center().x())) {
        m_hoverArrow = true;
        update();

        if (m_dirsMenu == 0) {
            startPopupDelay();
        } else if (m_dirsMenu->parent() != this) {
            m_dirsMenu->close();
            m_dirsMenu->deleteLater();
            m_dirsMenu = 0;

            startPopupDelay();
        }
    } else {
        if (m_popupDelay->isActive()) {
            stopPopupDelay();
        }
        delete m_dirsMenu;
        m_dirsMenu = 0;
        m_hoverArrow = false;
        update();
    }
}

void KUrlNavigatorButton::dragLeaveEvent(QDragLeaveEvent* event)
{
    KUrlButton::dragLeaveEvent(event);

    m_hoverArrow = false;
    setDisplayHintEnabled(DraggedHint, false);
    update();
}

void KUrlNavigatorButton::mousePressEvent(QMouseEvent* event)
{
    if (isAboveArrow(event->x()) && (event->button() == Qt::LeftButton)) {
        // the mouse is pressed above the [>] button
        startListJob();
    }
    KUrlButton::mousePressEvent(event);
}

void KUrlNavigatorButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (!isAboveArrow(event->x()) || (event->button() != Qt::LeftButton)) {
        // the mouse has been released above the text area and not
        // above the [>] button
        emit clicked(m_url, event->button());
    }
    KUrlButton::mouseReleaseEvent(event);
}

void KUrlNavigatorButton::mouseMoveEvent(QMouseEvent* event)
{
    KUrlButton::mouseMoveEvent(event);

    const bool hoverArrow = isAboveArrow(event->x());
    if (hoverArrow != m_hoverArrow) {
        m_hoverArrow = hoverArrow;
        update();
    }
}

void KUrlNavigatorButton::startPopupDelay()
{
    if (!m_popupDelay->isActive() && (m_listJob == 0)) {
        m_popupDelay->start(300);
    }
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

    m_listJob = KIO::listDir(m_url, KIO::HideProgressInfo, false /*no hidden files*/);
    m_subDirs.clear(); // just to be ++safe

    connect(m_listJob, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList &)),
            this, SLOT(entriesList(KIO::Job*, const KIO::UDSEntryList&)));
    connect(m_listJob, SIGNAL(result(KJob*)), this, SLOT(listJobFinished(KJob*)));
}

void KUrlNavigatorButton::entriesList(KIO::Job* job, const KIO::UDSEntryList& entries)
{
    if (job != m_listJob) {
        return;
    }

    foreach (const KIO::UDSEntry& entry, entries) {
        if (entry.isDir()) {
            QString name = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            QString displayName = entry.stringValue(KIO::UDSEntry::UDS_DISPLAY_NAME);
            if (displayName.isEmpty()) {
                displayName = name;
            }
            if ((name != ".") && (name != "..")) {
                m_subDirs.append(qMakePair(name, displayName));
            }
        }
    }
}

void KUrlNavigatorButton::urlsDropped(QAction* action, QDropEvent* event)
{
    const int result = action->data().toInt();
    KUrl url = m_url;
    url.addPath(m_subDirs.at(result).first);
    urlsDropped(url, event);
}

void KUrlNavigatorButton::statFinished(KJob* job)
{
    if (m_pendingTextChange) {
        m_pendingTextChange = false;

        const KIO::UDSEntry entry = static_cast<KIO::StatJob*>(job)->statResult();
        QString name = entry.stringValue(KIO::UDSEntry::UDS_DISPLAY_NAME);
        if (name.isEmpty()) {
            name = m_url.fileName();
        }
        setText(name);
    }
}

/**
 * Helper function for listJobFinished
 */
static bool naturalLessThan(const QPair<QString, QString>& s1, const QPair<QString, QString>& s2)
{
    return KStringHandler::naturalCompare(s1.first, s2.first, Qt::CaseInsensitive) < 0;
}

void KUrlNavigatorButton::listJobFinished(KJob* job)
{
    if (job != m_listJob) {
        return;
    }

    m_listJob = 0;
    if (job->error() || m_subDirs.isEmpty()) {
        // clear listing
        return;
    }

    qSort(m_subDirs.begin(), m_subDirs.end(), naturalLessThan);
    setDisplayHintEnabled(PopupActiveHint, true);
    update(); // ensure the button is drawn highlighted

    if (m_dirsMenu != 0) {
        m_dirsMenu->close();
        m_dirsMenu->deleteLater();
        m_dirsMenu = 0;
    }

    m_dirsMenu = new KUrlNavigatorMenu(this);
    connect(m_dirsMenu, SIGNAL(urlsDropped(QAction*, QDropEvent*)),
            this, SLOT(urlsDropped(QAction*, QDropEvent*)));

    m_dirsMenu->setLayoutDirection(Qt::LeftToRight);

    const int subDirsCount = m_subDirs.count();
    for (int i = 0; i < subDirsCount; ++i) {
        const QString subDirName = m_subDirs[i].first;
        const QString subDirDisplayName = m_subDirs[i].second;
        QString text = KStringHandler::csqueeze(subDirDisplayName, 60);
        text.replace('&', "&&");
        QAction* action = new QAction(text, this);
        if (m_subDir == subDirName) {
            QFont font(action->font());
            font.setBold(true);
            action->setFont(font);
        }
        action->setData(i);
        m_dirsMenu->addAction(action);

        if (i > 100) {
            // Opening a menu with several 100 items makes no sense from
            // a usability view. Also there are implementation issues in
            // QMenu if the number of menu items don't fit into the available
            // screen -> skip remaining items
            QAction* limitReached = new QAction("...", this);
            limitReached->setEnabled(false);
            m_dirsMenu->addAction(limitReached);
            break;
        }
    }

    const bool leftToRight = (layoutDirection() == Qt::LeftToRight);
    const int popupX = leftToRight ? width() - arrowWidth() - BorderWidth : 0;
    const QPoint popupPos  = parentWidget()->mapToGlobal(geometry().bottomLeft() + QPoint(popupX, 0));

    const QAction* action = m_dirsMenu->exec(popupPos);
    if (action != 0) {
        const int result = action->data().toInt();
        KUrl url = m_url;
        url.addPath(m_subDirs[result].first);
        emit clicked(url, Qt::LeftButton);
    }

    m_subDirs.clear();
    delete m_dirsMenu;
    m_dirsMenu = 0;

    setDisplayHintEnabled(PopupActiveHint, false);
}

int KUrlNavigatorButton::arrowWidth() const
{
    // if there isn't arrow then return 0
    int width = 0;
    if (!m_subDir.isEmpty()) {
        width = height() / 2;
        if (width < 4) {
            width = 4;
        }
    }

    return width;
}

bool KUrlNavigatorButton::isAboveArrow(int x) const
{
    const bool leftToRight = (layoutDirection() == Qt::LeftToRight);
    return leftToRight ? (x >= width() - arrowWidth()) : (x < arrowWidth());
}

bool KUrlNavigatorButton::isTextClipped() const
{
    int availableWidth = width() - 2 * BorderWidth;
    if (!m_subDir.isEmpty()) {
        availableWidth -= arrowWidth() - BorderWidth;
    }

    QFontMetrics fontMetrics(font());
    return fontMetrics.width(text()) >= availableWidth;
}

void KUrlNavigatorButton::updateMinimumWidth()
{
    const int oldMinWidth = minimumWidth();

    int minWidth = sizeHint().width();
    if (minWidth < 40) {
        minWidth = 40;
    }
    else if (minWidth > 150) {
        // don't let an overlong path name waste all the URL navigator space
        minWidth = 150;
    }
    if (oldMinWidth != minWidth) {
        setMinimumWidth(minWidth);
    }
}

#include "kurlnavigatorbutton_p.moc"
