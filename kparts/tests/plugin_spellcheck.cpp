#include "notepad.h" // this plugin applies to a notepad part
#include <qmultilineedit.h>
#include "plugin_spellcheck.h"
#include <kaction.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <klocale.h>

PluginSpellCheck::PluginSpellCheck( QObject* parent, const char* name )
    : Plugin( parent, name )
{
    (void) new KAction( i18n( "&Select current line (plugin)" ), 0, this, SLOT(slotSpellCheck()),
                        actionCollection(), "spellcheck" );
}

PluginSpellCheck::~PluginSpellCheck()
{
}

void PluginSpellCheck::slotSpellCheck()
{
    qDebug("Plugin parent : %s (%s)", parent()->name(), parent()->className());
    // The parent is assumed to be a NotepadPart
    if ( !parent()->inherits("NotepadPart") )
       KMessageBox::error(0L,"You just called the spell-check action on a wrong part (not NotepadPart)");
    else
    {
         NotepadPart * part = (NotepadPart *) parent();
         QMultiLineEdit * widget = (QMultiLineEdit *) part->widget();
         widget->selectAll(); //selects current line !
    }
}

KPluginFactory::KPluginFactory( QObject* parent, const char* name )
  : KLibFactory( parent, name )
{
  s_instance = new KInstance("KPluginFactory");
}

QObject* KPluginFactory::create( QObject* parent, const char* name, const char*, const QStringList & )
{
    QObject *obj = new PluginSpellCheck( parent, name );
    emit objectCreated( obj );
    return obj;
}

extern "C"
{
  void* init_libspellcheck()
  {
    return new KPluginFactory;
  }

}

KInstance* KPluginFactory::s_instance = 0L;

#include <plugin_spellcheck.moc>
