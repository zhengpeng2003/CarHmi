#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <qpainter.h>
#include "QNetworkAccessManager"
#include "QNetworkReply"
#include "QNetworkRequest"
#include "qaudiooutput.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaPlayer>
#include <qjsonarray.h>
#include <qmessagebox.h>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
struct MusicItem {
    QString title;   // 歌曲显示名
    QString path;    // 歌曲文件路径
};
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QPoint movepos;          // 窗口拖动偏移量
    bool mousestate;         // 鼠标按下状态
    void paintEvent(QPaintEvent *event);  // 绘制背景

    // 酷狗API（预留）
    QString kugouSearchApi= "http://mobilecdn.kugou.com/api/v3/search/song?";
    QString kugouDownloadApi="https://wwwapi.kugou.com/yy/index.php?";
    QNetworkAccessManager *manager=NULL;
    QNetworkRequest request;
    QNetworkReply *reply;

private slots:
    void on_pushButton_about_clicked();     // 关于
    void on_pushButton_skin_clicked();      // 换肤
    void on_pushButton_close_clicked();     // 关闭
    void playCurrentMusic();                // 播放当前
    void on_pushButton_play_clicked();      // 播放/暂停
    void on_pushButton_next_clicked();      // 下一首
    void on_pushButton_pre_clicked();       // 上一首
    void updateProgress();                 // 更新进度
    void on_horizontalSlider_proce_sliderMoved(int value);  // 进度拖动
    void on_horizontalSlider_sound_valueChanged(int value); // 音量调节
    void on_listWidget_itemClicked(QListWidgetItem *item);//切换歌曲根据item
protected:
    void mouseMoveEvent(QMouseEvent *event);    // 鼠标移动
    void mousePressEvent(QMouseEvent *event);   // 鼠标按下
    void mouseReleaseEvent(QMouseEvent *event); // 鼠标释放
    void update_lyric();                        // 更新歌词
    void initmusicground();//初始化音乐碟片图片
private:
    Ui::Widget *ui;
    QPixmap m_background;           // 背景图
    QMediaPlayer *m_player;         // 播放器
    QAudioOutput *m_audio;          // 音频输出
    QList<MusicItem> m_musicList;   // 音乐列表
    QTimer *m_timer;               // 进度定时器
    int m_currentIndex = -1;       // 当前索引

    void initMusicList();           // 初始化列表
    void playIndex(int index);      // 播放指定
    QPixmap m_discPixmap;    // 碟片图片
    qreal m_angle = 0;       // 当前旋转角度
    QTimer *m_discTimer;     // 碟片旋转定时器
};

#endif // WIDGET_H
