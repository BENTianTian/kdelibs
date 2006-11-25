//
//  MAIN -- a little demo of the capabilities of the "KProcess" class
//
//  version 0.2, Aug 2nd 1997
//
//  Copyright 1997 Christian Czezatke <e9025461@student.tuwien.ac.at>
//


#include "kprocess.h"

#include <stdio.h>
#include <string.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kinstance.h>
#include <QApplication>

#include <signal.h>

#include "kprociotest.h"



//
// A nice input for "sort"... ;- )
//
static const char txt[] = "hat\nder\nalte\nhexenmeister\nsich\ndoch\neinmal\nwegbegeben\n\
und\nnun\nsollen\nseine\ngeister\nsich\nnach\nmeinem\nwillen\nregen\nseine\nwort\nund\n\
werke\nmerkt\nich\nund\nden\nbrauch\nund\nmit\ngeistesstaerke\ntu\nich\nwunder\nauch\n";


int main(int argc, char *argv[])
{
 Dummy dummy;

 KAboutData about("kprociotest", "kprociotest", "version");
 //KCmdLineArgs::init(argc, argv, &about);
 KInstance instance(&about);

 QApplication app(argc, argv);

 printf("Welcome to the KProcIO Demo Application!\n");


 KProcIO p;

 p << "rev";

 p.connect(&p, SIGNAL(processExited(KProcess*)), &dummy, SLOT(printMessage(KProcess*)));
 p.connect(&p, SIGNAL(readReady(KProcIO*)), &dummy, SLOT(gotOutput(KProcIO*)));

 bool b;

 b = p.start();
 printf("Start returns %s\n", b ? "true" : "false");

 b = p.writeStdin(QString("Hello World!"));
 printf("writeStdin returns %s\n", b ? "true" : "false");

 b = p.writeStdin(QString("This is a test. It should come out in reverse (esrever)"));
 printf("writeStdin returns %s\n", b ? "true" : "false");

 p.closeWhenDone();

 printf("Entering man Qt event loop -- press <CTRL><C> to abort\n");
 return app.exec();
}
#include "kprociotest.moc"
