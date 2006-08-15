/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2006 Matt Broadstone (mbroadst@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QSharedData>
#include <QDebug>

#include <ktoolbar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kguiitem.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <kshortcut.h>
#include <kconfig.h>
#include <kconfigbase.h>
#include <kapplication.h>
#include <kstringhandler.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>

#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/cursor.h>

#include "kjs_proxy.h"
#include "kjs_dom.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "khtml_pagecache.h"
#include "khtml_settings.h"
#include "khtml_factory.h"
#include "misc/decoder.h"
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>
#include <kjs/value.h>

#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QToolButton>

#include "numberedtextview.h"
#include "breakpointsdock.h"
#include "consoledock.h"
#include "localvariabledock.h"
#include "watchesdock.h"
#include "callstackdock.h"
#include "scriptsdock.h"

#include "debugwindow.h"
#include "debugwindow.moc"

using namespace KJS;

DebugWindow* DebugWindow::m_debugger = 0;
DebugWindow *DebugWindow::createInstance()
{
    Q_ASSERT(!m_debugger);
    m_debugger = new DebugWindow();
    return m_debugger;
}

void DebugWindow::destroyInstance()
{
    Q_ASSERT(m_debugger);
    m_debugger->hide();
    delete m_debugger;
}

DebugWindow * DebugWindow::window()
{
    return m_debugger;
}

// ----------------------------------------------

void DebugWindow::setNextSourceInfo(QString url, int baseLine)
{
    m_nextUrl = url;
    m_nextBaseLine = baseLine;
}

// ----------------------------------------------

DebugWindow::DebugWindow(QWidget *parent)
  : KMainWindow(parent, "DebugWindow", Qt::WType_TopLevel),
    KInstance("kjs_debugger")
{
    setCaption(i18n("JavaScript Debugger"));
    kDebug() << "creating DebugWindow" << endl;

    m_editor = KTextEditor::EditorChooser::editor();
    if ( !m_editor )
    {
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                                      "please check your KDE installation."));
        kapp->exit(1);
    }

    m_watches = new WatchesDock;
    m_localVariables = new LocalVariablesDock;
    m_scripts = new ScriptsDock;
    m_callStack = new CallStackDock;
    m_breakpoints = new BreakpointsDock;
    m_console = new ConsoleDock;
    m_docFrame = new QFrame;

    addDockWidget(Qt::LeftDockWidgetArea, m_scripts);
    addDockWidget(Qt::LeftDockWidgetArea, m_localVariables);
    addDockWidget(Qt::LeftDockWidgetArea, m_callStack);
    addDockWidget(Qt::LeftDockWidgetArea, m_breakpoints);
    addDockWidget(Qt::LeftDockWidgetArea, m_watches);

    QFrame *mainFrame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout(mainFrame);
    layout->setSpacing(0);
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(m_docFrame);
    splitter->addWidget(m_console);
    layout->addWidget(splitter);

    setCentralWidget(mainFrame);
    resize(800, 500);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createTabWidget();
    m_tabWidget->hide();

    connect(m_scripts, SIGNAL(displayScript(KJS::DebugDocument*)),
            this, SLOT(displayScript(KJS::DebugDocument*)));
}

void DebugWindow::createActions()
{
    // Standard actions
    m_exitAct = new KAction(i18n("E&xit"), actionCollection(), "exit");
    m_exitAct->setShortcut(i18n("Ctrl+Q"));
    m_exitAct->setStatusTip(i18n("Exit the application"));
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(close()));

    // Flow control actions
    m_continueAct = new KAction(KIcon(":/images/continue.png"), i18n("Continue"), actionCollection(), "continue");
    m_continueAct->setStatusTip(i18n("Continue script execution"));
    m_continueAct->setToolTip(i18n("Continue script execution"));
    connect(m_continueAct, SIGNAL(triggered(bool)), this, SLOT(continueExecution()));

    m_stopAct = new KAction(KIcon(":/images/stop.png"), i18n("Stop"), actionCollection(), "stop");
    m_stopAct->setStatusTip(i18n("Stop script execution"));
    m_stopAct->setToolTip(i18n("Stop script execution"));
    connect(m_stopAct, SIGNAL(triggered(bool)), this, SLOT(stopExecution()));

    m_stepIntoAct = new KAction(KIcon(":/images/step-into.png"), i18n("Step Into"), actionCollection(), "stepInto");
    m_stepIntoAct->setStatusTip(i18n("Step Into"));
    m_stepIntoAct->setToolTip(i18n("Step Into"));
    connect(m_stepIntoAct, SIGNAL(triggered(bool)), this, SLOT(stepInto()));

    m_stepOutAct = new KAction(KIcon(":/images/step-out.png"), i18n("Step Out"), actionCollection(), "stepOut");
    m_stepOutAct->setStatusTip(i18n("Step Out"));
    m_stepOutAct->setToolTip(i18n("Step Out"));
    connect(m_stepOutAct, SIGNAL(triggered(bool)), this, SLOT(stepOut()) );

    m_stepOverAct = new KAction(KIcon(":/images/step-over.png"), i18n("Step Over"), actionCollection(), "stepOver");
    m_stepOverAct->setStatusTip(i18n("Step Over"));
    m_stepOverAct->setToolTip(i18n("Step Over"));
    connect(m_stepOverAct, SIGNAL(triggered(bool)), this, SLOT(stepOver()) );
}

void DebugWindow::createMenus()
{
/*
    KMenu *debugMenu = new KMenu(this);
    debugMenu->addAction(m_nextAction);
    debugMenu->addAction(m_stepAction);
    debugMenu->addAction(m_continueAction);
    debugMenu->addAction(m_breakAction);

    menuBar()->insertItem("&Debug", debugMenu);
*/
    KMenu *fileMenu = new KMenu("F&ile", this);
    fileMenu->addAction(m_exitAct);
    menuBar()->addMenu(fileMenu);
}

void DebugWindow::createToolBars()
{
    toolBar()->addAction(m_stopAct);
    toolBar()->addSeparator();
    toolBar()->addAction(m_continueAct);
    toolBar()->addAction(m_stepIntoAct);
    toolBar()->addAction(m_stepOutAct);
    toolBar()->addAction(m_stepOverAct);
}

void DebugWindow::createTabWidget()
{
    QVBoxLayout *layout = new QVBoxLayout(m_docFrame);
    m_tabWidget = new QTabWidget;
/*
    QToolButton *newTabButton = new QToolButton(this);
    m_tabWidget->setCornerWidget(newTabButton, Qt::TopLeftCorner);
    newTabButton->setCursor(Qt::ArrowCursor);
    newTabButton->setAutoRaise(true);
    newTabButton->setIcon(QIcon(ImageLocation + QLatin1String("addtab.png")));
    QObject::connect(newTabButton, SIGNAL(clicked()), this, SLOT(newTab()));
    newTabButton->setToolTip(tr("Add page"));
*/
    QToolButton *closeTabButton = new QToolButton(m_tabWidget);
    m_tabWidget->setCornerWidget(closeTabButton, Qt::TopRightCorner);
    closeTabButton->setCursor(Qt::ArrowCursor);
    closeTabButton->setAutoRaise(true);
    closeTabButton->setIcon(QIcon(":/images/removetab.png"));
    connect(closeTabButton, SIGNAL(clicked()), this, SLOT(closeTab()));
    closeTabButton->setToolTip(tr("Close source"));
    closeTabButton->setEnabled(true);
    layout->addWidget(m_tabWidget);
}

// -------------------------------------------------------------
void DebugWindow::createStatusBar()
{
    statusBar()->showMessage(i18n("Ready"));
}

void DebugWindow::stopExecution()
{
    m_mode = Stop;

//    while (!m_execStates.isEmpty())
//        leaveSession();
}

void DebugWindow::continueExecution()
{
    enableOtherWindows();
    m_mode = Continue;
}

void DebugWindow::stepInto()
{
    KMessageBox::information(this, "Step Into!");
}

void DebugWindow::stepOut()
{
    KMessageBox::information(this, "Step Out Of!");
}

void DebugWindow::stepOver()
{
    KMessageBox::information(this, "Step Over!");
}

DebugWindow::~DebugWindow()
{
    while (!m_documentList.isEmpty())
        delete m_documentList.takeFirst();

    m_documentList.clear();
}

// -------------------------------------------------------------
bool DebugWindow::sourceParsed(ExecState *exec, int sourceId, const UString &sourceURL,
                               const UString &source, int startingLineNumber, int errorLine, const UString &errorMsg)
{
    Q_UNUSED(exec);

    kDebug() << "***************************** sourceParsed **************************************************" << endl
             << "      sourceId: " << sourceId << endl
             << "     sourceURL: " << sourceURL.qstring() << endl
             << "     m_nextUrl: " << m_nextUrl << endl
             << "m_nextBaseLine: " << m_nextBaseLine << endl
             << "        source: " << source.qstring() << endl
             << "     errorLine: " << errorLine << endl
             << "*********************************************************************************************" << endl;

    // Determine key
    QString key = QString("%1|%2").arg((long)exec->dynamicInterpreter()).arg(m_nextUrl);

    DebugDocument *document = 0;
    if (!m_nextUrl.isEmpty())
        document = m_documents[key];
    if (!document)
    {
//        if (!m_nextUrl.isEmpty()) // Not in our cache, but has a URL
//        {
            document = new DebugDocument(m_nextUrl, exec->dynamicInterpreter());
            m_documents[key] = document;
            m_sourceIdLookup[sourceId] = document;
//        }
    }
    else
    {
        // interpreter should already be there, if it isn't then we should look above to the problem
    }


    if (document)
    {
        document->addCodeFragment(sourceId, m_nextBaseLine, source.qstring());
        m_scripts->addDocument(document);

        m_nextBaseLine = 0;
        m_nextUrl = "";
    }

    return (m_mode != Stop);
}

bool DebugWindow::sourceUnused(ExecState *exec, int sourceId)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);

    // Remove the debug document associated with this sourceId
    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
        m_scripts->documentDestroyed(document);
        if (!document->deleteFragment(sourceId))   // this means we've removed all the source fragments
            delete document;
    }

    return (m_mode != Stop);
}

bool DebugWindow::exception(ExecState *exec, int sourceId, int lineno, JSObject *exceptionObj)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(exceptionObj);

    kDebug() << "exception" << endl;

    return (m_mode != Stop);
}


// This is where we are going to check for a breakpoint. First check for breakpoint
// then if one is found, stop execution and display local variables in the localVariables
// dock.

bool DebugWindow::atStatement(ExecState *exec, int sourceId, int firstLine, int lastLine)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(firstLine);
    Q_UNUSED(lastLine);

//     kDebug() << "***************************** atStatement ***************************************************" << endl
//              << "      sourceId: " << sourceId << endl
//              << "     firstLine: " << firstLine << endl
//              << "      lastLine: " << lastLine << endl;

    DebugDocument *document = m_sourceIdLookup[sourceId];
    if (document)
    {
//         kDebug() << "found document for sourceId" << endl;
//         QVector<int> bpoints = document->breakpoints();
//         foreach (int bpoint, bpoints)
//         {
//             kDebug() << " > " << bpoint << endl;
//         }

/*
        int numLines = lastLine - firstLine;
        for (int i = 0; i < numLines; i++)
        {
            int lineNumber = firstLine + i;
            kDebug() << "breakpoint at line " << lineNumber << "?" << endl;
            if (document->hasBreakpoint(lineNumber))
            {
                kDebug() << "Hey! we actually found a breakpoint!" << endl;
                // Lets try a dump of the scope chain now..
                m_localVariables->display(exec->dynamicInterpreter());
            }
      }
*/
        if (document->hasBreakpoint(firstLine))
        {
//             kDebug() << "Hey! we actually found a breakpoint!" << endl;
            // Lets try a dump of the scope chain now..
            // m_localVariables->display(exec);
            enterDebugSession(exec);
        }
    }

//     kDebug() << "*********************************************************************************************" << endl;

    return (m_mode != Stop);
}

bool DebugWindow::callEvent(ExecState *exec, int sourceId, int lineno, JSObject *function, const List &args)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(function);
    Q_UNUSED(args);

//     kDebug() << "callEvent" << endl;

    return (m_mode != Stop);
}

bool DebugWindow::returnEvent(ExecState *exec, int sourceId, int lineno, JSObject *function)
{
    Q_UNUSED(exec);
    Q_UNUSED(sourceId);
    Q_UNUSED(lineno);
    Q_UNUSED(function);

//     kDebug() << "returnEvent" << endl;

    return (m_mode != Stop);
}

// End KJS::Debugger overloads

void DebugWindow::enableKateHighlighting(KTextEditor::Document *document)
{
    KTextEditor::HighlightingInterface *highlightingInterface = qobject_cast<KTextEditor::HighlightingInterface*>(document);
    if (!m_highlightingMode)
    {
        if (highlightingInterface)
        {
            int count = highlightingInterface->hlModeCount();
            for (int i=0; i<count; i++)
            {
                QString modeName = highlightingInterface->hlModeName(i);
                QString sectionName = highlightingInterface->hlModeSectionName(i);
                if (modeName == "JavaScript")
                {
                    m_highlightingMode = i;
                    break;
                }
            }
        }
    }

    if (m_highlightingMode && highlightingInterface)
        highlightingInterface->setHlMode(m_highlightingMode);
}

void DebugWindow::displayScript(KJS::DebugDocument *document)
{
    if (m_tabWidget->isHidden())
        m_tabWidget->show();

    if (m_openDocuments.contains(document))
    {
        int idx = m_openDocuments.indexOf(document);
        m_tabWidget->setCurrentIndex(idx);
        return;
    }

    KTextEditor::Document *doc = 0;
    doc = m_debugLut[document];     // Check to see if we've already worked on this document
    if (!doc)
    {
        doc = m_editor->createDocument(0);
        m_documentList.append(doc);

        m_debugLut[document] = doc;
        m_documentLut[doc] = document;
    }

    enableKateHighlighting(doc);
    QList<SourceFragment> fragments = document->fragments();
    foreach (SourceFragment fragment, fragments)
    {
        kDebug() << "fragment: " << fragment.source << endl;

        int line = fragment.baseLine;
        int col = 1;

        KTextEditor::Cursor cur;
        cur.setPosition(line, col);
        doc->insertText(cur, fragment.source);
    }
//    doc->setText(document->source());

    KTextEditor::View *view = qobject_cast<KTextEditor::View*>(doc->createView(this));
    KTextEditor::ConfigInterface *configInterface = qobject_cast<KTextEditor::ConfigInterface*>(view);
    if (configInterface)
    {
        if (configInterface->configKeys().contains("line-numbers"))
            configInterface->setConfigValue("line-numbers", true);
        if (configInterface->configKeys().contains("icon-bar"))
            configInterface->setConfigValue("icon-bar", true);
        if (configInterface->configKeys().contains("dynamic-word-wrap"))
            configInterface->setConfigValue("dynamic-word-wrap", true);
    }

    KTextEditor::MarkInterface *markInterface = qobject_cast<KTextEditor::MarkInterface*>(doc);
    if (markInterface)
    {
//        markInterface->setEditableMarks(KTextEditor::MarkInterface::BreakpointActive);
        connect(doc, SIGNAL(markChanged(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)),
                this, SLOT(markSet(KTextEditor::Document*, KTextEditor::Mark, KTextEditor::MarkInterface::MarkChangeAction)));
    }

    m_openDocuments.append(document);
    m_tabWidget->addTab(view, document->name());
}

void DebugWindow::markSet(KTextEditor::Document *document, KTextEditor::Mark mark,
                          KTextEditor::MarkInterface::MarkChangeAction action)
{
    DebugDocument *debugDocument = m_documentLut[document];
    if (!debugDocument)
        return;

    int lineNumber = mark.line + 1;         // we do this because bookmarks are technically set
                                            // the line before what looks like the line you chose..

    switch(action)
    {
        case KTextEditor::MarkInterface::MarkAdded:
            debugDocument->setBreakpoint(lineNumber);
            break;
        case KTextEditor::MarkInterface::MarkRemoved:
            debugDocument->removeBreakpoint(lineNumber);
            break;
    }

    kDebug() << "breakpoint set for: " << endl
             << "document: " << document->documentName() << endl
             << "line: " << lineNumber << " type: " << mark.type << endl;

    kDebug() << "breakpoints at lines:" << endl;
    QVector<int> bpoints = debugDocument->breakpoints();
    foreach (int bpoint, bpoints)
        kDebug() << " > " << bpoint << endl;
}

void DebugWindow::closeTab()
{
    int idx = m_tabWidget->currentIndex();
    m_tabWidget->removeTab(idx);
    m_openDocuments.removeAt(idx);
    if (m_openDocuments.isEmpty())
        m_tabWidget->hide();
}


void DebugWindow::enterDebugSession(KJS::ExecState *exec)
{
    // This "enters" a new debugging session, i.e. enables usage of the debugging window
    // It re-enters the qt event loop here, allowing execution of other parts of the
    // program to continue while the script is stopped. We have to be a bit careful here,
    // i.e. make sure the user can't quit the app, and disable other event handlers which
    // could interfere with the debugging session.

    if (!isVisible())
        show();

    m_mode = Stop;

//    if (m_execStates.isEmpty())
    {
        m_continueAct->setEnabled(true);
        m_stopAct->setEnabled(true);
        m_stepIntoAct->setEnabled(true);
        m_stepOutAct->setEnabled(true);
        m_stepOverAct->setEnabled(true);
    }


    m_localVariables->display(exec);
    disableOtherWindows();
/*
    bool done = false;
    while (!done)
    {
        if (m_mode == Continue)
            done = true;
        kapp->processEvents(QEventLoop::ExcludeSocketNotifiers | 
                            QEventLoop::DeferredDeletion | QEventLoop::X11ExcludeTimers);

    }


    m_localVariables->clear(); */
}




//// Event handling - ripped from old kjsdebugger

bool DebugWindow::eventFilter(QObject *object, QEvent *event)
{
    /*
    if (object == this)
        return QObject::eventFilter(object, event);
    return true;
    */

/*
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::Destroy:
        case QEvent::Close:
        case QEvent::Quit:
            if (object == this)
                return QWidget::eventFilter(object, event);
            else
                return true;
            break;
        default:
            return QWidget::eventFilter(object, event);
    }
    */
}

void DebugWindow::disableOtherWindows()
{
    kapp->installEventFilter(this);
/*
  QWidgetList widgets = QApplication::allWidgets();
  QListIterator<QWidget*> it(widgets);
  while (it.hasNext()) {
    QWidget* widget = it.next();
    widget->installEventFilter(this);
  }
*/
}

void DebugWindow::enableOtherWindows()
{
    kapp->removeEventFilter(this);
/*
  QWidgetList widgets = QApplication::allWidgets();
  QListIterator<QWidget*> it(widgets);
  while (it.hasNext()) {
    QWidget* widget = it.next();
    widget->removeEventFilter(this);
  }
*/
}















////////////////////// DEBUG DOCUMENT


class DebugDocument::Private : public QSharedData
{
public:
    QString url;
    QString name;
    QString source;
    Interpreter *interpreter;
    QHash<int, SourceFragment> codeFragments;
    QVector<int> breakpoints;

};

DebugDocument::DebugDocument(const QString &url, Interpreter *interpreter)
{
    d = new DebugDocument::Private;
    d->url = url;
    d->interpreter = interpreter;

    QStringList splitUrl = url.split('/');
    if (!splitUrl.isEmpty())
    {
        qDebug() << splitUrl;
        while (d->name.isEmpty() && !splitUrl.isEmpty())
            d->name = splitUrl.takeLast();

        if (d->name.isEmpty())
            d->name = "undefined";
    }
    else
        d->name = "undefined";

    if (d->interpreter)
    {
        ScriptInterpreter *scriptInterpreter = static_cast<ScriptInterpreter*>(d->interpreter);
        KHTMLPart *part = qobject_cast<KHTMLPart*>(scriptInterpreter->part());
        if (part &&
            d->url == part->url().url())
        {
//            connect(part, SIGNAL(completed()), this, SLOT(readSource()));
        }
    }
}

DebugDocument::DebugDocument(const DebugDocument &other)
{
    d = other.d;
}

DebugDocument::~DebugDocument()
{
}

QString DebugDocument::name() const
{
    return d->name;
}

QString DebugDocument::url() const
{
    return d->url;
}

Interpreter * DebugDocument::interpreter() const
{
    return d->interpreter;
}

QList<SourceFragment> DebugDocument::fragments()
{
    return d->codeFragments.values();
}

bool DebugDocument::deleteFragment(int sourceId)
{
    if (d->codeFragments.contains(sourceId))
    {
        d->codeFragments.remove(sourceId);
        return true;
    }
    return false;
}

SourceFragment DebugDocument::fragment(int sourceId)
{
    if (d->codeFragments.contains(sourceId))
        return d->codeFragments[sourceId];
    else
        return SourceFragment();
}

QString DebugDocument::source() const
{
    return d->source;
}

void DebugDocument::addCodeFragment(int sourceId, int baseLine, const QString &source)
{
    SourceFragment code;
    code.sourceId = sourceId;
    code.baseLine = baseLine;
    code.source = source;

    d->codeFragments[sourceId] = code;
}

void DebugDocument::setBreakpoint(int lineNumber)
{
    d->breakpoints.append(lineNumber);
}

void DebugDocument::removeBreakpoint(int lineNumber)
{
    int idx = d->breakpoints.indexOf(lineNumber);
    if (idx != -1)
        d->breakpoints.remove(idx);
}

bool DebugDocument::hasBreakpoint(int lineNumber)
{
    return d->breakpoints.contains(lineNumber);
}

QVector<int> DebugDocument::breakpoints()
{
    return d->breakpoints;
}

void DebugDocument::readSource()
{
    if (d->interpreter)
    {
        ScriptInterpreter *scriptInterpreter = static_cast<ScriptInterpreter*>(d->interpreter);
        KHTMLPart *part = qobject_cast<KHTMLPart*>(scriptInterpreter->part());
        if (part &&
            d->url == part->url().url() &&
            !part->inProgress())
        {
            d->source = part->documentSource();
        }
    }
}

