#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include <player.h>
#include <card.h>
#include <cards.h>
#include <robot.h>
#include <user.h>
#include <QRandomGenerator>
#include <QDebug>

struct  Betrest
{
    Betrest()
    {
        rest();
    }
    void rest()
    {
        times=0;
        Bet=0;
        _player=nullptr;
    }
    int times;
    int Bet;
    player * _player;

};
class gamecontrol :public QObject
{
    Q_OBJECT
public:
    gamecontrol();
    enum GameState{GIVECARD=1,//出牌
                     GETLORD,//抢地主
                     PENDCARD//发牌
    };
    enum USERSTATE{USERGETLORD,//抢地主
                     USERPEND,//出牌
                     USERWIN//游戏获胜
    }
    ;
    //玩家初始化
    void InitPlayer();
    //当前玩家
    void SetCurrentPlayer(player * a);
    player * GetCurrentPlayer();
    //打出的当前牌
    void SetCurrentCards(Cards * a);
    Cards* GetCurrentCards();
    //初始化卡牌
    void SetAllCards();
    //发牌
    Card  *TakeOneCard();
    Cards * TakeCards();
    //重置数据
    void RetCardDate();
    //获取当前出牌玩家
    player * GetPendplayer();
    //准备叫地主
    void StartPrepareLord();
    //成为地主
    void BecomeLord(player * player,int Bet);
    //出牌
    void GamePlayhand(player * player,Cards * cards);
    //清空分数
    void ClearScore();
    //获取卡牌池还有多少张
    int GetCardcount();
    Cards * GetAllCards();
    // 当前叫分最高分（0 表示无人叫分）
    int GetCurrentMaxBet() const { return _Betrect.Bet; }
    //开始下注
    void Onbet(player * player,int Bet);

    player * GetLeftroot();
    player * GetRightroot();
    player * GetUSer();
signals:
    //用户游戏状态改变
    void S_PlayerStateChange(player * player,USERSTATE state);
    //通知主页面要抢地主
    void S_gamenotifyGetLoard(player * player,int Bet,bool first);
    //通知主页面当前要进行的游戏状态
    void S_gameStateChange(GameState state);
    //告诉主界谁出牌了
    void S_gamePlayHand(player * player,Cards *cards);
    //存储当前出牌玩家和出的牌
    void S_PlayerPlayHand(player * player,Cards *cards);
    // 地主确定
    void S_LordDetermined(player* lordPlayer);
    // 停止/隐藏倒计时
    void S_StopCountdown();
    // 播放结算结果
    void S_PlayResult(bool isWin);
private:
    robot * _Leftrobot=nullptr;
    robot * _Rightrobot=nullptr;
    user * _UserPlayer=nullptr;
    player * _CurrentPlayer=nullptr;
    Cards *_CurrentCards=nullptr;
    Cards * _PlayHandCards=nullptr;
    player * _PlayHandplayer;
    Cards * _AllCards;
    int _Countcard;
    Betrest _Betrect;
    int _Bet=0;
};

#endif // GAMECONTROL_H
