/* This file is part of the KDE libraries

   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KCOMPLETIONBOX_H
#define KCOMPLETIONBOX_H

#include <qevent.h>
#include <qstringlist.h>

#include <klistbox.h>

/**
 * A little utility class for "completion-widgets", like KLineEdit or
 * KComboBox. KCompletionBox is a listbox, displayed as a rectangle without
 * any window-decoration, usually directly under the lineedit or combobox.
 * It is filled with all possible matches for a completion, so the user
 * can select the one he wants.
 *
 * It is used when KGlobalSettings::Completion == CompletionPopup.
 *
 * @short A helper widget for "completion-widgets" (KLineEdit, KComboBox))
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KCompletionBox : public KListBox
{
    Q_OBJECT

public:
    /**
     * Constructs a KCompletionBox.
     *
     * Notice: the parent needs to be always 0L,
     * so you can't specify it in the constructor. Because of that, Qt's
     * auto-deletion does not take place, so you have to explicitly delete
     * this widget when you don't need it anymore.
     *
     * The parent widget is used to give the focus back when pressing the
     * up-button on the very first item.
     */
    KCompletionBox( QWidget *parent, const char *name = 0 );

    /**
     * Destroys the box
     */
    ~KCompletionBox();

    virtual QSize sizeHint() const;

public slots:
    /**
     * @returns a list of all items currently in the box.
     */
    QStringList items() const;

    /**
     * Adjusts the size of the box to fit the width of the parent given in the
     * constructor and pops it up at the most appropriate place, relative to
     * the parent.
     *
     * Depending on the screensize and the position of the parent, this may
     * be a different place, however the default is to pop it up and the
     * lower left corner of the parent.
     *
     * Make sure to hide() the box when appropriate.
     */
    virtual void popup();

    /**
     * Reimplemented for internal reasons.
     */
    virtual void show();
    
signals:
    /**
     * Emitted when an item was selected, contains the text of the selected
     * item.
     */
    void activated( const QString& );

protected:
    /**
     * Reimplemented from KListBox to get events from the viewport (to hide
     * this widget on mouse-click, Escape-presses, etc.
     */
    virtual bool eventFilter( QObject *, QEvent * );

protected slots:
    /**
     * Called when an item was activated. Hides the widget and emits
     * @ref activated() with the item.
     */
    virtual void slotActivated( QListBoxItem * );

private slots:
    void slotSetCurrentItem( QListBoxItem *i ) { setCurrentItem( i ); } // grrr

private:
    QWidget *m_parent; // necessary to set the focus back
    void revertFocus();
};


#endif // KCOMPLETIONBOX_H
