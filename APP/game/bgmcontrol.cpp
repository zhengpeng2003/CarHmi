#include "bgmcontrol.h"
#include "player.h"
#include "playhand.h"
#include <QDebug>
#include <QTimer>

Bgmcontrol::Bgmcontrol(QWidget *parent)
    : QWidget(parent)
{
    for (int i = 0; i < 5; ++i) {
        QMediaPlayer *player = new QMediaPlayer(this);

        if (i == 2) {
            player->setVolume(40);
        } else {
            player->setVolume(80);
        }

        _MPlayer.append(player);
    }
}

void Bgmcontrol::InitMusicPlayer()
{
    QFile file(":/conf/playList.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开音频配置文件";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull()) {
        qWarning() << "JSON解析失败";
        return;
    }

    QJsonObject root = doc.object();

    if (root.contains("Man") && root["Man"].isArray()) {
        QJsonArray array = root["Man"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Manbgm.append(value.toString());
            }
        }
    }

    if (root.contains("Woman") && root["Woman"].isArray()) {
        QJsonArray array = root["Woman"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Womanbgm.append(value.toString());
            }
        }
    }

    if (root.contains("BGM") && root["BGM"].isArray()) {
        QJsonArray array = root["BGM"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Bgm.append(value.toString());
            }
        }
    }

    if (root.contains("Result") && root["Result"].isObject()) {
        QJsonObject resultObj = root["Result"].toObject();
        _WinResult = resultObj.value("Win").toString();
        _LoseResult = resultObj.value("Lose").toString();
    }

    if (root.contains("Other") && root["Other"].isArray()) {
        QJsonArray array = root["Other"].toArray();
        for (const QJsonValue &value : array) {
            if (value.isString()) {
                _Otherbgm.append(value.toString());
            }
        }
    }
}

void Bgmcontrol::StartBgm()
{
    if (_Bgm.isEmpty()) return;

    QMediaPlayer *bgmPlayer = _MPlayer[2];

    static int index = 0;
    QString bgmPath = _Bgm[index];

    bgmPlayer->setMedia(QUrl(bgmPath));

    connect(bgmPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged),
            [=](QMediaPlayer::State state) {
                if (state == QMediaPlayer::StoppedState) {
                    index = (index + 1) % _Bgm.size();
                    QString nextBgmPath = _Bgm[index];
                    bgmPlayer->setMedia(QUrl(nextBgmPath));
                    bgmPlayer->play();
                }
            });

    bgmPlayer->play();
}

void Bgmcontrol::StopBgm()
{
    QMediaPlayer *bgmPlayer = _MPlayer[2];
    bgmPlayer->stop();
    bgmPlayer->disconnect();
}

void Bgmcontrol::StartEndBgm(bool isWin)
{
    playResultBgm(isWin);
}

void Bgmcontrol::playResultBgm(bool isWin)
{
    StopBgm();

    const QString &endPath = isWin ? _WinResult : _LoseResult;
    if (endPath.isEmpty()) return;

    QMediaPlayer *endPlayer = _MPlayer[4];
    endPlayer->stop();
    endPlayer->setPosition(0);
    endPlayer->setMedia(QUrl(endPath));
    endPlayer->play();
}

void Bgmcontrol::PlayeHandBgm(player::Sex sex, bool isfirst, Cards *cards)
{
    if (!cards || cards->isempty()) {
        NoPlayerHandBgm(sex);
        return;
    }

    PlayHand playHand(cards);
    PlayHand::HandType handType = playHand.Getplayhandtype();

    if (handType == PlayHand::Hand_Unknown) {
        NoPlayerHandBgm(sex);
        return;
    }

    Sound soundIndex = Sound::_3;
    Card::cardpoint point = playHand.Getplayhandpoint();
    bool shouldPlayDani = false;
    const bool isBeatStage = !isfirst;

    const bool forceDaniOnly = isBeatStage && (handType == PlayHand::Hand_Bomb
                                               || handType == PlayHand::Hand_Bomb_Jokers
                                               || handType == PlayHand::Hand_Seq_Sim
                                               || handType == PlayHand::Hand_Seq_Pair
                                               || handType == PlayHand::Hand_Plane
                                               || handType == PlayHand::Hand_Plane_Two_Single
                                               || handType == PlayHand::Hand_Plane_Two_Pair
                                               || handType == PlayHand::Hand_Triple_Single
                                               || handType == PlayHand::Hand_Triple_Pair);

    switch (handType) {
    case PlayHand::Hand_Single:
        if (point >= Card::Card_3 && point <= Card::Card_BJ) {
            if (point >= Card::Card_3 && point <= Card::Card_2) {
                soundIndex = static_cast<Sound>(point - Card::Card_3);
            }
            else if (point == Card::Card_SJ) {
                soundIndex = Sound::_14;
            }
            else if (point == Card::Card_BJ) {
                soundIndex = Sound::_15;
            }
        }
        break;

    case PlayHand::Hand_Pair:
        if (point >= Card::Card_3 && point <= Card::Card_2) {
            soundIndex = static_cast<Sound>(15 + (point - Card::Card_3));
        }
        break;

    case PlayHand::Hand_Triple:
        if (point >= Card::Card_3 && point <= Card::Card_2) {
            soundIndex = static_cast<Sound>(28 + (point - Card::Card_3));
        }
        break;

    case PlayHand::Hand_Triple_Single:
        soundIndex = Sound::SANDAIYI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Triple_Pair:
        soundIndex = Sound::SANDAIYIDUI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Seq_Sim:
        soundIndex = Sound::SHUNZI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Seq_Pair:
        soundIndex = Sound::LIANDUI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Plane:
        soundIndex = Sound::FEIJI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Plane_Two_Single:
    case PlayHand::Hand_Plane_Two_Pair:
        soundIndex = Sound::FEIJI;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Bomb:
        soundIndex = Sound::ZHADAN;
        shouldPlayDani = true;
        break;

    case PlayHand::Hand_Bomb_Jokers:
        soundIndex = Sound::WANGZHA;
        shouldPlayDani = true;
        break;

    default:
        if (point >= Card::Card_3 && point <= Card::Card_BJ) {
            if (point >= Card::Card_3 && point <= Card::Card_2) {
                soundIndex = static_cast<Sound>(point - Card::Card_3);
            } else if (point == Card::Card_SJ) {
                soundIndex = Sound::_14;
            } else if (point == Card::Card_BJ) {
                soundIndex = Sound::_15;
            }
        }
        break;
    }

    if (forceDaniOnly) {
        playSoundBySex(sex, Sound::DANI3, "压牌特效音效");
        return;
    }

    playSoundBySex(sex, soundIndex, "出牌音效");

    if(isBeatStage && shouldPlayDani)
    {
        QTimer::singleShot(150, this, [=](){
            playSoundBySex(sex, Sound::DANI3, "压牌特效音效");
        });
    }
}

void Bgmcontrol::NoPlayerHandBgm(player::Sex sex)
{
    playSoundBySex(sex, Sound::BUYAO1, "不要音效");
}

void Bgmcontrol::OtherBgm(OtherSound type)
{
    if (_Otherbgm.isEmpty()) return;

    int index = static_cast<int>(type);
    if (index < 0 || index >= _Otherbgm.size()) return;

    delete _MPlayer[3];

    QMediaPlayer *otherPlayer = new QMediaPlayer(this);
    otherPlayer->setVolume(80);
    _MPlayer[3] = otherPlayer;

    QString soundPath = _Otherbgm[index];
    otherPlayer->setMedia(QUrl(soundPath));

    connect(otherPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
            [soundPath, type](QMediaPlayer::Error error) {
                qWarning() << "播放错误:" << error << soundPath;
            });

    if (type == OtherSound::DISPATCH) {
        connect(otherPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged),
                [otherPlayer](QMediaPlayer::State state) {
                    if (state == QMediaPlayer::StoppedState) {
                        otherPlayer->setPosition(0);
                        otherPlayer->play();
                    }
                });
    } else {
        connect(otherPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged),
                [otherPlayer](QMediaPlayer::State state) {
                    if (state == QMediaPlayer::StoppedState) {
                        otherPlayer->stop();
                    }
                });
    }

    otherPlayer->play();
}

void Bgmcontrol::StopOtherBgm()
{
    QMediaPlayer *otherPlayer = _MPlayer[3];
    otherPlayer->stop();
    otherPlayer->disconnect();
}

void Bgmcontrol::GetlordBgm(int point, player::Sex sex, bool isfirst)
{
    Sound soundIndex = Sound::NOORDER;

    if (point <= 0) {
        soundIndex = isfirst ? Sound::NOORDER : Sound::NOROB;
    } else if (isfirst) {
        soundIndex = Sound::ORDER;
    } else {
        switch (point) {
        case 1:
            soundIndex = Sound::ROB1;
            break;
        case 2:
        case 3:
            soundIndex = Sound::ROB3;
            break;
        default:
            soundIndex = Sound::ROB3;
            break;
        }
    }

    playSoundBySex(sex, soundIndex, "抢地主音效");
}

void Bgmcontrol::playSoundBySex(player::Sex sex, Sound soundIndex, const QString &logType)
{
    int playerIndex = (sex == player::Sex::MAN) ? 0 : 1;
    QMediaPlayer *player = _MPlayer[playerIndex];
    QList<QString> &soundList = (sex == player::Sex::MAN) ? _Manbgm : _Womanbgm;

    int index = static_cast<int>(soundIndex);

    if (index >= 0 && index < soundList.size()) {
        QString soundPath = soundList[index];

        QFile file(soundPath);
        if (!file.exists()) {
            qWarning() << "音效文件不存在:" << soundPath;
        }

        player->stop();
        player->setPosition(0);
        player->setMedia(QUrl(soundPath));
        player->play();
    } else {
        if(!soundList.isEmpty()) {
            player->stop();
            player->setPosition(0);
            player->setMedia(QUrl(soundList.first()));
            player->play();
        }
    }
}
