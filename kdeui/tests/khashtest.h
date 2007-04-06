//
// Copyright Daniel M. Duley <mosfet@kde.org>
// Licensed under the Artistic License

#ifndef KHASH_TEST_H
#define KHASH_TEST_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

class KHashWidget : public QWidget
{
    Q_OBJECT

public:
    KHashWidget(QWidget *parent=0)
        : QWidget(parent){;}
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QPixmap pix;
};

#endif
