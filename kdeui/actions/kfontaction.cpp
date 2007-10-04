/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfontaction.h"

#include <QtGui/QToolBar>
#include <QtGui/QFontComboBox>

#include <kdebug.h>
#include <kfontdialog.h>
#include <kicon.h>
#include <klocale.h>
#include <kfontchooser.h>

class KFontAction::KFontActionPrivate
{
    public:
        KFontActionPrivate(KFontAction *parent)
            : q(parent)
        {
        }

        void _k_slotFontChanged(const QFont &font)
        {
            q->triggered(font.family());
        }


        KFontAction *q;
};

KFontAction::KFontAction(uint fontListCriteria, QObject *parent)
  : KSelectAction(parent), d(new KFontActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList( list, fontListCriteria );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction(QObject *parent)
  : KSelectAction(parent), d(new KFontActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction(const QString & text, QObject *parent)
  : KSelectAction(text, parent), d(new KFontActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::KFontAction(const KIcon &icon, const QString &text, QObject *parent)
  : KSelectAction(icon, text, parent), d(new KFontActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList( list, 0 );
    KSelectAction::setItems( list );
    setEditable( true );
}

KFontAction::~KFontAction()
{
    delete d;
}

QString KFontAction::font() const
{
    return currentText();
}

QWidget* KFontAction::createWidget(QWidget* _parent)
{
    QToolBar *parent = qobject_cast<QToolBar *>(_parent);
    if (!parent)
        return KSelectAction::createWidget(_parent);

#ifdef __GNUC__
#warning FIXME: items need to be converted
#endif
    QFontComboBox *cb = new QFontComboBox( parent );
    connect( cb, SIGNAL( currentFontChanged( const QFont & ) ), SLOT(_k_slotFontChanged( const QFont&  ) ) );
    cb->setMinimumWidth( cb->sizeHint().width() );
    return cb;
}

/*
 * Maintenance note: Keep in sync with QFontComboBox::setCurrentFont()
 */
void KFontAction::setFont( const QString &family )
{
    QString lowerName = family.toLower();
    if (setCurrentAction(lowerName, Qt::CaseInsensitive))
       return;

    int i = lowerName.indexOf(" [");
    if (i > -1)
    {
       lowerName = lowerName.left(i);
       i = 0;
       if (setCurrentAction(lowerName, Qt::CaseInsensitive))
          return;
    }

    lowerName += " [";
    if (setCurrentAction(lowerName, Qt::CaseInsensitive))
      return;

    kDebug(129) << "Font not found " << family.toLower();
}

/* vim: et sw=2 ts=2
 */

#include "kfontaction.moc"
