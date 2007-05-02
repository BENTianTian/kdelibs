/****************************************************************************
**
** Copyright (C) 2005-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QApplication>
#include <QtDesigner/QDesignerComponents>
#include <QtDesigner/abstractformeditor.h>

#include "mainwindow.h"
#include "numberedtextview.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(mdi);

    QApplication app(argc, argv);
    MainWindow mainWin;
    mainWin.show();

/*    NumberedTextView ntv;
    ntv.show()*/;

//     QDesignerComponents::initializeResources();
//     QWidget top;
//     QDesignerFormEditorInterface *formEditor = QDesignerComponents::createFormEditor(&top);
//     top.show();
//
//     QDesignerWidgetBoxInterface *widgetBox = QDesignerComponents::createWidgetBox(formEditor, 0);

    return app.exec();
}
