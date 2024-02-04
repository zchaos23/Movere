/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-01-28
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 主函数
*/

#include "mainwindow.h"
#include "toolbar.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
