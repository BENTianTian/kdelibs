
#include <kparts/event.h>

#include "parts.h"

#include <qcheckbox.h>
#include <QtCore/QFile>
#include <qdir.h>
#include <qtextstream.h>
#include <QTextEdit>
#include <qlineedit.h>

#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>

Part1::Part1( QObject *parent, QWidget * parentWidget )
    : KParts::ReadOnlyPart(parent),
    m_componentData("kpartstestpart")
{
    setObjectName( "Part1" );
    setComponentData( m_componentData );
    m_edit = new QTextEdit( parentWidget );
    setWidget( m_edit );

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // This line is for test programs only!
    m_componentData.dirs()->addResourceDir( "data", KDESRCDIR );
    setXMLFile( "kpartstest_part1.rc" );

    /*KAction * paBlah = */ //new KAction( "Blah", "filemail", 0, actionCollection(), "p1_blah" );
}

Part1::~Part1()
{
}

bool Part1::openFile()
{
    kDebug() << "Part1: opening " << QFile::encodeName(m_file) << endl;
    // Hehe this is from a tutorial I did some time ago :)
    QFile f(m_file);
    QString s;
    if ( f.open(QIODevice::ReadOnly) ) {
        QTextStream t( &f );
        while ( !t.atEnd() ) {
            s += t.readLine() + "\n";
        }
        f.close();
    } else
        return false;
    m_edit->setPlainText(s);

    emit setStatusBarText( m_url.prettyUrl() );

    return true;
}

Part2::Part2( QObject *parent, QWidget * parentWidget )
    : KParts::Part(parent),
    m_componentData("part2")
{
    setObjectName( "Part2" );
    setComponentData( m_componentData );
    QWidget * w = new QWidget( parentWidget );
    w->setObjectName( "Part2Widget" );
    setWidget( w );

    /*QCheckBox * cb =*/ new QCheckBox( "something", w );

    QLineEdit * l = new QLineEdit( "something", widget() );
    l->move(0,50);
    // Since the main widget is a dummy one, we HAVE to set
    // strong focus for it, otherwise we get the
    // the famous activating-file-menu-switches-part bug.
    w->setFocusPolicy( Qt::ClickFocus );

    // setXMLFile( ... ); // no actions currently
}

Part2::~Part2()
{
}

void Part2::guiActivateEvent( KParts::GUIActivateEvent * event )
{
    if (event->activated())
        emit setWindowCaption("[part2 activated]");
}

#include "parts.moc"
