#include "kdatewidget.h"
#include <QtGui/QLineEdit>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalog("kdelibs");
  KAboutData about("KDateWidgettest", "KDateWidgettest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KDateWidget dateWidget;
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}

