/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-02-02
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 该类用于连接主窗口、控件与 VLC，实现播放器内核功能
*/

#ifndef VLCCONTROL_H
#define VLCCONTROL_H

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QString>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <QFileDialog>
#include <QThread>
#include <QTimer>

#include <vlc/vlc.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "toolbar.h"
#include "ui_toolbar.h"

class vlccontrol : public QThread
{
    Q_OBJECT

public:
    vlccontrol(MainWindow *mainWindow, Toolbar *toolbar);
    ~vlccontrol();

public slots:
    void        mediaDropFile(const QString &filePath);
    void        mediaSetFilePath();
    void        mediaSetUrlPath();
    void        mediaStartPlay(QString url, MainWindow *mainWindow, bool isWebUrl);
    void        mediaPlay();
    void        mediaPause();
    void        mediaStop();
    void        mediaChangePosition(float pos);
    void        mediaChangeTime(int msTime);
    void        mediaChangeVolume(int vol);
    void        mediaMute();
    void        mediaUnmute();
    void        mediaChangeSpeed(float speed);
    float       mediaGetPosition();
    int         mediaGetTime();
    int         mediaGetVolume();
    float       mediaGetSpeed();
    QString     mediaSnapshot();
    bool        mediaIsWebVideo();
    void        mediaResizeWindow(int width, int height);
    void        sendMessage(QString message);

    char            getTitle();
    int             getWidth();
    int             getHeight();
    libvlc_time_t   getDuration();
    float           getFps();
    int             getAudioChannel();
    void            getMetadata();

private slots:
    void updatePosition();
    void updateDuration();

private:
    void vlcVersionCheck();
    void sendIsUrl();
    void attachEvents(libvlc_event_manager_t *vlcEvent, vlccontrol *thread);
    void detachEvents(libvlc_event_manager_t *vlcEvent, vlccontrol *thread);
    static void handleEvents(const libvlc_event_t *event, void *data);

private:
    float   pos;
    int     m_time;
    int     vol;
    int     volBeforeMute;
    float   speed;
    bool    isWebVideo;
    bool    isPlaying;

    const char      *title;
    int             width;
    int             height;
    libvlc_time_t   duration;
    float           fps;
    int             audioChannel;

    QTimer*         timer;
    vlccontrol      *thread;

    MainWindow              *pMainWindow;
    libvlc_instance_t       *m_pInstance;
    libvlc_media_t          *m_pMedia;
    libvlc_media_player_t   *m_pPlayer;
    libvlc_event_manager_t  *vlcEvent;
};

#endif // VLCCONTROL_H
