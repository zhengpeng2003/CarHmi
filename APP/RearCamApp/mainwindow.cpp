#include "mainwindow.h"
#include "videowidget.h"
#include <QLabel>
#include <QPushButton>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

static QString buttonStyle()
{
    return QString(
        "QPushButton { background-color: #4a5a6a; color: #e0e0e0; border: 1px solid #7a8a9a; border-radius: 4px; font-size: 12px; padding: 4px; min-width: 60px; }"
        "QPushButton:checked { background-color: #3a6ea5; border-color: #8ab3d0; }"
        "QPushButton:pressed { background-color: #2a4a6a; }"
        );
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_server(nullptr)
    , m_clientSocket(nullptr)
    , m_senderProcess(nullptr)
    , m_frameSize(0)
    , m_frameCounter(0)
    , m_fps(0)
    , m_started(false)
    , m_introPlayed(false)
    , m_paused(false)
    , m_trackMode(true)
{
    setFixedSize(480, 272);
    setWindowTitle("倒车影像");
    setupUi();

    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &MainWindow::onNewConnection);

    QTimer *fpsTimer = new QTimer(this);
    fpsTimer->setInterval(1000);
    connect(fpsTimer, &QTimer::timeout, this, &MainWindow::updateFps);
    fpsTimer->start();

    m_senderProcess = new QProcess(this);
    connect(m_senderProcess, &QProcess::readyReadStandardOutput, this, [this](){
        qDebug() << m_senderProcess->readAllStandardOutput();
    });
    connect(m_senderProcess, &QProcess::readyReadStandardError, this, [this](){
        qDebug() << m_senderProcess->readAllStandardError();
    });
}

MainWindow::~MainWindow()
{
    stopSenderProcess();
}

void MainWindow::setupUi()
{
    m_central = new QWidget(this);
    m_central->setStyleSheet("background-color: #1a1e24;");
    setCentralWidget(m_central);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_central);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(6);

    // 顶部栏：左侧FPS，右侧关闭按钮
    QHBoxLayout *topBar = new QHBoxLayout();
    topBar->setContentsMargins(4, 0, 4, 0);
    m_fpsLabel = new QLabel("0 fps");
    m_fpsLabel->setStyleSheet("color: #c0c0c0; background-color: #2a2e33; padding: 2px 6px; border-radius: 3px; font-size: 11px;");
    m_fpsLabel->setFixedHeight(22);
    topBar->addWidget(m_fpsLabel);
    topBar->addStretch();
    m_closeBtn = new QPushButton("✕");
    m_closeBtn->setFixedSize(24, 22);
    m_closeBtn->setStyleSheet("QPushButton { background-color: #6a4a4a; color: white; border: none; border-radius: 3px; font-size: 14px; } QPushButton:hover { background-color: #8b5a5a; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    topBar->addWidget(m_closeBtn);
    mainLayout->addLayout(topBar);

    // 视频区域
    m_videoWidget = new VideoWidget(m_central);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(m_videoWidget, 1);

    // 底部按钮栏
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(16);
    btnLayout->addStretch();

    m_btnPause = new QPushButton("暂停");
    m_btnTrack = new QPushButton("轨迹");
    m_btnShot = new QPushButton("拍照");

    m_btnPause->setCheckable(true);
    m_btnTrack->setCheckable(true);
    m_btnTrack->setChecked(true);

    for (auto btn : {m_btnPause, m_btnTrack, m_btnShot}) {
        btn->setStyleSheet(buttonStyle());
        btn->setFixedSize(70, 28);
        btnLayout->addWidget(btn);
    }
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(m_btnPause, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(m_btnTrack, &QPushButton::clicked, this, &MainWindow::onTrackClicked);
    connect(m_btnShot, &QPushButton::clicked, this, &MainWindow::onShotClicked);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_introPlayed) {
        m_introPlayed = true;
    }
    if (!m_started) {
        m_started = true;
        startLocalServer();
        QTimer::singleShot(500, this, &MainWindow::startSenderProcess);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    stopSenderProcess();
    if (m_clientSocket) {
        m_clientSocket->disconnectFromServer();
        m_clientSocket->deleteLater();
    }
    if (m_server && m_server->isListening()) {
        m_server->close();
    }
    QFile::remove("/tmp/rearcam_ipc.sock");
    QMainWindow::closeEvent(event);
}

void MainWindow::startLocalServer()
{
    const QString path = "/tmp/rearcam_ipc.sock";
    QFile::remove(path);
    if (!m_server->listen(path)) {
        qWarning() << "Server listen failed:" << m_server->errorString();
    }
}

void MainWindow::startSenderProcess()
{
    if (m_senderProcess->state() != QProcess::NotRunning) return;
    const QString prog = "/home/debian/rearcam_tcp_test/board_sender/build/camera_sender";
    if (!QFileInfo::exists(prog)) {
        qWarning() << "camera_sender not found";
        return;
    }
    m_senderProcess->start(prog);
    m_senderProcess->waitForStarted(2000);
}

void MainWindow::stopSenderProcess()
{
    if (m_senderProcess->state() == QProcess::NotRunning) return;
    m_senderProcess->terminate();
    m_senderProcess->waitForFinished(1500);
}

void MainWindow::onNewConnection()
{
    if (m_clientSocket) {
        if (auto *extra = m_server->nextPendingConnection()) {
            extra->disconnectFromServer();
            extra->deleteLater();
        }
        return;
    }
    m_clientSocket = m_server->nextPendingConnection();
    if (!m_clientSocket) return;
    connect(m_clientSocket, &QLocalSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    connect(m_clientSocket, &QLocalSocket::disconnected, this, &MainWindow::onClientDisconnected);
    resetFrameParser();
}

void MainWindow::onClientDisconnected()
{
    if (m_clientSocket) {
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    resetFrameParser();
}

void MainWindow::resetFrameParser()
{
    m_buffer.clear();
    m_frameSize = 0;
}

void MainWindow::onSocketReadyRead()
{
    if (!m_clientSocket) return;
    m_buffer.append(m_clientSocket->readAll());
    processData();
}

void MainWindow::processData()
{
    while (true) {
        if (m_frameSize == 0) {
            if (m_buffer.size() < 4) return;
            m_frameSize = (static_cast<uchar>(m_buffer[0]) << 24) |
                          (static_cast<uchar>(m_buffer[1]) << 16) |
                          (static_cast<uchar>(m_buffer[2]) << 8) |
                          (static_cast<uchar>(m_buffer[3]));
            m_buffer.remove(0, 4);
            if (m_frameSize == 0 || m_frameSize > 10*1024*1024) {
                resetFrameParser();
                return;
            }
        }
        if (m_buffer.size() < (int)m_frameSize) return;
        QByteArray jpeg = m_buffer.left(m_frameSize);
        m_buffer.remove(0, m_frameSize);
        m_frameSize = 0;

        QImage img;
        if (img.loadFromData(jpeg, "JPG")) {
            if (!m_paused) {
                m_videoWidget->setFrame(img);
                ++m_frameCounter;
            }
        }
    }
}

void MainWindow::updateFps()
{
    m_fps = m_frameCounter;
    m_frameCounter = 0;
    m_fpsLabel->setText(QString("%1 fps").arg(m_fps));
}

void MainWindow::onPauseClicked()
{
    m_paused = !m_paused;
    m_btnPause->setChecked(m_paused);
    m_videoWidget->setPaused(m_paused);
}

void MainWindow::onTrackClicked()
{
    m_trackMode = !m_trackMode;
    m_btnTrack->setChecked(m_trackMode);
    m_videoWidget->setShowGuideLine(m_trackMode);
}

void MainWindow::onShotClicked()
{
    QString dir = QDir::homePath() + "/RearCamShots";
    QDir().mkpath(dir);
    QString path = dir + "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png";
    if (m_videoWidget->grab().save(path))
        qDebug() << "截图保存:" << path;
    else
        qWarning() << "截图失败";
}

void MainWindow::onCloseClicked()
{
    close();
}
