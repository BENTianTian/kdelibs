// programm to test the new khtml implementation


#include "decoder.h"
#include "kapp.h"
#include <qfile.h>
#include "html_document.h"
#include "htmltokenizer.h"
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#include "khtmlview.h"
#include "khtml_part.h"
#undef protected
#include <qpushbutton.h>
#include "testkhtml.h"
#include "testkhtml.moc"
#include "misc/loader.h"
#include <qcursor.h>
#include <qcolor.h>
#include <dom_string.h>
#include <qstring.h>
#include <qfile.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qvaluelist.h>
#include "dom/dom2_range.h"
#include "dom/html_document.h"
#include "dom/dom_exception.h"
#include <stdio.h>
#include <khtml_factory.h>
#include <css/cssstyleselector.h>
#include <html/html_imageimpl.h>
#include <rendering/render_style.h>
#include <kmainwindow.h>

int main(int argc, char *argv[])
{
    KHTMLFactory *fac = new KHTMLFactory();
    KApplication a(argc, argv, "testkhtml");

    KMainWindow *toplevel = new KMainWindow;
    KHTMLPart *doc = new KHTMLPart( toplevel, 0, toplevel, 0, KHTMLPart::BrowserViewGUI );
    doc->widget()->resize(800,500);
    toplevel->setCentralWidget( doc->widget() );

    toplevel->guiFactory()->addClient( doc );

    doc->enableJScript(true);
    doc->enableJava(true);
    //doc->setCharset("unicode");

    //doc->setFollowsLinks(false);

    //a.setTopWidget(doc);
    doc->setURLCursor(QCursor(PointingHandCursor));
    //doc->setDefaultTextColors(QColor(Qt::black), QColor(Qt::red),
    //			      QColor(Qt::green));
    a.setTopWidget(doc->widget());
    QWidget::connect(doc, SIGNAL(setWindowCaption(const QString &)),
		     doc->widget(), SLOT(setCaption(const QString &)));
    doc->widget()->show();

    toplevel->show();
    ((QScrollView *)doc->widget())->viewport()->show();

    Dummy *dummy = new Dummy( doc );
       QObject::connect( doc->browserExtension(), SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
             dummy, SLOT( slotOpenURL( const KURL&, const KParts::URLArgs & ) ) );
    doc->openURL( KURL( argv[1] ) );

    QPushButton *p = new QPushButton("&Quit", 0);
    QWidget::connect(p, SIGNAL(pressed()), &a, SLOT(quit()));
    p->show();

    int ret = a.exec();

    delete p;
    delete doc;

    khtml::Cache::clear();
    khtml::CSSStyleSelector::clear();
    DOM::HTMLMapElementImpl::clear();
    khtml::RenderStyle::cleanup();
    
    delete fac;
    return ret;
}

