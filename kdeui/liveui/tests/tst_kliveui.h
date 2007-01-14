#ifndef TST_KLIVEUI_H
#define TST_KLIVEUI_H

#include <qtest_kde.h>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include <kmainwindow.h>
#include "../kliveui.h"

class tst_GuiEditor : public QObject
{
    Q_OBJECT
public slots:
    void init();
    void cleanup();
private slots:
    void actionGroups();
//    void recursiveActionGroups();
    void menuDeletion();
    void changeExistingMenu();
    void implicitBeginMenuBar();
    void subComponents();
    void deleteActions();

private:
    KMainWindow *mw;
    QAction *testAction1;
    QAction *testAction2;
    QAction *testAction3;
};

#endif
