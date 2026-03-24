#include "mainwindow.h"
#include "videowidget.h"

#include <QColor>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEasingCurve>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QImage>
#include <QLabel>
#include <QList>
#include <QProcess>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShowEvent>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStringList>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>

namespace {

QString rgbaString(const QColor &color, int alpha)
{
    return QString("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(alpha);
}

QString panelStyle(const QColor &top, const QColor &bottom, const QColor &border)
{
    return QString(
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 %1, stop:1 %2);"
        "border: 1px solid %3;"
        "border-radius: 13px;")
        .arg(top.name(), bottom.name(), rgbaString(border, 188));
}

QString chipStyle(const QColor &top, const QColor &bottom, const QColor &border,
                  const QColor &textColor)
{
    return QString(
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 %1, stop:1 %2);"
        "border: 1px solid %3;"
        "border-radius: 10px;"
        "color: %4;"
        "padding: 0 8px;")
        .arg(top.name(), bottom.name(), rgbaString(border, 200), textColor.name());
}

QString buttonStyle(const QColor &accent)
{
    const QColor softBorder = accent.lighter(115);
    const QColor softFill = accent.darker(245);
    const QColor activeFill = accent.darker(170);
    const QColor activeGlow = accent.lighter(125);

    return QString(
        "QPushButton {"
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #273646, stop:1 #15202b);"
        "color: #eef7ff;"
        "border: 1px solid %1;"
        "border-radius: 10px;"
        "font-size: 11px;"
        "font-weight: 600;"
        "padding: 0 6px;"
        "}"
        "QPushButton:hover {"
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 %2, stop:1 #1c2a36);"
        "border: 1px solid %3;"
        "}"
        "QPushButton:pressed {"
        "background-color: #111923;"
        "}"
        "QPushButton:checked {"
        "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 %4, stop:1 %2);"
        "border: 1px solid %5;"
        "color: white;"
        "}"
        "QPushButton:disabled {"
        "background-color: #10171f;"
        "color: #6e7f8f;"
        "border: 1px solid rgba(103, 117, 129, 120);"
        "}")
        .arg(rgbaString(softBorder, 182),
             rgbaString(softFill, 210),
             rgbaString(accent, 236),
             rgbaString(activeFill, 236),
             rgbaString(activeGlow, 255));
}

void applySoftShadow(QWidget *widget, const QColor &color, int blur, int yOffset)
{
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setOffset(0, yOffset);
    shadow->setColor(color);
    widget->setGraphicsEffect(shadow);
}

void animatePanel(QObject *owner, QWidget *widget, const QPoint &offset,
                  int delayMs, int durationMs)
{
    const QPoint endPos = widget->pos();
    const QPoint startPos = endPos + offset;
    widget->move(startPos);

    auto *animation = new QPropertyAnimation(widget, "pos", owner);
    animation->setStartValue(startPos);
    animation->setEndValue(endPos);
    animation->setDuration(durationMs);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    QTimer::singleShot(delayMs, widget, [animation]() {
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

} // namespace

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
      m_introPlayed(false),
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

    auto *recPulse = new QVariantAnimation(this);
    recPulse->setDuration(1600);
    recPulse->setLoopCount(-1);
    recPulse->setKeyValueAt(0.0, 0.25);
    recPulse->setKeyValueAt(0.5, 1.0);
    recPulse->setKeyValueAt(1.0, 0.25);
    connect(recPulse, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const qreal level = value.toReal();
        const QColor border = m_recording ? QColor(255, 103, 86) : QColor(94, 114, 130);
        const QColor top = m_recording ? QColor(89, 22, 26) : QColor(30, 39, 47);
        const QColor bottom = m_recording ? QColor(40, 10, 14) : QColor(19, 26, 33);
        const QColor text = m_recording ? QColor(255, 236, 236) : QColor(160, 174, 186);

        m_labelRec->setStyleSheet(chipStyle(top, bottom, border, text));

        auto *shadow = qobject_cast<QGraphicsDropShadowEffect *>(m_labelRec->graphicsEffect());
        if (!shadow) {
            return;
        }

        if (m_recording) {
            shadow->setBlurRadius(16.0 + level * 12.0);
            shadow->setColor(QColor(border.red(), border.green(), border.blue(),
                                    static_cast<int>(100 + level * 90)));
        } else {
            shadow->setBlurRadius(0.0);
            shadow->setColor(QColor(0, 0, 0, 0));
        }
    });
    recPulse->start();

    resetFrameParser();
    updateRadarByDistance(m_fakeDistance);
}

MainWindow::~MainWindow()
{
    stopSenderProcess();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (!m_introPlayed) {
        m_introPlayed = true;
        playIntroAnimation();
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
        "#centralBg {"
        "background-color: qradialgradient(cx:0.16, cy:0.06, radius:1.15, fx:0.16, fy:0.06, "
        "stop:0 #1d3a50, stop:0.36 #0b1623, stop:1 #04080d);"
        "}");

    setupTopBar();
    setupVideoArea();
    setupRadarArea();
    setupBottomButtons();
    setupRecordButtons();

    applySoftShadow(m_topBar, QColor(0, 0, 0, 78), 18, 4);
    applySoftShadow(m_videoWidget, QColor(26, 118, 175, 88), 26, 9);
    applySoftShadow(m_radarBar, QColor(0, 0, 0, 62), 16, 5);
    applySoftShadow(m_funcBar, QColor(0, 0, 0, 54), 14, 4);
    applySoftShadow(m_recordBar, QColor(0, 0, 0, 42), 12, 3);

    setRecording(false);
    setPaused(false);
    m_btnTrack->setChecked(m_trackMode);
    m_btnNight->setChecked(m_nightMode);
}

void MainWindow::setupTopBar()
{
    m_topBar = new QWidget(m_central);
    m_topBar->setGeometry(10, 8, 460, 28);
    m_topBar->setStyleSheet(panelStyle(QColor(28, 43, 58), QColor(10, 17, 25), QColor(91, 114, 136)));

    m_labelGear = new QLabel("R", m_topBar);
    m_labelGear->setGeometry(10, 4, 34, 20);
    m_labelGear->setAlignment(Qt::AlignCenter);
    m_labelGear->setStyleSheet(chipStyle(QColor(120, 33, 35), QColor(65, 18, 22),
                                         QColor(255, 109, 90), QColor(255, 243, 243)));

    m_labelSpeed = new QLabel("12 KM/H", m_topBar);
    m_labelSpeed->setGeometry(50, 4, 82, 20);
    m_labelSpeed->setAlignment(Qt::AlignCenter);
    m_labelSpeed->setStyleSheet(chipStyle(QColor(64, 53, 19), QColor(34, 28, 12),
                                          QColor(255, 196, 92), QColor(255, 242, 214)));

    m_labelFps = new QLabel("0 FPS", m_topBar);
    m_labelFps->setGeometry(138, 4, 62, 20);
    m_labelFps->setAlignment(Qt::AlignCenter);
    m_labelFps->setStyleSheet(chipStyle(QColor(16, 65, 88), QColor(11, 34, 46),
                                        QColor(92, 213, 255), QColor(228, 248, 255)));

    m_labelTemp = new QLabel("42 C", m_topBar);
    m_labelTemp->setGeometry(206, 4, 56, 20);
    m_labelTemp->setAlignment(Qt::AlignCenter);
    m_labelTemp->setStyleSheet(chipStyle(QColor(42, 51, 60), QColor(20, 28, 36),
                                         QColor(146, 164, 180), QColor(224, 232, 238)));

    m_labelRec = new QLabel("REC OFF", m_topBar);
    m_labelRec->setGeometry(310, 3, 140, 22);
    m_labelRec->setAlignment(Qt::AlignCenter);
    m_labelRec->setStyleSheet(chipStyle(QColor(30, 39, 47), QColor(19, 26, 33),
                                        QColor(94, 114, 130), QColor(160, 174, 186)));

    auto *recGlow = new QGraphicsDropShadowEffect(m_labelRec);
    recGlow->setBlurRadius(0.0);
    recGlow->setOffset(0, 0);
    recGlow->setColor(QColor(0, 0, 0, 0));
    m_labelRec->setGraphicsEffect(recGlow);
}

void MainWindow::setupVideoArea()
{
    m_videoWidget = new VideoWidget(m_central);
    m_videoWidget->move(74, 36);
    m_videoWidget->setDistanceText("1.2m");
}

void MainWindow::setupRadarArea()
{
    m_radarBar = new QWidget(m_central);
    m_radarBar->setGeometry(18, 196, 444, 28);
    m_radarBar->setStyleSheet(panelStyle(QColor(18, 29, 40), QColor(10, 16, 23), QColor(83, 110, 134)));

    m_leftRadar = new QLabel("###---", m_radarBar);
    m_leftRadar->setGeometry(18, 4, 110, 20);
    m_leftRadar->setAlignment(Qt::AlignCenter);

    m_centerDistance = new QLabel("1.2m", m_radarBar);
    m_centerDistance->setGeometry(170, 3, 104, 22);
    m_centerDistance->setAlignment(Qt::AlignCenter);
    m_centerDistance->setStyleSheet(chipStyle(QColor(20, 39, 55), QColor(11, 22, 31),
                                              QColor(101, 205, 248), QColor(237, 247, 252)));

    m_rightRadar = new QLabel("---###", m_radarBar);
    m_rightRadar->setGeometry(316, 4, 110, 20);
    m_rightRadar->setAlignment(Qt::AlignCenter);
}

void MainWindow::setupBottomButtons()
{
    m_funcBar = new QWidget(m_central);
    m_funcBar->setGeometry(18, 226, 444, 22);
    m_funcBar->setStyleSheet(panelStyle(QColor(23, 35, 48), QColor(11, 18, 25), QColor(81, 105, 126)));

    m_btnView = new QPushButton("VIEW", m_funcBar);
    m_btnNight = new QPushButton("NIGHT", m_funcBar);
    m_btnTrack = new QPushButton("TRACK", m_funcBar);
    m_btnPause = new QPushButton("PAUSE", m_funcBar);
    m_btnShot = new QPushButton("SHOT", m_funcBar);

    m_btnNight->setCheckable(true);
    m_btnTrack->setCheckable(true);
    m_btnPause->setCheckable(true);

    QList<QPushButton *> buttons = {m_btnView, m_btnNight, m_btnTrack, m_btnPause, m_btnShot};
    QList<QColor> accents = {
        QColor(74, 203, 255),
        QColor(126, 232, 130),
        QColor(255, 196, 92),
        QColor(255, 139, 82),
        QColor(255, 104, 104)
    };

    int x = 10;
    for (int i = 0; i < buttons.size(); ++i) {
        QPushButton *button = buttons[i];
        button->setGeometry(x, 2, 80, 18);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(buttonStyle(accents[i]));
        x += 86;
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
    m_recordBar->setGeometry(18, 249, 444, 20);
    m_recordBar->setStyleSheet(panelStyle(QColor(16, 26, 36), QColor(8, 13, 18), QColor(70, 94, 115)));

    m_btnRecord = new QPushButton("REC", m_recordBar);
    m_btnStop = new QPushButton("STOP", m_recordBar);
    m_btnPauseRecord = new QPushButton("HOLD", m_recordBar);
    m_btnFile = new QPushButton("FILES", m_recordBar);
    m_btnSetting = new QPushButton("SET", m_recordBar);

    m_btnRecord->setCheckable(true);
    m_btnPauseRecord->setCheckable(true);

    QList<QPushButton *> buttons = {m_btnRecord, m_btnStop, m_btnPauseRecord, m_btnFile, m_btnSetting};
    QList<QColor> accents = {
        QColor(255, 100, 84),
        QColor(255, 181, 88),
        QColor(120, 219, 255),
        QColor(114, 228, 171),
        QColor(167, 194, 222)
    };

    int x = 10;
    for (int i = 0; i < buttons.size(); ++i) {
        QPushButton *button = buttons[i];
        button->setGeometry(x, 2, 80, 16);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(buttonStyle(accents[i]));
        x += 86;
    }

    connect(m_btnRecord, &QPushButton::clicked, this, &MainWindow::onRecordClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_btnPauseRecord, &QPushButton::clicked, this, &MainWindow::onPauseRecordClicked);
    connect(m_btnFile, &QPushButton::clicked, this, &MainWindow::onFileClicked);
    connect(m_btnSetting, &QPushButton::clicked, this, &MainWindow::onSettingClicked);
}

void MainWindow::playIntroAnimation()
{
    animatePanel(this, m_topBar, QPoint(0, -18), 0, 420);
    animatePanel(this, m_videoWidget, QPoint(-24, 10), 80, 560);
    animatePanel(this, m_radarBar, QPoint(0, 18), 170, 460);
    animatePanel(this, m_funcBar, QPoint(0, 24), 250, 420);
    animatePanel(this, m_recordBar, QPoint(0, 28), 330, 400);
}

void MainWindow::startLocalServer()
{
    const QString sockPath = "/tmp/rearcam_ipc.sock";

    if (QFile::exists(sockPath)) {
        QFile::remove(sockPath);
    }

    const bool ok = m_server->listen(sockPath);
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

    const QString program = "/home/debian/rearcam_tcp_test/board_sender/build/camera_sender";

    if (!QFileInfo::exists(program)) {
        qWarning() << "camera_sender not found:" << program;
        return;
    }

    m_senderProcess->setProgram(program);
    m_senderProcess->setArguments(QStringList());
    m_senderProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_senderProcess->start();

    const bool ok = m_senderProcess->waitForStarted(2000);
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

            const uchar b0 = static_cast<uchar>(m_recvBuffer[0]);
            const uchar b1 = static_cast<uchar>(m_recvBuffer[1]);
            const uchar b2 = static_cast<uchar>(m_recvBuffer[2]);
            const uchar b3 = static_cast<uchar>(m_recvBuffer[3]);

            m_expectedFrameSize =
                (static_cast<quint32>(b0) << 24) |
                (static_cast<quint32>(b1) << 16) |
                (static_cast<quint32>(b2) << 8) |
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

        const QByteArray jpegData = m_recvBuffer.left(m_expectedFrameSize);
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
    m_labelFps->setText(QString("%1 FPS").arg(m_currentFps));
}

void MainWindow::simulateStatus()
{
    static int sec = 23;
    ++sec;

    if (m_recording) {
        m_labelRec->setText(QString("REC 00:%1").arg(sec % 60, 2, 10, QChar('0')));
    } else {
        m_labelRec->setText("REC OFF");
    }

    const int speed = 5 + (sec % 4) + static_cast<int>(m_fakeDistance * 4.0f);
    const int temp = 41 + (sec % 3);
    m_labelSpeed->setText(QString("%1 KM/H").arg(speed));
    m_labelTemp->setText(QString("%1 C").arg(temp));

    static bool expand = false;
    if (!expand) {
        m_fakeDistance -= 0.1f;
        if (m_fakeDistance <= 0.4f) {
            expand = true;
        }
    } else {
        m_fakeDistance += 0.1f;
        if (m_fakeDistance >= 1.8f) {
            expand = false;
        }
    }

    const QString distanceText = QString::number(m_fakeDistance, 'f', 1) + "m";
    m_centerDistance->setText(distanceText);
    m_videoWidget->setDistanceText(distanceText);
    updateRadarByDistance(m_fakeDistance);
}

void MainWindow::updateRadarByDistance(float meters)
{
    QString color = "#6fd7ff";
    QString left = "###---";
    QString right = "---###";
    QColor chipBorder(101, 205, 248);
    QColor chipTop(20, 39, 55);
    QColor chipBottom(11, 22, 31);
    QColor chipText(237, 247, 252);

    if (meters <= 0.6f) {
        color = "#ff6457";
        left = "######";
        right = "######";
        chipBorder = QColor(255, 106, 88);
        chipTop = QColor(90, 22, 26);
        chipBottom = QColor(41, 10, 15);
        chipText = QColor(255, 238, 236);
    } else if (meters <= 1.0f) {
        color = "#ffbe5a";
        left = "####--";
        right = "--####";
        chipBorder = QColor(255, 196, 92);
        chipTop = QColor(73, 55, 20);
        chipBottom = QColor(36, 27, 11);
        chipText = QColor(255, 245, 216);
    } else if (meters <= 1.5f) {
        color = "#79e882";
        left = "###---";
        right = "---###";
        chipBorder = QColor(126, 232, 130);
        chipTop = QColor(21, 63, 31);
        chipBottom = QColor(9, 28, 14);
        chipText = QColor(235, 251, 236);
    } else {
        color = "#8ea1b4";
        left = "##----";
        right = "----##";
        chipBorder = QColor(128, 148, 168);
        chipTop = QColor(34, 42, 51);
        chipBottom = QColor(17, 22, 29);
        chipText = QColor(225, 232, 239);
    }

    const QString radarLabelStyle =
        QString("color: %1; font-size: 12px; font-weight: 700;").arg(color);
    m_leftRadar->setText(left);
    m_rightRadar->setText(right);
    m_leftRadar->setStyleSheet(radarLabelStyle);
    m_rightRadar->setStyleSheet(radarLabelStyle);
    m_centerDistance->setStyleSheet(chipStyle(chipTop, chipBottom, chipBorder, chipText));
}

void MainWindow::setRecording(bool on)
{
    m_recording = on;
    m_btnRecord->setChecked(on);
    m_btnStop->setEnabled(on);
    m_btnPauseRecord->setEnabled(on);

    if (!on) {
        m_btnPauseRecord->setChecked(false);
        m_btnPauseRecord->setText("HOLD");
    }
}

void MainWindow::setPaused(bool on)
{
    m_paused = on;
    m_btnPause->setChecked(on);
    m_videoWidget->setPaused(on);
}

void MainWindow::onViewClicked()
{
    qDebug() << "[TODO] switch view";
}

void MainWindow::onNightClicked()
{
    m_nightMode = !m_nightMode;
    m_btnNight->setChecked(m_nightMode);
    m_videoWidget->setNightMode(m_nightMode);
    qDebug() << "night mode =" << m_nightMode;
}

void MainWindow::onTrackClicked()
{
    m_trackMode = !m_trackMode;
    m_btnTrack->setChecked(m_trackMode);
    m_videoWidget->setShowGuideLine(m_trackMode);
    qDebug() << "track mode =" << m_trackMode;
}

void MainWindow::onPauseClicked()
{
    setPaused(!m_paused);
    qDebug() << "preview paused =" << m_paused;
}

void MainWindow::onShotClicked()
{
    const QString shotDir = QDir::homePath() + "/RearCamShots";
    QDir().mkpath(shotDir);

    const QString filePath =
        shotDir + "/" + QDateTime::currentDateTime().toString("'rearcam_'yyyyMMdd_hhmmss'.png'");

    if (m_videoWidget->grab().save(filePath)) {
        qDebug() << "screenshot saved:" << filePath;
    } else {
        qWarning() << "screenshot save failed:" << filePath;
    }
}

void MainWindow::onRecordClicked()
{
    setRecording(true);
    qDebug() << "record start";
}

void MainWindow::onStopClicked()
{
    setRecording(false);
    qDebug() << "record stop";
}

void MainWindow::onPauseRecordClicked()
{
    if (!m_recording) {
        m_btnPauseRecord->setChecked(false);
        return;
    }

    const bool holdOn = m_btnPauseRecord->isChecked();
    m_btnPauseRecord->setText(holdOn ? "RESUME" : "HOLD");
    qDebug() << "record hold =" << holdOn;
}

void MainWindow::onFileClicked()
{
    qDebug() << "shots directory =" << QDir::homePath() + "/RearCamShots";
}

void MainWindow::onSettingClicked()
{
    qDebug() << "[TODO] open settings";
}
