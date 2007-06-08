/* This file is part of the KDE libraries

   Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcharselect.h"

#include "kcharselect_p.h"

#include <QtGui/QColor>
#include <QtGui/QFontComboBox>
#include <QtGui/QComboBox>
#include <QtGui/QActionEvent>
#include <QtGui/QFont>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QStyle>

#include <QtGui/QBoxLayout>
#include <QtGui/QSplitter>
#include <QtGui/QTextBrowser>
#include <QtGui/QPushButton>
#include <QtCore/QTimer>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <QtGui/QHeaderView>
#include <klineedit.h>

class KCharSelectTablePrivate
{
public:
    KCharSelectTablePrivate(KCharSelectTable *q): q(q), model(0) {}
    KCharSelectTable *q;

    QFont font;
    KCharSelectItemModel *model;
    QList<QChar> chars;
    QChar chr;

    QTimer* resizeTimer;

    void _k_resizeCells();
    void _k_doubleClicked(const QModelIndex & index);
    void _k_slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous);
};

class KCharSelect::KCharSelectPrivate
{
public:
    KLineEdit* searchLine;
    QLineEdit *unicodeLine;
    QFontComboBox *fontCombo;
    QSpinBox *fontSizeSpinBox;
    QComboBox *sectionCombo;
    QComboBox *blockCombo;
    KCharSelectTable *charTable;
    QTextBrowser *detailBrowser;

    KCharSelect *q;

    QString createLinks(QString s);
    void _k_fontSelected();
    void _k_slotUpdateUnicode(const QChar &c);
    void _k_sectionSelected(int index);
    void _k_blockSelected(int index);
    void _k_searchEditChanged();
    void _k_search();
    void _k_linkClicked(QUrl url);
};

/******************************************************************/
/* Class: KCharSelectTable                                        */
/******************************************************************/

KCharSelectTable::KCharSelectTable(QWidget *parent, const QFont &_font)
        : QTableView(parent), d(new KCharSelectTablePrivate(this))
{
    d->font = _font;
    d->model = 0;

    d->resizeTimer = new QTimer(this);
    d->resizeTimer->setSingleShot(true);
    connect(d->resizeTimer, SIGNAL(timeout()), this, SLOT(_k_resizeCells()));

    setTabKeyNavigation(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    QPalette _palette;
    _palette.setColor(backgroundRole(), palette().color(QPalette::Base));
    setPalette(_palette);
    verticalHeader()->setVisible(false);
    verticalHeader()->setResizeMode(QHeaderView::Custom);
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setResizeMode(QHeaderView::Custom);

    setFocusPolicy(Qt::StrongFocus);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setDragDropMode(QAbstractItemView::DragDrop);

    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(_k_doubleClicked(QModelIndex)));

    d->resizeTimer->start();
}

KCharSelectTable::~KCharSelectTable()
{
    delete d;
}

void KCharSelectTable::setFont(const QFont &_font)
{
    QTableView::setFont(_font);
    d->font = _font;
    if (d->model) d->model->setFont(_font);
    d->resizeTimer->start();
}

QChar KCharSelectTable::chr()
{
    return d->chr;
}

QFont KCharSelectTable::font() const
{
    return d->font;
}

QList<QChar> KCharSelectTable::displayedChars() const
{
    return d->chars;
}

void KCharSelectTable::setChar(const QChar &c)
{
    int pos = d->chars.indexOf(c);
    if (pos != -1) {
        setCurrentIndex(model()->index(pos / model()->columnCount(), pos % model()->columnCount()));
    }
}

void KCharSelectTable::setContents(QList<QChar> chars)
{
    d->chars = chars;

    KCharSelectItemModel *m = d->model;
    d->model = new KCharSelectItemModel(chars, d->font, this);
    setModel(d->model);
    d->resizeTimer->start();
    QItemSelectionModel *selectionModel = new QItemSelectionModel(d->model);
    setSelectionModel(selectionModel);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(selectionModel, SIGNAL(currentChanged(const QModelIndex & , const QModelIndex &)), this, SLOT(_k_slotCurrentChanged(const QModelIndex &, const QModelIndex &)));
    connect(d->model, SIGNAL(showCharRequested(QChar)), this, SIGNAL(showCharRequested(QChar)));
    delete m; // this should hopefully delete aold selection models too, since it is the parent of them (didn't track, if there are setParent calls somewhere. Check that (jowenn)
}

void KCharSelectTable::scrollTo(const QModelIndex & index, ScrollHint hint)
{
    // this prevents horizontal scrolling when selecting a character in the last column
    if (index.isValid() && index.column() != 0) {
        QTableView::scrollTo(d->model->index(index.row(), 0), hint);
    } else {
        QTableView::scrollTo(index, hint);
    }
}

void KCharSelectTablePrivate::_k_slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);
    if (!model) return;
    QVariant temp = model->data(current, KCharSelectItemModel::CharacterRole);
    if (temp.type() != QVariant::Char)
        return;
    QChar c = temp.toChar();
    chr = c;
    emit q->focusItemChanged(c);
}

void KCharSelectTable::resizeEvent(QResizeEvent * e)
{
    QTableView::resizeEvent(e);
    if (e->size().width() != e->oldSize().width()) {
        d->resizeTimer->start();
    }
}

void KCharSelectTablePrivate::_k_resizeCells()
{
    if (!q->model()) return;
    static_cast<KCharSelectItemModel*>(q->model())->updateColumnCount(q->viewport()->size().width());

    QChar oldChar = q->chr();

    const int new_w   = q->viewport()->size().width() / q->model()->columnCount(QModelIndex());
    const int columns = q->model()->columnCount(QModelIndex());
    const int rows = q->model()->rowCount(QModelIndex());
    q->setUpdatesEnabled(false);
    QHeaderView* hv = q->horizontalHeader();
    int spaceLeft = q->viewport()->size().width() % new_w + 1;
    for (int i = 0;i <= columns;i++) {
        if (i < spaceLeft) {
            hv->resizeSection(i, new_w + 1);
        } else {
            hv->resizeSection(i, new_w);
        }
    }

    hv = q->verticalHeader();
    const int new_h = QFontMetrics(font).xHeight() * 3;
    for (int i = 0;i < rows;i++) {
        hv->resizeSection(i, new_h);
    }

    q->setUpdatesEnabled(true);
    q->setChar(oldChar);
}

void KCharSelectTablePrivate::_k_doubleClicked(const QModelIndex & index)
{
    QChar c = model->data(index, KCharSelectItemModel::CharacterRole).toChar();
    if (c.isPrint()) {
        emit q->activated(c);
    }
}

void KCharSelectTable::keyPressEvent(QKeyEvent *e)
{
    if (d->model)
        switch (e->key()) {
        case Qt::Key_Space:
            emit activated(' ');
            return;
            break;
    case Qt::Key_Enter: case Qt::Key_Return: {
            if (!currentIndex().isValid()) return;
            QChar c = d->model->data(currentIndex(), KCharSelectItemModel::CharacterRole).toChar();
            if (c.isPrint()) {
                emit activated(c);
            }
        }
        return;
        break;
        }
    QTableView::keyPressEvent(e);
}


/******************************************************************/
/* Class: KCharSelect                                             */
/******************************************************************/

KCharSelect::KCharSelect(QWidget *parent, const Controls controls)
        : QWidget(parent), d(new KCharSelectPrivate)
{
    d->q = this;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    if (SearchLine & controls) {
        QHBoxLayout *searchLayout = new QHBoxLayout();
        mainLayout->addLayout(searchLayout);
        d->searchLine = new KLineEdit(this);
        searchLayout->addWidget(d->searchLine);
        d->searchLine->setClickMessage(i18n("Enter a search term here"));
        d->searchLine->setClearButtonShown(true);
        d->searchLine->setToolTip(i18n("Enter a search term here"));
        connect(d->searchLine, SIGNAL(textChanged(QString)), this, SLOT(_k_searchEditChanged()));

        QPushButton* searchButton = new QPushButton(i18n("Search"), this);
        searchLayout->addWidget(searchButton);
        connect(d->searchLine, SIGNAL(returnPressed(QString)), searchButton, SLOT(animateClick()));
        connect(searchButton, SIGNAL(pressed()), this, SLOT(_k_search()));
    }

    if ((SearchLine & controls) && ((FontCombo & controls) || (FontSize & controls) || (BlockCombos & controls))) {
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line);
    }

    QHBoxLayout *comboLayout = new QHBoxLayout();


    d->fontCombo = new QFontComboBox(this);
    comboLayout->addWidget(d->fontCombo);
    d->fontCombo->setEditable(true);
    d->fontCombo->resize(d->fontCombo->sizeHint());
    d->fontCombo->setToolTip(i18n("Set font"));

    d->fontSizeSpinBox = new QSpinBox(this);
    comboLayout->addWidget(d->fontSizeSpinBox);
    d->fontSizeSpinBox->setValue(QWidget::font().pointSize());
    d->fontSizeSpinBox->setRange(1, 400);
    d->fontSizeSpinBox->setSingleStep(1);
    d->fontSizeSpinBox->setToolTip(i18n("Set font size"));

    if (((FontCombo & controls) || (FontSize & controls)) && (BlockCombos & controls)) {
        comboLayout->addSpacing(20);
    }
    connect(d->fontCombo, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(_k_fontSelected()));
    connect(d->fontSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(_k_fontSelected()));


    d->sectionCombo = new QComboBox(this);
    d->sectionCombo->setToolTip(i18n("Select a category"));
    comboLayout->addWidget(d->sectionCombo);
    d->blockCombo = new QComboBox(this);
    d->blockCombo->setToolTip(i18n("Select a block to be displayed"));
    d->blockCombo->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    comboLayout->addWidget(d->blockCombo, 1);
    d->sectionCombo->addItems(KCharSelectData::sectionList());
    d->blockCombo->setMinimumWidth(QFontMetrics(QWidget::font()).averageCharWidth() * 25);

    connect(d->sectionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_k_sectionSelected(int)));
    connect(d->blockCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_k_blockSelected(int)));
    if ((FontCombo & controls) || (FontSize & controls) || (BlockCombos & controls)) {
        mainLayout->addLayout(comboLayout);
    }
    if (!(FontCombo & controls)) {
        d->fontCombo->hide();
    }
    if (!(FontSize & controls)) {
        d->fontSizeSpinBox->hide();
    }
    if (!(BlockCombos & controls)) {
        d->sectionCombo->hide();
        d->blockCombo->hide();
    }

    QSplitter *splitter = new QSplitter(this);
    if ((CharacterTable & controls) || (DetailBrowser & controls)) {
        mainLayout->addWidget(splitter);
    } else {
        splitter->hide();
    }
    d->charTable = new KCharSelectTable(this, QFont());
    if (CharacterTable & controls) {
        splitter->addWidget(d->charTable);
        d->charTable->setFocus(Qt::OtherFocusReason);
    } else {
        d->charTable->hide();
    }

    const QSize sz(200, 200);
    d->charTable->resize(sz);
    d->charTable->setMinimumSize(sz);

    d->charTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setCurrentFont(QFont());

    connect(d->charTable, SIGNAL(focusItemChanged(const QChar &)), this, SLOT(_k_slotUpdateUnicode(const QChar &)));
    connect(d->charTable, SIGNAL(activated(const QChar &)), this, SIGNAL(charSelected(const QChar &)));
    connect(d->charTable, SIGNAL(focusItemChanged(const QChar &)),
            this, SIGNAL(currentCharChanged(const QChar &)));

    connect(d->charTable, SIGNAL(showCharRequested(QChar)), this, SLOT(setCurrentChar(QChar)));

    d->detailBrowser = new QTextBrowser(this);
    if (DetailBrowser & controls) {
        splitter->addWidget(d->detailBrowser);
    } else {
        d->detailBrowser->hide();
    }
    connect(d->detailBrowser, SIGNAL(anchorClicked(QUrl)), this, SLOT(_k_linkClicked(QUrl)));

    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(d->charTable);
    d->_k_sectionSelected(0);
    d->_k_blockSelected(0);
    setCurrentChar(0x0);
}

KCharSelect::~KCharSelect()
{
    delete d;
}

QSize KCharSelect::sizeHint() const
{
    return QWidget::sizeHint();
}

void KCharSelect::setCurrentFont(const QFont &_font)
{
    d->fontCombo->setCurrentFont(_font);
    d->fontSizeSpinBox->setValue(_font.pointSize());
    d->_k_fontSelected();
}

QChar KCharSelect::currentChar() const
{
    return d->charTable->chr();
}

QFont KCharSelect::currentFont() const
{
    return d->charTable->font();
}

QList<QChar> KCharSelect::displayedChars() const
{
    return d->charTable->displayedChars();
}

void KCharSelect::setCurrentChar(const QChar &c)
{
    int block = KCharSelectData::blockIndex(c);
    int section = KCharSelectData::sectionIndex(block);
    d->sectionCombo->setCurrentIndex(section);
    int index = d->blockCombo->findData(block);
    if (index != -1) {
        d->blockCombo->setCurrentIndex(index);
    }
    d->charTable->setChar(c);
}

void KCharSelect::KCharSelectPrivate::_k_fontSelected()
{
    QFont font = fontCombo->currentFont();
    font.setPointSize(fontSizeSpinBox->value());
    charTable->setFont(font);
    emit q->currentFontChanged(font);
}

void KCharSelect::KCharSelectPrivate::_k_slotUpdateUnicode(const QChar &c)
{
    QString html;
    if (c.isPrint()) {
        html = QString("<p>" + i18n("Character:") + " <font size=\"+4\" face=\"") + charTable->font().family() + "\">&#" + QString::number(c.unicode()) + ";</font> " + KCharSelectData::formatCode(c.unicode())  + "<br>";
    } else {
        html = QString("<p>" + i18n("Character:") + " <b>" + i18n("Non-printable") + "</b> ") + KCharSelectData::formatCode(c.unicode())  + "<br>";
    }
    QString name = KCharSelectData::name(c);
    if (!name.isEmpty()) {
        html += i18n("Name: ") + Qt::escape(name) + "</p>";
    }
    QStringList aliases = KCharSelectData::aliases(c);
    QStringList notes = KCharSelectData::notes(c);
    QList<QChar> seeAlso = KCharSelectData::seeAlso(c);
    QStringList equivalents = KCharSelectData::equivalents(c);
    QStringList approxEquivalents = KCharSelectData::approximateEquivalents(c);
    if (!(aliases.isEmpty() && notes.isEmpty() && seeAlso.isEmpty() && equivalents.isEmpty() && approxEquivalents.isEmpty())) {
        html += "<p><b>" + i18n("Annotations and Cross References") + "</b></p>";
    }

    if (!aliases.isEmpty()) {
        html += "<p style=\"margin-bottom: 0px;\">" + i18n("Alias names:") + "</p><ul style=\"margin-top: 0px;\">";
        foreach(QString alias, aliases) {
            html += "<li>" + Qt::escape(alias) + "</li>";
        }
        html += "</ul>";
    }

    if (!notes.isEmpty()) {
        html += "<p style=\"margin-bottom: 0px;\">" + i18n("Notes:") + "</p><ul style=\"margin-top: 0px;\">";
        foreach(QString note, notes) {
            html += "<li>" + createLinks(Qt::escape(note)) + "</li>";
        }
        html += "</ul>";
    }

    if (!seeAlso.isEmpty()) {
        html += "<p style=\"margin-bottom: 0px;\">" + i18n("See also:") + "</p><ul style=\"margin-top: 0px;\">";
        foreach(QChar c2, seeAlso) {
            html += "<li><a href=\"" + QString::number(c2.unicode(), 16) + "\">";
            if (c2.isPrint()) {
                html += "&#" + QString::number(c2.unicode()) + "; ";
            }
            html += KCharSelectData::formatCode(c2.unicode()) + ' ' + Qt::escape(KCharSelectData::name(c2)) + "</a></li>";
        }
        html += "</ul>";
    }

    if (!equivalents.isEmpty()) {
        html += "<p style=\"margin-bottom: 0px;\">" + i18n("Equivalents:") + "</p><ul style=\"margin-top: 0px;\">";
        foreach(QString equivalent, equivalents) {
            html += "<li>" + createLinks(Qt::escape(equivalent)) + "</li>";
        }
        html += "</ul>";
    }

    if (!approxEquivalents.isEmpty()) {
        html += "<p style=\"margin-bottom: 0px;\">" + i18n("Approximate equivalents:") + "</p><ul style=\"margin-top: 0px;\">";
        foreach(QString approxEquivalent, approxEquivalents) {
            html += "<li>" + createLinks(Qt::escape(approxEquivalent)) + "</li>";
        }
        html += "</ul>";
    }

    QStringList unihan = KCharSelectData::unihanInfo(c);
    if (unihan.count() == 7) {
        html += "<p><b>" + i18n("CJK Ideograph Information") + "</b></p><p>";
        bool newline = true;
        if (!unihan[0].isEmpty()) {
            html += i18n("Definition in English: ") + unihan[0];
            newline = false;
        }
        if (!unihan[2].isEmpty()) {
            if (!newline) html += "<br>";
            html += i18n("Mandarin Pronunciation: ") + unihan[2];
            newline = false;
        }
        if (!unihan[1].isEmpty()) {
            if (!newline) html += "<br>";
            html += i18n("Cantonese Pronunciation: ") + unihan[1];
            newline = false;
        }
        if (!unihan[6].isEmpty()) {
            if (!newline) html += "<br>";
            html += i18n("Japanese On Pronunciation: ") + unihan[6];
            newline = false;
        }
        if (!unihan[5].isEmpty()) {
            if (!newline) html += "<br>";
            html += i18n("Japanese Kun Pronunciation: ") + unihan[5];
            newline = false;
        }
        if (!unihan[3].isEmpty()) {
            if (!newline) html += "<br>";
            html += i18n("Tang Pronunciation: ") + unihan[3];
            newline = false;
        }
        if (!unihan[4].isEmpty()) {
            if (!newline) html += "<br>";
            html += i18n("Korean Pronunciation: ") + unihan[4];
            newline = false;
        }
        html += "</p>";
    }

    html += "<p><b>" + i18n("General Character Properties") + "</b><br>";
    html += i18n("Block: ") + KCharSelectData::block(c) + "<br>";
    html += i18n("Unicode category: ") + KCharSelectData::categoryText(c.category()) + "</p>";

    QByteArray utf8 = QString(c).toUtf8();

    html += "<p><b>" + i18n("Various Useful Representations") + "</b><br>";
    html += i18n("UTF-8:");
    foreach(unsigned char c, utf8)
    html += ' ' + KCharSelectData::formatCode(c, 2, "0x");
    html += "<br>" + i18n("UTF-16: ") + KCharSelectData::formatCode(c.unicode(), 4, "0x") + "<br>";
    html += i18n("C octal escaped UTF-8: ");
    foreach(unsigned char c, utf8)
    html += KCharSelectData::formatCode(c, 3, "\\", 7);
    html += "<br>" + i18n("XML decimal entity:") + " &amp;#" + QString::number(c.unicode()) + ";</p>";

    detailBrowser->setHtml(html);
}

QString KCharSelect::KCharSelectPrivate::createLinks(QString s)
{
    QRegExp rx("\\b([\\dABCDEF]{4})\\b");

    QStringList chars;
    int pos = 0;

    while ((pos = rx.indexIn(s, pos)) != -1) {
        chars << rx.cap(1);
        pos += rx.matchedLength();
    }

    foreach(QString c, chars) {
        int unicode = c.toInt(0, 16);
        QString link = "<a href=\"" + c + "\">";
        if (QChar(unicode).isPrint()) {
            link += "&#" + QString::number(unicode) + "; U+" + c + ' ';
        }
        link += KCharSelectData::name(QChar(unicode)) + "</a>";
        s.replace(c, link);
    }
    return s;
}

void KCharSelect::KCharSelectPrivate::_k_sectionSelected(int index)
{
    blockCombo->clear();
    QList<int> blocks = KCharSelectData::sectionContents(index);
    foreach(int block, blocks) {
        blockCombo->addItem(KCharSelectData::blockName(block), QVariant(block));
    }
    blockCombo->setCurrentIndex(0);
}

void KCharSelect::KCharSelectPrivate::_k_blockSelected(int index)
{
    int block = blockCombo->itemData(index).toInt();
    QList<QChar> contents = KCharSelectData::blockContents(block);
    if(contents.count() <= index) {
        return;
    }
    charTable->setContents(contents);
    emit q->displayedCharsChanged();
    charTable->setChar(contents[0]);
}

void KCharSelect::KCharSelectPrivate::_k_searchEditChanged()
{
    if (searchLine->text().isEmpty()) {
        sectionCombo->setEnabled(true);
        blockCombo->setEnabled(true);
        _k_blockSelected(blockCombo->currentIndex());
    } else {
        sectionCombo->setEnabled(false);
        blockCombo->setEnabled(false);
    }
}

void KCharSelect::KCharSelectPrivate::_k_search()
{
    if (searchLine->text().isEmpty()) {
        return;
    }
    charTable->setContents(KCharSelectData::find(searchLine->text()));
}

void  KCharSelect::KCharSelectPrivate::_k_linkClicked(QUrl url)
{
    QString hex = url.toString();
    if (hex.size() > 4) {
        return;
    }
    int unicode = hex.toInt(0, 16);
    q->setCurrentChar(QChar(unicode));
}

#include "kcharselect.moc"
#include "kcharselect_p.moc"
