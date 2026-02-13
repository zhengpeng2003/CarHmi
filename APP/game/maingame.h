//最终代码
#pragma once
#ifndef MAINGAME_H
#define MAINGAME_H
#include <QPropertyAnimation>
#include "cardpanel.h"
#include <QMainWindow>
#include <mybuttongroup.h>
#include <QPixmap>
#include <QPainter>
#include <QRandomGenerator>
#include <gamecontrol.h>
#include <player.h>
#include <card.h>
#include <cardpanel.h>
#include <cards.h>
#include <QLabel>
#include <QTimer>
#include <QCloseEvent>
#include <anmationpixmap.h>
#include <endpanel.h>
#include <timecount.h>
#include <bgmcontrol.h>
#include <QList>
QT_BEGIN_NAMESPACE
namespace Ui {
class Maingame;
}
QT_END_NAMESPACE
// 123456
//test分支1234564897
class Maingame : public QMainWindow
{
    Q_OBJECT

public:
    enum  cardAlign{Horizontal,
                     Vertical};
    struct _Playercontext
    {
        //出牌位置
        QRect _PlayerHandRect;
        //卡牌位置
        QRect _PLayerCardsRect;
        //头像参考位置
        QPoint _RoleImgPos;
        //信息提示参考位置
        QPoint _InfoPos;
        //不出的标签
        QLabel * _NOCardlabel;
        //角色标签
        QLabel * _ROlelabel;
        //卡牌正反面
        bool Isfront;
        //卡牌
        Cards * _Mycards;
        //水平还是竖直
        cardAlign _Align;
        //存储最后一张出的牌
        Cards *_Last_Cards=new Cards;
    };
    Maingame(QWidget *parent = nullptr);
    //游戏控制类的连接
    void InitGamecontrol();
    //初始化分数
    void InitScore();
    //初始化所有的卡牌
    void InitCardpanelMap();
    //设置卡牌的图片
    void InitCardImage(QPixmap Card_front,QPixmap Card_back,Card *card);
    //初始化按钮
    void InitGroupbtn();
    //显示卡牌位置
    void InitGameScene();
    //开始发牌
    void SatrtPend();
    //触发时间的函数
    void PlayHandtimer(player * Player,int Movetime);
    //初始化移动位置
    void InitMovepoint();
    //设置当前的的状态
    void SetCurrentGameStatue(gamecontrol::GameState state);
    //发牌到玩家手上
    void PendCardplayer(player *player);
    //发到玩家手上的位置 玩家手上的牌的窗口
    void PendCardpos(player *player);
    //处理玩家出的牌的槽函数
    void OndisPosePlayhand(player *player,Cards * cards);
    //隐藏上一轮玩家打出的牌
    void HidePlayhand(player * player);
    //改变状态后执行操作
    void PlayerStateChange(player *player,gamecontrol::USERSTATE state);
    //是否抢地主
    void gamenotifyGetLoard(player * player,int Bet,bool first);
    //卡牌点击函数
    void Cardpanel(Qt::MouseButton event);
    //游戏结算页面
    void InitEndPanel(player * player);
    //玩家出牌
    void UserPlayHand();
    //玩家要不起
    void UserNoPlayer();
    // 自由出牌超时自动打出第一张牌
    void AutoPlayFirstCard();
    //重新游戏
    void RePlayGame();
    //出牌时间闹钟
    void InitPlayerTimer();
    //展示出牌的动画
    void Showanimation(PlayHand::HandType type);
    //成为地主换图片
    void OnLordDetermined(player* lordPlayer);
    void ResetCountdown();
    void ShowPlayerInfoImage(player *player, const QPixmap &pixmap);
    QPoint CalculateLabelPosAbovePlayArea(_Playercontext* ctx, const QSize &labelSize, int extraOffset = 0) const;
    QPoint CalculateRoleLabelPos(_Playercontext* ctx, const QSize &labelSize) const;//角色标签位置
    QPoint CalculateCenteredPos(const QPoint &anchor, const QSize &labelSize) const;
    //清空当前玩家的选中状态
    void ClearSelectedPanels();
    bool IsFreePlayStage() const;
    void HandleUserPlaySuccess();
    CardPanel* PanelFromPos(const QPoint &pos) const;
    void ShowRobotHands();
    void ClearRobotHands();
    ~Maingame();
protected:
    virtual void paintEvent(QPaintEvent *event)override;
    virtual void mouseMoveEvent(QMouseEvent *event)override;
    virtual void mousePressEvent(QMouseEvent *event)override;
    virtual void mouseReleaseEvent(QMouseEvent *event)override;
    void closeEvent(QCloseEvent *event) override;

private:
    void SaveLastGameScores();
    QPoint _Base_point;
    QPixmap _IMage_Map;
    QPixmap _IMage_Cards;
    QPixmap _Card_back;
    QSize _IMage_Card_Size;
    gamecontrol * _Gamecontrol;
    QVector<player*> _Players;//玩家
    QMap<Card,CardPanel*>_CardPenelMap;//卡牌的图片
    int _Movetime;

    QMap<player*,_Playercontext*> _Playercontexts;//卡牌在谁手上
    CardPanel *_PendCards;//发的牌
    CardPanel *_MoveCards;//移动牌
    CardPanel *_LordCards[3];//地主牌
    QSet<CardPanel*> _SelcetPanel;//被选中的牌
    QHash<CardPanel *,QRect> _PanelPositon;
    CardPanel * _CurrtPanel;
    QRect _Mycardsrect;
    QTimer *_Timer_PlayHand;
    Ui::Maingame *ui;
    QPoint _xy[3];
    AnmationPixmap *_MyAnmation;
    Timecount * _Timecount;
    Bgmcontrol * _Bgmcontrol;
    gamecontrol::GameState _CurrentGameState = gamecontrol::PENDCARD;
    bool _CanSelectCards = false;
    bool _IsUserFirstLordPlay = false;
    bool _IsDraggingSelect = false;
    int _InfoLabelSeq = 0;
    QList<CardPanel*> _RobotRevealPanels;

    // 记录上一局的最终分数
    int _LastUserScore = 0;
    int _LastLeftRobotScore = 0;
    int _LastRightRobotScore = 0;

};
#endif // MAINGAME_H
