#ifndef __DLG_H__
#define __DLG_H__
#include <QDialog>
#include <ui_testled.h>
class MyDlg : public QDialog
{
     Q_OBJECT
     public:
         MyDlg();
 public slots:
         void ControlLed();
      private:
         Ui::Form ui;
};
#endif 
