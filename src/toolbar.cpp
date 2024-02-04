/*
 * BFA 程序设计结课作业
工程: Movere - A Lite And Modern Media Player
日期: 2024-01-29
作者: Akiko
环境: win10 QT6.6.1 MinGW 64-bit
功能: 该类用于创建控件窗口，实现播放器控制功能
*/

#include "toolbar.h"
#include "ui_toolbar.h"
#include "mainwindow.h"
#include "vlccontrol.h"

Toolbar::Toolbar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Toolbar)
{
    ui->setupUi(this);

    // 设置无边框透明样式
    setWindowFlags(Qt::FramelessWindowHint|Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置允许拖放
    setAcceptDrops(true);

    // 设置 Label
    ui->metadataLabel->setWordWrap(true);
    ui->metadataLabel->setOpenExternalLinks(true);
    ui->metadataLabel->hide();
    ui->infoLabel->hide();

    // 获取实例
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parent);
    Toolbar *toolbar = qobject_cast<Toolbar*>(parent);

    // 创建 vlccontrol 类
    vlccontrol *vlc = new vlccontrol(mainWindow, nullptr);

    // 初始化变量
    isUrl = false;
    isStartPlay = false;
    isPlaying = false;
    isLeaveWindow = false;
    isFullScreen = false;
    isShowMetadata = false;
    isShowMessage = false;

    // 初始化定时器
    m_timer.setInterval(2000);
    m_cursorTimer.setInterval(50);
    m_showMetadataTimer.setInterval(5000);
    m_showMessageTimer.setInterval(1000);

    // 连接定时器信号与槽
    connect(&m_timer, &QTimer::timeout, this, &Toolbar::checkMouseActivity);
    connect(&m_cursorTimer, &QTimer::timeout, this, &Toolbar::checkMouseActivityShowToolbar);
    connect(&m_showMetadataTimer, &QTimer::timeout, this, &Toolbar::hideMetadata);
    connect(&m_showMessageTimer, &QTimer::timeout, this, &Toolbar::hideMessage);

    // 连接信号与槽
    connect(this, &Toolbar::fileDropped, vlc, &vlccontrol::mediaDropFile);
    connect(this, &Toolbar::mediaPause, vlc, &vlccontrol::mediaPause);
    connect(this, &Toolbar::mediaPlay, vlc, &vlccontrol::mediaPlay);
    connect(this, &Toolbar::mediaMute, vlc, &vlccontrol::mediaMute);
    connect(this, &Toolbar::mediaUnmute, vlc, &vlccontrol::mediaUnmute);
    connect(this, &Toolbar::sendPosition, vlc, &vlccontrol::mediaChangePosition);

    // 连接控件
    connect(ui->openFileButton, SIGNAL(clicked()), vlc, SLOT(mediaSetFilePath()));
    connect(ui->openUrlButton, SIGNAL(clicked()), vlc, SLOT(mediaSetUrlPath()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));
    connect(ui->minimizeButton, SIGNAL(clicked()), this, SLOT(onMinimizeButtonClicked()));
    connect(ui->fullScreenButton, SIGNAL(clicked()), this, SLOT(onFullscreenButtonClicked()));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(onPlayButtonClicked()));
    connect(ui->muteButton, SIGNAL(clicked()), this, SLOT(onMuteButtonClicked()));
    connect(ui->slider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));

    // 创建并连接快捷键
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(onFullscreenButtonClicked()));
    QShortcut *spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(onPlayButtonClicked()));
    QShortcut *leftArrowShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this, SLOT(rewind5Seconds()));
    QShortcut *rightArrowShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this, SLOT(forward5Seconds()));
    QShortcut *upArrowShortcut = new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(increaseVolume()));
    QShortcut *downArrowShortcut = new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(decreaseVolume()));
    QShortcut *pShortcut = new QShortcut(QKeySequence(Qt::Key_P), this, SLOT(mediaSnapshot()));
    QShortcut *zShortcut = new QShortcut(QKeySequence(Qt::Key_Z), this, SLOT(slowPlayback()));
    QShortcut *xShortcut = new QShortcut(QKeySequence(Qt::Key_X), this, SLOT(fastPlayback()));
    QShortcut *iShortcut = new QShortcut(QKeySequence(Qt::Key_I), this, SLOT(iPressed()));

    // 连接快捷键
    connect(this, &Toolbar::mediaChangeSeconds, vlc, &vlccontrol::mediaChangeTime);
    connect(this, &Toolbar::mediaChangeVolume, vlc, &vlccontrol::mediaChangeVolume);
    connect(this, &Toolbar::mediaSendSnapshot, vlc, &vlccontrol::mediaSnapshot);
    connect(this, &Toolbar::mediaChangePlayback, vlc, &vlccontrol::mediaChangeSpeed);
    connect(this, &Toolbar::mediaGetMetadata, vlc, &vlccontrol::getMetadata);
}

Toolbar::~Toolbar()
{
    delete ui;

    m_timer.stop();
    m_cursorTimer.stop();
}

// 处理主窗口状态事件
bool Toolbar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize || event->type() == QEvent::Move || event->type() == QEvent::Paint)
    {
        // 设置子窗口的大小和位置
        this->setGeometry(parentWidget()->geometry());
    }

    // 将事件传递给父类
    return QWidget::eventFilter(obj, event);
}

// 鼠标按下事件
void Toolbar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 记录鼠标按下时的全局坐标和窗口位置
        m_lastPos = event->globalPos();
        isPressedWidget = true;
    }
}

// 鼠标移动事件
void Toolbar::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressedWidget) {
        if (event->buttons() & Qt::LeftButton) {
            if(!isFullScreen) {
                // 移动窗口位置
                parentWidget()->move(this->x() + (event->globalX() - m_lastPos.x()),
                                     this->y() + (event->globalY() - m_lastPos.y()));
            }
            m_lastPos = event->globalPos();
        }
    }
}

// 鼠标松开事件
void Toolbar::mouseReleaseEvent(QMouseEvent *event)
{
    m_lastPos = event->globalPos();
    isPressedWidget = false;
}

// 鼠标拖动事件
void Toolbar::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);

            if (isVideoFile(filePath)) {
                // 处理文件路径
                qDebug() << "Dropped file path:" << filePath;
                qDebug() << "File name:" << fileInfo.fileName();
                qDebug() << "File size:" << fileInfo.size();

                emit fileDropped(filePath);
            } else {
                showMessage("Unsupported file type");
            }
        }

        event->acceptProposedAction();
    }
}

// 鼠标拖动事件
void Toolbar::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

// 检查文件扩展名是否为视频文件
bool Toolbar::isVideoFile(const QString &filePath)
{
    QString fileExtension = QFileInfo(filePath).suffix().toLower();
    QStringList videoExtensions = {"mp4", "mov", "mkv"};
    return videoExtensions.contains(fileExtension);
}

// 当鼠标进入窗口时启动定时器
void Toolbar::enterEvent(QEnterEvent *event)
{
    if (Qt::LeftButton) {
        if (isPlaying) {
            m_timer.stop();
            isLeaveWindow = false;
        }
    }
}

// 当鼠标离开窗口时停止定时器
void Toolbar::leaveEvent(QEvent *event)
{
    if (isPlaying) {
        m_timer.start();
        isLeaveWindow = true;

        showCursor();
    }
}

// 定时器触发时检查鼠标位置
void Toolbar::checkMouseActivity() {
    // 定时器触发时检查鼠标位置
    QPoint currentPos = QCursor::pos();
    if (currentPos == m_lastPos) {
        // 如果鼠标位置未变，表示鼠标不动，可以隐藏 toolbar
        this->hide();
        m_cursorTimer.start();
        if (!isLeaveWindow) {
            this->hideCursor();
        }
    } else {
        // 如果鼠标位置有变化，更新最后的位置并显示 toolbar
        m_lastPos = currentPos;
        this->show();
        this->showCursor();
    }
}

// 检查鼠标是否活动
void Toolbar::checkMouseActivityShowToolbar() {
    QPoint currentPos = QCursor::pos();
    if (currentPos != m_lastPos) {
        this->show();
        m_cursorTimer.stop();
    }
}

// 隐藏鼠标光标
void Toolbar::hideCursor()
{
    this->setCursor(Qt::BlankCursor);
}

// 恢复显示鼠标光标
void Toolbar::showCursor()
{
    this->setCursor(Qt::ArrowCursor);
}

// 修改 startPlay 值
void Toolbar::setStartPlay(bool start)
{
    isStartPlay = start;
}

// 修改 isPlaying 值
void Toolbar::setIsPlaying(bool playing)
{
    isPlaying = playing;
}

// 关闭按钮点击时退出应用程序
void Toolbar::onCloseButtonClicked()
{
    QCoreApplication::quit();
}

// 最小化按钮点击时最小化窗口
void Toolbar::onMinimizeButtonClicked()
{
    showMinimized();
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parent());
    mainWindow->showMinimized();
}

// 全屏、窗口化
void Toolbar::onFullscreenButtonClicked()
{
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parent());
    if (isFullScreen) {
        // 设为窗口化
        mainWindow->showNormal();
        ui->fullScreenButton->setIcon(QIcon("icon/fullscreen.svg"));
        isFullScreen = false;
    } else {
        // 设为全屏
        mainWindow->showFullScreen();
        ui->fullScreenButton->setIcon(QIcon("icon/fullscreen-exit.svg"));

        isFullScreen = true;
    }
}

// 播放、暂停点击
void Toolbar::onPlayButtonClicked()
{
    if (isStartPlay && !isUrl) {
        updatePlayPause();
    }
}

// 播放、暂停点击
void Toolbar::onMainWindowClicked()
{
    if (isStartPlay && !isUrl) {
        updatePlayPause();
    }
}

// 播放、暂停
void Toolbar::updatePlayPause()
{
    if (isStartPlay && !isUrl) {
        if (isPlaying) {
            emit mediaPause();
            ui->playButton->setIcon(QIcon("icon/play.svg"));
        } else {
            emit mediaPlay();
            ui->playButton->setIcon(QIcon("icon/pause.svg"));
        }
    }
}

// 拖动进度条
void Toolbar::onSliderReleased()
{
    if (isStartPlay && !isUrl) {
        int sliderValue = ui->slider->value();
        float vlcPosition = static_cast<float>(sliderValue) / 1000.0;
        emit sendPosition(vlcPosition);
    }
}

// 设置 Slider Position Value
void Toolbar::setSliderPosition(float pos)
{
    if (!isPressedWidget && !isUrl) {
        ui->slider->setValue(pos);
    }
}

// 静音、取消
void Toolbar::onMuteButtonClicked()
{
    if (isStartPlay) {
        if (isMute) {
            isMute = false;
            emit mediaUnmute();
            ui->muteButton->setIcon(QIcon("icon/volume-high.svg"));
        } else {
            isMute = true;
            emit mediaMute();
            ui->muteButton->setIcon(QIcon("icon/volume-off.svg"));
        }
    }
}

// 后退五秒
void Toolbar::rewind5Seconds()
{
    if (isStartPlay && !isUrl) {
        emit mediaChangeSeconds(-5000);
    }
}

// 前进五秒
void Toolbar::forward5Seconds()
{
    if (isStartPlay && !isUrl) {
        emit mediaChangeSeconds(5000);
    }
}

// 音量提高
void Toolbar::increaseVolume()
{
    if (isStartPlay) {
        emit mediaChangeVolume(10);
    }
}

// 音量降低
void Toolbar::decreaseVolume()
{
    if (isStartPlay) {
        emit mediaChangeVolume(-10);
    }
}

// 视频截图
void Toolbar::mediaSnapshot()
{
    if (isStartPlay) {
        emit mediaSendSnapshot();
    }
}

// 播放速度降低
void Toolbar::slowPlayback()
{
    if (isStartPlay && !isUrl) {
        emit mediaChangePlayback(-0.25);
    }
}

// 播放速度提高
void Toolbar::fastPlayback()
{
    if (isStartPlay && !isUrl) {
        emit mediaChangePlayback(0.25);
    }
}

// I Pressed 显示视频 Metadata
void Toolbar::iPressed()
{
    if (isStartPlay && !isUrl) {
        emit mediaGetMetadata();
    }
}

// 显示视频 Metadata
void Toolbar::showMetadata(QString md)
{
    metadata = md;
    if (isShowMetadata) {
        m_showMetadataTimer.stop();
    } else {
        ui->metadataLabel->setText(metadata);
        ui->metadataLabel->show();
    }
    isShowMetadata = true;
    m_showMetadataTimer.start();
}

// 隐藏视频 Metadata
void Toolbar::hideMetadata()
{
    isShowMetadata = false;
    m_showMetadataTimer.stop();
    ui->metadataLabel->hide();
}

// 显示消息
void Toolbar::showMessage(QString msg)
{
    QString message = msg;
    if (isShowMessage) {
        m_showMessageTimer.stop();
    } else {
        ui->infoLabel->show();
    }
    ui->infoLabel->setText(msg);
    isShowMessage = true;
    m_showMessageTimer.start();
}

// 隐藏消息
void Toolbar::hideMessage()
{
    isShowMessage = false;
    m_showMessageTimer.stop();
    ui->infoLabel->hide();
}

// 设置是否为流媒体
void Toolbar::setIsUrl()
{
    isUrl = true;
}
