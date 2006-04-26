//
// Simple little hack to show off new diagonal gradients.
//
// (C) KDE Artistic Daniel M. Duley <mosfet@kde.org>
//

#ifndef KGRADIENT_TEST_H
#define KGRADIENT_TEST_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qslider.h>
#include <qlabel.h>
#include <qdatetime.h>

class KGradientWidget : public QWidget
{
public:
    KGradientWidget(QWidget *parent=0);

    void setBalance(int a, int b) { xbalance = a; ybalance = b; }
protected:
    void paintEvent(QPaintEvent *ev);
private:
    QPixmap pix;
    int xbalance, ybalance;
    QTime time;
 
};

class myTopWidget: public QWidget
{
  Q_OBJECT
public:
  myTopWidget(QWidget *parent=0);
  
private:
  QLabel *bLabel;
  QSlider *xSlider, *ySlider;
  KGradientWidget *grds;

  QTime time;

  int itime, otime;

private Q_SLOTS:
  void rebalance();
};
#endif
