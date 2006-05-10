/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// remove when Q3SimpleRichText is ported
#define QT3_SUPPORT
#define QT3_SUPPORT_WARNINGS

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>
#include <QTextEdit>

#include <Q3GroupBox>
#include <Q3SimpleRichText>

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kguiitem.h>
#include <klistbox.h>
#include <klocale.h>
#include <knotification.h>
#include <kstdguiitem.h>
#include <kactivelabel.h>
#include <kiconloader.h>
#include <kvbox.h>
#include <kpushbutton.h>

#include "kmessagebox.h"

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#endif

 /**
  * Easy MessageBox Dialog.
  *
  * Provides convenience functions for some i18n'ed standard dialogs,
  * as well as audible notification via @ref KNotification
  *
  * @author Waldo Bastian (bastian@kde.org)
  */

static bool KMessageBox_queue = false;

static QPixmap themedMessageBoxIcon(QMessageBox::Icon icon)
{
    QString icon_name;

    switch(icon)
    {
    case QMessageBox::NoIcon:
        return QPixmap();
        break;
    case QMessageBox::Information:
        icon_name = "messagebox_info";
        break;
    case QMessageBox::Warning:
        icon_name = "messagebox_warning";
        break;
    case QMessageBox::Critical:
        icon_name = "messagebox_critical";
        break;
    default:
        break;
    }

   QPixmap ret = KGlobal::instance()->iconLoader()->loadIcon(icon_name, K3Icon::NoGroup, K3Icon::SizeMedium, K3Icon::DefaultState, 0, true);

   if (ret.isNull())
       return QMessageBox::standardIcon(icon);
   else
       return ret;
}

static void sendNotification( QString message,
                              const QStringList& strlist,
                              QMessageBox::Icon icon,
                              WId parent_id )
{
    // create the message for KNotify
    QString messageType;
    switch ( icon )
    {
        case QMessageBox::Warning:
            messageType = "messageWarning";
            break;
        case QMessageBox::Critical:
            messageType = "messageCritical";
            break;
        case QMessageBox::Question:
            messageType = "messageQuestion";
            break;
        default:
            messageType = "messageInformation";
            break;
    }

    if ( !strlist.isEmpty() )
    {
        for ( QStringList::ConstIterator it = strlist.begin(); it != strlist.end(); ++it )
            message += "\n" + *it;
    }

    if ( !message.isEmpty() )
        KNotification::event( messageType, message , QPixmap() , QWidget::find( parent_id ) );
}

static QString qrichtextify( const QString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  QStringList lines = text.split('\n',QString::SkipEmptyParts);
  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = Qt::convertFromPlainText( *it, Qt::WhiteSpaceNormal );
  }

  return lines.join(QString());
}

int KMessageBox::createKMessageBox(KDialog *dialog, QMessageBox::Icon icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn,
                             Options options, const QString &details)
{
    return createKMessageBox(dialog, themedMessageBoxIcon(icon), text, strlist,
                      ask, checkboxReturn, options, details, icon);
}

int KMessageBox::createKMessageBox(KDialog *dialog, QPixmap icon,
                             const QString &text, const QStringList &strlist,
                             const QString &ask, bool *checkboxReturn, Options options,
                             const QString &details, QMessageBox::Icon notifyType)
{
    KVBox *topcontents = new KVBox (dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint());

    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint());

    QLabel *label1 = new QLabel( contents);

    if (!icon.isNull())
       label1->setPixmap(icon);

    lay->addWidget( label1, 0, Qt::AlignCenter );
    lay->addSpacing(KDialog::spacingHint());
    // Enforce <p>text</p> otherwise the word-wrap doesn't work well
    QString qt_text = qrichtextify( text );

    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       Q3SimpleRichText rt(qt_text, dialog->font());
       QRect d = KGlobalSettings::desktopGeometry(dialog);

       pref_width = d.width() / 3;
       rt.setWidth(pref_width);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (3*pref_height > 2*d.height())
       {
          // Very high dialog.. make it wider
          pref_width = d.width() / 2;
          rt.setWidth(pref_width);
          used_width = rt.widthUsed();
          pref_height = rt.height();
       }
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width);
             int new_height = rt.height();
             if (new_height > pref_height)
                break;
             used_width = rt.widthUsed();
             if (used_width > new_width)
                break;
          }
          pref_width = used_width;
       }
       else
       {
          if (used_width > (pref_width *2))
             pref_width = pref_width *2;
          else
             pref_width = used_width;
       }
    }
    KActiveLabel *label2 = new KActiveLabel( qt_text, contents );
    if (!(options & KMessageBox::AllowLink))
    {
       QObject::disconnect(label2, SIGNAL(anchorClicked (const QUrl &)),
                        label2, SLOT(openLink(const QUrl &)));
    }

    // We add 10 pixels extra to compensate for some KActiveLabel margins.
    // TODO: find out why this is 10.
    label2->setFixedSize(QSize(pref_width+10, pref_height));
    lay->addWidget( label2 );
    lay->addStretch();

    KListBox *listbox = 0;
    if (!strlist.isEmpty())
    {
       listbox=new KListBox( topcontents );
       listbox->insertStringList( strlist );
       listbox->setSelectionMode( Q3ListBox::NoSelection );
       topcontents->setStretchFactor(listbox, 1);
    }

    QPointer<QCheckBox> checkbox = 0;
    if (!ask.isEmpty())
    {
       checkbox = new QCheckBox(ask, topcontents);
       if (checkboxReturn)
         checkbox->setChecked(*checkboxReturn);
    }

    if (!details.isEmpty())
    {
       Q3GroupBox *detailsGroup = new Q3GroupBox( i18n("Details"), dialog);
       detailsGroup->setOrientation(Qt::Vertical);
       if ( details.length() < 512 ) {
         KActiveLabel *label3 = new KActiveLabel(qrichtextify(details),
                                                 detailsGroup);
         label3->setMinimumSize(label3->sizeHint());
         if (!(options & KMessageBox::AllowLink))
         {
           QObject::disconnect(label3, SIGNAL(anchorClicked (const QUrl &)),
                            label3, SLOT(openLink(const QUrl &)));
         }
       } else {
         QTextEdit* te = new QTextEdit(details, detailsGroup);
         te->setReadOnly( true );
         te->setMinimumHeight( te->fontMetrics().lineSpacing() * 11 );
       }
       dialog->setDetailsWidget(detailsGroup);
    }

    dialog->setMainWidget(topcontents);
    dialog->enableButtonSeparator(false);
    if (!listbox)
        dialog->setFixedSize( dialog->sizeHint() );

    const KDialog::ButtonCode buttons[] = {
        KDialog::Help,
        KDialog::Default,
        KDialog::Ok,
        KDialog::Apply,
        KDialog::Try,
        KDialog::Cancel,
        KDialog::Close,
        KDialog::User1,
        KDialog::User2,
        KDialog::User3,
        KDialog::No,
        KDialog::Yes,
        KDialog::Details };
    for( unsigned int i = 0; i < sizeof( buttons )/sizeof( buttons[ 0 ] ); ++i )
        if( QPushButton* btn = dialog->actionButton( buttons[ i ] ))
            if( btn->isDefault())
                btn->setFocus();

    if ( (options & KMessageBox::Notify) )
        sendNotification( text, strlist, notifyType, dialog->topLevelWidget()->winId());

    if (KMessageBox_queue)
    {
       KDialogQueue::queueDialog(dialog);
       return KMessageBox::Cancel; // We have to return something.
    }

    if ( (options & KMessageBox::NoExec) )
    {
       return KMessageBox::Cancel; // We have to return something.
    }

    // We use a QGuardedPtr because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the guarded ptr will reset to 0.
    QPointer<KDialog> guardedDialog = dialog;

    int result = guardedDialog->exec();
    if (checkbox && checkboxReturn)
       *checkboxReturn = checkbox->isChecked();
    delete (KDialog *) guardedDialog;
    return result;
}

int
KMessageBox::questionYesNo(QWidget *parent, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
   return questionYesNoList(parent, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::questionYesNoWId(WId parent_id, const QString &text,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
   return questionYesNoListWId(parent_id, text, QStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

bool
KMessageBox::shouldBeShownYesNo(const QString &dontShowAgainName,
                                ButtonCode &result)
{
    if ( dontShowAgainName.isEmpty() ) return true;
    QString grpNotifMsgs = QLatin1String("Notification Messages");
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroup cg( config, grpNotifMsgs );
    QString dontAsk = cg.readEntry(dontShowAgainName, QString()).toLower();
    if (dontAsk == "yes") {
        result = Yes;
        return false;
    }
    if (dontAsk == "no") {
        result = No;
        return false;
    }
    return true;
}

bool
KMessageBox::shouldBeShownContinue(const QString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) return true;
    QString grpNotifMsgs = QLatin1String("Notification Messages");
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroup cg( config, grpNotifMsgs );
    return cg.readEntry(dontShowAgainName, true);
}

void
KMessageBox::saveDontShowAgainYesNo(const QString &dontShowAgainName,
                                    ButtonCode result)
{
    if ( dontShowAgainName.isEmpty() ) return;
    QString grpNotifMsgs = QLatin1String("Notification Messages");
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroup cg( config, grpNotifMsgs );
    cg.writeEntry( dontShowAgainName, result==Yes ? "yes" : "no", ((dontShowAgainName[0] == ':')?KConfigBase::Global:KConfigBase::Normal));
    config->sync();
}

void
KMessageBox::saveDontShowAgainContinue(const QString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) return;
    QString grpNotifMsgs = QLatin1String("Notification Messages");
    KConfig *config = againConfig ? againConfig : KGlobal::config();
    KConfigGroup cg( config, grpNotifMsgs );
    cg.writeEntry( dontShowAgainName, false, ((dontShowAgainName[0] == ':')?KConfigBase::Global:KConfigBase::Normal));
    config->sync();
}

KConfig* KMessageBox::againConfig = NULL;
void
KMessageBox::setDontShowAskAgainConfig(KConfig* cfg)
{
  againConfig = cfg;
}

int
KMessageBox::questionYesNoList(QWidget *parent, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{ // in order to avoid code duplication, convert to WId, it will be converted back
    return questionYesNoListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::questionYesNoListWId(WId parent_id, const QString &text,
                           const QStringList &strlist,
                           const QString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const QString &dontAskAgainName,
                           Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Question") : caption,
      KDialog::Yes | KDialog::No, Qt::Dialog);
    dialog->setObjectName( "questionYesNo" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::No );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Information, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialog::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::questionYesNoCancel(QWidget *parent,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    return questionYesNoCancelWId( parent ? parent->winId() : 0, text, caption, buttonYes, buttonNo,
        dontAskAgainName, options );
}

int
KMessageBox::questionYesNoCancelWId(WId parent_id,
                          const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Question") : caption,
      KDialog::Yes | KDialog::No | KDialog::Cancel, Qt::Dialog);
    dialog->setObjectName( "questionYesNoCancel" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setDefaultButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Information,
                       text, QStringList(),
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    if ( result==KDialog::Cancel ) return Cancel;
    res = (result==KDialog::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::warningYesNo(QWidget *parent, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
   return warningYesNoList(parent, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoWId(WId parent_id, const QString &text,
                          const QString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const QString &dontAskAgainName,
                          Options options)
{
   return warningYesNoListWId(parent_id, text, QStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoList(QWidget *parent, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              Options options)
{
    return warningYesNoListWId( parent ? parent->winId() : 0, text, strlist, caption,
        buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::warningYesNoListWId(WId parent_id, const QString &text,
                              const QStringList &strlist,
                              const QString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const QString &dontAskAgainName,
                              Options options)
{
    // warningYesNo and warningYesNoList are always "dangerous"
    // ### Remove this line for KDE 4, when the 'options' default parameter
    // takes effects.
    options |= Dangerous;

    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Warning") : caption,
      KDialog::Yes | KDialog::No, Qt::Dialog);
    dialog->setObjectName( "warningYesNoList" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setDefaultButton( KDialog::No );
    dialog->setEscapeButton( KDialog::No );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialog::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::warningContinueCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &caption,
                                   const KGuiItem &buttonContinue,
                                   const QString &dontAskAgainName,
                                   Options options)
{
   return warningContinueCancelList(parent, text, QStringList(), caption,
                                buttonContinue, dontAskAgainName, options);
}

int
KMessageBox::warningContinueCancelWId(WId parent_id,
                                   const QString &text,
                                   const QString &caption,
                                   const KGuiItem &buttonContinue,
                                   const QString &dontAskAgainName,
                                   Options options)
{
   return warningContinueCancelListWId(parent_id, text, QStringList(), caption,
                                buttonContinue, dontAskAgainName, options);
}

int
KMessageBox::warningContinueCancelList(QWidget *parent, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const QString &dontAskAgainName,
                             Options options)
{
    return warningContinueCancelListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonContinue, dontAskAgainName, options );
}

int
KMessageBox::warningContinueCancelListWId(WId parent_id, const QString &text,
                             const QStringList &strlist,
                             const QString &caption,
                             const KGuiItem &buttonContinue,
                             const QString &dontAskAgainName,
                             Options options)
{
    if ( !shouldBeShownContinue(dontAskAgainName) )
        return Continue;

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Warning") : caption,
      KDialog::Yes | KDialog::No, Qt::Dialog);
    dialog->setObjectName( "warningYesNo" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonContinue );
    dialog->setButtonText( KDialog::No, KStdGuiItem::cancel().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::No );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result==KDialog::No )
        return Cancel;
    if (checkboxResult)
        saveDontShowAgainContinue(dontAskAgainName);
    return Continue;
}

int
KMessageBox::warningYesNoCancel(QWidget *parent, const QString &text,
                                const QString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const QString &dontAskAgainName,
                                Options options)
{
   return warningYesNoCancelList(parent, text, QStringList(), caption,
                      buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoCancelWId(WId parent_id, const QString &text,
                                const QString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const QString &dontAskAgainName,
                                Options options)
{
   return warningYesNoCancelListWId(parent_id, text, QStringList(), caption,
                      buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoCancelList(QWidget *parent, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    return warningYesNoCancelListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::warningYesNoCancelListWId(WId parent_id, const QString &text,
                                    const QStringList &strlist,
                                    const QString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const QString &dontAskAgainName,
                                    Options options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Warning") : caption,
      KDialog::Yes | KDialog::No | KDialog::Cancel, Qt::Dialog);
    dialog->setObjectName( "warningYesNoCancel" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
    dialog->setButtonGuiItem( KDialog::No, buttonNo );
    dialog->setDefaultButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, QMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                       &checkboxResult, options);
    if ( result==KDialog::Cancel ) return Cancel;
    res = (result==KDialog::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

void
KMessageBox::error(QWidget *parent,  const QString &text,
                   const QString &caption, Options options)
{
    return errorListWId( parent ? parent->winId() : 0, text, QStringList(), caption, options );
}

void
KMessageBox::errorWId(WId parent_id, const QString &text,
                      const QString &caption, Options options)
{
    errorListWId( parent_id, text, QStringList(), caption, options );
}

void
KMessageBox::errorList(QWidget *parent, const QString &text, const QStringList &strlist,
                       const QString &caption, Options options)
{
    return errorListWId( parent ? parent->winId() : 0, text, strlist, caption, options );
}

void
KMessageBox::errorListWId(WId parent_id,  const QString &text, const QStringList &strlist,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Error") : caption,
      KDialog::Yes, Qt::Dialog);
    dialog->setObjectName( "error" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStdGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, QMessageBox::Critical, text, strlist, QString(), 0, options);
}

void
KMessageBox::detailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    return detailedErrorWId( parent ? parent->winId() : 0, text, details, caption, options );
}

void
KMessageBox::detailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Error") : caption,
      KDialog::Yes | KDialog::Details, Qt::Dialog);
    dialog->setObjectName( "error" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStdGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, QMessageBox::Critical, text, QStringList(), QString(), 0, options, details);
}

void
KMessageBox::queuedDetailedError(QWidget *parent,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
    return queuedDetailedErrorWId( parent ? parent->winId() : 0, text, details, caption );
}

void
KMessageBox::queuedDetailedErrorWId(WId parent_id,  const QString &text,
                   const QString &details,
                   const QString &caption)
{
   KMessageBox_queue = true;
   (void) detailedErrorWId(parent_id, text, details, caption);
   KMessageBox_queue = false;
}


void
KMessageBox::sorry(QWidget *parent, const QString &text,
                   const QString &caption, Options options)
{
    return sorryWId( parent ? parent->winId() : 0, text, caption, options );
}

void
KMessageBox::sorryWId(WId parent_id, const QString &text,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Sorry") : caption,
      KDialog::Yes, Qt::Dialog);
    dialog->setObjectName( "sorry" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStdGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString(), 0, options);
}

void
KMessageBox::detailedSorry(QWidget *parent, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    return detailedSorryWId( parent ? parent->winId() : 0, text, details, caption, options );
}

void
KMessageBox::detailedSorryWId(WId parent_id, const QString &text,
                   const QString &details,
                   const QString &caption, Options options)
{
    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Sorry") : caption,
      KDialog::Yes | KDialog::Details, Qt::Dialog);
    dialog->setObjectName( "sorry" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStdGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, QMessageBox::Warning, text, QStringList(), QString(), 0, options, details);
}

void
KMessageBox::information(QWidget *parent,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, Options options)
{
  informationList(parent, text, QStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationWId(WId parent_id,const QString &text,
			 const QString &caption, const QString &dontShowAgainName, Options options)
{
  informationListWId(parent_id, text, QStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationList(QWidget *parent,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    return informationListWId( parent ? parent->winId() : 0, text, strlist, caption,
        dontShowAgainName, options );
}

void
KMessageBox::informationListWId(WId parent_id,const QString &text, const QStringList & strlist,
                         const QString &caption, const QString &dontShowAgainName, Options options)
{
    if ( !shouldBeShownContinue(dontShowAgainName) )
        return;

    QWidget* parent = QWidget::find( parent_id );
    KDialog *dialog= new KDialog(parent, caption.isEmpty() ? i18n("Information") : caption,
      KDialog::Yes, Qt::Dialog);
    dialog->setObjectName( "information" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStdGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef Q_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( QX11Info::display(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;

    createKMessageBox(dialog, QMessageBox::Information, text, strlist,
		dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                &checkboxResult, options);

    if (checkboxResult)
        saveDontShowAgainContinue(dontShowAgainName);
}

void
KMessageBox::enableAllMessages()
{
   KConfig *config = againConfig ? againConfig : KGlobal::config();
   QString grpNotifMsgs = QLatin1String("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   QString oldgroup = config->group();
   config->setGroup( grpNotifMsgs );

   typedef QMap<QString, QString> configMap;

   configMap map = config->entryMap(grpNotifMsgs);

   configMap::Iterator it;
   for (it = map.begin(); it != map.end(); ++it)
      config->deleteEntry( it.key() );
   config->sync();
   config->setGroup( oldgroup );
}

void
KMessageBox::enableMessage(const QString &dontShowAgainName)
{
   KConfig *config = againConfig ? againConfig : KGlobal::config();
   QString grpNotifMsgs = QLatin1String("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   KConfigGroup cg( config, grpNotifMsgs );

   cg.deleteEntry(dontShowAgainName);
   config->sync();
}

void
KMessageBox::about(QWidget *parent, const QString &text,
                   const QString &caption, Options options)
{
    QString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("About %1", kapp->caption());

    KDialog *dialog= new KDialog(parent, caption,
      KDialog::Yes, Qt::Dialog);
    dialog->setObjectName( "about" );
    dialog->setModal( true );
    dialog->enableButtonSeparator( true );
    dialog->setButtonText( KDialog::Yes, KStdGuiItem::ok().text() );
    dialog->setDefaultButton( KDialog::Yes );
    dialog->setEscapeButton( KDialog::Yes );
    if (qApp->windowIcon().isNull()){
        QPixmap ret = QMessageBox::standardIcon(QMessageBox::Information);
        dialog->setWindowIcon(ret);
    }
  
    int size = IconSize(K3Icon::Desktop);
    QPixmap icon = qApp->windowIcon().pixmap(size,size);
    createKMessageBox(dialog, icon, text, QStringList(), QString(), 0, options);
    return;
}

int KMessageBox::messageBox( QWidget *parent, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const QString &dontShowAskAgainName,
                             Options options )
{
    return messageBoxWId( parent ? parent->winId() : 0, type, text, caption,
        buttonYes, buttonNo, dontShowAskAgainName, options );
}

int KMessageBox::messageBox( QWidget *parent, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, Options options )
{
    return messageBoxWId( parent ? parent->winId() : 0, type, text, caption,
        buttonYes, buttonNo, QString(), options );
}

int KMessageBox::messageBoxWId( WId parent_id, DialogType type, const QString &text,
                             const QString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const QString &dontShow,
                             Options options )
{
    switch (type) {
        case QuestionYesNo:
            return KMessageBox::questionYesNoWId( parent_id,
                                               text, caption, buttonYes, buttonNo, dontShow, options );
        case QuestionYesNoCancel:
            return KMessageBox::questionYesNoCancelWId( parent_id,
                                               text, caption, buttonYes, buttonNo, dontShow, options );
        case WarningYesNo:
            return KMessageBox::warningYesNoWId( parent_id,
                                              text, caption, buttonYes, buttonNo, dontShow, options );
        case WarningContinueCancel:
            return KMessageBox::warningContinueCancelWId( parent_id,
                                              text, caption, buttonYes.text(), dontShow, options );
        case WarningYesNoCancel:
            return KMessageBox::warningYesNoCancelWId( parent_id,
                                              text, caption, buttonYes, buttonNo, dontShow, options );
        case Information:
            KMessageBox::informationWId( parent_id,
                                      text, caption, dontShow, options );
            return KMessageBox::Ok;

        case Error:
            KMessageBox::errorWId( parent_id, text, caption, options );
            return KMessageBox::Ok;

        case Sorry:
            KMessageBox::sorryWId( parent_id, text, caption, options );
            return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption, Options options )
{
    return queuedMessageBoxWId( parent ? parent->winId() : 0, type, text, caption, options );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption, Options options )
{
   KMessageBox_queue = true;
   (void) messageBoxWId(parent_id, type, text, caption, KStdGuiItem::yes(),
                     KStdGuiItem::no(), QString(), options);
   KMessageBox_queue = false;
}

void KMessageBox::queuedMessageBox( QWidget *parent, DialogType type, const QString &text, const QString &caption )
{
    return queuedMessageBoxWId( parent ? parent->winId() : 0, type, text, caption );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const QString &text, const QString &caption )
{
   KMessageBox_queue = true;
   (void) messageBoxWId(parent_id, type, text, caption);
   KMessageBox_queue = false;
}
