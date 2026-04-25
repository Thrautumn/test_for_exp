#include "mainwindow.h"
#include <QApplication>
#include <QFont> // 必须包含这个头文件

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // --- 关键点：为了全汉化显示分数 ---
    // 实验手册中提到的实验九/十通常要求使用 unifont 字库
    // 这一步是设置全局字体，让界面能正常显示中文
    QFont font("unifont", 12); 
    a.setFont(font);
    // ----------------------------

    MainWindow w;

    // 这一步是让窗口全屏显示，在实验箱的小屏幕上效果更好
    w.showFullScreen(); 

    return a.exec();
}