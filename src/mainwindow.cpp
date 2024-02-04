/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-01-29
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 该类用于创建主窗口
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口样式
    setWindowFlags(Qt::FramelessWindowHint|Qt::Window);
    setWindowTitle(QStringLiteral("Movere"));

    // 创建控件子窗口
    toolbar = new Toolbar(this);
    this->installEventFilter(toolbar);
    toolbar->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 鼠标按下事件
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    toolbar->onMainWindowClicked();
}

// 隐藏打开视频的控件
void MainWindow::hideOpenWidget()
{
    toolbar->ui->openFileWidget->hide();
    toolbar->ui->openFileIconLabel->hide();
}

// 显示打开视频的控件
void MainWindow::showOpenWidget()
{
    toolbar->ui->openFileWidget->show();
    toolbar->ui->openFileIconLabel->show();
}

// 设置初始化
void MainWindow::setStartPlay(bool startPlay)
{
    toolbar->ui->playButton->setIcon(QIcon("icon/pause.svg"));
    toolbar->setStartPlay(startPlay);
}

// 设置是否在播放
void MainWindow::setIsPlaying(bool isPlaying)
{
    toolbar->setIsPlaying(isPlaying);
}

// 设置 Toolbar 当前播放时长
void MainWindow::setPositionLabelText(QString formattedPosition)
{
    toolbar->ui->leftTimecode->setText(formattedPosition);
}

// 设置 Toolbar 总时长
void MainWindow::setDurationLabelText(QString formattedDuration)
{
    toolbar->ui->rightTimecode->setText(formattedDuration);
}

// 设置 Slider Position Value
void MainWindow::setSliderPosition(float pos)
{
    toolbar->setSliderPosition(pos);
}

// 传递 Metadata
void MainWindow::sendMetadata(QString metadata)
{
    toolbar->showMetadata(metadata);
}

// 传递信息
void MainWindow::sendMessage(QString message)
{
    toolbar->showMessage(message);
}

// 传递为流媒体信息
void MainWindow::sendIsUrl()
{
    toolbar->setIsUrl();
}
