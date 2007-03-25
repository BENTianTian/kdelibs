/***************************************************************************
 * action.h
 * This file is part of the KDE project
 * copyright (C)2004-2007 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_ACTION_H
#define KROSS_ACTION_H

#include <QString>
#include <QVariant>
#include <QObject>
#include <QDir>
#include <QtXml/QDomElement>
#include <QAction>

#include <kurl.h>

#include "errorinterface.h"
#include "childreninterface.h"

namespace Kross {

    /**
     * The Action class is an abstract container to deal with scripts
     * like a single standalone scriptfile. Each action holds a reference
     * to by the matching @a Kross::Interpreter created @a Kross::Script
     * instance.
     *
     * The \a Manager takes care of handling the \a Action instances
     * application by providing access to \a ActionCollection containers
     * for those \a Action instances.
     *
     * Once you've such a Action instance you're able to perform actions
     * with it like to execute scripting code.
     *
     * Following sample shows "Hello World." executed with the python
     * interpreter;
     * \code
     * # Create a new Kross::Action instance.
     * Kross::Action* action = new Kross::Action(0,"MyFirstScript");
     * # Set the interpreter we like to use. This could be e.g. "python", "ruby" or "kjs".
     * action->setInterpreter("python");
     * # Set the scripting code.
     * action->setCode("print \"Hello World.\"");
     * # Execute the scripting code.
     * action->trigger();
     * \endcode
     *
     * Following sample demonstrates how to execute an external python script
     * file. The script file itself is named "mytest.py" and contains;
     * \code
     * # this function got called from within C++
     * def myfunction(args):
     *     print "Arguments are: %s" % args
     * # Import the published QObject's
     * import MyFirstQObject, MySecondQObject
     * # Call a slot MyFirstQObject provides.
     * MyFirstQObject.someSlot("Some string")
     * # Set a property MySecondQObject provides.
     * MySecondQObject.someProperty = "Other string"
     * \endcode
     * Then you are able to load the script file, publish QObject instances
     * and let the script do whatever it likes to do;
     * \code
     * # Publish a QObject instance for all Kross::Action instances.
     * Kross::Manager::self().addChild(myqobject1, "MyFirstQObject")
     * # Create a new Kross::Action instance.
     * Kross::Action* action = new Kross::Action(0,"MySecondScript");
     * # Publish a QObject instance only for the Kross::Action instance.
     * action->addChild(myqobject2, "MySecondQObject");
     * # Set the script file we like to execute.
     * action->setFile("/home/myuser/mytest.py");
     * # Execute the script file.
     * action->trigger();
     * # Call the "myfunction" defined in the "mytest.py" python script.
     * QVariant result = action->callFunction("myfunction", QVariantList()<<"Arg");
     * \endcode
     */
    class KROSSCORE_EXPORT Action : public QAction, public ChildrenInterface, public ErrorInterface
    {
            Q_OBJECT

        public:

           /**
             * Constructor.
             *
             * \param parent The parent QObject this \a Action is child of.
             * \param name The unique name this Action has. It's used
             * e.g. at the \a Manager to identify the Action. The
             * name is accessible via \a QObject::objectName .
             */
            Action(QObject* parent, const QString& name, const QDir& packagepath = QDir());

            /**
             * Constructor.
             *
             * \param parent The parent QObject this \a Action is child of.
             * \param url The URL should point  to a valid scriptingfile.
             * This \a Action will be filled with the content of the
             * file (e.g. the file is readed and \a code should return
             * it's content and it's also tried to determinate the
             * \a interpreter ).
             */
            Action(QObject* parent, const KUrl& url);

            /**
             * Destructor.
             */
            virtual ~Action();

            /**
             * Method to read settings from the QDomElement \p element that
             * contains details about e.g. the displayed text, the file to
             * execute or the used interpreter.
             */
            void fromDomElement(const QDomElement& element);

            /**
             * \return a QDomElement that contains the settings like e.g. the
             * displayed text, the file to execute or the used interpreter
             * of this \a Action instance.
             */
            QDomElement toDomElement() const;

            /**
             * \return a map of options this \a Action defines.
             * The options are returned call-by-ref, so you are able to
             * manipulate them.
             */
            QMap<QString, QVariant>& options() const;

            /**
             * \return the value of the option defined with \p name .
             * If there doesn't exists an option with such a name,
             * the \p defaultvalue is returned.
             */
            QVariant option(const QString& name, QVariant defaultvalue = QVariant());

            /**
             * Set the \a Interpreter::Option value.
             */
            bool setOption(const QString& name, const QVariant& value);

            /**
             * \return the list of functionnames.
             */
            QStringList functionNames();

            /**
             * Call a function in the script.
             *
             * \param name The name of the function which should be called.
             * \param args The optional list of arguments.
             */
            QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());

            /**
             * Initialize the \a Script instance.
             *
             * Normaly it's not needed to call this function direct cause
             * if will be internaly called if needed (e.g. on \a execute ).
             *
             * \return true if the initialization was successfully else
             * false is returned.
             */
            bool initialize();

            /**
             * Finalize the \a Script instance and free's any cached or still
             * running executions. Normaly it's not needed to call this
             * function direct cause the \a Action will take care
             * of calling it if needed.
             */
            void finalize();

            /**
             * \return true if the action is finalized what means the
             * action is currently not running.
             */
            bool isFinalized() const;

        public Q_SLOTS:

            /**
             * \return the objectName for this Action.
             */
            QString name() const;

            /**
             * \return the version number this Action has.
             * Per default 0 is returned.
             */
            int version() const;

            /**
             * \return the optional description for this Action.
             */
            QString description() const;

            /**
             * Set the optional description for this Action.
             */
            void setDescription(const QString& description);

            /**
             * Return the name of the icon.
             */
            QString iconName() const;

            /**
             * Set the name of the icon to \p iconname .
             */
            void setIconName(const QString& iconname);

            /**
             * Return true if this Action is enabled else false is returned.
             */
            bool isEnabled() const;

            /**
             * Set the enable state of this Action to \p enabled .
             */
            void setEnabled(bool enabled);

            /**
             * \return the scriptfile that should be executed.
             */
            QString file() const;

            /**
             * Set the scriptfile that should be executed.
             */
            bool setFile(const QString& scriptfile);

            /**
             * \return the scriptcode this Action holds.
             */
            QString code() const;

            /**
             * Set the scriptcode \p code this Action should execute.
             */
            void setCode(const QString& code);

            /**
             * \return the name of the interpreter. Could be for
             * example "python" or "ruby".
             */
            QString interpreter() const;

            /**
             * Set the name of the interpreter.
             */
            void setInterpreter(const QString& interpretername);

            /**
             * \return the current path the script is running in or
             * an empty string if there is no current path defined.
             */
            QString currentPath() const;

            /**
             * Returns the names of all properties.
             */
            QStringList propertyNames() const;

            /**
             * Returns true if there exist a property with the name \p name .
             */
            bool hasProperty(const QString& name);

            /**
             * Return the value of the property with the name \p name . If there
             * exist no such property the \p defaultvalue got returned.
             */
            QString property(const QString& name, const QString& defaultvalue = QString());

            /**
             * Set the value of the property with the name \p name . If there
             * exist no such property a new one got added. If \p persistent is
             * true the property will be automatically saved and restored.
             */
            void setProperty(const QString& name, const QString& value, bool persistent = false);

            /**
             * Remove the property with the name \p name .
             */
            void removeProperty(const QString& name);

        Q_SIGNALS:

            /**
             * This signal is emitted if the content of the Action
             * was changed. The \a ActionCollection instances this Action
             * is child of are connected with this signal to fire up
             * there own updated signal if an Action of them was updated.
             */
            void updated();

            /**
             * This signal is emitted before the script got executed.
             */
            void started(Kross::Action*);

            /**
             * This signal is emitted after the script got executed.
             */
            void finished(Kross::Action*);

        private Q_SLOTS:

            /**
             * This private slot is connected with the \a QAction::triggered
             * signal. To execute the script just emit that signal and this
             * slot tries to execute the script.
             */
            void slotTriggered();

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

