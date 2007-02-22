//
// Simple little hack to show off blending effects.
//
// Copyright Cristian Tibirna <tibirna@kde.org>
// Licensed under the Artistic License.

#ifndef KBLEND_TEST_H
#define KBLEND_TEST_H

#include <QtGui/QWidget>
#include <QtGui/QImage>
#include <knuminput.h>

class KDesatWidget : public QWidget
{
Q_OBJECT
public:
    KDesatWidget(QWidget *parent=0);

public Q_SLOTS:
    void change(double);

protected:
    void paintEvent(QPaintEvent *ev);
private:
    float desat_value;
    QImage image;
    KDoubleNumInput *slide;
};

#endif
