#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QLocalServer;
class QLocalSocket;
class QTimer;
class QProcess;
QT_END_NAMESPACE

class VideoWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void startSenderProcess();
    void stopSenderProcess();

    void startLocalServer();
    void onNewLocalConnection();
    void onClientDisconnected();
    void onSocketReadyRead();

    void updateUiFps();
    void simulateStatus();

    void onViewClicked();
    void onNightClicked();
    void onTrackClicked();
    void onPauseClicked();
    void onShotClicked();

    void onRecordClicked();
    void onStopClicked();
    void onPauseRecordClicked();
    void onFileClicked();
    void onSettingClicked();

private:
    void setupUi();
    void setupTopBar();
    void setupVideoArea();
    void setupRadarArea();
    void setupBottomButtons();
    void setupRecordButtons();

    void resetFrameParser();
    void processIncomingData();
    void updateRadarByDistance(float meters);
    void setRecording(bool on);
    void setPaused(bool on);

private:
    QWidget *m_central;

    QWidget *m_topBar;
    QLabel *m_labelGear;
    QLabel *m_labelSpeed;
    QLabel *m_labelFps;
    QLabel *m_labelTemp;
    QLabel *m_labelRec;

    VideoWidget *m_videoWidget;

    QWidget *m_radarBar;
    QLabel *m_leftRadar;
    QLabel *m_centerDistance;
    QLabel *m_rightRadar;

    QWidget *m_funcBar;
    QPushButton *m_btnView;
    QPushButton *m_btnNight;
    QPushButton *m_btnTrack;
    QPushButton *m_btnPause;
    QPushButton *m_btnShot;

    QWidget *m_recordBar;
    QPushButton *m_btnRecord;
    QPushButton *m_btnStop;
    QPushButton *m_btnPauseRecord;
    QPushButton *m_btnFile;
    QPushButton *m_btnSetting;

    QLocalServer *m_server;
    QLocalSocket *m_clientSocket;
    QTimer *m_fpsTimer;
    QTimer *m_statusTimer;
    QProcess *m_senderProcess;

    QByteArray m_recvBuffer;
    quint32 m_expectedFrameSize;

    int m_frameCounter;
    int m_currentFps;
    bool m_started;
    bool m_recording;
    bool m_paused;
    bool m_nightMode;
    bool m_trackMode;
    float m_fakeDistance;
};

#endif // MAINWINDOW_H
