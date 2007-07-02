#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>

int main( int argc, char **argv )
{
    KAboutData aboutData( "kglobalacceltest", 0, ki18n("kglobalacceltest"), "1.0" );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KActionCollection* coll = new KActionCollection( &app );

    KAction* action = static_cast<KAction *>(coll->addAction("test"));
    action->setGlobalShortcutAllowed(true);
    action->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_F5));

    QObject::connect(action, SIGNAL(triggered(bool)), &app, SLOT(quit()));

    kDebug() << "Press Meta+F5 to complete the test." << endl;

    return app.exec();
}
