/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kfile.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *           (C) 2000 Kurt Granroth <granroth@kde.org>
 *           (C) 1997 Christoph Neerfeld <chris@kde.org>
 *
 * This is free software; it comes under the GNU Library General 
 * Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __KIconDialog_h__
#define __KIconDialog_h__

#include <qstring.h>
#include <qstringlist.h>
#include <qpushbutton.h>

#include <kicontheme.h>
#include <kdialogbase.h>
#include <kiconview.h>

class QComboBox;
class QTimer;
class QKeyEvent;
class QRadioButton;
class KProgress;
class KIconLoader;

class KIconCanvasData;
class KIconDialogData;
class KIconButtonData;


/**
 * Icon canvas for KIconDialog.
 */
class KIconCanvas: public KIconView
{
    Q_OBJECT

public:
    KIconCanvas(QWidget *parent=0L, const char *name=0L);
    ~KIconCanvas();

    /** Load icons into the canvas. */
    void loadFiles(QStringList files);

    /** Returns the current icon. */
    QString getCurrent();

signals:
    /** Emitted when the current icon has changed. */
    void nameChanged(QString);

    void startLoading(int);
    void progress(int);
    void finished();

private slots:
    void slotLoadFiles();
    void slotCurrentChanged(QIconViewItem *item);

private:
    QStringList mFiles;
    QTimer *mpTimer;
    KIconLoader *mpLoader;
    KIconCanvasData *d;
};


/**
 * Dialog for interactive selection of icons. Use the function 
 * selectIcon() let the user select an icon.
 */
class KIconDialog: public KDialogBase
{
    Q_OBJECT

public:
    /** Construct the iconloader dialog. Uses the global iconloader. */
    KIconDialog(QWidget *parent=0L, const char *name=0L);

    /** Alternate constructor to use a different iconloader. */
    KIconDialog(KIconLoader *loader, QWidget *parent=0, 
	    const char *name=0);

    ~KIconDialog();

    /**
     * Pops up the dialog an lets the user select an icon.
     *
     * @param group The icon group this icon is intended for. Providing the
     * group shows the icons in the dialog with the same appearance as when
     * used outside the dialog.
     * @param context The initial icon context. Initially, the icons having
     * this context are shown in the dialog. The user can change this.
     * @param user Begin with the "user icons" instead of "system icons".
     * @return The name of the icon, suitable for loading with KIconLoader.
     */
    QString selectIcon(int group=KIcon::Desktop, int
	    context=KIcon::Application, bool user=false);
    
private slots:
    void slotButtonClicked(int);
    void slotContext(int);
    void slotStartLoading(int);
    void slotProgress(int);
    void slotFinished();

private:
    void init();
    void showIcons();

    int mGroup, mContext, mType;

    QStringList mFileList;
    QComboBox *mpCombo;
    QPushButton *mpBrowseBut;
    QRadioButton *mpRb1, *mpRb2;
    KProgress *mpProgress;
    KIconLoader *mpLoader;
    KIconCanvas *mpCanvas;
    KIconDialogData *d;
};


/**
 * A pushbutton for choosing an icon. Pressing on the button will open a
 * KIconDialog for the user to select an icon. The current icon will be 
 * displayed on the button.
 */
class KIconButton: public QPushButton
{
    Q_OBJECT

public:
    /** Creates a new KIconButton. Uses the global iconloader. */
    KIconButton(QWidget *parent=0L, const char *name=0L);

    /** Alternate constructor for use with a different iconloader. */
    KIconButton(KIconLoader *loader, QWidget *parent, const char *name=0L);

    ~KIconButton();

    /** Set the icon group and context. */
    void setIconType(int group, int context, bool user=false);

    /** Set the button's initial icon. */
    void setIcon(QString icon);

    /** Returns the selected icon name. */
    const QString icon() { return mIcon; }

signals:
    /** Emitted when the icon has changed. */
    void iconChanged(QString icon);

private slots:
    void slotChangeIcon();

private:
    bool mbUser;
    int mGroup, mContext;

    QString mIcon;
    KIconDialog *mpDialog;
    KIconLoader *mpLoader;
    KIconButtonData *d;
};


#endif // __KIconDialog_h__
