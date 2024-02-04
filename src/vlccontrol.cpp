/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-02-02
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 该类用于连接主窗口、控件与 VLC，实现播放器内核功能
*/

#include "vlccontrol.h"

vlccontrol::vlccontrol(MainWindow *mainWindow, Toolbar *toolbar)
{
    pMainWindow = mainWindow;

    vlcVersionCheck();

    pos = 0.0;
    vol = 100;
    speed = 1.0;
    volBeforeMute = 100;
    isWebVideo = false;
    isPlaying = false;
}

vlccontrol::~vlccontrol()
{
    if (m_pInstance) {
        libvlc_media_player_stop(m_pPlayer);
        libvlc_media_release(m_pMedia);
        libvlc_release(m_pInstance);
        m_pInstance = NULL;
        m_pMedia = NULL;
        m_pPlayer = NULL;
    }

    // 解除注册
    detachEvents(vlcEvent, thread);
}

// 检查 vlc 版本信息
void vlccontrol::vlcVersionCheck()
{
    const char* libVLCVersion = libvlc_get_version();
    qDebug() << "Libvlc Version:" << libVLCVersion << Qt::endl;
    QString versionString(libVLCVersion);

    versionString = versionString.split(" ").value(0);

    QStringList versionParts = versionString.split(".");
    int majorVersion = versionParts.value(0).toInt();
    int minorVersion = versionParts.value(1).toInt();
    int patchVersion = versionParts.value(2).toInt();

    // 检查版本是否低于 3.0.20
    if (majorVersion < 3 || (majorVersion == 3 && minorVersion < 0) || (majorVersion == 3 && minorVersion == 0 && patchVersion < 20))
    {
        QMessageBox::critical(nullptr, "Error", "VLC version is too low. Please update to version 3.0.20 or higher.");
        exit(EXIT_FAILURE);
        QCoreApplication::quit();
    }
}

// 设置播放视频路径（拖动）
void vlccontrol::mediaDropFile(const QString &filePath)
{
    QString convFilePath = filePath;
    if(filePath.length() != 0)
    {
        convFilePath = QDir::toNativeSeparators(filePath);
        mediaStartPlay(convFilePath, pMainWindow, false);
    } else {
        // 传递信息
        sendMessage("Unsupported File");
    }
}

// 设置播放视频路径
void vlccontrol::mediaSetFilePath()
{
    QString filePath = QFileDialog::getOpenFileName(pMainWindow, "Open media file...", ".", "Media Files (*.mp4 *.mov *.mkv);;All Files (*)");
    if(filePath.length() != 0)
    {
        filePath = QDir::toNativeSeparators(filePath);
        qDebug()<<filePath.toUtf8().constData();

        mediaStartPlay(filePath, pMainWindow, false);
    } else {
        // 传递信息
        sendMessage("Unsupported File");
    }
}

// 设置播放流媒体路径
void vlccontrol::mediaSetUrlPath()
{
    bool ok;
    QString url = QInputDialog::getText(nullptr, "Enter URL", "Enter RTSP/RTMP URL:", QLineEdit::Normal, "", &ok);

    if (ok && !url.isEmpty()) {
        if (url.startsWith("rtsp://") || url.startsWith("rtmp://") || url.startsWith("screen://")) {
            mediaStartPlay(url, pMainWindow, true);
        }  else {
            // 传递信息
            sendMessage("Unsupported Link");
        }
    } else {
        // 传递信息
        sendMessage("Unsupported File");
    }

    sendIsUrl();
}

// 开始播放视频
void vlccontrol::mediaStartPlay(QString url, MainWindow *mainWindow, bool isWebUrl)
{
    // 储存是否为流媒体
    vlccontrol::isWebVideo = isWebUrl;

    // 创建
    try {
        m_pInstance = libvlc_new(0,NULL);
        if (isWebUrl) {
            m_pMedia = libvlc_media_new_location(m_pInstance, url.toUtf8().constData());
        } else {
            m_pMedia = libvlc_media_new_path(m_pInstance, url.toUtf8().constData());
        }
        m_pPlayer = libvlc_media_player_new_from_media(m_pMedia);

        // 播放
        libvlc_media_player_set_hwnd(m_pPlayer,(void*)pMainWindow->ui->videoWidget->winId());
        libvlc_media_player_play(m_pPlayer);

        // 设置播放状态
        pMainWindow->setStartPlay(true);
        pMainWindow->setIsPlaying(true);
        isPlaying = true;

        // 创建定时器
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &vlccontrol::updatePosition);
        timer->start(1000);  // 1000毫秒 = 1秒
    } catch (...) {
        QMessageBox::information(nullptr, "Failed", "Player Encounter Error, please check your file or link.");
        sendMessage("Encounter error");
    }

    // 隐藏图标
    pMainWindow->hideOpenWidget();

    // 注册事件
    libvlc_event_manager_t *vlcEvent = libvlc_media_player_event_manager(m_pPlayer);
    attachEvents(vlcEvent, thread);
}

// 继续播放
void vlccontrol::mediaPlay()
{
    // 设置播放状态
    pMainWindow->setIsPlaying(true);
    isPlaying = true;

    // 传递信息
    sendMessage("Play");

    libvlc_media_player_play(m_pPlayer);
}

// 暂停播放
void vlccontrol::mediaPause()
{
    // 设置播放状态
    pMainWindow->setIsPlaying(false);
    isPlaying = false;

    // 传递信息
    sendMessage("Pause");

    libvlc_media_player_pause(m_pPlayer);
}

// 停止播放
void vlccontrol::mediaStop()
{
    // 设置播放状态
    pMainWindow->setIsPlaying(false);
    isPlaying = false;

    // 显示图标
    pMainWindow->showOpenWidget();

    // 传递信息
    sendMessage("Stop");

    // 重启播放器
    QString program = QCoreApplication::applicationFilePath();
    QStringList arguments = QCoreApplication::arguments();
    QProcess::startDetached(program, arguments);
    QCoreApplication::quit();
}

// 改变播放进度 （百分比）
void vlccontrol::mediaChangePosition(float pos)
{
    int currentTime = mediaGetTime();

    libvlc_media_player_set_position(m_pPlayer, (float)pos);

    int changedTime = mediaGetTime();

    // 传递信息
    sendMessage("Move "+QString::number(changedTime-currentTime/1000)+" seconds");
}

// 改变播放进度 （毫秒）
void vlccontrol::mediaChangeTime(int msTime)
{
    int currentTime = mediaGetTime();
    int intDuration = getDuration();
    int setTime = mediaGetTime() + msTime;
    qDebug()<<"Current Time: "<<currentTime<<", Duration: "<<intDuration<<", Set: "<<setTime;
    if (setTime <= 0) {
        libvlc_media_player_set_time(m_pPlayer, 0);
    }
    else if (setTime >= intDuration) {
        libvlc_media_player_set_time(m_pPlayer, intDuration);
    }
    else {
        libvlc_media_player_set_time(m_pPlayer, setTime);
    }

    // 传递信息
    sendMessage("Move "+QString::number(msTime/1000)+" seconds");
}

// 改变播放音量
void vlccontrol::mediaChangeVolume(int vol)
{
    int currentVol = mediaGetVolume();
    int setVol = currentVol + vol;
    qDebug()<<"Current Vol: "<<currentVol<<", Set: "<<setVol;
    if (setVol > 0 && setVol < 100) {
        libvlc_audio_set_volume(m_pPlayer, setVol);
    }

    // 传递信息
    sendMessage("Volume: "+QString::number(setVol)+"%");

    volBeforeMute = setVol;
}

// 静音
void vlccontrol::mediaMute()
{
    if (volBeforeMute != 0) {
        libvlc_audio_set_volume(m_pPlayer, 0);
    }

    // 传递信息
    sendMessage("Mute");
}

// 取消静音
void vlccontrol::mediaUnmute()
{
    if (volBeforeMute != 0) {
        libvlc_audio_set_volume(m_pPlayer, volBeforeMute);
    } else {
        libvlc_audio_set_volume(m_pPlayer, 100);
        volBeforeMute = 100;
    }

    // 传递信息
    sendMessage("Unmute");
}

// 改变播放速度
void vlccontrol::mediaChangeSpeed(float speed)
{
    float currentSpeed = mediaGetSpeed();
    float setSpeed = currentSpeed + speed;
    if (setSpeed > 0.25 && setSpeed < 4.0) {
        libvlc_media_player_set_rate(m_pPlayer, setSpeed);
    }
    qDebug()<<"Current Speed: "<<currentSpeed<<", Set: "<<setSpeed;

    // 传递信息
    sendMessage("Play speed: *"+QString::number(setSpeed));
}

// 获取当前播放进度 （百分比）
float vlccontrol::mediaGetPosition()
{
    pos = libvlc_media_player_get_position(m_pPlayer);
    return pos;
}

// 获取当前播放进度 (毫秒）
int vlccontrol::mediaGetTime()
{
    int m_time = libvlc_media_player_get_time(m_pPlayer);
    return m_time;
}

// 获取当前播放音量
int vlccontrol::mediaGetVolume()
{
    vol = libvlc_audio_get_volume(m_pPlayer);
    return vol;
}

// 获取当前播放速度
float vlccontrol::mediaGetSpeed()
{
    speed = libvlc_media_player_get_rate(m_pPlayer);
    return speed;
}

// 视频截图
QString vlccontrol::mediaSnapshot()
{
    // 设置相对路径下的 Snapshots 文件夹
    QString snapshotsFolderPath = "Snapshots";
    QDir snapshotsDir(snapshotsFolderPath);

    // 如果文件夹不存在，则创建
    if (!snapshotsDir.exists()) {
        snapshotsDir.mkpath(".");
    }

    // 获取系统时间作为文件名
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyyMMddhhmmsszzz");

    // 文件名
    QString fileName = "snapshot_" + formattedDateTime + ".png";

    // 完整的文件路径
    QString filePath = snapshotsDir.filePath(fileName);
    libvlc_video_take_snapshot(m_pPlayer, 0, filePath.toUtf8().constData(), getWidth(), getHeight());

    qDebug()<<"Snapshot taken, File path: "<<filePath;

    return filePath;
}

// 获取是否为流媒体
bool vlccontrol::mediaIsWebVideo()
{
    return isWebVideo;
}

// 获取标题
char vlccontrol::getTitle()
{
    const char *title = libvlc_media_get_meta(m_pMedia, libvlc_meta_Title);
    qDebug() << "Title: " << (title ? title : "N/A");
    return *title;
}

// 获取宽
int vlccontrol::getWidth()
{
    int width = libvlc_video_get_width(m_pPlayer);
    qDebug() << "Width: " << width;
    return width;
}

// 获取高
int vlccontrol::getHeight()
{
    int height = libvlc_video_get_height(m_pPlayer);
    qDebug() << " Height: " << height;
    return height;
}

// 获取时长
libvlc_time_t vlccontrol::getDuration()
{
    libvlc_time_t duration = libvlc_media_get_duration(m_pMedia);
    qDebug() << "Duration: " << duration << " milliseconds";
    return duration;
}

// 获取帧率
float vlccontrol::getFps()
{
    float fps = libvlc_media_player_get_fps(m_pPlayer);
    qDebug() << "Frame Rate: " << fps << " frames per second";
    return fps;
}

// 获取音频频道
int vlccontrol::getAudioChannel()
{
    int audioChannel = libvlc_audio_get_channel(m_pPlayer);
    qDebug() << "Audio Channel: " << audioChannel;
    return audioChannel;
}

// 获取视频元数据
void vlccontrol::getMetadata()
{
    QString title = QString(getTitle());
    QString width = QString::number(getWidth());
    QString height = QString::number(getHeight());
    QString duration = QString::number(getDuration());
    QString fps = QString::number(getFps());
    QString audioChannel = QString::number(getAudioChannel());

    QString metadata = "<font color='#FFD700'>Title: <font color='#FF4500'>" + title + "</font></font>" +
                        "<br><font color='#8A2BE2'>Width: <font color='#FF4500'>" + width + "</font></font>" +
                        "<font color='#8A2BE2'> Height: <font color='#FF4500'>" + height + "</font></font>" +
                        "<br><font color='#006400'>Duration: <font color='#FF4500'>" + duration + "</font></font>" +
                        "<font color='#006400'> FPS: <font color='#FF4500'>" + fps + "</font></font>" +
                        "<br><font color='#3232CD'> Audio Channel: <font color='#FF4500'>" + audioChannel + "</font></font>";

    pMainWindow->sendMetadata(metadata);
}

// 传递当前播放进度
void vlccontrol::updatePosition()
{
    // 调用mediaGetTime获取当前位置
    int intPosition = mediaGetTime() / 1000;
    QString currentPosition = QString::number(intPosition);

    // 将当前位置转换为HH:MM:SS格式
    int seconds = currentPosition.toInt();
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;
    QString formattedPosition = QString("%1:%2:%3")
                                    .arg(hours, 2, 10, QChar('0'))
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0'));

    qDebug()<<"当前位置"<<currentPosition<<"转换后"<<formattedPosition;

    // 将格式化后的位置设置为Toolbar UI的label text值
    pMainWindow->setPositionLabelText(formattedPosition);

    // 调用mediaGetPosition获取当前位置
    float floatPosition = mediaGetPosition() * 1000;

    // 将位置设置为Toolbar UI的Slider Value值
    pMainWindow->setSliderPosition(floatPosition);
}

// 传递总时长
void vlccontrol::updateDuration()
{
    // 调用mediaGetDuration获取当前位置
    int intDuration = getDuration() / 1000;
    QString currentDuration = QString::number(intDuration);

    // 将当前位置转换为HH:MM:SS格式
    int seconds = currentDuration.toInt();
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;
    QString formattedDuration = QString("%1:%2:%3")
                                    .arg(hours, 2, 10, QChar('0'))
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0'));

    qDebug()<<"当前位置"<<currentDuration<<"转换后"<<formattedDuration;

    // 将格式化后的位置设置为Toolbar UI的label text值
    pMainWindow->setDurationLabelText(formattedDuration);
}

// 重设窗口宽高
void vlccontrol::mediaResizeWindow(int width, int height)
{
    pMainWindow->resize(width, height);
}

// 传递操作信息
void vlccontrol::sendMessage(QString message)
{
    pMainWindow->sendMessage(message);
}

// 传递为流媒体信息
void vlccontrol::sendIsUrl()
{
    pMainWindow->sendIsUrl();
}

// 注册事件
void vlccontrol::attachEvents(libvlc_event_manager_t *vlcEvent, vlccontrol *thread)
{
    // 如需开发功能则在此解除相应注释
    // libvlc_event_attach(vlcEvent, libvlc_MediaPlayerLengthChanged, handleEvents, this);
    // libvlc_event_attach(vlcEvent, libvlc_MediaPlayerTimeChanged, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerAudioVolume, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerMuted, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerUnmuted, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerSnapshotTaken, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerPositionChanged, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaParsedChanged, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerOpening, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerPlaying, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerPaused, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerStopped, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerEndReached, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerEncounteredError, handleEvents, this);
    libvlc_event_attach(vlcEvent, libvlc_MediaPlayerVout, handleEvents, this);
}

// 解除注册
void vlccontrol::detachEvents(libvlc_event_manager_t *vlcEvent, vlccontrol *thread)
{
    // 如需开发功能则在此解除相应注释
    // libvlc_event_detach(vlcEvent, libvlc_MediaPlayerLengthChanged, handleEvents, this);
    // libvlc_event_detach(vlcEvent, libvlc_MediaPlayerTimeChanged, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerAudioVolume, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerMuted, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerUnmuted, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerSnapshotTaken, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerPositionChanged, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaParsedChanged, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerOpening, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerPlaying, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerPaused, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerStopped, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerEndReached, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerEncounteredError, handleEvents, this);
    libvlc_event_detach(vlcEvent, libvlc_MediaPlayerVout, handleEvents, this);
}

// 处理事件
void vlccontrol::handleEvents(const libvlc_event_t *event, void *data)
{
    vlccontrol *thread = (vlccontrol *)data;

    vlccontrol* instance = static_cast<vlccontrol*>(data);

    switch (event->type) {
        case libvlc_MediaPlayerLengthChanged: {
            qint64 duration = event->u.media_player_length_changed.new_length;
            QMetaObject::invokeMethod(thread, "receiveDuration", Q_ARG(qint64, duration));
        }
        break;

        case libvlc_MediaPlayerTimeChanged: {
            qint64 position = event->u.media_player_time_changed.new_time;
            QMetaObject::invokeMethod(thread, "receivePosition", Q_ARG(qint64, position));

            if (instance->getDuration() - position < 500) {
                QMetaObject::invokeMethod(thread, "setPosition", Q_ARG(qint64, 0));
                instance->mediaStop();
            }
        }
        break;

        case libvlc_MediaPlayerAudioVolume: {
            float volume = event->u.media_player_audio_volume.volume * 100;
            QMetaObject::invokeMethod(thread, "receiveVolume", Q_ARG(int, volume));
        }
        break;

        case libvlc_MediaPlayerMuted: {
            bool muted = true;
            QMetaObject::invokeMethod(thread, "receiveMuted", Q_ARG(bool, muted));
        }
        break;

        case libvlc_MediaPlayerUnmuted: {
            bool muted = false;
            QMetaObject::invokeMethod(thread, "receiveMuted", Q_ARG(bool, muted));
        }
        break;

        case libvlc_MediaPlayerSnapshotTaken: {
            QMetaObject::invokeMethod(thread, "snapFinsh");
        }
        break;

        case libvlc_MediaPlayerPositionChanged: {
            float position = event->u.media_player_position_changed.new_position;
            qDebug()<<"播放进度" + QString("进度: %1").arg(position);
        }
        break;

        case libvlc_MediaParsedChanged:
            qDebug()<<"媒体信息";
            break;

        case libvlc_MediaPlayerOpening:
            qDebug()<<"播放结束";
            break;

        case libvlc_MediaPlayerPlaying: {
            // 传递总时长
            instance->updateDuration();

            // 重设窗口宽高
            if (instance->getWidth()/2<1920 && instance->getHeight()/2<1080) {
                // 将窗口重设为视频的宽度和高度
                instance->mediaResizeWindow(instance->getWidth(), instance->getHeight());
            } else {
                // 将窗口重设为视频宽度和高度的一半
                instance->mediaResizeWindow(instance->getWidth()/2, instance->getHeight()/2);
            }
            qDebug()<<"播放开始";
        }
        break;

        case libvlc_MediaPlayerPaused:
            qDebug()<<"播放暂停";
            break;

        case libvlc_MediaPlayerStopped: {
            qDebug()<<"播放停止";
            instance->mediaStop();
        }
        break;

        case libvlc_MediaPlayerEndReached: {
            qDebug()<<"播放结束";
            instance->mediaStop();
        }
        break;

        case libvlc_MediaPlayerEncounteredError: {
            qDebug()<<"发生错误";
            instance->mediaStop();
            QMessageBox::information(nullptr, "Encounter error", "Program restarting...");
        }
        break;

        case libvlc_MediaPlayerVout: {

        }
        break;

        default:
            break;
    }
}
