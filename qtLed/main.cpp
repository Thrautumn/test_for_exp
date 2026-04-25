#include <QApplication>
#include "myqtled.h"
int main(int argc, char **argv)
{        QApplication app(argc, argv);
        MyDlg *mydlg = new MyDlg;
		  mydlg->setFont(QFont("unifont",16,QFont::Normal));
        return mydlg->exec();
}
