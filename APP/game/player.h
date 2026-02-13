#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <card.h>
#include <cards.h>
#include <qpixmap.h>
#include <QRandomGenerator>
class player : public QObject
{
    Q_OBJECT
public:
    enum PlayerType{USER,ROOT,NOKNOW,};
    enum Location{LEFT,RIGHT,};
    enum Role{LORD,FORMAR,UN_KONWROLE};
    enum Sex{MAN,WOMAN,UN_KONWSEX};
    explicit player(QObject *parent = nullptr);
    player(QString Name,PlayerType PlayerType,Location Location,QObject *parent);

    void SetScore(int score);
    int GetScore();
    //获取前节点
    player * GetPrePlayer();
    //获取后节点
    player * GetNextPlayer();
    //获取存储卡牌
    void StoreGetCard(Card * card);
    //获取最后三张地主牌
    void StoreGetCards(Cards * cards);
    //获取当前牌
    Cards GetCards();
    //移除指定牌组
    void RemoveCards(Cards *cards);
    //清理牌
    void ClearCards();
    //出牌
    void PlayHand(Cards * cards);
    //出牌信息
    void SetPendinfo(player * PendPlayer, Cards *PendCard);
    //获取出牌者
    player * GetPendPlayer();
    //获取出牌信息
    Cards *GetPendCard();
    //设置获胜还是失败
    void Setwin(bool is_win);
    //获取胜利还是失败
    bool Getwin();
    //设置当前角色图片
    QPixmap GetPlayerRolePixmap(Role Role,Sex Sex, Location Location);
    //设置前一个玩家
    void SetPrePlayer(player * a);
    //设置下个
    void SetNextPlayer(player * a);
    //设置性别
    void SetSex(Sex a);
    //设置角色
    void setRole(Role a );
    //设置卡牌正反面
    void Setfront(bool Isfront);
    //获取卡牌正反面
    bool GetIsfront();
    // 开始新一局时重置玩家状态
    void ResetForNewGame();
    //获取性别
    Sex GetSex();
    //获取位置
    Location GetLocation();
    //获取角色
    Role GetRole();
    //设置下注
    void grablordbet(int bet);
    //抢地主
    virtual void PrepareGetLord() ;
    //出牌
    virtual void PreparePlayCard() ;
    //机器人考虑抢地主
    virtual void RobotGetLard() ;
    //机器人考虑出牌
    virtual void RobotThinkPlayhand();


protected:
    bool _Isfront;
    QString _Name;//姓名
    int _Score;//分数
    PlayerType _PlayerType;//机器人还是用户
    Location _Location;//位置
    Role _Role;//角色
    Sex _Sex;//性别
    player *_Preplayer;//前一个玩家
    player *_Nextplayer;//后一个玩家
    Cards _Cards;//当前牌
    player * _PendPlayer;//出牌者
    Cards * _PendCard;//出牌信息
    bool _Win;

    
signals:
    void notifygrablordbet(player * player,int bet);//通知游戏控制类抢地主
    void notifyplayhand(player * player,Cards* cards);//告诉游戏控制类出的牌
    //告诉要存储牌了
    void notifystorecard(player * player);
    //出牌信号给闹钟 闹钟提示
    void notifyTime();
};

#endif // PLAYER_H
