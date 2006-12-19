#ifndef KJSERRORDLG_H
#define KJSERRORDLG_H

#include <QDialog>

#include "ui_kjserrordlgbase.h"

class KJSErrorDlg : public QDialog, public Ui_KJSErrorDlgBase
{
  Q_OBJECT
  public:
    KJSErrorDlg( QWidget *parent = 0 );

    void addError( const QString &error );
    void setURL( const QString &url );
  protected slots:
    void clear();
  private:
    void init();
};

#endif
