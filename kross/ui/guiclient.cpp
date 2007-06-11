/***************************************************************************
 * guiclient.cpp
 * This file is part of the KDE project
 * copyright (C) 2005-2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "guiclient.h"

#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/actioncollection.h>

#include <kapplication.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kurl.h>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class GUIClient::Private
    {
        public:
            /// The \a KXMLGUIClient that is parent of the \a GUIClient instance.
            KXMLGUIClient* guiclient;
            /// The menu used to display the scripts.
            KActionMenu* scriptsmenu;
            /// The collection of actions this guiclient should operate on.
            Kross::ActionCollection* collection;
    };

}

GUIClient::GUIClient(KXMLGUIClient* guiclient, QObject* parent)
    : QObject(parent)
    , KXMLGUIClient(guiclient)
    , d(new Private())
{
    setComponentData( GUIClient::componentData() );

    d->guiclient = guiclient;
    d->collection = 0;

    d->scriptsmenu = new KActionMenu(i18n("Scripts"), this);
    actionCollection()->addAction("scripts", d->scriptsmenu);
    connect(d->scriptsmenu->menu(), SIGNAL(aboutToShow()), this, SLOT(slotMenuAboutToShow()));

    // action to execute a scriptfile.
    KAction* execfileaction = new KAction(i18n("Execute Script File..."), this);
    actionCollection()->addAction("executescriptfile", execfileaction);
    connect(execfileaction, SIGNAL( triggered() ), this, SLOT( slotShowExecuteScriptFile() ));

    // The GUIClient provides feedback if e.g. an execution failed.
    connect(&Manager::self(), SIGNAL( started(Kross::Action*) ), this, SLOT( started(Kross::Action*) ));
    connect(&Manager::self(), SIGNAL( finished(Kross::Action*) ), this, SLOT( finished(Kross::Action*) ));
}

GUIClient::~GUIClient()
{
    delete d;
}

void GUIClient::initialize(Kross::ActionCollection* collection)
{
    if( d->collection )
        finalize();
    d->collection = collection;

    QByteArray partname = d->guiclient->componentData().componentName(); //KApplication::kApplication()->objectName()
    d->collection->readXmlResources("data", partname + "/scripts/*.rc");
}

void finalizeCollection(Kross::ActionCollection* c)
{
    foreach(Kross::Action* a, c->actions())
        a->finalize();
    foreach(QString s, c->collections())
        finalizeCollection( c->collection(s) );
    //Manager::self().actionCollection()->clear();
}

void GUIClient::finalize()
{
    finalizeCollection( d->collection );
    d->collection = 0;
}

void GUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}

void GUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    //ActionCollection* installedcollection = d->collections["installedscripts"];
    //if(! merge && installedcollection) installedcollection->clear();

    KXMLGUIClient::setDOMDocument(document, merge);
    //loadScriptConfigDocument(xmlFile(), document);
}

#if 0
bool GUIClient::writeConfigFromPackages()
{
    KConfig* config = KApplication::kApplication()->sessionConfig();
    krossdebug( QString("GUIClient::readConfigFromPackages hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );
    if(config->isReadOnly())
        return false;

    config->setGroup("scripts");
    QStringList names = config->readEntry("names", QStringList());

    QByteArray partname = d->guiclient->componentData().componentName();
    QStringList files = KGlobal::dirs()->findAllResources("data", partname + "/scripts/*/install.rc");
    files.sort();
    foreach(QString file, files) {
        krossdebug( QString("GUIClient::readConfigFromPackages trying to read \"%1\"").arg(file) );
        QFile f(file);
        if(! f.open(QIODevice::ReadOnly)) {
            krossdebug( QString("GUIClient::readAllConfigs reading \"%1\" failed. Skipping package.").arg(file) );
            continue;
        }

        QDomDocument domdoc;
        bool ok = domdoc.setContent(&f);
        f.close();
        if(! ok) {
            krossdebug( QString("GUIClient::readConfigFromPackages parsing \"%1\" failed. Skipping package.").arg(file) );
            continue;
        }

        QDomNodeList nodelist = domdoc.elementsByTagName("ScriptAction");
        int nodelistcount = nodelist.count();
        for(int i = 0; i < nodelistcount; ++i) {
            QDomElement element = nodelist.item(i).toElement();
            const QString name = element.attribute("name");
            if(d->actions->action(name) != 0) {
                // if the script-package is already in the list of actions, it's an
                // already enabled one and therefore it's not needed to add it again.
                continue;
            }

            names << name;
            config->writeEntry(QString("%1_text").arg(name).toLatin1(), element.attribute("text"));
            config->writeEntry(QString("%1_description").arg(name).toLatin1(), element.attribute("description"));
            config->writeEntry(QString("%1_icon").arg(name).toLatin1(), element.attribute("icon"));
            config->writeEntry(QString("%1_interpreter").arg(name).toLatin1(), element.attribute("interpreter"));

            QString f = element.attribute("file");
            QFileInfo fi(f);
            if(! QFileInfo(f).exists()) {
                const QDir packagepath = QFileInfo(file).dir();
                QFileInfo fi2(packagepath, f);
                if( fi2.exists() ) {
                    f = fi2.absoluteFilePath();
                }
                else {
                    QString resource = KGlobal::dirs()->findResource("appdata", QString("scripts/%1/%2").arg(name).arg(f));
                    if( ! resource.isNull() )
                        f = resource;
                }
            }
            config->writeEntry(QString("%1_file").arg(name).toLatin1(), f);
        }
    }

    config->writeEntry("names", names);
    config->sync();
    return true;
}
#endif

#if 0
void GUIClient::setXMLFile(const QString& file, bool merge, bool setXMLDoc)
{
    KXMLGUIClient::setXMLFile(file, merge, setXMLDoc);
}
void GUIClient::setDOMDocument(const QDomDocument &document, bool merge)
{
    ActionCollection* installedcollection = d->collections["installedscripts"];
    if(! merge && installedcollection) installedcollection->clear();
    KXMLGUIClient::setDOMDocument(document, merge);
    loadScriptConfigDocument(xmlFile(), document);
}
#endif

void addMenu(QMenu* menu, ActionCollection* collection)
{
    foreach(Action* a, collection->actions())
        menu->addAction(a);
    foreach(QString collectionname, collection->collections()) {
        ActionCollection* c = collection->collection(collectionname);
        if( c->isEnabled() )
            addMenu(menu->addMenu( c->text() ), c);
    }
}

void GUIClient::slotMenuAboutToShow()
{
    d->scriptsmenu->menu()->clear();
    if( d->collection )
        addMenu(d->scriptsmenu->menu(), d->collection);
}

void GUIClient::slotShowExecuteScriptFile()
{
    krossdebug( QString("GUIClient::slotShowExecuteScriptFile") );
    showExecuteScriptFile();
}

void GUIClient::started(Kross::Action* action)
{
    Q_UNUSED(action);
    krossdebug( QString("GUIClient::started(Kross::Action*) name='%1'").arg(action->objectName()) );
}

void GUIClient::finished(Kross::Action* action)
{
    krossdebug( QString("GUIClient::finished(Kross::Action*) name='%1'").arg(action->objectName()) );
    if( action->hadError() ) {
        if( action->errorTrace().isNull() )
            KMessageBox::error(0, action->errorMessage());
        else
            KMessageBox::detailedError(0, action->errorMessage(), action->errorTrace());
    }
    //emit executionFinished(action);
}

bool GUIClient::showExecuteScriptFile()
{
    QStringList mimetypes;
    foreach(QString interpretername, Manager::self().interpreters()) {
        InterpreterInfo* info = Manager::self().interpreterInfo(interpretername);
        Q_ASSERT( info );
        mimetypes.append( info->mimeTypes().join(" ").trimmed() );
    }
    KFileDialog* filedialog = new KFileDialog(
        KUrl("kfiledialog:///KrossExecuteScript"), // startdir
        mimetypes.join(" "), // filter
        0, // custom widget
        0 // parent
    );
    filedialog->setCaption( i18n("Execute Script File") );
    filedialog->setOperationMode( KFileDialog::Opening );
    filedialog->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    return filedialog->exec() ? Manager::self().executeScriptFile( filedialog->selectedUrl().path() ) : false;
}

#include "guiclient.moc"
