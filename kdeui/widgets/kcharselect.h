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

#ifndef kcharselect_h
#define kcharselect_h

#include <qtableview.h>

#include <qstring.h>
#include <qpoint.h>
#include <qstringlist.h>

#include <kvbox.h>

class QFontComboBox;
class QFont;
class QFontDatabase;
class QMouseEvent;
class QSpinBox;
class KCharSelectTablePrivate;
class KCharSelectPrivate;
class KCharSelectItemModel;
/**
 * @short Character selection table
 *
 * A table widget which displays the characters of a font. Internally
 * used by KCharSelect. See the KCharSelect documentation for further
 * details.
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 */

class KDEUI_EXPORT KCharSelectTable : public QTableView
{
    Q_OBJECT

public:
    /**
     * Constructor. Using @p _font, draw a table of chars from unicode
     * table @p _tableNum. Character @p _chr in this table is highlighted.
     */
    KCharSelectTable( QWidget *parent, const QString &_font,
		      const QChar &_chr, int _tableNum );

    virtual QSize sizeHint() const;
    virtual void resizeEvent( QResizeEvent * );

    /** Set the font (name) to be displayed to @p _font . */
    virtual void setFont( const QString &_font );
    /** Set the highlighted character to @p _chr . */
    virtual void setChar( const QChar &_chr );
    /** Set the table number (offset of 256 characters into
        the font) to @p _tableNum . */
    /*virtual no more*/ void setTableNum( int _tableNum );

    /** @return Currently highlighted character. */
    virtual QChar chr() { return vChr; }

protected:
    //virtual void paintCell( class QPainter *p, int row, int col );

    virtual void mousePressEvent( QMouseEvent *e ) {  mouseMoveEvent( e ); QTableView::mousePressEvent(e);}
    virtual void mouseDoubleClickEvent ( QMouseEvent *e ){  mouseMoveEvent( e ); emit doubleClicked();}
    virtual void mouseReleaseEvent( QMouseEvent *e ) { mouseMoveEvent( e ); emit activated( chr() ); emit activated(); }
    virtual void mouseMoveEvent( QMouseEvent *e );

    virtual void keyPressEvent( QKeyEvent *e );

    /** Current font name. @see setFont() */
    QString vFont;
    /** Currently highlighted character. @see chr() @see setChar() */
    QChar vChr;
    /** Current table number. @see setTable() */
    int vTableNum;
    QPoint vPos;
    QChar focusItem;
    QPoint focusPos;
    int temp;

Q_SIGNALS:
    void activated( const QChar &c );
    void activated();
    void focusItemChanged();
    void focusItemChanged( const QChar &c );
    void tableUp();
    void tableDown();
    void doubleClicked();

private Q_SLOTS:
    void slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    
private:
    virtual void setFont(const QFont &f) { QTableView::setFont(f); }
private:
    KCharSelectItemModel *m_model;
    KCharSelectTablePrivate* const d;
};

/**
 * @short Character selection widget
 *
 * This widget allows the user to select a character of a
 * specified font in a table
 *
 * \image html kcharselect.png "Character Selection Widget"
 *
 * You can specify the font whose characters should be displayed via
 * setFont() or in the constructor. Using enableFontCombo() you can allow the
 * user to choose the font from a combob-box. As only 256 characters
 * are displayed at once in the table, using the spinbox on the top
 * the user can choose starting from which character the table
 * displays them. This spinbox also can be enabled or disabled using
 * enableTableSpinBox().
 *
 * KCharSelect supports keyboard and mouse navigation. Click+Move
 * always selects the character below the mouse cursor. Using the
 * arrow keys moves the focus mark around and pressing RETURN
 * or SPACE selects the cell which contains the focus mark.
 *
 * To get the current selected character, use the chr()
 * method. You can set the character which should be displayed with
 * setChar() and the table number which should be displayed with
 * setTableNum().
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 */

class KDEUI_EXPORT KCharSelect : public KVBox
{
    Q_OBJECT
    Q_PROPERTY( QString fontFamily READ font WRITE setFont )
    Q_PROPERTY( int tableNum READ tableNum WRITE setTableNum )
    Q_PROPERTY( bool fontComboEnabled READ isFontComboEnabled WRITE enableFontCombo )
    Q_PROPERTY( bool tableSpinBoxEnabled READ isTableSpinBoxEnabled WRITE enableTableSpinBox )

public:
    /**
     * Constructor. @p font specifies which font should be displayed, @p
     * chr which character should be selected and @p tableNum specifies
     * the number of the table which should be displayed.
     */
    KCharSelect( QWidget *parent,
		 const QString &font = QString(), const QChar &chr = ' ', int tableNum = 0 );
    ~KCharSelect();
    /**
     * Reimplemented.
     */
    virtual QSize sizeHint() const;

    /**
     * Sets the font which is displayed to @p font
     */
    void setFont( const QString &font );

    /**
     * Sets the currently selected character to @p chr.
     */
    void setChar( const QChar &chr );

    /**
     * Sets the currently displayed table to @p tableNum.
     */
    void setTableNum( int tableNum );

    /**
     * Returns the currently selected character.
     */
    virtual QChar chr() const;

    /**
     * Returns the currently displayed font.
     */
    QString font() const;

    /**
     * Returns the currently displayed table
     */
    int tableNum() const;

    /**
     * If @p e is set to true, the combobox which allows the user to
     * select the font which should be displayed is enabled, else
     * disabled.
     */
    void enableFontCombo( bool e );

    /**
     * If @p e is set to true, the spinbox which allows the user to
     * specify which characters of the font should be displayed, is
     * enabled, else disabled.
     */
    void enableTableSpinBox( bool e );

    /**
     * Returns wether the font combobox on the top is enabled or
     * disabled.
     *
     * @see enableFontCombo()
     */
    bool isFontComboEnabled() const;

    /**
     * Returns wether the table spinbox on the top is enabled or
     * disabled.
     *
     * @see enableTableSpinBox()
     */
    bool isTableSpinBoxEnabled() const;

Q_SIGNALS:
    void highlighted( const QChar &c );
    void highlighted();
    void activated( const QChar &c );
    void activated();
    void fontChanged( const QString &_font );
    void focusItemChanged();
    void focusItemChanged( const QChar &c );
    void doubleClicked();

private:
    Q_PRIVATE_SLOT(d, void _k_charTableUp())
    Q_PRIVATE_SLOT(d, void _k_charTableDown())
    Q_PRIVATE_SLOT(d, void _k_fontSelected( const QString &))
    Q_PRIVATE_SLOT(d, void _k_tableChanged( int _value ))
    Q_PRIVATE_SLOT(d, void _k_slotUnicodeEntered())
    Q_PRIVATE_SLOT(d, void _k_slotUpdateUnicode( const QChar &c ))

    class KCharSelectPrivate;
    KCharSelectPrivate* const d;
};

#endif
