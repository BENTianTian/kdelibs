/***************************************************************************
                          kmultitabbar.cpp -  description
                             -------------------
    begin                :  2001
    copyright            : (C) 2001,2002,2003 by Joseph Wenninger <jowenn@kde.org>
 ***************************************************************************/

/***************************************************************************
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
 ***************************************************************************/

#include "kmultitabbar.h"
#include "kmultitabbar.moc"
#include "kmultitabbar_p.h"
#include "kmultitabbar_p.moc"

#include <QtGui/QActionEvent>
#include <QtGui/QLayout>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QStyle>
#include <QStyleOptionButton>

#include <kiconloader.h>
#include <kdebug.h>
#include <QtGui/QApplication>
#include <math.h>

class KMultiTabBarPrivate
{
public:
    class KMultiTabBarInternal *m_internal;
    QBoxLayout *m_l;
    QFrame *m_btnTabSep;
    QList<KMultiTabBarButton*> m_buttons;
    KMultiTabBar::KMultiTabBarPosition m_position;
};

class KMultiTabBarButtonPrivate
{
public:
    KMultiTabBarButtonPrivate(int id) : m_id(id) {}
    int m_id;
};

class KMultiTabBarTabPrivate
{
public:
    KMultiTabBarTabPrivate() : m_showActiveTabText(false), m_expandedSize(24) {}
    QPixmap pix;
    bool m_showActiveTabText;
    int m_expandedSize;
};


KMultiTabBarInternal::KMultiTabBarInternal(QWidget *parent, KMultiTabBar::KMultiTabBarMode bm):QScrollArea(parent)
{
	m_expandedTabSize=-1;
	m_showActiveTabTexts=false;
	m_barMode=bm;
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	if (bm==KMultiTabBar::Vertical)
	{
		box=new QWidget(viewport());
		mainLayout=new QVBoxLayout(box);
		box->setFixedWidth(24);
		setFixedWidth(24);
	}
	else
	{
		box=new QWidget(viewport());
		mainLayout=new QHBoxLayout(box);
		box->setFixedHeight(24);
		setFixedHeight(24);
	}
//	mainLayout->setAutoAdd(true);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);
	setFrameStyle(NoFrame);
	viewport()->setBackgroundRole(QPalette::Background);
}

KMultiTabBarInternal::~KMultiTabBarInternal()
{
  qDeleteAll(m_tabs);
  m_tabs.clear();
}

void KMultiTabBarInternal::setStyle(enum KMultiTabBar::KMultiTabBarStyle style)
{
	m_style=style;
        for (int i=0;i<m_tabs.count();i++)
                m_tabs.at(i)->setStyle(m_style);

	if  ( (m_style==KMultiTabBar::KDEV3) ||
		(m_style==KMultiTabBar::KDEV3ICON ) ) {
		delete mainLayout;
		mainLayout=0;
		resizeEvent(0);
	} else if (mainLayout==0) {
		if (m_barMode==KMultiTabBar::Vertical)
		{
			box=new QWidget(viewport());
			mainLayout=new QVBoxLayout(box);
			box->setFixedWidth(24);
			setFixedWidth(24);
		}
		else
		{
			box=new QWidget(viewport());
			mainLayout=new QHBoxLayout(box);
			box->setFixedHeight(24);
			setFixedHeight(24);
		}
	        for (int i=0;i<m_tabs.count();i++)
        	        mainLayout->addWidget(m_tabs.at(i));
//		mainLayout->setAutoAdd(true);
	}
	viewport()->update();
}

#ifdef __GNUC__
#warning "This needs porting"
#endif
/*
void KMultiTabBarInternal::drawContents ( QPainter * paint, int clipx, int clipy, int clipw, int cliph )
{
	QScrollArea::drawContents (paint , clipx, clipy, clipw, cliph );

	if (m_position==KMultiTabBar::Right)
	{
      paint->setPen( palette().color( QPalette::Shadow ) );
      paint->drawLine(0,0,0,viewport()->height());
      paint->setPen( palette().color( QPalette::Background ).dark( 120 ) );
      paint->drawLine(1,0,1,viewport()->height());
	}
	else
	if (m_position==KMultiTabBar::Left)
	{
      paint->setPen( palette().color( QPalette::Light ) );
      paint->drawLine(23,0,23,viewport()->height());
      paint->drawLine(22,0,22,viewport()->height());

      paint->setPen( palette().color( QPalette::Shadow ) );
      paint->drawLine(0,0,0,viewport()->height());
	}
	else
	if (m_position==KMultiTabBar::Bottom)
	{
		paint->setPen( palette().color( QPalette::Shadow ) );
		paint->drawLine(0,0,viewport()->width(),0);
                paint->setPen( palette().color( QPalette::Background ).dark( 120 ) );
                paint->drawLine(0,1,viewport()->width(),1);
	}
	else
	{
	    paint->setPen( palette().color( QPalette::Light ) );
      paint->drawLine(0,23,viewport()->width(),23);
      paint->drawLine(0,22,viewport()->width(),22);
	}


}
*/
void KMultiTabBarInternal::contentsMousePressEvent(QMouseEvent *ev)
{
	ev->ignore();
}

void KMultiTabBarInternal::mousePressEvent(QMouseEvent *ev)
{
	ev->ignore();
}


#define CALCDIFF(m_tabs,diff,i) if (m_lines>(int)lines) {\
					/*kDebug()<<"i="<<i<<" tabCount="<<tabCount<<" space="<<space<<endl;*/ \
					uint ulen=0;\
					diff=0; \
					for (uint i2=i;i2<tabCount;i2++) {\
						uint l1=m_tabs.at(i2)->neededSize();\
						if ((ulen+l1)>space){\
							if (ulen==0) diff=0;\
							else diff=((float)(space-ulen))/(i2-i);\
							break;\
						}\
						ulen+=l1;\
					}\
				} else {diff=0; }


void KMultiTabBarInternal::resizeEvent(QResizeEvent *ev) {
/*	kDebug()<<"KMultiTabBarInternal::resizeEvent"<<endl;
	kDebug()<<"KMultiTabBarInternal::resizeEvent - box geometry"<<box->geometry()<<endl;
	kDebug()<<"KMultiTabBarInternal::resizeEvent - geometry"<<geometry()<<endl;*/
	if (ev) QScrollArea::resizeEvent(ev);

	if ( (m_style==KMultiTabBar::KDEV3) ||
		(m_style==KMultiTabBar::KDEV3ICON) ){
		box->setGeometry(0,0,width(),height());
		int lines=1;
		uint space;
		float tmp=0;
		if ((m_position==KMultiTabBar::Bottom) || (m_position==KMultiTabBar::Top))
			space=width();
		else
			space=height();

		int cnt=0;
//CALCULATE LINES
		const uint tabCount=m_tabs.count();
	        for (uint i=0;i<tabCount;i++) {
			cnt++;
			tmp+=m_tabs.at(i)->neededSize();
			if (tmp>space) {
				if (cnt>1)i--;
				else if (i==(tabCount-1)) break;
				cnt=0;
				tmp=0;
				lines++;
			}
		}
//SET SIZE & PLACE
		float diff=0;
		cnt=0;

		if ((m_position==KMultiTabBar::Bottom) || (m_position==KMultiTabBar::Top)) {

			setFixedHeight(lines*24);
			box->setFixedHeight(lines*24);
			m_lines=height()/24-1;
			lines=0;
			CALCDIFF(m_tabs,diff,0)
			tmp=-diff;

			//kDebug()<<"m_lines recalculated="<<m_lines<<endl;
		        for (uint i=0;i<tabCount;i++) {
				KMultiTabBarTab *tab=m_tabs.at(i);
				cnt++;
				tmp+=tab->neededSize()+diff;
				if (tmp>space) {
					//kDebug()<<"about to start new line"<<endl;
					if (cnt>1) {
						CALCDIFF(m_tabs,diff,i)
						i--;
					}
					else {
						//kDebug()<<"placing line on old line"<<endl;
						kDebug()<<"diff="<<diff<<endl;
						tab->removeEventFilter(this);
						tab->move(qRound(tmp-tab->neededSize()),lines*24);
//						tab->setFixedWidth(tab->neededSize()+diff);
						tab->setFixedWidth(qRound(tmp+diff)-tab->x());;
						tab->installEventFilter(this);
						CALCDIFF(m_tabs,diff,(i+1))

					}
					tmp=-diff;
					cnt=0;
					lines++;
					//kDebug()<<"starting new line:"<<lines<<endl;

				} else 	{
					//kDebug()<<"Placing line on line:"<<lines<<" pos: (x/y)=("<<tmp-m_tabs.at(i)->neededSize()<<"/"<<lines*24<<")"<<endl;
					//kDebug()<<"diff="<<diff<<endl;
					tab->removeEventFilter(this);
					tab->move(qRound(tmp-tab->neededSize()),lines*24);
					tab->setFixedWidth(qRound(tmp+diff)-tab->x());;

					//tab->setFixedWidth(tab->neededSize()+diff);
					tab->installEventFilter(this);

				}
			}
		}
		else {
			setFixedWidth(lines*24);
			box->setFixedWidth(lines*24);
			m_lines=lines=width()/24;
			lines=0;
			CALCDIFF(m_tabs,diff,0)
			tmp=-diff;

		        for (uint i=0;i<tabCount;i++) {
				KMultiTabBarTab *tab=m_tabs.at(i);
				cnt++;
				tmp+=tab->neededSize()+diff;
				if (tmp>space) {
					if (cnt>1) {
						CALCDIFF(m_tabs,diff,i);
						tmp=-diff;
						i--;
					}
					else {
						tab->removeEventFilter(this);
						tab->move(lines*24,qRound(tmp-tab->neededSize()));
                                                tab->setFixedHeight(qRound(tmp+diff)-tab->y());;
						tab->installEventFilter(this);
					}
					cnt=0;
					tmp=-diff;
					lines++;
				} else 	{
					tab->removeEventFilter(this);
					tab->move(lines*24,qRound(tmp-tab->neededSize()));
                                        tab->setFixedHeight(qRound(tmp+diff)-tab->y());;
					tab->installEventFilter(this);
				}
			}
		}


		//kDebug()<<"needed lines:"<<m_lines<<endl;
	} else {
		int size=0; /*move the calculation into another function and call it only on add tab and tab click events*/
		for (int i=0;i<(int)m_tabs.count();i++)
			size+=(m_barMode==KMultiTabBar::Vertical?m_tabs.at(i)->height():m_tabs.at(i)->width());
		if ((m_position==KMultiTabBar::Bottom) || (m_position==KMultiTabBar::Top))
			box->setGeometry(0,0,size,height());
		else box->setGeometry(0,0,width(),size);

	}
}


void KMultiTabBarInternal::showActiveTabTexts(bool show)
{
	m_showActiveTabTexts=show;
}


KMultiTabBarTab* KMultiTabBarInternal::tab(int id) const
{
	QListIterator<KMultiTabBarTab*> it(m_tabs);
	while (it.hasNext()){
		KMultiTabBarTab *tab = it.next();
		if (tab->id()==id) return tab;
	}
        return 0;
}

bool KMultiTabBarInternal::eventFilter(QObject *, QEvent *e) {
	if (e->type()==QEvent::Resize) resizeEvent(0);
	return false;
}

int KMultiTabBarInternal::appendTab(const QPixmap &pic ,int id,const QString& text)
{
	KMultiTabBarTab  *tab;
	m_tabs.append(tab= new KMultiTabBarTab(pic,text,id,box,m_position,m_style));
	tab->installEventFilter(this);
	tab->showActiveTabText(m_showActiveTabTexts);

	if (m_style==KMultiTabBar::KONQSBC)
	{
		if (m_expandedTabSize<tab->neededSize()) {
			m_expandedTabSize=tab->neededSize();
			for (int i=0;i<m_tabs.count();i++)
				m_tabs.at(i)->setSize(m_expandedTabSize);

		} else tab->setSize(m_expandedTabSize);
	} else tab->updateState();
	tab->show();
	resizeEvent(0);
	return 0;
}

void KMultiTabBarInternal::removeTab(int id)
{
	for (int pos=0;pos<m_tabs.count();pos++)
	{
		if (m_tabs.at(pos)->id()==id)
		{
                        // remove & delete the tab
			delete m_tabs.takeAt(pos);
			resizeEvent(0);
			break;
		}
	}
}

void KMultiTabBarInternal::setPosition(enum KMultiTabBar::KMultiTabBarPosition pos)
{
	m_position=pos;
	for (int i=0;i<m_tabs.count();i++)
		m_tabs.at(i)->setTabsPosition(m_position);
	viewport()->update();
}

KMultiTabBarButton::KMultiTabBarButton(const QPixmap& pic,const QString& text, QMenu *popup,
		int id,QWidget *parent,KMultiTabBar::KMultiTabBarPosition pos,KMultiTabBar::KMultiTabBarStyle style)
	:QPushButton(QIcon(),text,parent),m_style(style), d(new KMultiTabBarButtonPrivate(id))
{
	setIcon(pic);
	setText(text);
	m_position=pos;
  	if (popup) setMenu(popup);
	setFlat(true);
	setFixedHeight(24);
	setFixedWidth(24);
	setToolTip(text);
	connect(this,SIGNAL(clicked()),this,SLOT(slotClicked()));
}

KMultiTabBarButton::KMultiTabBarButton(const QString& text, QMenu *popup,
		int id,QWidget *parent,KMultiTabBar::KMultiTabBarPosition pos,KMultiTabBar::KMultiTabBarStyle style)
	:QPushButton(QIcon(),text,parent),m_style(style), d(new KMultiTabBarButtonPrivate(id))
{
	setText(text);
	m_position=pos;
  	if (popup) setMenu(popup);
	setFlat(true);
	setFixedHeight(24);
	setFixedWidth(24);
	setToolTip(text);
	connect(this,SIGNAL(clicked()),this,SLOT(slotClicked()));
}

KMultiTabBarButton::~KMultiTabBarButton() {
    delete d;
}

int KMultiTabBarButton::id() const{
	return d->m_id;
}

void KMultiTabBarButton::setText(const QString& text)
{
	QPushButton::setText(text);
	m_text=text;
	setToolTip(text);
}

void KMultiTabBarButton::slotClicked()
{
	emit clicked(d->m_id);
}

void KMultiTabBarButton::setPosition(KMultiTabBar::KMultiTabBarPosition pos)
{
	m_position=pos;
	update();
}

void KMultiTabBarButton::setStyle(KMultiTabBar::KMultiTabBarStyle style)
{
	m_style=style;
	update();
}

void KMultiTabBarButton::hideEvent( QHideEvent* he) {
	QPushButton::hideEvent(he);
	KMultiTabBar *tb=dynamic_cast<KMultiTabBar*>(parentWidget());
	if (tb) tb->updateSeparator();
}

void KMultiTabBarButton::showEvent( QShowEvent* he) {
	QPushButton::showEvent(he);
	KMultiTabBar *tb=dynamic_cast<KMultiTabBar*>(parentWidget());
	if (tb) tb->updateSeparator();
}


QSize KMultiTabBarButton::sizeHint() const
{
    ensurePolished();

    int w = 0, h = 0;

    // calculate contents size...
#ifndef QT_NO_ICONSET
    int iw = 0, ih = 0;
    if ( !icon().isNull() ) {
        iw = icon().pixmap( style()->pixelMetric( QStyle::PM_SmallIconSize ), QIcon::Normal ).width() + 4;
        ih = icon().pixmap( style()->pixelMetric( QStyle::PM_SmallIconSize ), QIcon::Normal ).height();
        w += iw;
        h = qMax( h, ih );
    }
#endif
    if ( menu() != 0 )
        w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, 0L, this);

        QString s( text() );
        bool empty = s.isEmpty();
        if ( empty )
            s = QLatin1String("XXXX");
        QFontMetrics fm = fontMetrics();
        QSize sz = fm.size( Qt::TextShowMnemonic, s );
        if(!empty || !w)
            w += sz.width();
        if(!empty || !h)
            h = qMax(h, sz.height());


    QStyleOptionToolButton opt;
    opt.init(this);
    opt.rect = QRect(0, 0, w, h);
    opt.subControls       = QStyle::SC_All;
    opt.activeSubControls = 0;
    opt.text              = text();
    opt.font              = font();
    opt.icon              = icon();
    opt.iconSize          = QSize(iw, ih);
    return (style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(w, h), this).
            expandedTo(QApplication::globalStrut()));
}


KMultiTabBarTab::KMultiTabBarTab(const QPixmap& pic, const QString& text,
		int id,QWidget *parent,KMultiTabBar::KMultiTabBarPosition pos,
		KMultiTabBar::KMultiTabBarStyle style)
	:KMultiTabBarButton(text,0,id,parent,pos,style),
	d(new KMultiTabBarTabPrivate())
{
	setIcon(pic);
	setCheckable(true);
	if(parent->layout())
		parent->layout()->addWidget(this);
}

KMultiTabBarTab::~KMultiTabBarTab() {
	delete d;
}


void KMultiTabBarTab::setTabsPosition(KMultiTabBar::KMultiTabBarPosition pos)
{
	if ((pos!=m_position) && ((pos==KMultiTabBar::Left) || (pos==KMultiTabBar::Right))) {
		if (!d->pix.isNull()) {
			QMatrix temp;// (1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
			temp.rotate(180);
			d->pix=d->pix.transformed(temp);
			setIcon(d->pix);
		}
	}

	setPosition(pos);
}

void KMultiTabBarTab::setIcon(const QString& icon)
{
	QPixmap pic=SmallIcon(icon);
	setIcon(pic);
}

void KMultiTabBarTab::setIcon(const QPixmap& icon)
{

	if (m_style!=KMultiTabBar::KDEV3) {
		if ((m_position==KMultiTabBar::Left) || (m_position==KMultiTabBar::Right)) {
		        QMatrix rotateMatrix;
			if (m_position==KMultiTabBar::Left)
		        	rotateMatrix.rotate(90);
			else
				rotateMatrix.rotate(-90);
			QPixmap pic=icon.transformed(rotateMatrix); //TODO FIX THIS, THIS SHOWS WINDOW
			d->pix=pic;
			KMultiTabBarButton::setIcon(pic);
		} else KMultiTabBarButton::setIcon(icon);
	}
}

void KMultiTabBarTab::slotClicked()
{
	updateState();
	KMultiTabBarButton::slotClicked();
}

void KMultiTabBarTab::setState(bool b)
{
	setChecked(b);
	updateState();
}

void KMultiTabBarTab::updateState()
{

	if (m_style!=KMultiTabBar::KONQSBC) {
		if ((m_style==KMultiTabBar::KDEV3) || (m_style==KMultiTabBar::KDEV3ICON) || (isChecked())) {
			QPushButton::setText(m_text);
		} else {
			kDebug()<<"KMultiTabBarTab::updateState(): setting text to an empty QString***************"<<endl;
			QPushButton::setText(QString());
		}

		if ((m_position==KMultiTabBar::Right || m_position==KMultiTabBar::Left)) {
			setFixedWidth(24);
			if ((m_style==KMultiTabBar::KDEV3)  || (m_style==KMultiTabBar::KDEV3ICON) || (isChecked())) {
				setFixedHeight(KMultiTabBarButton::sizeHint().width());
			} else setFixedHeight(36);
		} else {
			setFixedHeight(24);
			if ((m_style==KMultiTabBar::KDEV3)  || (m_style==KMultiTabBar::KDEV3ICON) || (isChecked())) {
				setFixedWidth(KMultiTabBarButton::sizeHint().width());
			} else setFixedWidth(36);
		}
	} else {
                if ((!isChecked()) || (!d->m_showActiveTabText))
                {
	                setFixedWidth(24);
	                setFixedHeight(24);
                        return;
                }
                if ((m_position==KMultiTabBar::Right || m_position==KMultiTabBar::Left))
                        setFixedHeight(d->m_expandedSize);
                else
                        setFixedWidth(d->m_expandedSize);
	}
	QApplication::sendPostedEvents(0,QEvent::Paint | QEvent::Move | QEvent::Resize | QEvent::LayoutRequest);
	QApplication::flush();
}

int KMultiTabBarTab::neededSize()
{
	return (((m_style!=KMultiTabBar::KDEV3)?24:0)+QFontMetrics(QFont()).width(m_text)+6);
}

void KMultiTabBarTab::setSize(int size)
{
	d->m_expandedSize=size;
	updateState();
}

void KMultiTabBarTab::showActiveTabText(bool show)
{
	d->m_showActiveTabText=show;
}

void KMultiTabBarTab::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	drawButton(&painter);
}

void KMultiTabBarTab::drawButtonLabel(QPainter *p) {
	drawButton(p);
}
void KMultiTabBarTab::drawButton(QPainter *paint)
{
	if (m_style!=KMultiTabBar::KONQSBC) drawButtonStyled(paint);
	else  drawButtonClassic(paint);
}

void KMultiTabBarTab::drawButtonStyled(QPainter *paint) {

	QSize sh;
	const int width = 36; // rotated
	const int height = 24;
	if ((m_style==KMultiTabBar::KDEV3) || (m_style==KMultiTabBar::KDEV3ICON) || (isChecked())) {
		 if ((m_position==KMultiTabBar::Left) || (m_position==KMultiTabBar::Right))
			sh=QSize(this->height(),this->width());//KMultiTabBarButton::sizeHint();
			else sh=QSize(this->width(),this->height());
	}
	else
		sh=QSize(width,height);

	QPixmap pixmap( sh.width(),height); ///,sh.height());
	pixmap.fill(palette().color(QPalette::Background));
	QPainter painter(&pixmap);


	QStyle::State st=QStyle::State_None;

	st|=QStyle::State_Enabled;

	if (isChecked()) st|=QStyle::State_On;

	QStyleOptionButton options;
	options.init(this);
	options.state = st;
	options.rect = QRect(0,0,pixmap.width(),pixmap.height());
	options.palette  = palette();
	options.text     = text();
	options.icon     = icon();
	options.iconSize = iconSize();

	style()->drawControl(QStyle::CE_PushButton, &options, &painter, this);

	switch (m_position) {
		case KMultiTabBar::Left:
			paint->rotate(-90);
			paint->drawPixmap(1-pixmap.width(),0,pixmap);
			break;
		case KMultiTabBar::Right:
			paint->rotate(90);
			paint->drawPixmap(0,1-pixmap.height(),pixmap);
			break;

		default:
			paint->drawPixmap(0,0,pixmap);
			break;
	}
}

void KMultiTabBarTab::drawButtonClassic(QPainter *paint)
{
        QPixmap pixmap;
	if ( !icon().isNull())
        	pixmap = icon().pixmap( style()->pixelMetric( QStyle::PM_SmallIconSize ), QIcon::Normal );
	paint->fillRect(0, 0, 24, 24, palette().color( QPalette::Background ) );

	if (!isChecked())
	{

		if (m_position==KMultiTabBar::Right)
		{
			paint->fillRect(0,0,21,21,QBrush( palette().color( QPalette::Background ) ));

			paint->setPen( palette().color( QPalette::Background ).dark( 150 ) );
			paint->drawLine(0,22,23,22);

			paint->drawPixmap(12-pixmap.width()/2,12-pixmap.height()/2,pixmap);

			paint->setPen( palette().color( QPalette::Shadow ) );
			paint->drawLine(0,0,0,23);
			paint->setPen( palette().color( QPalette::Background ).dark( 120 ) );
			paint->drawLine(1,0,1,23);

		}
		else
		if ((m_position==KMultiTabBar::Bottom) || (m_position==KMultiTabBar::Top))
		{
                        paint->fillRect(0,1,23,22,QBrush(palette().color( QPalette::Background )));

                        paint->drawPixmap(12-pixmap.width()/2,12-pixmap.height()/2,pixmap);

                        paint->setPen(palette().color( QPalette::Background ).dark(120));
                        paint->drawLine(23,0,23,23);


                        paint->setPen(palette().color( QPalette::Light ));
                        paint->drawLine(0,22,23,22);
                        paint->drawLine(0,23,23,23);
                	paint->setPen(palette().color( QPalette::Shadow ));
                	paint->drawLine(0,0,23,0);
                        paint->setPen(palette().color( QPalette::Background ).dark(120));
                        paint->drawLine(0,1,23,1);

		}
		else
		{
			paint->setPen(palette().color(QPalette::Background).dark(120));
			paint->drawLine(0,23,23,23);
			paint->fillRect(0,0,23,21,QBrush(palette().color(QPalette::Background)));
			paint->drawPixmap(12-pixmap.width()/2,12-pixmap.height()/2,pixmap);

			paint->setPen(palette().color(QPalette::Light));
			paint->drawLine(23,0,23,23);
			paint->drawLine(22,0,22,23);

			paint->setPen(palette().color(QPalette::Shadow));
			paint->drawLine(0,0,0,23);

		}


	}
	else
	{
		if (m_position==KMultiTabBar::Right)
		{
			paint->setPen(palette().color(QPalette::Shadow));
			paint->drawLine(0,height()-1,23,height()-1);
			paint->drawLine(0,height()-2,23,height()-2);
			paint->drawLine(23,0,23,height()-1);
			paint->drawLine(22,0,22,height()-1);
			paint->fillRect(0,0,21,height()-3,QBrush(palette().color(QPalette::Light)));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);

			if (d->m_showActiveTabText)
			{
				if (height()<25+4) return;

				QPixmap tpixmap(height()-25-3, width()-2);
				QPainter painter(&tpixmap);

				painter.fillRect(0,0,tpixmap.width(),tpixmap.height(),QBrush(palette().color(QPalette::Light)));

				painter.setPen(palette().color(QPalette::Text));
				painter.drawText(0,+width()/2+QFontMetrics(QFont()).height()/2,m_text);

				paint->rotate(90);
				kDebug()<<"tpixmap.width:"<<tpixmap.width()<<endl;
				paint->drawPixmap(25,-tpixmap.height()+1,tpixmap);
			}

		}
		else
		if (m_position==KMultiTabBar::Top)
		{
			paint->fillRect(0,0,width()-1,23,QBrush(palette().color(QPalette::Light)));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);
			if (d->m_showActiveTabText)
			{
				paint->setPen(palette().color(QPalette::Text));
				paint->drawText(25,height()/2+QFontMetrics(QFont()).height()/2,m_text);
			}
		}
		else
		if (m_position==KMultiTabBar::Bottom)
		{
			paint->setPen(palette().color(QPalette::Shadow));
			paint->drawLine(0,23,width()-1,23);
			paint->drawLine(0,22,width()-1,22);
			paint->fillRect(0,0,width()-1,21,QBrush(palette().color(QPalette::Light)));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);
			if (d->m_showActiveTabText)
			{
				paint->setPen(palette().color(QPalette::Text));
				paint->drawText(25,height()/2+QFontMetrics(QFont()).height()/2,m_text);
			}

		}
		else
		{


			paint->setPen(palette().color(QPalette::Shadow));
			paint->drawLine(0,height()-1,23,height()-1);
			paint->drawLine(0,height()-2,23,height()-2);
			paint->fillRect(0,0,23,height()-3,QBrush(palette().color(QPalette::Light)));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);
			if (d->m_showActiveTabText)
			{

		       		if (height()<25+4) return;

                                QPixmap tpixmap(height()-25-3, width()-2);
                                QPainter painter(&tpixmap);

                                painter.fillRect(0,0,tpixmap.width(),tpixmap.height(),QBrush(palette().color(QPalette::Light)));

                                painter.setPen(palette().color(QPalette::Text));
                                painter.drawText(tpixmap.width()-QFontMetrics(QFont()).width(m_text),+width()/2+QFontMetrics(QFont()).height()/2,m_text);

                                paint->rotate(-90);
                                kDebug()<<"tpixmap.width:"<<tpixmap.width()<<endl;

				paint->drawPixmap(-24-tpixmap.width(),2,tpixmap);

			}

		}

	}
}

KMultiTabBar::KMultiTabBar(KMultiTabBarMode bm, QWidget *parent)
    : QWidget(parent),
    d(new KMultiTabBarPrivate)
{
	if (bm==Vertical)
	{
		d->m_l=new QVBoxLayout(this);
		setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding/*, true*/);
	}
	else
	{
		d->m_l=new QHBoxLayout(this);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed/*, true*/);
	}
	d->m_l->setMargin(0);
//	d->m_l->setAutoAdd(false);

	d->m_internal=new KMultiTabBarInternal(this,bm);
	setPosition((bm==KMultiTabBar::Vertical)?KMultiTabBar::Right:KMultiTabBar::Bottom);
	setStyle(VSNET);
	//	setStyle(KDEV3);
	//setStyle(KONQSBC);
	d->m_l->insertWidget(0,d->m_internal);
	d->m_l->insertWidget(0,d->m_btnTabSep=new QFrame(this));
	d->m_btnTabSep->setFixedHeight(4);
	d->m_btnTabSep->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	d->m_btnTabSep->setLineWidth(2);
	d->m_btnTabSep->hide();

	updateGeometry();
}

KMultiTabBar::~KMultiTabBar()
{
  qDeleteAll( d->m_buttons );
  d->m_buttons.clear();
  delete d;
}

/*int KMultiTabBar::insertButton(QPixmap pic,int id ,const QString&)
{
  (new KToolbarButton(pic,id,d->m_internal))->show();
  return 0;
}*/

int KMultiTabBar::appendButton(const QPixmap &pic ,int id,QMenu *popup,const QString&)
{
	KMultiTabBarButton  *btn;
	d->m_buttons.append(btn= new KMultiTabBarButton(pic,QString(),
			popup,id,this,d->m_position,d->m_internal->m_style));
	d->m_l->insertWidget(0,btn);
	btn->show();
	d->m_btnTabSep->show();
	return 0;
}

void KMultiTabBar::updateSeparator() {
	bool hideSep=true;
  QListIterator<KMultiTabBarButton*> it(d->m_buttons);
	while (it.hasNext()){
		if (it.next()->isVisibleTo(this)) {
			hideSep=false;
			break;
		}
	}
	if (hideSep) d->m_btnTabSep->hide();
		else d->m_btnTabSep->show();

}

int KMultiTabBar::appendTab(const QPixmap &pic ,int id ,const QString& text)
{
 d->m_internal->appendTab(pic,id,text);
 return 0;
}

KMultiTabBarButton* KMultiTabBar::button(int id) const
{
  QListIterator<KMultiTabBarButton*> it(d->m_buttons);
  while ( it.hasNext() ) {
    KMultiTabBarButton *button = it.next();
    if ( button->id() == id )
      return button;
  }

  return 0;
}

KMultiTabBarTab* KMultiTabBar::tab(int id) const
{
	return d->m_internal->tab(id);
}



void KMultiTabBar::removeButton(int id)
{
	for (int pos=0;pos<d->m_buttons.count();pos++)
	{
		if (d->m_buttons.at(pos)->id()==id)
		{
			d->m_buttons.takeAt(pos)->deleteLater();
			break;
		}
	}
	if (d->m_buttons.count()==0) d->m_btnTabSep->hide();
}

void KMultiTabBar::removeTab(int id)
{
	d->m_internal->removeTab(id);
}

void KMultiTabBar::setTab(int id,bool state)
{
	KMultiTabBarTab *ttab=tab(id);
	if (ttab)
	{
		ttab->setState(state);
	}
}

bool KMultiTabBar::isTabRaised(int id) const
{
	KMultiTabBarTab *ttab=tab(id);
	if (ttab)
	{
		return ttab->isChecked();
	}

	return false;
}


void KMultiTabBar::showActiveTabTexts(bool show)
{
	d->m_internal->showActiveTabTexts(show);
}

void KMultiTabBar::setStyle(KMultiTabBarStyle style)
{
	d->m_internal->setStyle(style);
}

KMultiTabBar::KMultiTabBarStyle KMultiTabBar::tabStyle() const
{
	return d->m_internal->m_style;
}

void KMultiTabBar::setPosition(KMultiTabBarPosition pos)
{
	d->m_position=pos;
	d->m_internal->setPosition(pos);
	for (int i=0;i<d->m_buttons.count();i++)
		d->m_buttons.at(i)->setPosition(pos);
}

KMultiTabBar::KMultiTabBarPosition KMultiTabBar::position() const
{
	return d->m_position;
}
void KMultiTabBar::fontChange(const QFont& /* oldFont */)
{
	foreach( KMultiTabBarTab* tab, * d->m_internal->tabs() )
		tab->resize();
	update();
}

QList<KMultiTabBarTab*> KMultiTabBar::tabs() const {return * d->m_internal->tabs();}
QList<KMultiTabBarButton*> KMultiTabBar::buttons() const {return d->m_buttons;}
