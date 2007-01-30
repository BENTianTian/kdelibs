#ifdef _WIN32
#define LIBXML_DLL_IMPORT __declspec(dllimport)
#else
extern int xmlLoadExtDtdDefaultValue;
#endif

#include <config-kdoctools.h>
#include <config.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/parserInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <qstring.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <xslt.h>
#include <qfile.h>
#include <qdir.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <stdlib.h>
#include <kdebug.h>
#include <qtextcodec.h>
#include <qfileinfo.h>
#include <kprocess.h>
#include <QList>

class MyPair {
public:
    QString word;
    int base;};

typedef QList<MyPair> PairList;

void parseEntry(PairList &list, xmlNodePtr cur, int base)
{
    if ( !cur )
        return;

    base += atoi( ( const char* )xmlGetProp(cur, ( const xmlChar* )"header") );
    if ( base > 10 ) // 10 is the maximum
        base = 10;

    /* We don't care what the top level element name is */
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {

        if ( cur->type == XML_TEXT_NODE ) {
            QString words = QString::fromUtf8( ( char* )cur->content );
            QStringList wlist = words.simplified().split( ' ',QString::SkipEmptyParts );
            for ( QStringList::ConstIterator it = wlist.begin();
                  it != wlist.end(); ++it )
            {
                MyPair m;
                m.word = *it;
                m.base = base;
                list.append( m );
            }
        } else if ( !xmlStrcmp( cur->name, (const xmlChar *) "entry") )
            parseEntry( list, cur, base );

    	cur = cur->next;
    }

}

static KCmdLineOptions options[] =
{
    { "stylesheet <xsl>",  I18N_NOOP( "Stylesheet to use" ), 0 },
    { "stdout", I18N_NOOP( "Output whole document to stdout" ), 0 },
    { "o", 0, 0 },
    { "output <file>", I18N_NOOP("Output whole document to file" ), 0 },
    { "htdig", I18N_NOOP( "Create a ht://dig compatible index" ), 0 },
    { "check", I18N_NOOP( "Check the document for validity" ), 0 },
    { "cache <file>", I18N_NOOP( "Create a cache file for the document" ), 0},
    { "srcdir <dir>", I18N_NOOP( "Set the srcdir, for kdelibs" ), 0},
    { "param <key>=<value>", I18N_NOOP( "Parameters to pass to the stylesheet" ), 0},
    { "+xml", I18N_NOOP("The file to transform"), 0},
    KCmdLineLastOption // End of options.
};


int main(int argc, char **argv) {

    // xsltSetGenericDebugFunc(stderr, NULL);

    KAboutData aboutData( "meinproc", I18N_NOOP("XML-Translator" ),
	"$Revision$",
	I18N_NOOP("KDE Translator for XML"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    KLocale::setMainCatalog("kio_help");
    KComponentData ins("meinproc");
    KGlobal::locale();


    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() != 1 ) {
        args->usage();
        return ( 1 );
    }

    // Need to set SRCDIR before calling fillInstance
    QString srcdir;
    if ( args->isSet( "srcdir" ) )
        srcdir = QDir( QFile::decodeName( args->getOption( "srcdir" ) ) ).absolutePath();
    fillInstance(ins,srcdir);

    LIBXML_TEST_VERSION

    QString checkFilename = QFile::decodeName(args->arg( 0 ));
    QFileInfo checkFile(checkFilename);
    if (!checkFile.exists())
    {
        kError() << "File '" << checkFilename << "' does not exist." << endl;
        return ( 2 );
    }
    if (!checkFile.isFile())
    {
        kError() << "'" << checkFilename << "' is not a file." << endl;
        return ( 2 );
    }
    if (!checkFile.isReadable())
    {
        kError() << "File '" << checkFilename << "' is not readable." << endl;
        return ( 2 );
    }

    if ( args->isSet( "check" ) ) {
        QString pwd_buffer = QDir::currentPath();
        QFileInfo file( QFile::decodeName(args->arg( 0 )) );

        QString catalogs;
        catalogs += KStandardDirs::locate( "dtd", "customization/catalog" );
        catalogs += " ";
        catalogs += KStandardDirs::locate( "dtd", "docbook/xml-dtd-4.1.2/docbook.cat" );

        setenv( "SGML_CATALOG_FILES", QFile::encodeName( catalogs ).constData(), 1);
        QString exe;
#if defined( XMLLINT )
        exe = XMLLINT;
#endif
        if ( !QFileInfo( exe ).isExecutable() ) {
            exe = KStandardDirs::findExe( "xmllint" );
            if (exe.isEmpty())
                exe = KStandardDirs::locate( "exe", "xmllint" );
        }
        if ( QFileInfo( exe ).isExecutable() ) {
            QDir::setCurrent( file.absolutePath() );
            QString cmd = exe;
            cmd += " --catalogs --valid --noout ";
#ifdef Q_OS_WIN
            cmd += file.fileName();
#else
            cmd += KProcess::quote(file.fileName());
#endif
            cmd += " 2>&1";
            FILE *xmllint = popen( QFile::encodeName( cmd ).constData(), "r" );
            char buf[ 512 ];
            bool noout = true;
            unsigned int n;
            while ( ( n = fread(buf, 1, sizeof( buf ) - 1, xmllint ) ) ) {
                noout = false;
                buf[ n ] = '\0';
                fputs( buf, stderr );
            }
            pclose( xmllint );
            QDir::setCurrent( pwd_buffer );
            if ( !noout )
                return 1;
        } else {
            kWarning() << "couldn't find xmllint" << endl;
        }
    }

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;

    QVector<const char *> params;
    if (args->isSet( "output" ) ) {
        params.append( qstrdup( "outputFile" ) );
        params.append( qstrdup( QFile::decodeName( args->getOption( "output" ) ).toLatin1() ) );
    }
    {
        const QByteArrayList paramList = args->getOptionList( "param" );
        QByteArrayList::ConstIterator it = paramList.begin();
        QByteArrayList::ConstIterator end = paramList.end();
        for ( ; it != end; ++it ) {
            const QByteArray tuple = *it;
            const int ch = tuple.indexOf( '=' );
            if ( ch == -1 ) {
                kError() << "Key-Value tuple '" << tuple << "' lacks a '='!" << endl;
                return( 2 );
            }
            params.append( qstrdup( tuple.left( ch ) ) );
            params.append( qstrdup( tuple.mid( ch + 1 ) )  );
        }
    }
    params.append( NULL );

    bool index = args->isSet( "htdig" );
    QString tss = args->getOption( "stylesheet" );
    if ( tss.isEmpty() )
        tss =  "customization/kde-chunk.xsl";
    if ( index )
        tss = "customization/htdig_index.xsl" ;

    tss = KStandardDirs::locate( "dtd", tss );

    if ( index ) {
        xsltStylesheetPtr style_sheet =
            xsltParseStylesheetFile((const xmlChar *)tss.toLatin1().data());

        if (style_sheet != NULL) {

            xmlDocPtr doc = xmlParseFile( QFile::encodeName( args->arg( 0 ) ).constData() );

            xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, &params[0]);

            xmlFreeDoc(doc);
            xsltFreeStylesheet(style_sheet);
            if (res != NULL) {
                xmlNodePtr cur = xmlDocGetRootElement(res);
                if (!cur || xmlStrcmp(cur->name, (const xmlChar *) "entry")) {
                    fprintf(stderr,"document of the wrong type, root node != entry");
                    xmlFreeDoc(res);
                    return(1);
                }
                PairList list;
                parseEntry( list, cur, 0 );
                int wi = 0;
                for ( PairList::ConstIterator it = list.begin(); it != list.end();
                      ++it, ++wi )
                    fprintf( stdout, "w\t%s\t%d\t%d\n", ( *it ).word.toUtf8().data(),
                             1000*wi/list.count(), ( *it ).base );

                xmlFreeDoc(res);
            } else {
                kDebug() << "couldn't parse document " << args->arg( 0 ) << endl;
            }
        } else {
            kDebug() << "couldn't parse style sheet " << tss << endl;
        }

    } else {
        QString output = transform(args->arg( 0 ) , tss, params);
        if (output.isEmpty()) {
            fprintf(stderr, "unable to parse %s\n", args->arg( 0 ));
            return(1);
        }

        QString cache = args->getOption( "cache" );
        if ( !cache.isEmpty() ) {
            if ( !saveToCache( output, cache ) ) {
                kError() << i18n( "Could not write to cache file %1." ,  cache ) << endl;
            }
            goto end;
        }

        if (output.indexOf( "<FILENAME " ) == -1 || args->isSet( "stdout" ) || args->isSet("output") )
        {
            QFile file;
            if (args->isSet( "stdout" ) ) {
                file.open( stdout, QIODevice::WriteOnly );
            } else {
                if (args->isSet( "output" ) )
                   file.setFileName( QFile::decodeName(args->getOption( "output" )));
                else
                   file.setFileName( "index.html" );
                file.open(QIODevice::WriteOnly);
            }
            replaceCharsetHeader( output );

            QByteArray data = output.toLocal8Bit();
            file.write(data.data(), data.length());
            file.close();
        } else {
            int index = 0;
            while (true) {
                index = output.indexOf("<FILENAME ", index);
                if (index == -1)
                    break;
                int filename_index = index + strlen("<FILENAME filename=\"");

                QString filename = output.mid(filename_index,
                                              output.indexOf("\"", filename_index) -
                                              filename_index);

                QString filedata = splitOut(output, index);
                QFile file(filename);
                file.open(QIODevice::WriteOnly);
                replaceCharsetHeader( filedata );
                QByteArray data = fromUnicode( filedata );
                file.write(data.data(), data.length());
                file.close();

                index += 8;
            }
        }
    }
 end:
    xmlCleanupParser();
    xmlMemoryDump();
    return(0);
}

