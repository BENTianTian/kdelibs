#include <kapplication.h>
#include <keditcl.h>
#include <q3popupmenu.h>

int main( int argc, char **argv )
{
  KApplication app;
  KEdit *edit = new KEdit( 0L );
  Q3PopupMenu *pop = new Q3PopupMenu( 0L );
  pop->insertItem( "Popupmenu item" );
  edit->installRBPopup( pop );
  edit->show();
  return app.exec();
}
