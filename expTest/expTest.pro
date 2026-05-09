QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 如果你之后要用数据库，请取消下面这行的注释
# QT += sql

TARGET = expTest
TEMPLATE = app

# 检查你的文件名，必须与文件夹里的实际文件名对齐
SOURCES += main.cpp \
           mainwindow.cpp

HEADERS += mainwindow.h

FORMS   += mainwindow.ui