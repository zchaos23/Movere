/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-01-29
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 该类用于创建主窗口
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QMouseEvent>

#include <vlc/vlc.h>
#include "toolbar.h"
#include "ui_toolbar.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Toolbar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Ui::MainWindow *ui;

public:
    void hideOpenWidget();
    void showOpenWidget();
    void setStartPlay(bool startPlay);
    void setIsPlaying(bool isPlaying);
    void setPositionLabelText(QString formattedPosition);
    void setDurationLabelText(QString formattedDuration);
    void setSliderPosition(float pos);
    void sendMetadata(QString metadata);
    void sendMessage(QString message);
    void sendIsUrl();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Toolbar *toolbar;
};

#endif // MAINWINDOW_H
