/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-01-29
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 该类用于创建控件窗口，实现播放器控制功能
*/

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QMimeData>
#include <QShortcut>
#include <QLabel>

#include "mainwindow.h"
#include "ui_mainwindow.h"

class vlccontrol;

namespace Ui {
class Toolbar;
}

class Toolbar : public QWidget
{
    Q_OBJECT

public:
    explicit Toolbar(QWidget *parent = nullptr);
    ~Toolbar();

public:
    Ui::Toolbar *ui;

    void setStartPlay(bool Play);
    void setIsPlaying(bool playing);
    void onMainWindowClicked();
    void setSliderPosition(float pos);
    void showMetadata(QString md);
    void showMessage(QString msg);
    void setIsUrl();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    bool isVideoFile(const QString &filePath);
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void fileDropped(const QString &filePath);
    void mediaPause();
    void mediaPlay();
    void mediaMute();
    void mediaUnmute();
    void sendPosition(float vlcPosition);
    void mediaChangeSeconds(int msTime);
    void mediaChangeVolume(int vol);
    void mediaSendSnapshot();
    void mediaChangePlayback(float speed);
    void mediaGetMetadata();

private slots:
    void checkMouseActivity();
    void checkMouseActivityShowToolbar();
    void hideCursor();
    void showCursor();
    void onCloseButtonClicked();
    void onMinimizeButtonClicked();
    void onFullscreenButtonClicked();
    void onPlayButtonClicked();
    void onMuteButtonClicked();
    void updatePlayPause();
    void onSliderReleased();
    void rewind5Seconds();
    void forward5Seconds();
    void increaseVolume();
    void decreaseVolume();
    void mediaSnapshot();
    void slowPlayback();
    void fastPlayback();
    void iPressed();
    void hideMetadata();
    void hideMessage();

private:
    bool    isUrl;
    bool    isFullScreen;
    bool    isPressedWidget;
    bool    isStartPlay;
    bool    isPlaying;
    bool    isMute;
    bool    isLeaveWindow;
    bool    isShowMetadata;
    bool    isShowMessage;
    const QMimeData *mimeData;
    QPoint          m_lastPos;
    QTimer          m_timer;
    QTimer          m_cursorTimer;
    QTimer          m_showMetadataTimer;
    QTimer          m_showMessageTimer;
    QString         metadata;
    vlccontrol      *vlc;
};

#endif // TOOLBAR_H
