#include "mainwindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 全汉化设置
    QFont font("unifont", 12); 
    a.setFont(font);

    // <--- 修改：实例化 AttendanceSystem --->
    AttendanceSystem w; 
    w.showFullScreen(); 

    return a.exec();
}