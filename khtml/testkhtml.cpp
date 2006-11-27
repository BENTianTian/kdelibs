// program to test the new khtml implementation

#include <stdlib.h>
#include "decoder.h"
#include "kapplication.h"
#include "khtmlview.h"
#include "html_document.h"
#include "htmltokenizer.h"
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#include "khtml_part.h"
#undef protected
#include "testkhtml.h"
#include "testkhtml.moc"
#include "misc/loader.h"
#include <qcursor.h>
#include <dom_string.h>
#include "dom/dom2_range.h"
#include "dom/html_document.h"
#include "dom/dom_exception.h"
#include <stdio.h>
#define protected public
#include "khtml_factory.h"
#undef protected
#include "css/cssstyleselector.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include <kmainwindow.h>
#include <kcmdlineargs.h>
#include <ktoggleaction.h>
#include <kicon.h>
#include "domtreeview.h"
#include "kxmlguifactory.h"

static KCmdLineOptions options[] = { { "+file", "url to open", 0 } , KCmdLineLastOption };

int main(int argc, char *argv[])
{

    KCmdLineArgs::init(argc, argv, "testkhtml", "Testkhtml",
            "a basic web browser using the KHTML library", "1.0");
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication a;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );
    if ( args->count() == 0 ) {
	KCmdLineArgs::usage();
	::exit( 1 );
    }

#ifndef __KDE_HAVE_GCC_VISIBILITY
    KHTMLFactory *fac = new KHTMLFactory(true);
#endif

    KMainWindow *toplevel = new KMainWindow();
    KHTMLPart *doc = new KHTMLPart( toplevel, toplevel, KHTMLPart::BrowserViewGUI );

    Dummy *dummy = new Dummy( doc );
    QObject::connect( doc->browserExtension(), SIGNAL( openUrlRequest( const KUrl &, const KParts::URLArgs & ) ),
		      dummy, SLOT( slotOpenURL( const KUrl&, const KParts::URLArgs & ) ) );

    if (args->url(0).url().right(4).toLower() == ".xml") {
        KParts::URLArgs ags(doc->browserExtension()->urlArgs());
        ags.serviceType = "text/xml";
        doc->browserExtension()->setUrlArgs(ags);
    }

    doc->openUrl( args->url(0) );

//     DOMTreeView * dtv = new DOMTreeView(0, doc, "DomTreeView");
//     dtv->show();

    toplevel->setCentralWidget( doc->widget() );
    toplevel->resize( 800, 600);

//     dtv->resize(toplevel->width()/2, toplevel->height());

    QDomDocument d = doc->domDocument();
    QDomElement viewMenu = d.documentElement().firstChild().childNodes().item( 2 ).toElement();
    QDomElement e = d.createElement( "action" );
    e.setAttribute( "name", "debugRenderTree" );
    viewMenu.appendChild( e );
    e = d.createElement( "action" );
    e.setAttribute( "name", "debugDOMTree" );
    viewMenu.appendChild( e );
    QDomElement toolBar = d.documentElement().firstChild().nextSibling().toElement();
    e = d.createElement( "action" );
    e.setAttribute( "name", "editable" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "navigable" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "reload" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "print" );
    toolBar.insertBefore( e, toolBar.firstChild() );

    KAction *action = new KAction(KIcon("reload"),  "Reload", doc->actionCollection(), "reload" );
    QObject::connect(action, SIGNAL(triggered(bool)), dummy, SLOT( reload() ));
    action->setShortcut(Qt::Key_F5);
    KAction *kprint = new KAction(KIcon("print"),  "Print", doc->actionCollection(), "print" );
    QObject::connect(kprint, SIGNAL(triggered(bool)), doc->browserExtension(), SLOT( print() ));
    kprint->setEnabled(true);
    KToggleAction *ta = new KToggleAction( "Navigable", "editclear", KShortcut(), doc->actionCollection(), "navigable" );
    ta->setChecked(doc->isCaretMode());
    QWidget::connect(ta, SIGNAL(toggled(bool)), dummy, SLOT( toggleNavigable(bool) ));
    ta = new KToggleAction( "Editable", "edit", KShortcut(), doc->actionCollection(), "editable" );
    ta->setChecked(doc->isEditable());
    QWidget::connect(ta, SIGNAL(toggled(bool)), dummy, SLOT( toggleEditable(bool) ));
    toplevel->guiFactory()->addClient( doc );

    doc->setJScriptEnabled(true);
    doc->setJavaEnabled(true);
    doc->setPluginsEnabled( true );
    doc->setURLCursor(QCursor(Qt::PointingHandCursor));
    a.setTopWidget(doc->widget());
    QWidget::connect(doc, SIGNAL(setWindowCaption(const QString &)),
		     doc->widget()->topLevelWidget(), SLOT(setCaption(const QString &)));
    doc->widget()->show();
    toplevel->show();
    ((QScrollArea *)doc->widget())->viewport()->show();
    ((QScrollArea *)doc->widget())->widget()->show();


    int ret = a.exec();
#ifndef __KDE_HAVE_GCC_VISIBILITY
    fac->deref();
#endif
    return ret;
}

