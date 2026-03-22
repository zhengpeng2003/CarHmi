#include "mainwindow.h"
#include "videowidget.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <QProcess>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QImage>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_central(nullptr),
    m_topBar(nullptr),
    m_labelGear(nullptr),
    m_labelSpeed(nullptr),
    m_labelFps(nullptr),
    m_labelTemp(nullptr),
    m_labelRec(nullptr),
    m_videoWidget(nullptr),
    m_radarBar(nullptr),
    m_leftRadar(nullptr),
    m_centerDistance(nullptr),
    m_rightRadar(nullptr),
    m_funcBar(nullptr),
    m_btnView(nullptr),
    m_btnNight(nullptr),
    m_btnTrack(nullptr),
    m_btnPause(nullptr),
    m_btnShot(nullptr),
    m_recordBar(nullptr),
    m_btnRecord(nullptr),
    m_btnStop(nullptr),
    m_btnPauseRecord(nullptr),
    m_btnFile(nullptr),
    m_btnSetting(nullptr),
    m_server(new QLocalServer(this)),
    m_clientSocket(nullptr),
    m_fpsTimer(new QTimer(this)),
    m_statusTimer(new QTimer(this)),
    m_senderProcess(new QProcess(this)),
    m_expectedFrameSize(0),
    m_frameCounter(0),
    m_currentFps(0),
    m_started(false),
    m_recording(false),
    m_paused(false),
    m_nightMode(false),
    m_trackMode(true),
    m_fakeDistance(1.2f)
{
    setFixedSize(480, 272);
    setWindowTitle("Rear Camera");
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    setupUi();

    connect(m_server, &QLocalServer::newConnection,
            this, &MainWindow::onNewLocalConnection);

    m_fpsTimer->setInterval(1000);
    connect(m_fpsTimer, &QTimer::timeout, this, &MainWindow::updateUiFps);
    m_fpsTimer->start();

    m_statusTimer->setInterval(800);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::simulateStatus);
    m_statusTimer->start();

    connect(m_senderProcess, &QProcess::readyReadStandardOutput, [this]() {
        qDebug().noquote() << m_senderProcess->readAllStandardOutput();
    });

    connect(m_senderProcess, &QProcess::readyReadStandardError, [this]() {
        qDebug().noquote() << m_senderProcess->readAllStandardError();
    });

    resetFrameParser();
}

MainWindow::~MainWindow()
{
    stopSenderProcess();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

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
        m_clientSocket = nullptr;
    }

    if (m_server->isListening()) {
        m_server->close();
    }

    QFile::remove("/tmp/rearcam_ipc.sock");

    QMainWindow::closeEvent(event);
}

void MainWindow::setupUi()
{
    m_central = new QWidget(this);
    m_central->setObjectName("centralBg");
    setCentralWidget(m_central);

    m_central->setStyleSheet(
        "#centralBg { background-color: #111111; }"
        );

    setupTopBar();
    setupVideoArea();
    setupRadarArea();
    setupBottomButtons();
    setupRecordButtons();
}

void MainWindow::setupTopBar()
{
    m_topBar = new QWidget(m_central);
    m_topBar->setGeometry(0, 0, 480, 22);
    m_topBar->setStyleSheet("background-color:#1b1b1b; color:white;");

    m_labelGear = new QLabel("[R]", m_topBar);
    m_labelGear->setGeometry(8, 2, 28, 18);
    m_labelGear->setStyleSheet("color:#ff4040; font-weight:bold;");

    m_labelSpeed = new QLabel("12km/h", m_topBar);
    m_labelSpeed->setGeometry(45, 2, 60, 18);

    m_labelFps = new QLabel("0FPS", m_topBar);
    m_labelFps->setGeometry(120, 2, 55, 18);

    m_labelTemp = new QLabel("42°C", m_topBar);
    m_labelTemp->setGeometry(185, 2, 45, 18);

    m_labelRec = new QLabel("🔴REC 00:23", m_topBar);
    m_labelRec->setGeometry(320, 2, 120, 18);
    m_labelRec->setStyleSheet("color:#ff4040; font-weight:bold;");
}

void MainWindow::setupVideoArea()
{
    m_videoWidget = new VideoWidget(m_central);
    m_videoWidget->move(80, 35);
    m_videoWidget->setDistanceText("1.2m");
}

void MainWindow::setupRadarArea()
{
    m_radarBar = new QWidget(m_central);
    m_radarBar->setGeometry(0, 205, 480, 20);
    m_radarBar->setStyleSheet("background-color:#151515; color:white;");

    m_leftRadar = new QLabel("●●●○○○", m_radarBar);
    m_leftRadar->setGeometry(28, 1, 90, 18);
    m_leftRadar->setAlignment(Qt::AlignCenter);
    m_leftRadar->setStyleSheet("color:#55ff55; font-weight:bold;");

    m_centerDistance = new QLabel("1.2m", m_radarBar);
    m_centerDistance->setGeometry(200, 1, 80, 18);
    m_centerDistance->setAlignment(Qt::AlignCenter);
    m_centerDistance->setStyleSheet("color:white; font-weight:bold;");

    m_rightRadar = new QLabel("○○○●●●", m_radarBar);
    m_rightRadar->setGeometry(360, 1, 90, 18);
    m_rightRadar->setAlignment(Qt::AlignCenter);
    m_rightRadar->setStyleSheet("color:#55ff55; font-weight:bold;");
}

void MainWindow::setupBottomButtons()
{
    m_funcBar = new QWidget(m_central);
    m_funcBar->setGeometry(0, 225, 480, 24);
    m_funcBar->setStyleSheet("background-color:#202020;");

    QString btnStyle =
        "QPushButton {"
        "background-color:#2b2b2b;"
        "color:white;"
        "border:1px solid #555;"
        "border-radius:3px;"
        "font-size:12px;"
        "}"
        "QPushButton:pressed {"
        "background-color:#3a3a3a;"
        "}";

    m_btnView = new QPushButton("[视角▼]", m_funcBar);
    m_btnNight = new QPushButton("[夜视]", m_funcBar);
    m_btnTrack = new QPushButton("[轨迹]", m_funcBar);
    m_btnPause = new QPushButton("[暂停]", m_funcBar);
    m_btnShot = new QPushButton("[截图]", m_funcBar);

    QList<QPushButton*> list = {m_btnView, m_btnNight, m_btnTrack, m_btnPause, m_btnShot};
    int x = 10;
    for (QPushButton *btn : list) {
        btn->setGeometry(x, 2, 84, 20);
        btn->setStyleSheet(btnStyle);
        x += 92;
    }

    connect(m_btnView, &QPushButton::clicked, this, &MainWindow::onViewClicked);
    connect(m_btnNight, &QPushButton::clicked, this, &MainWindow::onNightClicked);
    connect(m_btnTrack, &QPushButton::clicked, this, &MainWindow::onTrackClicked);
    connect(m_btnPause, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(m_btnShot, &QPushButton::clicked, this, &MainWindow::onShotClicked);
}

void MainWindow::setupRecordButtons()
{
    m_recordBar = new QWidget(m_central);
    m_recordBar->setGeometry(0, 249, 480, 23);
    m_recordBar->setStyleSheet("background-color:#111111;");

    QString btnStyle =
        "QPushButton {"
        "background-color:#2b2b2b;"
        "color:white;"
        "border:1px solid #555;"
        "border-radius:3px;"
        "font-size:11px;"
        "}"
        "QPushButton:pressed {"
        "background-color:#3a3a3a;"
        "}";

    m_btnRecord = new QPushButton("🔴录制", m_recordBar);
    m_btnStop = new QPushButton("[⏹停止]", m_recordBar);
    m_btnPauseRecord = new QPushButton("[⏸暂停]", m_recordBar);
    m_btnFile = new QPushButton("[📁文件]", m_recordBar);
    m_btnSetting = new QPushButton("[⚙设置]", m_recordBar);

    QList<QPushButton*> list = {m_btnRecord, m_btnStop, m_btnPauseRecord, m_btnFile, m_btnSetting};
    int x = 4;
    for (QPushButton *btn : list) {
        btn->setGeometry(x, 1, 92, 20);
        btn->setStyleSheet(btnStyle);
        x += 95;
    }

    connect(m_btnRecord, &QPushButton::clicked, this, &MainWindow::onRecordClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_btnPauseRecord, &QPushButton::clicked, this, &MainWindow::onPauseRecordClicked);
    connect(m_btnFile, &QPushButton::clicked, this, &MainWindow::onFileClicked);
    connect(m_btnSetting, &QPushButton::clicked, this, &MainWindow::onSettingClicked);
}

void MainWindow::startLocalServer()
{
    const QString sockPath = "/tmp/rearcam_ipc.sock";

    if (QFile::exists(sockPath)) {
        QFile::remove(sockPath);
    }

    bool ok = m_server->listen(sockPath);
    qDebug() << "listen result =" << ok;
    if (ok) {
        qDebug() << "listen ok:" << sockPath;
    } else {
        qWarning() << "listen failed:" << m_server->errorString();
    }
}

void MainWindow::startSenderProcess()
{
    if (m_senderProcess->state() != QProcess::NotRunning) {
        return;
    }

    QString program = "/home/debian/rearcam_tcp_test/board_sender/build/camera_sender";

    if (!QFileInfo::exists(program)) {
        qWarning() << "camera_sender not found:" << program;
        return;
    }

    QStringList args;
    m_senderProcess->setProgram(program);
    m_senderProcess->setArguments(args);
    m_senderProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_senderProcess->start();

    bool ok = m_senderProcess->waitForStarted(2000);
    qDebug() << "camera_sender started =" << ok;
}

void MainWindow::stopSenderProcess()
{
    if (m_senderProcess->state() == QProcess::NotRunning) {
        return;
    }

    m_senderProcess->terminate();
    if (!m_senderProcess->waitForFinished(1500)) {
        m_senderProcess->kill();
        m_senderProcess->waitForFinished(1000);
    }
}

void MainWindow::onNewLocalConnection()
{
    if (m_clientSocket) {
        QLocalSocket *extra = m_server->nextPendingConnection();
        if (extra) {
            extra->disconnectFromServer();
            extra->deleteLater();
        }
        return;
    }

    m_clientSocket = m_server->nextPendingConnection();
    if (!m_clientSocket) {
        return;
    }

    qDebug() << "client connected";

    connect(m_clientSocket, &QLocalSocket::readyRead,
            this, &MainWindow::onSocketReadyRead);
    connect(m_clientSocket, &QLocalSocket::disconnected,
            this, &MainWindow::onClientDisconnected);

    resetFrameParser();
}

void MainWindow::onClientDisconnected()
{
    qDebug() << "client disconnected";

    if (m_clientSocket) {
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }

    resetFrameParser();
}

void MainWindow::resetFrameParser()
{
    m_recvBuffer.clear();
    m_expectedFrameSize = 0;
}

void MainWindow::onSocketReadyRead()
{
    if (!m_clientSocket) {
        return;
    }

    m_recvBuffer.append(m_clientSocket->readAll());
    processIncomingData();
}

void MainWindow::processIncomingData()
{
    while (true) {
        if (m_expectedFrameSize == 0) {
            if (m_recvBuffer.size() < 4) {
                return;
            }

            uchar b0 = static_cast<uchar>(m_recvBuffer[0]);
            uchar b1 = static_cast<uchar>(m_recvBuffer[1]);
            uchar b2 = static_cast<uchar>(m_recvBuffer[2]);
            uchar b3 = static_cast<uchar>(m_recvBuffer[3]);

            m_expectedFrameSize =
                (static_cast<quint32>(b0) << 24) |
                (static_cast<quint32>(b1) << 16) |
                (static_cast<quint32>(b2) << 8)  |
                (static_cast<quint32>(b3));

            m_recvBuffer.remove(0, 4);

            if (m_expectedFrameSize == 0 || m_expectedFrameSize > 10 * 1024 * 1024) {
                qWarning() << "invalid frame size =" << m_expectedFrameSize;
                resetFrameParser();
                return;
            }
        }

        if (m_recvBuffer.size() < static_cast<int>(m_expectedFrameSize)) {
            return;
        }

        QByteArray jpegData = m_recvBuffer.left(m_expectedFrameSize);
        m_recvBuffer.remove(0, m_expectedFrameSize);
        m_expectedFrameSize = 0;

        QImage img;
        if (img.loadFromData(jpegData, "JPG") || img.loadFromData(jpegData, "JPEG")) {
            if (!m_paused) {
                m_videoWidget->setFrame(img);
                ++m_frameCounter;
            }
        } else {
            qWarning() << "jpeg decode failed";
        }
    }
}

void MainWindow::updateUiFps()
{
    m_currentFps = m_frameCounter;
    m_frameCounter = 0;
    m_labelFps->setText(QString("%1FPS").arg(m_currentFps));
}

void MainWindow::simulateStatus()
{
    static int sec = 23;
    ++sec;

    QString timeText = QString("🔴REC 00:%1").arg(sec % 60, 2, 10, QChar('0'));
    if (m_recording) {
        m_labelRec->setText(timeText);
        m_labelRec->setStyleSheet("color:#ff4040; font-weight:bold;");
    } else {
        m_labelRec->setText("REC OFF");
        m_labelRec->setStyleSheet("color:#999999; font-weight:bold;");
    }

    static bool dir = false;
    if (!dir) {
        m_fakeDistance -= 0.1f;
        if (m_fakeDistance <= 0.4f) dir = true;
    } else {
        m_fakeDistance += 0.1f;
        if (m_fakeDistance >= 1.8f) dir = false;
    }

    QString d = QString::number(m_fakeDistance, 'f', 1) + "m";
    m_centerDistance->setText(d);
    m_videoWidget->setDistanceText(d);
    updateRadarByDistance(m_fakeDistance);
}

void MainWindow::updateRadarByDistance(float meters)
{
    QString color = "#55ff55";
    QString left = "●●●○○○";
    QString right = "○○○●●●";

    if (meters <= 0.6f) {
        color = "#ff3030";
        left = "●●●●●●";
        right = "●●●●●●";
    } else if (meters <= 1.0f) {
        color = "#ffaa00";
        left = "●●●●○○";
        right = "○○●●●●";
    } else if (meters <= 1.5f) {
        color = "#55ff55";
        left = "●●●○○○";
        right = "○○○●●●";
    } else {
        color = "#888888";
        left = "●●○○○○";
        right = "○○○○●●";
    }

    m_leftRadar->setText(left);
    m_rightRadar->setText(right);
    m_leftRadar->setStyleSheet(QString("color:%1; font-weight:bold;").arg(color));
    m_rightRadar->setStyleSheet(QString("color:%1; font-weight:bold;").arg(color));
}

void MainWindow::setRecording(bool on)
{
    m_recording = on;
    if (on) {
        m_btnRecord->setStyleSheet(
            "QPushButton { background-color:#a00000; color:white; border:1px solid #ff6060; border-radius:3px; font-size:11px; }");
    } else {
        m_btnRecord->setStyleSheet(
            "QPushButton { background-color:#2b2b2b; color:white; border:1px solid #555; border-radius:3px; font-size:11px; }");
    }
}

void MainWindow::setPaused(bool on)
{
    m_paused = on;
    m_videoWidget->setPaused(on);
}

void MainWindow::onViewClicked()
{
    qDebug() << "[TODO] switch view";
}

void MainWindow::onNightClicked()
{
    m_nightMode = !m_nightMode;
    m_videoWidget->setNightMode(m_nightMode);
    qDebug() << "[TODO] night mode =" << m_nightMode;
}

void MainWindow::onTrackClicked()
{
    m_trackMode = !m_trackMode;
    m_videoWidget->setShowGuideLine(m_trackMode);
    qDebug() << "[TODO] track mode =" << m_trackMode;
}

void MainWindow::onPauseClicked()
{
    setPaused(!m_paused);
    qDebug() << "[TODO] preview paused =" << m_paused;
}

void MainWindow::onShotClicked()
{
    qDebug() << "[TODO] screenshot";
}

void MainWindow::onRecordClicked()
{
    setRecording(true);
    qDebug() << "[TODO] start record";
}

void MainWindow::onStopClicked()
{
    setRecording(false);
    qDebug() << "[TODO] stop record";
}

void MainWindow::onPauseRecordClicked()
{
    qDebug() << "[TODO] pause record";
}

void MainWindow::onFileClicked()
{
    qDebug() << "[TODO] open record files";
}

void MainWindow::onSettingClicked()
{
    qDebug() << "[TODO] open settings";
}
