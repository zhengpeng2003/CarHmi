#include "gamecontrol.h"
#include "maingame.h"
#include "qdebug.h"

gamecontrol::gamecontrol():_AllCards(new Cards()),_Countcard(0),_PlayHandCards(nullptr),_PlayHandplayer(nullptr)
{}

void gamecontrol::InitPlayer()
{
    //机器人初始化
    _Rightrobot=new robot("机器人A",player::ROOT,player::LEFT,this);
    _Leftrobot=new robot("机器人B",player::ROOT,player::RIGHT,this);
    _UserPlayer=new user("玩家",player::USER,player::RIGHT,this);

    //前后节点初始化
    _Rightrobot->SetNextPlayer(_Leftrobot);
    _Rightrobot->SetPrePlayer(_UserPlayer);

    _Leftrobot->SetNextPlayer(_UserPlayer);
    _Leftrobot->SetPrePlayer(_Rightrobot);

    _UserPlayer->SetNextPlayer(_Rightrobot);
    _UserPlayer->SetPrePlayer(_Leftrobot);

    //性别初始化我们用随机数
    player::Sex R1=(player::Sex)QRandomGenerator::global()->bounded(0,2);
    player::Sex R2=(player::Sex)QRandomGenerator::global()->bounded(0,2);
    player::Sex R3=(player::Sex)QRandomGenerator::global()->bounded(0,2);

    _Rightrobot->SetSex(R1);
    _Leftrobot->SetSex(R2);
    _UserPlayer->SetSex(R3);
    //下注信号
    connect(_UserPlayer,&player::notifygrablordbet,this,&gamecontrol::Onbet);
    connect(_Leftrobot,&player::notifygrablordbet,this,&gamecontrol::Onbet);
    connect(_Rightrobot,&player::notifygrablordbet,this,&gamecontrol::Onbet);

    //存储出牌信息的信号
    connect(this,&gamecontrol::S_PlayerPlayHand,_Rightrobot,&player::SetPendinfo);
    connect(this,&gamecontrol::S_PlayerPlayHand,_Leftrobot,&player::SetPendinfo);
    connect(this,&gamecontrol::S_PlayerPlayHand,_UserPlayer,&player::SetPendinfo);

    //用户出的牌
    connect(_UserPlayer,&player::notifyplayhand,this,&gamecontrol::GamePlayhand);
    connect(_Leftrobot,&player::notifyplayhand,this,&gamecontrol::GamePlayhand);
    connect(_Rightrobot,&player::notifyplayhand,this,&gamecontrol::GamePlayhand);


}

void gamecontrol::SetCurrentPlayer(player *a)
{
    this->_CurrentPlayer=a;
}

player *gamecontrol::GetCurrentPlayer()
{
    return this->_CurrentPlayer;
}

void gamecontrol::SetCurrentCards(Cards *a)
{
    this->_CurrentCards=a;
}

Cards * gamecontrol::GetCurrentCards()
{
    return this->_CurrentCards;
}

void gamecontrol::SetAllCards()
{

    // 重新开局前清理旧牌堆和计数，避免累计上一局的数据
    _AllCards->clearcards();
    _Countcard = 0;

    for(int p=Card::Card_begin+1;p<Card::Card_SJ;p++)
    {

        for(int q=Card::Suit_Begin+1;q<Card::Suit_End;q++)
        {
            Card::cardpoint point =(Card::cardpoint)p;
            Card::cardsuit suit =(Card::cardsuit)q;
            Card *temp=new Card(suit,point);

            _AllCards->add(temp);
            _Countcard++;
        }
    }
    _AllCards->add(Card(Card::Suit_Begin,Card::Card_SJ));_Countcard++;
    _AllCards->add(Card(Card::Suit_Begin,Card::Card_BJ));_Countcard++;

}

Card *gamecontrol::TakeOneCard()
{
    _Countcard--;

    return _AllCards->takerandcard();
}

Cards* gamecontrol::TakeCards()
{

    return this->_AllCards;

}

void gamecontrol::RetCardDate()
{
    // 1. 牌堆和计数重置
    SetAllCards();

    // 2. 清空玩家手牌与上一轮记录
    _Rightrobot->ClearCards();
    _Leftrobot->ClearCards();
    _UserPlayer->ClearCards();

    _Rightrobot->ResetForNewGame();
    _Leftrobot->ResetForNewGame();
    _UserPlayer->ResetForNewGame();

    // 确保新局不继承上一局的出牌记录/要不起状态
    _Rightrobot->SetPendinfo(nullptr, nullptr);
    _Leftrobot->SetPendinfo(nullptr, nullptr);
    _UserPlayer->SetPendinfo(nullptr, nullptr);

    // 3. 当前局状态清零，避免上一局信息泄漏到新局
    _CurrentPlayer = nullptr;
    _CurrentCards = nullptr;
    _PlayHandCards = nullptr;
    _PlayHandplayer = nullptr;
    _Betrect.rest();
    _Bet = 0;

    emit S_StopCountdown();
}

player *gamecontrol::GetPendplayer()
{
    return _PlayHandplayer;
}

void gamecontrol::StartPrepareLord()
{

    emit S_StopCountdown();
    _CurrentPlayer->PrepareGetLord();
    emit S_PlayerStateChange(_CurrentPlayer,USERSTATE::USERGETLORD);
}

void gamecontrol::BecomeLord(player *player, int Bet)
{
    _CurrentPlayer = player;
    player->setRole(player::LORD);
    player->GetNextPlayer()->setRole(player::FORMAR);
    player->GetPrePlayer()->setRole(player::FORMAR);

    player->StoreGetCards(TakeCards());

    // 发射地主确定信号，包含地主玩家信息
    emit S_LordDetermined(player);

    emit S_StopCountdown();

    QTimer::singleShot(1000, this, [this](){
        emit S_gameStateChange(GIVECARD);
        emit S_PlayerStateChange(_CurrentPlayer, USERPEND);
    });

    _Bet = Bet;
    _CurrentPlayer->PreparePlayCard();
}

void gamecontrol::GamePlayhand(player *player, Cards *cards)
{
    // qDebug() << "=== GamePlayhand ===";
    // qDebug() << "出牌玩家:" << player;
    // qDebug() << "上一个出牌者:" << _PlayHandplayer;
    // qDebug() << "牌数:" << (cards ? cards->GetCardtotal() : 0);

    // 关键修复：正确处理"要不起"的情况
    if (!cards || cards->isempty()) {
        // qDebug() << "玩家要不起";

        // 重要修复：要不起时不更新出牌记录，保持上一个有效出牌者
        // 只有真正出牌时才更新出牌记录

        emit S_StopCountdown();
        // 轮到下个人出牌
        _CurrentPlayer = player->GetNextPlayer();
        // qDebug() << "轮到下一个玩家:" << _CurrentPlayer;

        // 通知界面层刷新“要不起”表现（文字与音效）
        emit S_gamePlayHand(player, cards);

        QTimer::singleShot(1000, this, [this]() {
            if (_CurrentPlayer) {
                _CurrentPlayer->PreparePlayCard();
                emit S_PlayerStateChange(_CurrentPlayer, USERPEND);
            }
        });
        return;
    }

    // qDebug() << "玩家正常出牌";

    // 重要修复：只有真正出牌（不是要不起）时才更新出牌记录
    _PlayHandplayer = player;
    _PlayHandCards = cards;
    _CurrentCards = cards;
    _CurrentPlayer = player;

    // 先从玩家手牌中移除这些牌，保证界面刷新时数量正确
    if (cards && !cards->isempty()) {
        player->RemoveCards(cards);
    }

    emit S_PlayerPlayHand(_PlayHandplayer, _PlayHandCards);
    // 告诉主界面出的牌（此时手牌已更新）
    emit S_gamePlayHand(player, cards);

    emit S_StopCountdown();

    // qDebug() << "玩家剩余牌数:" << player->GetCards().GetCardtotal();

    // 倍数翻倍逻辑
    PlayHand playHand(cards);
    PlayHand::HandType type = playHand.Getplayhandtype();
    // qDebug() << "牌型:" << type;

    if(type == PlayHand::Hand_Bomb) {
        _Bet *= 2;
        // qDebug() << "炸弹！倍数变为:" << _Bet;
    }

    // 检查游戏是否结束
    if(player->GetCards().isempty()) {
        // qDebug() << "游戏结束！";
        if(player->GetRole() == player::LORD)
        {
            player->SetScore(player->GetScore() + 2 * _Bet);
            player->GetPrePlayer()->SetScore(player->GetPrePlayer()->GetScore() - _Bet);
            player->GetNextPlayer()->SetScore(player->GetNextPlayer()->GetScore() - _Bet);
            player->Setwin(true);
            player->GetPrePlayer()->Setwin(false);
            player->GetNextPlayer()->Setwin(false);
        }
        else
        {
            player->SetScore(player->GetScore() + _Bet);
            if(player->GetPrePlayer()->GetRole() == player::LORD)
            {
                player->GetPrePlayer()->SetScore(player->GetPrePlayer()->GetScore() - 2 * _Bet);
                player->GetNextPlayer()->SetScore(player->GetNextPlayer()->GetScore() + _Bet);
                player->Setwin(true);
                player->GetPrePlayer()->Setwin(false);
                player->GetNextPlayer()->Setwin(true);
            }
            else
            {
                player->GetNextPlayer()->SetScore(player->GetNextPlayer()->GetScore() - 2 * _Bet);
                player->GetPrePlayer()->SetScore(player->GetPrePlayer()->GetScore() + _Bet);
                player->Setwin(true);
                player->GetNextPlayer()->Setwin(false);
                player->GetPrePlayer()->Setwin(true);
            }
        }
        const bool isUserWin = (player->GetRole() == _UserPlayer->GetRole());
        emit S_PlayResult(isUserWin);
        emit S_PlayerStateChange(player, USERWIN);
        emit S_StopCountdown();
        return;
    }

    // 轮到下个人出牌
    _CurrentPlayer = player->GetNextPlayer();
    // qDebug() << "轮到下一个玩家:" << _CurrentPlayer;

    QTimer::singleShot(1, this, [this]() {
        if (_CurrentPlayer) {
            _CurrentPlayer->PreparePlayCard();
            emit S_PlayerStateChange(_CurrentPlayer, USERPEND);
        }
    });
}

void gamecontrol::ClearScore()
{
    _Rightrobot->SetScore(0);
    _Leftrobot->SetScore(0);
    _UserPlayer->SetScore(0);

}

int gamecontrol::GetCardcount()
{
    return _Countcard;
}

Cards* gamecontrol::GetAllCards()
{
    return _AllCards;
}

void gamecontrol::Onbet(player *player, int Bet)
{

    // 是否首家叫分，用于区分"叫"和"抢"
    const bool isFirst = (_Betrect.times == 0 && Bet > 0);

    // 更新当前最高分：只有当有人真正叫分且分值更高时才更新
    if(Bet > 0)
    {
        if(_Betrect.Bet == 0 || Bet > _Betrect.Bet)
        {
            _Betrect.Bet = Bet;
            _Betrect._player = player;
        }
    }

    // 通知 UI 玩家本次操作（Bet 原样传递，避免被误判为“不抢”）
    emit S_gamenotifyGetLoard(player, Bet, isFirst);

    _Betrect.times++;

    if(_Betrect.times==3)//触发重新开始
    {

        emit S_StopCountdown();

        if(_Betrect.Bet==0)
        {
            // 全员都没有叫分，重新洗牌发牌
            RetCardDate();
            SetCurrentPlayer(_UserPlayer);
            gamecontrol::S_gameStateChange(PENDCARD);//极端就是发牌
        }
        else
        {
            BecomeLord(_Betrect._player,_Betrect.Bet);
            gamecontrol::S_gameStateChange(GIVECARD);//开始出牌
        }
        _Betrect.rest();
        return;
    }
    _CurrentPlayer=player->GetNextPlayer();
    _CurrentPlayer->PrepareGetLord();//下个玩家开始抢地主
}

player *gamecontrol::GetLeftroot()
{
    return _Leftrobot;
}

player *gamecontrol::GetRightroot()
{
    return _Rightrobot;
}

player *gamecontrol::GetUSer()
{
    return _UserPlayer;
}
// 在 gamecontrol 类中添加


