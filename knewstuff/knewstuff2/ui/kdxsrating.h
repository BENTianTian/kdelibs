/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF2_UI_KDXSRATING_H
#define KNEWSTUFF2_UI_KDXSRATING_H

#include <kdialog.h>

class QSlider;
class QLabel;
class QStarFrame;

/**
 * KNewStuff rating submission window.
 *
 * This dialog is shown whenever the user selects to add a rating
 * to an item in the download dialog.
 *
 * This class is used internally by the KDXSButton class.
 *
 * @internal
 */
class KDXSRating : public KDialog
{
    Q_OBJECT
public:
    KDXSRating(QWidget *parent);
    int rating();
public Q_SLOTS:
    void slotRating(int rating);
private:
    QLabel *m_rating;
    QStarFrame *m_starrating;
    QSlider *m_slider;
};

#endif
