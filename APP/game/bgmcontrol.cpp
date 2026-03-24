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
        _HasPlaybackBackend = _HasPlaybackBackend && player->isAvailable();
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

    if (root.contains("Ending") && root["Ending"].isArray()) {
        QJsonArray endingArray = root["Ending"].toArray();
        if (endingArray.size() > 0 && endingArray.at(0).isString()) {
            _WinResult = endingArray.at(0).toString();
        }
        if (endingArray.size() > 1 && endingArray.at(1).isString()) {
            _LoseResult = endingArray.at(1).toString();
        }
    } else if (root.contains("Result") && root["Result"].isObject()) {
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
    if (!_HasPlaybackBackend || !bgmPlayer || !bgmPlayer->isAvailable()) return;

    disconnect(bgmPlayer, nullptr, this, nullptr);
    connect(bgmPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged),
            this, [this, bgmPlayer](QMediaPlayer::State state) {
                if (state == QMediaPlayer::StoppedState && !_Bgm.isEmpty()) {
                    _CurrentBgmIndex = (_CurrentBgmIndex + 1) % _Bgm.size();
                    bgmPlayer->setMedia(QUrl(_Bgm[_CurrentBgmIndex]));
                    bgmPlayer->play();
                }
            });

    if (bgmPlayer->state() == QMediaPlayer::PlayingState) return;

    _CurrentBgmIndex = qBound(0, _CurrentBgmIndex, _Bgm.size() - 1);
    const QString bgmPath = _Bgm[_CurrentBgmIndex];
    bgmPlayer->setMedia(QUrl(bgmPath));
    bgmPlayer->play();
}

void Bgmcontrol::StopBgm()
{
    QMediaPlayer *bgmPlayer = _MPlayer[2];
    if (!bgmPlayer) return;
    bgmPlayer->stop();
    disconnect(bgmPlayer, nullptr, this, nullptr);
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
    if (!_HasPlaybackBackend || !endPlayer || !endPlayer->isAvailable()) return;
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

    QMediaPlayer *otherPlayer = _MPlayer[3];
    if (!_HasPlaybackBackend || !otherPlayer || !otherPlayer->isAvailable()) return;

    if (_CurrentOtherIndex == index && otherPlayer->state() == QMediaPlayer::PlayingState) {
        return;
    }

    disconnect(otherPlayer, nullptr, this, nullptr);
    otherPlayer->stop();
    otherPlayer->setPosition(0);

    QString soundPath = _Otherbgm[index];
    otherPlayer->setMedia(QUrl(soundPath));
    _CurrentOtherIndex = index;

    connect(otherPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
            this, [this, soundPath, type](QMediaPlayer::Error error) {
                Q_UNUSED(type);
                qWarning() << "播放错误:" << error << soundPath;
                if (error == QMediaPlayer::ServiceMissingError) {
                    _HasPlaybackBackend = false;
                }
            });

    if (type == OtherSound::DISPATCH) {
        connect(otherPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged),
                this, [otherPlayer](QMediaPlayer::State state) {
                    if (state == QMediaPlayer::StoppedState) {
                        otherPlayer->setPosition(0);
                        otherPlayer->play();
                    }
                });
    } else {
        connect(otherPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged),
                this, [otherPlayer](QMediaPlayer::State state) {
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
    if (!otherPlayer) return;
    otherPlayer->stop();
    disconnect(otherPlayer, nullptr, this, nullptr);
    _CurrentOtherIndex = -1;
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
    if (!_HasPlaybackBackend || !player || !player->isAvailable()) return;

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
