/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>, Alexander Neundorf <neundorf@kde.org>

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

#ifndef KEDITLISTBOX_H
#define KEDITLISTBOX_H

#include <QGroupBox>
#include <QStringList>
#include <QStringListModel>

#include <kdelibs_export.h>

class KLineEdit;
class KComboBox;
class QListView;
class QPushButton;

class KEditListBoxPrivate;
/**
 * An editable listbox
 *
 * This class provides a editable listbox ;-), this means
 * a listbox which is accompanied by a line edit to enter new
 * items into the listbox and pushbuttons to add and remove
 * items from the listbox and two buttons to move items up and down.
 *
 * \image html keditlistbox.png "KDE Edit List Box Widget"
 *
 */
class KDEUI_EXPORT KEditListBox : public QGroupBox
{
   Q_OBJECT

   Q_FLAGS( Buttons )
   Q_PROPERTY( Buttons buttons READ buttons WRITE setButtons )
   Q_PROPERTY( QStringList items READ items WRITE setItems USER true )

public:
    /**
     * Custom editor class
     **/
    class CustomEditor
    {
    public:
        CustomEditor()
            : m_representationWidget( 0L ),
              m_lineEdit( 0L ) {}
        CustomEditor( QWidget *repWidget, KLineEdit *edit )
            : m_representationWidget( repWidget ),
              m_lineEdit( edit ) {}
        CustomEditor( KComboBox *combo );

        virtual ~CustomEditor() {}

        void setRepresentationWidget( QWidget *repWidget ) {
            m_representationWidget = repWidget;
        }
        void setLineEdit( KLineEdit *edit ) {
            m_lineEdit = edit;
        }

        virtual QWidget   *representationWidget() const {
            return m_representationWidget;
        }
        virtual KLineEdit *lineEdit() const {
            return m_lineEdit;
        }

    protected:
        QWidget *m_representationWidget;
        KLineEdit *m_lineEdit;
    };

   public:

      /**
       * Enumeration of the buttons, the listbox offers. Specify them in the
       * constructor in the buttons parameter, or in setButtons.
       */
      enum Button {
        Add = 0x0001,
        Remove = 0x0002,
        UpDown = 0x0004,
        All = Add | Remove | UpDown
      };

      Q_DECLARE_FLAGS( Buttons, Button )

      /**
       * Create an editable listbox.
       *
       * If @p checkAtEntering is true, after every character you type
       * in the line edit KEditListBox will enable or disable
       * the Add-button, depending whether the current content of the
       * line edit is already in the listbox. Maybe this can become a
       * performance hit with large lists on slow machines.
       * If @p checkAtEntering is false,
       * it will be checked if you press the Add-button. It is not
       * possible to enter items twice into the listbox.
       */
      KEditListBox(QWidget *parent = 0, const char *name = 0,
		   bool checkAtEntering=false, Buttons buttons = All );
      /**
       * Create an editable listbox.
       *
       * The same as the other constructor, additionally it takes
       * @p title, which will be the title of the frame around the listbox.
       */
      KEditListBox(const QString& title, QWidget *parent = 0,
		   const char *name = 0, bool checkAtEntering=false,
		   Buttons buttons = All );

      /**
       * Another constructor, which allows to use a custom editing widget
       * instead of the standard KLineEdit widget. E.g. you can use a
       * KUrlRequester or a KComboBox as input widget. The custom
       * editor must consist of a lineedit and optionally another widget that
       * is used as representation. A KComboBox or a KUrlRequester have a
       * KLineEdit as child-widget for example, so the KComboBox is used as
       * the representation widget.
       *
       * @see KUrlRequester::customEditor()
       */
      KEditListBox( const QString& title,
                    const CustomEditor &customEditor,
                    QWidget *parent = 0, const char *name = 0,
                    bool checkAtEntering = false, Buttons buttons = All );

      virtual ~KEditListBox();

      /**
       * Return a pointer to the embedded QListView.
       */
      QListView* listView() const     { return m_listView; }
      /**
       * Return a pointer to the embedded KLineEdit.
       */
      KLineEdit* lineEdit() const     { return m_lineEdit; }
      /**
       * Return a pointer to the Add button
       */
      QPushButton* addButton() const     { return servNewButton; }
      /**
       * Return a pointer to the Remove button
       */
      QPushButton* removeButton() const     { return servRemoveButton; }
      /**
       * Return a pointer to the Up button
       */
      QPushButton* upButton() const     { return servUpButton; }
      /**
       * Return a pointer to the Down button
       */
      QPushButton* downButton() const     { return servDownButton; }

      /**
       * See QListBox::count()
       */
      int count() const   { return int(m_model->rowCount()); }
      /**
       * See QListBox::insertStringList()
       */
      void insertStringList(const QStringList& list, int index=-1);
      /**
       * See QListBox::insertStrList()
       */
      void insertItem(const QString& text, int index=-1);
      /**
       * Clears both the listbox and the line edit.
       */
      void clear();
      /**
       * See QListBox::text()
       */
      QString text(int index) const;
      /**
       * See QListBox::currentItem()
       */
      int currentItem() const;
      /**
       * See QListBox::currentText()
       */
      QString currentText() const;

      /**
       * @returns a stringlist of all items in the listbox
       */
      QStringList items() const;

      /**
       * Clears the listbox and sets the contents to @p items
       */
      void setItems(const QStringList& items);

      /**
       * Returns which buttons are visible
       */
      Buttons buttons() const;

      /**
       * Specifies which buttons should be visible
       */
      void setButtons( Buttons buttons );

   Q_SIGNALS:
      void changed();

      /**
       * This signal is emitted when the user adds a new string to the list,
       * the parameter is the added string.
       */
      void added( const QString & text );

      /**
       * This signal is emitted when the user removes a string from the list,
       * the parameter is the removed string.
       */
      void removed( const QString & text );

   protected Q_SLOTS:
      //the names should be self-explaining
      void moveItemUp();
      void moveItemDown();
      void addItem();
      void removeItem();
      void enableMoveButtons(const QModelIndex&, const QModelIndex&);
      void typedSomething(const QString& text);

   private:
      QListView *m_listView;
      QPushButton *servUpButton, *servDownButton;
      QPushButton *servNewButton, *servRemoveButton;
      KLineEdit *m_lineEdit;
      QStringListModel *m_model;

      //this is called in both ctors, to avoid code duplication
      void init( bool checkAtEntering, Buttons buttons,
                 QWidget *representationWidget = 0 );

   protected:
      virtual void virtual_hook( int id, void* data );
   private:
      //our lovely private d-pointer
      KEditListBoxPrivate* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KEditListBox::Buttons)

#endif
