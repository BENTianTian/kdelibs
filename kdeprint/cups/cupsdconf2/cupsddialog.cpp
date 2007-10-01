/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "cupsddialog.h"

#include <config.h>

#include <kdeprint_export.h>

#include "cupsdpage.h"
#include "cupsdconf.h"
#include "cupsdsplash.h"
#include "cupsdserverpage.h"
#include "cupsdlogpage.h"
#include "cupsdjobspage.h"
#include "cupsdfilterpage.h"
#include "cupsddirpage.h"
#include "cupsdnetworkpage.h"
#include "cupsdbrowsingpage.h"
#include "cupsdsecuritypage.h"

#include <QtCore/QDir>
#include <QtGui/QWhatsThis>
#include <kvbox.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kpagewidgetmodel.h>
#include <QtCore/QStringList>
#include <kpassworddialog.h>
#include <kguiitem.h>
#include <QtCore/Q_PID>

#include <signal.h>
#include <cups/cups.h>

static bool dynamically_loaded = false;
static QString pass_string;

extern "C"
{
#include "cups-util.h"
    KDEPRINT_EXPORT bool restartServer(QString& msg) {
        return CupsdDialog::restartServer(msg);
    }
    KDEPRINT_EXPORT bool configureServer(QWidget *parent, QString& msg) {
        dynamically_loaded = true;
        bool result = CupsdDialog::configure(QString(), parent, &msg);
        dynamically_loaded = false;
        return result;
    }
}

int getServerPid()
{
    QDir dir("/proc", QString(), QDir::Name, QDir::Dirs);
    for (uint i = 0;i < dir.count();i++) {
        if (dir[i] == "." || dir[i] == ".." || dir[i] == "self") continue;
        QFile f("/proc/" + dir[i] + "/cmdline");
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            QTextStream t(&f);
            QString line;
            t >> line;
            f.close();
            if (line.endsWith("cupsd") ||
                    line.right(6).left(5) == "cupsd") // second condition for 2.4.x kernels
                // which add a null byte at the end
                return dir[i].toInt();
        }
    }
    return (-1);
}

const char* getPassword(const char*)
{
    static char buffer[1024];
    KPasswordDialog dlg(0L, KPasswordDialog::ShowUsernameLine | KPasswordDialog::UsernameReadOnly);
    dlg.setUsername(cupsUser());
    if (!dlg.exec())
        return NULL;
    strlcpy(buffer, dlg.password().toLocal8Bit() , 1024);
    return buffer;
}

//---------------------------------------------------

CupsdDialog::CupsdDialog(QWidget *parent, const char *name)
        : KPageDialog(parent)
{
    setFaceType(List);
    setCaption(i18n("CUPS Server Configuration"));
    setButtons(Ok | Cancel | User1);
    setButtonGuiItem(User1, KGuiItem(i18n("Short Help"), "help-contents"));
    setDefaultButton(Ok);
    setObjectName(name);
    setModal(true);
    showButtonSeparator(true);

    KIconLoader::global()->addAppDir("kdeprint4");
    KGlobal::locale()->insertCatalog("cupsdconf4");

    filename_ = "";
    conf_ = 0;
    constructDialog();
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
}

CupsdDialog::~CupsdDialog()
{
    delete conf_;
}

void CupsdDialog::addConfPage(CupsdPage *page)
{
    QPixmap icon = KIconLoader::global()->loadIcon(page->pixmap(),
                   KIconLoader::NoGroup,
                   KIconLoader::SizeMedium
                                                  );

    KVBox *box = new KVBox(this);
    page->setParent(box);
    pagelist_.append(page);

    KPageWidgetItem *item = addPage(box, page->pageLabel());
    item->setHeader(page->header());
    item->setIcon(KIcon(QIcon(icon)));
}

void CupsdDialog::constructDialog()
{
    addConfPage(new CupsdSplash(0));
    addConfPage(new CupsdServerPage(0));
    addConfPage(new CupsdNetworkPage(0));
    addConfPage(new CupsdSecurityPage(0));
    addConfPage(new CupsdLogPage(0));
    addConfPage(new CupsdJobsPage(0));
    addConfPage(new CupsdFilterPage(0));
    addConfPage(new CupsdDirPage(0));
    addConfPage(new CupsdBrowsingPage(0));

    conf_ = new CupsdConf();
    QListIterator<CupsdPage*> it(pagelist_);
    while (it.hasNext())
        it.next()->setInfos(conf_);
}

bool CupsdDialog::setConfigFile(const QString& filename)
{
    filename_ = filename;
    if (!conf_->loadFromFile(filename_)) {
        KMessageBox::error(this, i18n("Error while loading configuration file."), i18n("CUPS Configuration Error"));
        return false;
    }
    if (conf_->unknown_.count() > 0) {
        // there were some unknown options, warn the user
        QString msg;
        for (QList< QPair<QString, QString> >::ConstIterator it = conf_->unknown_.begin(); it != conf_->unknown_.end(); ++it)
            msg += ((*it).first + " = " + (*it).second + "<br>");
        msg.prepend("<p>" + i18n("Some options were not recognized by this configuration tool. "
                                 "They will be left untouched and you won't be able to change them.") + "</p>");
        KMessageBox::sorry(this, msg, i18n("Unrecognized Options"));
    }
    bool ok(true);
    QString msg;
    QListIterator<CupsdPage*> it(pagelist_);
    while (it.hasNext() && ok)
        ok = it.next()->loadConfig(conf_, msg);

    if (!ok) {
        KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"), i18n("CUPS Configuration Error"));
        return false;
    }
    return true;
}

bool CupsdDialog::restartServer(QString& msg)
{
    int serverPid = getServerPid();
    msg.truncate(0);
    if (serverPid <= 0) {
        msg = i18n("Unable to find a running CUPS server");
    } else {
        bool success = false;
        if (getuid() == 0)
            success = (::kill(serverPid, SIGHUP) == 0);
        else {
            success = !QProcess::execute("kdesu",
                                         QStringList() << "-c" << ("kill -SIGHUP " + QString::number(serverPid)));
        }
        if (!success)
            msg = i18n("Unable to restart CUPS server (pid = %1)", serverPid);
    }
    return (msg.isEmpty());
}

bool CupsdDialog::configure(const QString& filename, QWidget *parent, QString *msg)
{
    bool needUpload(false);
    QString errormsg;
    bool result = true;

    // init password dialog if needed
    if (!dynamically_loaded)
        cupsSetPasswordCB(getPassword);

    // load config file from server
    QString fn(filename);
    if (fn.isEmpty()) {
        fn = cupsGetConf();
        if (fn.isEmpty())
            errormsg = i18n("Unable to retrieve configuration file from the CUPS server. "
                            "You probably don't have the access permissions to perform this operation.");
        else needUpload = true;
    }

    // check read state (only if needed)
    if (!fn.isEmpty()) {
        QFileInfo fi(fn);
        if (!fi.exists() || !fi.isReadable() || !fi.isWritable())
            errormsg = i18n("Internal error: file '%1' not readable/writable.", fn);
        // check file size
        if (fi.size() == 0)
            errormsg = i18n("Internal error: empty file '%1'.", fn);
    }

    if (!errormsg.isEmpty()) {
        if (!dynamically_loaded)
            KMessageBox::error(parent, errormsg.prepend("<qt>").append("</qt>"), i18n("CUPS Configuration Error"));
        result = false;
    } else {
        KGlobal::locale()->insertCatalog("cupsdconf4"); // Must be before dialog is created to translate "Short Help"
        CupsdDialog dlg(parent);
        if (dlg.setConfigFile(fn) && dlg.exec()) {
            QByteArray encodedFn = QFile::encodeName(fn);
            if (!needUpload)
                KMessageBox::information(parent,
                                         i18n("The config file has not been uploaded to the "
                                              "CUPS server. The daemon will not be restarted."));
            else if (!cupsPutConf(encodedFn.data())) {
                errormsg = i18n("Unable to upload the configuration file to CUPS server. "
                                "You probably don't have the access permissions to perform this operation.");
                if (!dynamically_loaded)
                    KMessageBox::error(parent, errormsg, i18n("CUPS configuration error"));
                result = false;
            }
        }

    }
    if (needUpload)
        QFile::remove(fn);

    if (msg)
        *msg = errormsg;
    return result;
}

void CupsdDialog::slotOk()
{
    if (conf_ && !filename_.isEmpty()) { // try to save the file
        bool ok(true);
        QString msg;
        CupsdConf newconf_;
        QListIterator<CupsdPage*> it(pagelist_);
        while (it.hasNext() && ok)
            ok = it.next()->saveConfig(&newconf_, msg);
        // copy unknown options
        newconf_.unknown_ = conf_->unknown_;
        if (!ok) {
            ; // do nothing
        } else if (!newconf_.saveToFile(filename_)) {
            msg = i18n("Unable to write configuration file %1", filename_);
            ok = false;
        }
        if (!ok) {
            KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"), i18n("CUPS Configuration Error"));
        } else
            KPageDialog::accept();
    }
}

void CupsdDialog::slotUser1()
{
    QWhatsThis::enterWhatsThisMode();
}

int CupsdDialog::serverPid()
{
    return getServerPid();
}

int CupsdDialog::serverOwner()
{
    int pid = getServerPid();
    if (pid > 0) {
        QString str;
        str.sprintf("/proc/%d/status", pid);
        QFile f(str);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            QTextStream t(&f);
            while (!t.atEnd()) {
                str = t.readLine();
                if (str.indexOf("Uid:", 0, Qt::CaseInsensitive) == 0) {
                    QStringList list = str.split('\t', QString::SkipEmptyParts);
                    if (list.count() >= 2) {
                        bool ok;
                        int u = list[1].toInt(&ok);
                        if (ok) return u;
                    }
                }
            }
        }
    }
    return (-1);
}

#include "cupsddialog.moc"
