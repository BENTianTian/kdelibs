/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include "kmetaprops.h"

#include <kdebug.h>
#include <kfilemetainfowidget.h>
#include <kfilemetainfo.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kprotocolinfo.h>

#include <qvalidator.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qgroupbox.h>
#include <QResizeEvent>
#include <qlinkedlist.h>
#include <Q3ScrollView>
#include <QTextDocument>

#undef Bool

class MetaPropsScrollView : public Q3ScrollView
{
public:
    MetaPropsScrollView(QWidget* parent = 0, const char* name = 0)
        : Q3ScrollView(parent, name)
    {
      setFrameStyle(QFrame::NoFrame);
      m_frame = new QFrame(viewport());
	  m_frame->setObjectName("MetaPropsScrollView::m_frame");
      m_frame->setFrameStyle(QFrame::NoFrame);
      addChild(m_frame, 0, 0);
    };

    QFrame* frame() {return m_frame;};

protected:
    virtual void viewportResizeEvent(QResizeEvent* ev)
    {
      Q3ScrollView::viewportResizeEvent(ev);
      m_frame->resize( qMax(m_frame->sizeHint().width(), ev->size().width()),
                       qMax(m_frame->sizeHint().height(), ev->size().height()));
    };

private:
      QFrame* m_frame;
};

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate()  {}
    ~KFileMetaPropsPluginPrivate() {}

    QFrame*                       m_frame;
    QGridLayout*                  m_framelayout;
    KFileMetaInfo                 m_info;
//    QPushButton*                m_add;
    Q3PtrList<KFileMetaInfoWidget> m_editWidgets;
};

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropertiesDialogPlugin(props),d(new KFileMetaPropsPluginPrivate)
{

    KFileItem * fileitem = properties->item();
    kDebug(250) << "KFileMetaPropsPlugin constructor" << endl;

    d->m_info  = fileitem->metaInfo();
    if (!d->m_info.isValid())
    {
      d->m_info = KFileMetaInfo(properties->kurl().path(KUrl::RemoveTrailingSlash));
        fileitem->setMetaInfo(d->m_info);
    }

    if ( properties->items().count() > 1 )
    {
        // not yet supported
        // we should allow setting values for a list of files. Itt makes sense
        // in some cases, like the album of a list of mp3s
        return;
    }

    createLayout();

    setDirty(true);
}

void KFileMetaPropsPlugin::createLayout()
{
    QFileInfo file_info(properties->item()->url().path());

    kDebug(250) << "KFileMetaPropsPlugin::createLayout" << endl;

    // is there any valid and non-empty info at all?
    if ( !d->m_info.isValid() || (d->m_info.preferredKeys()).isEmpty() )
        return;

    // now get a list of groups
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    QStringList groupList = d->m_info.preferredGroups();

    const KFileMimeTypeInfo* mtinfo = prov->mimeTypeInfo(d->m_info.mimeType());
    if (!mtinfo)
    {
        kDebug(7034) << "no mimetype info there\n";
        return;
    }

    // let the dialog create the page frame
    QFrame* topframe = new QFrame();
    properties->addPage(topframe, i18n("&Meta Info"));
    topframe->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout* tmp = new QVBoxLayout(topframe);
    tmp->setMargin(0);

    // create a scroll view in the page
    MetaPropsScrollView* view = new MetaPropsScrollView(topframe);

    tmp->addWidget(view);

    d->m_frame = view->frame();

    QVBoxLayout *toplayout = new QVBoxLayout(d->m_frame);
    toplayout->setSpacing(KDialog::spacingHint());

    for (QStringList::Iterator git=groupList.begin();
            git!=groupList.end(); ++git)
    {
        kDebug(7033) << *git << endl;

        QStringList itemList = d->m_info.group(*git).preferredKeys();
        if (itemList.isEmpty())
            continue;

        QGroupBox *groupBox = new QGroupBox(
            Qt::escape(mtinfo->groupInfo(*git)->translatedName()),
            d->m_frame);
        QGridLayout *grouplayout = new QGridLayout(groupBox);
        grouplayout->activate();

        toplayout->addWidget(groupBox);

        QLinkedList<KFileMetaInfoItem> readItems;
        QLinkedList<KFileMetaInfoItem> editItems;

        for (QStringList::Iterator iit = itemList.begin();
                iit!=itemList.end(); ++iit)
        {
            KFileMetaInfoItem item = d->m_info[*git][*iit];
            if ( !item.isValid() ) continue;

            bool editable = file_info.isWritable() && item.isEditable();

            if (editable)
                editItems.append( item );
            else
                readItems.append( item );
        }

        KFileMetaInfoWidget* w = 0L;
        int row = 0;
        // then first add the editable items to the layout
        for (QLinkedList<KFileMetaInfoItem>::Iterator iit= editItems.begin();
                iit!=editItems.end(); ++iit)
        {
            QLabel* l = new QLabel((*iit).translatedKey() + ':', groupBox);
            grouplayout->addWidget(l, row, 0);
            l->setAlignment( Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs );
            QValidator* val = mtinfo->createValidator(*git, (*iit).key());
            if (!val) kDebug(7033) << "didn't get a validator for " << *git << "/" << (*iit).key() << endl;
            w = new KFileMetaInfoWidget(*iit, val, groupBox);
            grouplayout->addWidget(w, row, 1);
            d->m_editWidgets.append( w );
            connect(w, SIGNAL(valueChanged(const QVariant&)), this, SIGNAL(changed()));
            ++row;
        }

        // and then the read only items
        for (QLinkedList<KFileMetaInfoItem>::Iterator iit= readItems.begin();
                iit!=readItems.end(); ++iit)
        {
            QLabel* l = new QLabel((*iit).translatedKey() + ':', groupBox);
            grouplayout->addWidget(l, row, 0);
            l->setAlignment( Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs );
            w = new KFileMetaInfoWidget(*iit, KFileMetaInfoWidget::ReadOnly, 0L, groupBox);
            grouplayout->addWidget(w, row, 1);
            ++row;
        }
    }

    toplayout->addStretch(1);

    // the add key (disabled until fully implemented)
/*    d->m_add = new QPushButton(i18n("&Add"), topframe);
    d->m_add->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Fixed));
    connect(d->m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    tmp->addWidget(d->m_add);

    // if nothing can be added, deactivate it
    if ( !d->m_info.supportsVariableKeys() )
    {
        // if supportedKeys() does contain anything not in preferredKeys,
        // we have something addable

        QStringList sk = d->m_info.supportedKeys();
        d->m_add->setEnabled(false);
        for (QStringList::Iterator it = sk.begin(); it!=sk.end(); ++it)
        {
                if ( l.find(*it)==l.end() )
                {
                    d->m_add->setEnabled(true);
                    kDebug(250) << "**first addable key is " << (*it).toLatin1().constData() << "**" <<endl;
                    break;
                }
                kDebug(250) << "**already existing key is " << (*it).toLatin1().constData() << "**" <<endl;
        }
    } */
}

/*void KFileMetaPropsPlugin::slotAdd()
{
    // add a lineedit for the name



    // insert the item in the list

}*/

KFileMetaPropsPlugin::~KFileMetaPropsPlugin()
{
  delete d;
}

bool KFileMetaPropsPlugin::supports( const KFileItemList& _items )
{
#ifdef _GNUC
#warning TODO: Add support for more than one item
#endif
  // TODO check that KDesktopPropsPlugin is correct, i.e. that we never want metainfo for
  // a .desktop file? Used to be that only Application desktop files were filtered out
  if (KDesktopPropsPlugin::supports(_items) || KUrlPropsPlugin::supports(_items))
     return false; // Having both is redundant.

  bool metaDataEnabled = KGlobalSettings::showFilePreview(_items.first()->url());
  return _items.count() == 1 && metaDataEnabled;
}

void KFileMetaPropsPlugin::applyChanges()
{
  kDebug(250) << "applying changes" << endl;
  // insert the fields that changed into the info object

  Q3PtrListIterator<KFileMetaInfoWidget> it( d->m_editWidgets );
  KFileMetaInfoWidget* w;
  for (; (w = it.current()); ++it) w->apply();
  d->m_info.applyChanges(properties->kurl().path());
}

#include "kmetaprops.moc"
