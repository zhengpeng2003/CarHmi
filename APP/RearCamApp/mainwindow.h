#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <QProcess>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
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
    void onNewConnection();
    void onClientDisconnected();
    void onSocketReadyRead();
    void updateFps();
    void onPauseClicked();
    void onTrackClicked();
    void onShotClicked();
    void onCloseClicked();

private:
    void setupUi();
    void startLocalServer();
    void startSenderProcess();
    void stopSenderProcess();
    void resetFrameParser();
    void processData();

    // UI
    QWidget *m_central;
    QLabel *m_fpsLabel;
    QPushButton *m_closeBtn;
    VideoWidget *m_videoWidget;
    QPushButton *m_btnPause;
    QPushButton *m_btnTrack;
    QPushButton *m_btnShot;

    // IPC
    QLocalServer *m_server;
    QLocalSocket *m_clientSocket;
    QProcess *m_senderProcess;
    QByteArray m_buffer;
    quint32 m_frameSize;
    int m_frameCounter;
    int m_fps;
    bool m_started;
    bool m_introPlayed;
    bool m_paused;
    bool m_trackMode;
};

#endif
