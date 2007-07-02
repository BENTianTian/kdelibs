//
// Copyright Daniel M. Duley <mosfet@kde.org>
// Licensed under the Artistic License.

#include "kunbalancedgrdtest.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kpixmapeffect.h>

#include <QtGui/QPainter>
#include <QtGui/QGridLayout>

int cols = 3, rows = 3; // how many


KGradientWidget::KGradientWidget(QWidget *parent)
  :QWidget(parent)
{
  time.start();
  setMinimumSize(250 * cols, 250 * rows);
}

void KGradientWidget::paintEvent(QPaintEvent *)
{
    int it, ft;
    QString say;

    QColor ca = Qt::black, cb = Qt::cyan;

    int x = 0, y = 0;

    pix = QPixmap(width()/cols, height()/rows);
    QPainter p(this);
    p.setPen(Qt::white);

    // draw once, so that the benchmarking be fair :-)
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::VerticalGradient,
				      xbalance, ybalance);

    // vertical
    time.start();
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::VerticalGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::HorizontalGradient,
				      xbalance, ybalance);
    ft = time.elapsed() ;
    say.setNum( ft - it); say = "Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::EllipticGradient,
				      xbalance, ybalance);
    ft = time.elapsed() ;
    say.setNum( ft - it); say = "Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::DiagonalGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::CrossDiagonalGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::PyramidGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // rectangular
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::RectangleGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::PipeCrossGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);
}

myTopWidget::myTopWidget (QWidget *parent)
  :QWidget(parent)
{
  QGridLayout *lay = new QGridLayout ( this );
  lay->setMargin( 0 );

  grds = new KGradientWidget(this);
  lay->addWidget(grds, 0, 0 ,1, 3);

  bLabel = new QLabel("Balance: X = 000; Y = 000", this);
  lay->addWidget(bLabel, 1, 0);

  xSlider = new QSlider ( Qt::Horizontal, this);
  xSlider->setMinimum( -200 );
  xSlider->setMaximum(  200 );
  xSlider->setValue( 100 );
  lay->addWidget(xSlider, 1, 1);

  ySlider = new QSlider ( Qt::Horizontal, this);
  ySlider->setMinimum( -200 );
  ySlider->setMaximum(  200 );
  ySlider->setValue( 100 );
  lay->addWidget(ySlider, 1, 2);

  connect(xSlider, SIGNAL(valueChanged(int)), this, SLOT(rebalance()));
  connect(ySlider, SIGNAL(valueChanged(int)), this, SLOT(rebalance()));

  rebalance();

  itime = otime = 0;
  time.start();
}

void myTopWidget::rebalance()
{
  otime = time.elapsed();

  QString val; val.sprintf("Balance: X = %3d; Y = %3d",
			   xSlider->value(), ySlider->value());

  bLabel->setText(val);
  grds->setBalance(xSlider->value(), ySlider->value());

  if ((otime - itime )> 500)
    {
      grds->repaint();
      itime = time.elapsed();
    }
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "test", 0, ki18n("Test"), "1.0", ki18n("test app"));
    KApplication app;
    myTopWidget w;
    w.show();
    return app.exec();
}

#include "kunbalancedgrdtest.moc"
