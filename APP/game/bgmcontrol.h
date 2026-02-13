#ifndef BGMCONTROL_H
#define BGMCONTROL_H

#include <QWidget>
#include <QMediaPlayer>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
// Qt5 删除 QAudioOutput，QMediaPlayer 内置音频输出
// #include <QAudioOutput>
#include <player.h>

class Bgmcontrol : public QWidget
{
    Q_OBJECT
public:
    // 男女出牌音效
    enum class Sound {
        _3 = 0, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _1, _2, _14, _15,
        DUI3, DUI4, DUI5, DUI6, DUI7, DUI8, DUI9, DUI10, DUI11, DUI12, DUI13, DUI1, DUI2,
        TUPLE3, TUPLE4, TUPLE5, TUPLE6, TUPLE7, TUPLE8, TUPLE9, TUPLE10, TUPLE11, TUPLE12, TUPLE13, TUPLE1, TUPLE2,
        FEIJI, LIANDUI, SANDAIYI, SANDAIYIDUI, SHUNZI, SIDAIER, SIDAILIANGDUI, ZHADAN, WANGZHA,
        BUYAO1, BUYAO2, BUYAO3, BUYAO4,
        DANI1, DANI2, DANI3,
        NOORDER, NOROB, ORDER, ROB1, ROB3,
        BAOJING1, BAOJING2,
        SHARE, BUJIABEI, JIABEI,
        COUNT
    };

    enum class BGM {
        WELCOME = 0, NORMAL, NORMAL2, EXCITING, COUNT
    };

    enum class EndingSound {
        WIN = 0, LOSE, COUNT
    };

    enum class OtherSound {
        DISPATCH = 0, SELECT_CARD, PLANE, BOMB, ALERT,
    };

    explicit Bgmcontrol(QWidget *parent = nullptr);

    void InitMusicPlayer();
    void StartBgm();
    void StopBgm();
    void StartEndBgm(bool isWin);
    void playResultBgm(bool isWin);
    void PlayeHandBgm(player::Sex sex, bool isfirst, Cards *cards);
    void NoPlayerHandBgm(player::Sex sex);
    void OtherBgm(OtherSound type);
    void StopOtherBgm();
    void GetlordBgm(int point, player::Sex sex, bool isfirst);
    void playSoundBySex(player::Sex sex, Sound soundIndex, const QString &logType);

signals:

private:
    // Qt5: 只有 QMediaPlayer，没有 QAudioOutput
    QList<QMediaPlayer*> _MPlayer;
    // Qt5 删除 QList<QAudioOutput*> _MOutput;

    QList<QString> _Manbgm;
    QList<QString> _Womanbgm;
    QList<QString> _Bgm;
    QList<QString> _Otherbgm;
    QString _WinResult;
    QString _LoseResult;
};

#endif // BGMCONTROL_H
