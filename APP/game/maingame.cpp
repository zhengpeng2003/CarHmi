#include "maingame.h"
#include "qpainter.h"
#include "ui_maingame.h"
// 1
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QTimer>
#include <algorithm>
#include <QPushButton>
//2222
#include "maingame.h"
#include "ui_maingame.h"

#include <QPushButton>
#include <QRandomGenerator>
#include <QPainter>

Maingame::Maingame(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Maingame)
{
    /* ===============================
     * 1️⃣ 窗口基础属性（一定要最前）
     * =============================== */
    this->setFixedSize(480, 272);
    this->setWindowIcon(QIcon(":/images/logo.ico"));
    this->setWindowTitle("欢乐斗地主");

    /* ===============================
     * 2️⃣ UI 初始化（必须）
     * =============================== */
    ui->setupUi(this);

    /* ===============================
     * 3️⃣ 右上角退出按钮（关键）
     * =============================== */
    QWidget *cw = this->centralWidget();   // ⭐ QMainWindow 必须用 centralWidget

    QPushButton *exitBtn = new QPushButton(cw);
    exitBtn->setText("×");
    exitBtn->setFixedSize(32, 32);

    // 右上角（基于 centralWidget）
    exitBtn->move(
        440,
        0
        );

    // 样式
    exitBtn->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(0,0,0,120);"
        "color: white;"
        "border: 1px solid white;"
        "border-radius: 16px;"
        "font-size: 20px;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(255,0,0,160);"
        "}"
        );

    // ⭐ 连接：点击 = 关闭窗口（走 closeEvent）
    connect(exitBtn, &QPushButton::clicked, this, &QWidget::close);

    /* ===============================
     * 4️⃣ 背景图片初始化
     * =============================== */
    int rand = QRandomGenerator::global()->bounded(0, 10) + 1;
    QString imgPath = QString(":/images/background-%1.png").arg(rand);
    _IMage_Map = QPixmap(imgPath);

    /* ===============================
     * 5️⃣ 动画、控制器、场景初始化
     * =============================== */
    _MyAnmation = new AnmationPixmap(this);

    InitGamecontrol();
    InitScore();
    InitCardpanelMap();
    InitGameScene();
    InitGroupbtn();
    InitMovepoint();

    /* ===============================
     * 6️⃣ 出牌计时器
     * =============================== */
    _Timer_PlayHand = new QTimer(this);
    _Movetime = 0;

    connect(_Timer_PlayHand, &QTimer::timeout, this, [=]() {
        _Movetime++;
        PlayHandtimer(_Gamecontrol->GetCurrentPlayer(), _Movetime);
    });

    InitPlayerTimer();

    /* ===============================
     * 7️⃣ 背景音乐
     * =============================== */
    _Bgmcontrol = new Bgmcontrol(this);
    _Bgmcontrol->InitMusicPlayer();
    _Bgmcontrol->StartBgm();
}


void Maingame::InitGamecontrol()
{
    _Gamecontrol=new gamecontrol();
    //玩家初始阿虎
    _Gamecontrol->InitPlayer();
    _Gamecontrol->SetCurrentPlayer(_Gamecontrol->GetUSer());
    _Gamecontrol->SetAllCards();
    player * Leftroot =_Gamecontrol->GetLeftroot();
    player * Rightroot =_Gamecontrol->GetRightroot();
    player * User =_Gamecontrol->GetUSer();
    _Players<< Leftroot<<Rightroot<< User;

    //每当有卡牌来存储的时候就触发更新图片位置
    connect(Leftroot,&player::notifystorecard,this,&Maingame::PendCardplayer);
    connect(Rightroot,&player::notifystorecard,this,&Maingame::PendCardplayer);
    connect(User,&player::notifystorecard,this,&Maingame::PendCardplayer);

    connect(_Gamecontrol,&gamecontrol::S_PlayerStateChange,this,&Maingame::PlayerStateChange);//用户状态
    connect(_Gamecontrol,&gamecontrol::S_gameStateChange,this,&Maingame::SetCurrentGameStatue);//当前游戏状态
    connect(_Gamecontrol,&gamecontrol::S_gamenotifyGetLoard,this,&Maingame::gamenotifyGetLoard);//抢地主阶段
    connect(_Gamecontrol,&gamecontrol::S_gamePlayHand,this,&Maingame::OndisPosePlayhand);//出的牌的信号

    connect(_Gamecontrol, &gamecontrol::S_StopCountdown, this, &Maingame::ResetCountdown);
    connect(_Gamecontrol, &gamecontrol::S_PlayResult, this, [=](bool isWin){
        _Bgmcontrol->playResultBgm(isWin);
    });


    connect(_Gamecontrol, &gamecontrol::S_LordDetermined, this, &Maingame::OnLordDetermined);
}

void Maingame::InitScore()
{
    ui->widget_showscore->InitScore(_Players.at(0)->GetScore(),
                                    _Players.at(1)->GetScore(),
                                    _Players.at(2)->GetScore());
}

void Maingame::SaveLastGameScores()
{
    if(!_Gamecontrol)
    {
        return;
    }

    player* leftRobot = _Gamecontrol->GetLeftroot();
    player* rightRobot = _Gamecontrol->GetRightroot();
    player* userPlayer = _Gamecontrol->GetUSer();

    const int leftRoundScore = leftRobot ? leftRobot->GetScore() : 0;
    const int rightRoundScore = rightRobot ? rightRobot->GetScore() : 0;
    const int userRoundScore = userPlayer ? userPlayer->GetScore() : 0;

    // 先累加本局结算分，再刷新展示，确保连续多局累计有效
    _LastLeftRobotScore += leftRoundScore;
    _LastRightRobotScore += rightRoundScore;
    _LastUserScore += userRoundScore;

    ui->widget_showscore->InitScore(_LastLeftRobotScore, _LastRightRobotScore, _LastUserScore);
}

void Maingame::ResetCountdown()
{
    if(!_Timecount)
    {
        return;
    }

    _Timecount->Reset();
    _Timecount->hide();
}

bool Maingame::IsFreePlayStage() const
{
    if(!_Gamecontrol)
    {
        return false;
    }

    player* pendPlayer = _Gamecontrol->GetPendplayer();
    player* currentPlayer = _Gamecontrol->GetCurrentPlayer();
    return (pendPlayer == nullptr || pendPlayer == currentPlayer);
}

QPoint Maingame::CalculateCenteredPos(const QPoint &anchor, const QSize &labelSize) const
{
    if(anchor.isNull())
    {
        return QPoint();
    }

    return QPoint(anchor.x() - labelSize.width() / 2,
                  anchor.y() - labelSize.height() / 2);
}

QPoint Maingame::CalculateLabelPosAbovePlayArea(_Playercontext* ctx, const QSize &labelSize, int extraOffset) const
{
    if(!ctx)
    {
        return QPoint();
    }

    const int baseOffset = extraOffset >= 0 ? extraOffset : 0;
    QPoint anchor = ctx->_InfoPos;
    if(anchor.isNull())
    {
        anchor = ctx->_PlayerHandRect.center();
    }

    // 始终基于玩家的出牌区域（_PlayerHandRect）向上微调，确保标签位于出牌区域的中心或上方
    anchor.ry() -= baseOffset;

    return CalculateCenteredPos(anchor, labelSize);
}

QPoint Maingame::CalculateRoleLabelPos(_Playercontext* ctx, const QSize &labelSize) const
{
    if(!ctx)
    {
        return QPoint();
    }

    return CalculateCenteredPos(ctx->_RoleImgPos, labelSize);
}

void Maingame::ShowPlayerInfoImage(player *player, const QPixmap &pixmap)
{
    if(!player || pixmap.isNull() || !_Playercontexts.contains(player))
    {
        return;
    }

    _InfoLabelSeq++;

    auto ctx = _Playercontexts[player];
    ctx->_NOCardlabel->setPixmap(pixmap);
    ctx->_NOCardlabel->setFixedSize(pixmap.size());

    const QPoint targetPos = CalculateLabelPosAbovePlayArea(ctx, pixmap.size(), _IMage_Card_Size.height() / 4);
    ctx->_NOCardlabel->move(targetPos);
    ctx->_NOCardlabel->raise();
    ctx->_NOCardlabel->show();
}
//初始化卡牌
void Maingame::InitCardpanelMap()
{
    _IMage_Cards.load(":/images/card.png");
    _IMage_Card_Size.setWidth(_IMage_Cards.width()/13);
    _IMage_Card_Size.setHeight(_IMage_Cards.height()/5);

    _Card_back=_IMage_Cards.copy(_IMage_Card_Size.width()*2,_IMage_Card_Size.height()*4,_IMage_Card_Size.width(),_IMage_Card_Size.height());//背面图像
    for(int i=0;i<13;i++)//点数
    {
        for(int j=0;j<4;j++)//花色
        {
            Card::cardpoint point=(Card::cardpoint)(i+1);
            Card::cardsuit suit=(Card::cardsuit)(j+1);
            Card *TempCard=new Card(suit,point);
            QPixmap Temppixmap=_IMage_Cards.copy(_IMage_Card_Size.width()*i,_IMage_Card_Size.height()*j,
                                                   _IMage_Card_Size.width(),_IMage_Card_Size.height());
            InitCardImage(Temppixmap,_Card_back,TempCard);//插入卡牌 图片显示问题

        }
    }
    QPixmap TemppixmapSJ=_IMage_Cards.copy(0,_IMage_Card_Size.height()*4,
                                             _IMage_Card_Size.width(),_IMage_Card_Size.height());//大王牌
    QPixmap TemppixmapBJ=_IMage_Cards.copy(_IMage_Card_Size.width(),_IMage_Card_Size.height()*4,
                                             _IMage_Card_Size.width(),_IMage_Card_Size.height());//小王牌
    Card *CardSJ=new Card(Card::Suit_Begin,Card::Card_SJ);
    Card *CardBJ=new Card(Card::Suit_Begin,Card::Card_BJ);
    InitCardImage(TemppixmapSJ,_Card_back,CardSJ);
    InitCardImage(TemppixmapBJ,_Card_back,CardBJ);

}

void Maingame::InitCardImage(QPixmap Card_front,QPixmap Card_back,Card *card)//设置卡牌图片初始化
{
    CardPanel * cardpenel=new CardPanel(this);
    cardpenel->setimage(Card_front,Card_back);
    cardpenel->setcard(card);
    cardpenel->hide();//隐藏
    connect(cardpenel,&CardPanel::S_Cardsselect,this,&Maingame::Cardpanel);
    _CardPenelMap.insert(*card,cardpenel);
    _CardPenelMap.size();

}

//
void Maingame::InitGroupbtn()
{
    ui->widget->Initbutton();
    ui->widget->Setbtngroupstate(MybuttonGroup::Start);
    //3个位置
    // 1. 放置玩家扑克牌的区域
    const QRect cardsRect[] =
        {
            // x, y, width, height
            QRect(90, 130, 100, height() - 200),                    // 左侧机器人
            QRect(rect().right() - 190, 130, 100, height() - 200),  // 右侧机器人
            QRect(250, rect().bottom() - 120, width() - 500, 100)   // 当前玩家
        };

    // 2. 玩家出牌的区域
    const QRect playHandRect[] =
        {
            QRect(260, 210, 100, 100),                             // 左侧机器人
            QRect(rect().right() - 360, 210, 100, 100),            // 右侧机器人
            QRect(150, rect().bottom() - 290, width() - 300, 100)  // 当前玩家
        };

    // 3. 玩家头像显示的位置
    const QPoint roleImgPos[] =
        {
            QPoint(cardsRect[0].left() - _IMage_Card_Size.width()+30, cardsRect[0].center().y()),  // 左侧机器人头像
            QPoint(cardsRect[1].right() + _IMage_Card_Size.width() / 4+20, cardsRect[1].center().y()), // 右侧机器人头像
            QPoint(cardsRect[2].right() - _IMage_Card_Size.width() / 2+70,
                   cardsRect[2].top() - _IMage_Card_Size.height() / 2+80) // 当前玩家头像
        };
    // 4.信息提示位置
    const QPoint info[] =
        {
            playHandRect[0].center(),
            playHandRect[1].center(),
            playHandRect[2].center()

        };
    int index=_Players.indexOf(_Gamecontrol->GetUSer());

    for(int i=0; i<3; i++) {
        _Playercontext* tempcontext = new _Playercontext();  // 动态分配，手动管理内存
        tempcontext->Isfront = _Players.at(i)->GetIsfront();
        tempcontext->_Align = (i == index) ? Horizontal : Vertical;
        tempcontext->_PLayerCardsRect = cardsRect[i];
        tempcontext->_PlayerHandRect = playHandRect[i];
        tempcontext->_RoleImgPos = roleImgPos[i];
        tempcontext->_InfoPos = info[i];

        tempcontext->_NOCardlabel = new QLabel(this);
        tempcontext->_NOCardlabel->resize(160, 98);
        tempcontext->_NOCardlabel->move(playHandRect[i].x(),playHandRect[i].y());
        tempcontext->_NOCardlabel->setScaledContents(true);
        tempcontext->_NOCardlabel->hide();

        tempcontext->_ROlelabel = new QLabel(this);
        tempcontext->_ROlelabel->move(roleImgPos[i].x(),roleImgPos[i].y());
        tempcontext->_ROlelabel->hide();
        tempcontext->_ROlelabel->resize(84, 120);

        tempcontext->_Mycards=new Cards();
        _Playercontexts.insert(_Players.at(i), tempcontext);  // 存储指针

    }
    //开始按钮
    connect(ui->widget,&MybuttonGroup::S_Start,this,[=](){
        ui->widget->SetStartButtonVisible(false);
        SetCurrentGameStatue(gamecontrol::PENDCARD);
    });
    //玩家出牌
    connect(ui->widget,&MybuttonGroup::S_PlayHand,this,[=](){

        UserPlayHand();

    });
    //玩家要不起
    connect(ui->widget,&MybuttonGroup::S_NoHand,this,[=](){
        UserNoPlayer();
    });
    connect(ui->widget,&MybuttonGroup::S_GetLord,this,[=](){});
    connect(ui->widget,&MybuttonGroup::S_NoLord,this,[=](){});
    connect(ui->widget,&MybuttonGroup::S_Point,this,[this](int Bet){
        _Gamecontrol->GetCurrentPlayer()->grablordbet(Bet);//发射了一个信号

    });


}
void Maingame::SatrtPend()
{
    ui->widget->SetStartButtonVisible(false);
    _CanSelectCards = false;
    _Movetime = 0;
    ClearSelectedPanels();

    // 每局开始时重置抢分/动画控件到固定位置，避免上一局动画改变坐标
    _MyAnmation->setFixedSize(160, 98);
    _MyAnmation->move((width()-_MyAnmation->width())/2, (height()-_MyAnmation->height())/2-140);
    _MyAnmation->hide();

    // 恢复发牌动画控件的基准位置，确保每局都有完整发牌过程
    if(_PendCards && _MoveCards)
    {
        _PendCards->move(_Base_point);
        _MoveCards->move(_Base_point);
        _PendCards->show();
        _MoveCards->show();
    }
    // 1. 立刻隐藏/清空上一局的所有出牌
    for(auto it = _Playercontexts.begin(); it != _Playercontexts.end(); ++it)
    {
        auto ctx = it.value();

        // 确保指针有效，避免上一局清空后留下空指针导致重新开局崩溃
        if(!ctx->_Last_Cards)
        {
            ctx->_Last_Cards = new Cards();
        }

        if(!ctx->_Last_Cards->isempty())
        {
            QListcard list = ctx->_Last_Cards->Listcardssort();
            for(const Card& c : list)
            {
                auto panel = _CardPenelMap.value(c);
                if(panel) panel->hide();
            }
            ctx->_Last_Cards->clearcards();   // 清空数据
        }
        ctx->_NOCardlabel->hide();            // “不要”标签
    }

    // 2. 关掉闹钟
    ResetCountdown();
    // 开始循环播放发牌音效
    _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::DISPATCH);
    //把所有卡片初始化
    _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::DISPATCH);
    for(auto i =_CardPenelMap.begin();i!=_CardPenelMap.end();i++)
    {

        i.value()->setfront(false);
        i.value()->setselect(false);
    }
    for(auto i=_Playercontexts.begin();i!=_Playercontexts.end();i++)
    {

        i.value()->_Mycards->clearcards();
        i.value()->Isfront=false;
    }

    //开始发牌定时器启动
    _Timer_PlayHand->start(15);
}

void Maingame::PlayHandtimer(player * Player,int Movetime)
{


    QRect TempRect=_Playercontexts.find(Player).value()->_PLayerCardsRect;
    auto x=TempRect.x();
    auto y=TempRect.y();
    int Cardpool=_Gamecontrol->GetCardcount();
    if(Cardpool==3)
    {


        // 发牌结束音乐关闭
        _Bgmcontrol->StopOtherBgm();
        _Timer_PlayHand->stop();
        _MoveCards->hide();
        _PendCards->hide();
        // 1. 取出三张地主牌
        QVector<Card*> lordCards;
        for (int i = 0; i < 3; ++i) {
            Card* card = _Gamecontrol->TakeOneCard();
            if (card) {
                lordCards.append(card);

            }
        }

        // 2. 创建显示面板
        std::sort(lordCards.begin(), lordCards.end(), [](Card* a, Card* b) {
            if (!a || !b) return a < b;
            return a->getcardpoint() < b->getcardpoint();
        });

        for (int i = 0; i < lordCards.size(); ++i) {
            Card* card = lordCards[i];
            CardPanel* lordPanel = new CardPanel(this);

            if (_CardPenelMap.contains(*card)) {
                lordPanel->setimage(_CardPenelMap[*card]->Getimagefont(), _Card_back);
            }

            lordPanel->move((width()-_IMage_Card_Size.width()*5)/2 + i*2*_IMage_Card_Size.width(), 20);
            lordPanel->resize(_IMage_Card_Size.width(), _IMage_Card_Size.height());
            lordPanel->hide();
            lordPanel->setfront(false);
            _LordCards[i] = lordPanel;
        }

        // 3. 统一放回原牌
        for (Card* card : lordCards) {
            _Gamecontrol->GetAllCards()->add(card);
        }

        // qDebug() << "地主牌已放回牌堆";
        SetCurrentGameStatue(gamecontrol::GETLORD);

    }
    if(Movetime<5)//移动的动画
    {

        const int move_size=50;//要固定一个长度
        if(x==_xy[0].x()&&y==_xy[0].y())
            _MoveCards->move(_Base_point.x()-Movetime*move_size,_Base_point.y());
        if(x==_xy[2].x()&&y==_xy[2].y())
            _MoveCards->move(_Base_point.x(),_Base_point.y()+Movetime*move_size);
        if(x==_xy[1].x()&&y==_xy[1].y())
            _MoveCards->move(_Base_point.x()+Movetime*move_size,_Base_point.y());

    }
    else
    {
        Card* drawnCard = _Gamecontrol->TakeOneCard();
        Card tempCard(drawnCard->getcardsuit(), drawnCard->getcardpoint());  // 创建临时对象
        if(!_CardPenelMap.contains(tempCard)) {
            return;
        }
        Player->StoreGetCard(drawnCard);
        PendCardplayer(Player);
        _Movetime=0;
        _Gamecontrol->SetCurrentPlayer(Player->GetNextPlayer());
    }
}

void Maingame::InitMovepoint()
{
    for(int i=0;i<3;i++)
    {

        _xy[i]=QPoint(_Playercontexts.values().at(i)->_PLayerCardsRect.x(),_Playercontexts.values().at(i)->_PLayerCardsRect.y());

    }
}

void Maingame::SetCurrentGameStatue(gamecontrol::GameState state)
{
    _CurrentGameState = state;
    switch(state)
    {
    case gamecontrol::PENDCARD:
        ResetCountdown();
        _CanSelectCards = false;
        _IsUserFirstLordPlay = false;
        _MyAnmation->hide();
        ui->widget->Setbtngroupstate(MybuttonGroup::Null);
        SatrtPend();
        //开始游戏开始音乐
        _Bgmcontrol->StartBgm();
        break;

    case  gamecontrol::GIVECARD://开始出牌
    {    ResetCountdown();
        _CanSelectCards = false;
        ClearSelectedPanels();
        //显示地主牌 把地主牌个地主
        for(int i=0;i<3;i++)
        {
            _LordCards[i]->setfront(true);
        }
        //把所有的标签隐藏 //设置角色
        _MyAnmation->hide();
        const int expectedSeq = _InfoLabelSeq;
        QTimer::singleShot(1500, this, [this, expectedSeq]()
        {
            if(expectedSeq != _InfoLabelSeq)
            {
                return;
            }
            for(auto ctx : _Playercontexts)
            {
                if(ctx && ctx->_NOCardlabel)
                {
                    ctx->_NOCardlabel->clear();
                    ctx->_NOCardlabel->hide();
                }
            }
        });
        // 停止发牌音效（发牌结束）
        _Bgmcontrol->StopOtherBgm();
        break;
    }

    case gamecontrol::GETLORD://叫地主
    {    ResetCountdown();
        _CanSelectCards = false;
        ClearSelectedPanels();
        _MyAnmation->hide();
        _LordCards[0]->show();
        _LordCards[1]->show();
        _LordCards[2]->show();
        ui->widget->Setbtngroupstate(MybuttonGroup::Getloard);
        break;
    }
}
}

void Maingame::PendCardplayer(player *player)
{


    Cards cards=player->GetCards();
    QListcard Listcard=cards.Listcardssort();

    for(auto i=0;i<Listcard.size();i++)
    {
        CardPanel* cardpenl=_CardPenelMap[Listcard[i]];
        if(cardpenl)
        {
            cardpenl->setowner(*player);
            cardpenl->setselect(false);
        }
    }
    PendCardpos(player);//放到卡牌位置
}

void Maingame::PendCardpos(player* player) {
    Cards cards = player->GetCards();
    // 地主手牌展示强制按从小到大排序，便于识别
    QListcard sortedCards = cards.Listcardssort(player->GetRole() == player::LORD ? Cards::ASC : Cards::ASC);
    _Playercontext* context = _Playercontexts[player];
    QRect rect = context->_PLayerCardsRect;
    QRect Panelrect;
    const int cardSpace = 20; //对手手里面的牌进行显示
    if(context->_Align == Horizontal)
    {
        _PanelPositon.clear();
    }

    for(int i = 0; i < sortedCards.size(); i++) {
        CardPanel* panel = _CardPenelMap.value(sortedCards[i]);
        if(!panel) continue;

        if(context->_Align == Horizontal) {
            const int raiseOffset = 10;
            int leftX = rect.left() + (rect.width() - panel->width() - (sortedCards.size() - 1) * cardSpace) / 2;
            int baseTop = rect.top() + (rect.height() - panel->height()) / 2;

            _Mycardsrect = QRect(leftX, baseTop - raiseOffset,
                                 (sortedCards.size() - 1) * cardSpace + panel->width(),
                                 panel->height() + raiseOffset);

            int topY = baseTop;
            if(panel->GetSelect())
            {
                topY -= raiseOffset;
            }
            panel->move(leftX + i * cardSpace, topY);
            panel->setfront(true);//自己的牌要显示出来
            QRect temp(leftX + i * cardSpace, topY, panel->width(), panel->height());
            _PanelPositon.insert(panel, temp);
        }
        else {
            // 垂直布局
            if(context->Isfront)
                panel->setfront(true);
            else
                panel->setfront(false);
            int leftX = rect.left() + (rect.width() - panel->width()) / 2;
            int topY = rect.top() + (rect.height() - panel->height() - (sortedCards.size() - 1) * cardSpace) / 2;
            panel->move(leftX, topY + i * cardSpace);

        }

        panel->raise();//控件升至顶端
        panel->show();
    }

    const int Playhandspace = 20;

    // 打出的牌 - 重要修复：只有当有有效出牌时才显示
    if(_Playercontexts.find(player).value()->_Last_Cards != nullptr &&
        !_Playercontexts.find(player).value()->_Last_Cards->isempty())
    {
        QRect Location = _Playercontexts.find(player).value()->_PlayerHandRect;//出牌位置
        QListcard list = _Playercontexts.find(player).value()->_Last_Cards->Listcardssort();

        for(int i = 0; i < list.size(); i++)
        {
            CardPanel *tempPanel = _CardPenelMap.value(list.at(i));//找打卡牌信息
            if(tempPanel)
            {
                tempPanel->setfront(true);
                if(_Playercontexts.find(player).value()->_Align == Horizontal)//水平
                {
                    int x = (Location.width() - list.size() * Playhandspace - 1 + tempPanel->width()) / 2 + i * Playhandspace;
                    int y = (Location.height() - tempPanel->height()) / 2;
                    tempPanel->move(Location.left() + x-40, Location.top() + y);
                }
                else//竖直
                {
                    int x = (Location.width() - list.size() * Playhandspace - 1 + tempPanel->width()) / 2 + i * Playhandspace;
                    int y = (Location.height() - tempPanel->height()) / 2;
                    tempPanel->move(Location.left() - 50 + x, Location.top() + y + 50);

                }
                tempPanel->raise();//控件升至顶端
                tempPanel->show();
            }
        }
    }
}

void Maingame::OndisPosePlayhand(player *player, Cards *cards)
{
    // qDebug() << "OndisPosePlayhand - 玩家:" << player << "牌数:" << (cards ? cards->GetCardtotal() : 0);

    // 隐藏上轮玩家出的牌
    HidePlayhand(player);

    // 存储新一轮玩家出的牌
    _Playercontexts.find(player).value()->_Last_Cards = cards;

    // 处理"要不起"的情况
    if(!cards || cards->isempty())
    {
        // qDebug() << "玩家要不起";
        _InfoLabelSeq++;
        QPixmap passPixmap(":/images/pass.png");
        if(passPixmap.isNull())
        {
            qWarning() << "pass.png 资源加载失败";
        }
        else
        {
            auto noCardLabel = _Playercontexts.find(player).value()->_NOCardlabel;
            noCardLabel->setPixmap(passPixmap);
            noCardLabel->setFixedSize(passPixmap.size());
            const QPoint targetPos = CalculateLabelPosAbovePlayArea(_Playercontexts.find(player).value(), passPixmap.size(), _IMage_Card_Size.height() / 4);
            noCardLabel->move(targetPos);
            noCardLabel->raise();
            noCardLabel->show();
        }

        // 添加"不要"音效
        _Bgmcontrol->NoPlayerHandBgm(player->GetSex());
    }
    else
    {
        // 正常出牌的动画特效
        PlayHand temp(cards);
        PlayHand::HandType type = temp.Getplayhandtype();
        // qDebug() << "正常出牌，牌型:" << type;

        // 添加出牌音效
        bool isFirst = (_Gamecontrol->GetPendplayer() == nullptr || _Gamecontrol->GetPendplayer() == player);
        _Bgmcontrol->PlayeHandBgm(player->GetSex(), isFirst, cards);

        // 原有的动画逻辑
        Showanimation(type);
    }

    // 更新玩家手里牌的数量
    PendCardpos(player);
}

void Maingame::HidePlayhand(player *player)
{
    auto it = _Playercontexts.find(player);
    if(it != _Playercontexts.end())
    {
        // 重要修复：检查_Last_Cards是否为空，而不是检查它是否指向空牌
        if(it.value()->_Last_Cards == nullptr || it.value()->_Last_Cards->isempty())
        {
            // 没有出牌说明上轮出现了不出的标签
            it.value()->_NOCardlabel->hide();
        }
        else
        {
            // 找到上轮出的牌然后隐藏
            Cards *tempcards = it.value()->_Last_Cards;
            QListcard list = tempcards->Listcardssort();
            for(int i = 0; i < list.size(); i++)
            {
                auto cardPanel = _CardPenelMap.find(list.at(i));
                if(cardPanel != _CardPenelMap.end())
                {
                    cardPanel.value()->hide();
                }
            }
        }

        // 重要修复：清空上一轮的出牌记录
        it.value()->_Last_Cards = nullptr;
    }
}

void Maingame::PlayerStateChange(player *player, gamecontrol::USERSTATE state)
{

    switch (state)
    {
    case gamecontrol::USERGETLORD://抢地主
        _CanSelectCards = false;
        ClearSelectedPanels();
        if(player == _Gamecontrol->GetUSer())
        {
            _Gamecontrol->StartPrepareLord();//开始抢地主并且发送信号
        }
        break;

    case gamecontrol::USERPEND:
        _CanSelectCards = (player == _Gamecontrol->GetUSer());
        if(!_CanSelectCards)
        {
            ClearSelectedPanels();
        }
        if(player == _Gamecontrol->GetUSer())//当玩家出牌时的2种情况
        {

            // 情况1：没有出牌记录 或 上一轮是自己出的牌
            if(_Gamecontrol->GetPendplayer() == nullptr || _Gamecontrol->GetPendplayer() == player)
            {
                // qDebug() << "切换为自由出牌模式";
                ui->widget->Setbtngroupstate(MybuttonGroup::PlayCardfirst);
            }
            else
            {
                // qDebug() << "切换为压牌模式";
                ui->widget->Setbtngroupstate(MybuttonGroup::PlayCard);
            }
        }
        else
        {
            ui->widget->Setbtngroupstate(MybuttonGroup::Null);
        }
        HidePlayhand(player);
        break;

    default:
    {
        // qDebug()<<"游戏结束";
        // 机器人手牌亮牌
        ShowRobotHands();

        // 所有牌正面展示，直至下一局开始前都保持亮牌
        for(auto ctx : _Playercontexts)
        {
            if(ctx)
            {
                ctx->Isfront = true;
            }
        }
        for(auto * p : _Players)
        {
            PendCardpos(p);
        }
        SaveLastGameScores();
        ResetCountdown();      // ← 加
        // qDebug()<<"分数初始化";

        // 添加结束音效
        InitEndPanel(_Gamecontrol->GetUSer());
    }
    }
}

void Maingame::gamenotifyGetLoard(player *player, int Bet, bool first)
{
    // 先显示动画
    QPixmap statusPixmap;
    if(first)
    {
        statusPixmap.load(":/images/jiaodizhu.png");
    }
    else if(!first && Bet > 0)
    {
        statusPixmap.load(":/images/qiangdizhu.png");
    }
    else
    {
        statusPixmap.load(":/images/buqinag.png");
    }

    ShowPlayerInfoImage(player, statusPixmap);

    // 显示下注动画
    const int maxBet = _Gamecontrol->GetCurrentMaxBet();
    if(maxBet > 0)
    {
        _MyAnmation->ShowBet(maxBet);
        _MyAnmation->resize(160,98);
        _MyAnmation->move((width()-_MyAnmation->width())/2,(height()-_MyAnmation->height())/2-140);
        _MyAnmation->show();
    }
    else
    {
        _MyAnmation->hide();
    }

    ui->widget->Setbtngroupstate(MybuttonGroup::Null);

    // 确保音效播放 - 添加调试信息
    // qDebug() << "抢地主音效 - 玩家:" << player << "下注:" << Bet << "是否首家:" << first;
    _Bgmcontrol->GetlordBgm(Bet, player->GetSex(), first);
}

void Maingame::Cardpanel(Qt::MouseButton event)
{
    // 1. 安全检查
    if (!_Gamecontrol) return;

    // 选牌仅允许在出牌阶段且轮到用户
    if (!_CanSelectCards || _CurrentGameState != gamecontrol::GIVECARD)
    {
        return;
    }

    // 2. 判断是否是玩家
    if (_Gamecontrol->GetCurrentPlayer() == _Gamecontrol->GetLeftroot() ||
        _Gamecontrol->GetCurrentPlayer() == _Gamecontrol->GetRightroot()) {
        return;
    }
    // 3. 安全类型转换
    CardPanel *temp = qobject_cast<CardPanel*>(sender());
    if (!temp) return;

    // 4. 检查卡片所有者
    if (temp->getowner() == _Gamecontrol->GetLeftroot() ||
        temp->getowner() == _Gamecontrol->GetRightroot()) {
        return;
    }

    // 5. 修复比较运算符
    if (event == Qt::LeftButton) {
        temp->setselect(!temp->GetSelect());
        PendCardpos(temp->getowner());

        auto it = _SelcetPanel.find(temp);
        if (it == _SelcetPanel.end()) {
            _SelcetPanel.insert(temp);
            // 添加选牌音效
            _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::SELECT_CARD);
        } else {
            _SelcetPanel.erase(it);
            temp->setselect(false);
        }
    }
}

void Maingame::InitEndPanel(player *player)
{
    EndPanel *E1=new EndPanel(player,this);
    E1->move((width()-E1->width())/2,-E1->height());
    E1->show();//先显示出来 动画效果更好
    QPropertyAnimation *anim = new QPropertyAnimation(E1, "pos", this);
        anim->setDuration(1500);
        anim->setStartValue(QPoint((width()-E1->width())/2,-E1->height()));
        anim->setEndValue(QPoint((width()-E1->width())/2,(height()-E1->height())/2));
        anim->setEasingCurve(QEasingCurve::OutBounce);
        anim->start();
        QTimer::singleShot(100, [E1]() {
            E1->raise();
            E1->activateWindow();
        });
        connect(E1,&EndPanel::S_Continue,this,[=](){
            E1->hide();
            E1->deleteLater();   // 必须释放
            anim->deleteLater(); // 必须释放

            ResetCountdown();

            // 清理上一局亮出的机器人手牌
            ClearRobotHands();

            // 完全重置游戏数据
            _Gamecontrol->RetCardDate();
        _Gamecontrol->SetCurrentPlayer(_Gamecontrol->GetUSer());
        _Gamecontrol->SetCurrentCards(nullptr);
        _Gamecontrol->ClearScore();           // 清空分数
        _Bgmcontrol->StopBgm();
        _Bgmcontrol->StopOtherBgm();

        // 清空所有界面牌
        for(auto it=_CardPenelMap.begin();it!=_CardPenelMap.end();++it)
            it.value()->hide();

        // 清空上下文
        for(auto it=_Playercontexts.begin();it!=_Playercontexts.end();++it){
            if(it.value()->_Last_Cards){
                it.value()->_Last_Cards->clearcards();
            }
            else{
                it.value()->_Last_Cards = new Cards();
            }
            it.value()->_NOCardlabel->hide();
            it.value()->_ROlelabel->hide();
            it.value()->Isfront=false;
        }

        _InfoLabelSeq = 0;

        _SelcetPanel.clear();

        // 重新发牌
        SetCurrentGameStatue(gamecontrol::PENDCARD);
        ui->widget->Setbtngroupstate(MybuttonGroup::Start);
    },{Qt::QueuedConnection}); // 防止递归
    for (auto it = _CardPenelMap.begin(); it != _CardPenelMap.end(); ++it) {
        it.value()->hide();
    }
}
void Maingame::UserPlayHand()
{
    //判断当前游戏状态
    if(_Gamecontrol->GetCurrentPlayer() != _Gamecontrol->GetUSer())
    {
        return;
    }
    //判断打出的牌是否为空
    if(_SelcetPanel.isEmpty())
    {
        bool isFreePlay = IsFreePlayStage();
        if(isFreePlay)
        {
            Cards userCards = _Gamecontrol->GetUSer()->GetCards();
            QListcard list = userCards.Listcardssort();
            if(!list.isEmpty())
            {
                CardPanel *firstPanel = _CardPenelMap.value(list.first(), nullptr);
                if(firstPanel)
                {
                    firstPanel->setselect(true);
                    _SelcetPanel.insert(firstPanel);
                }
            }
        }

        if(_SelcetPanel.isEmpty())
        {
            return;
        }
    }

    // qDebug() << "=== 玩家出牌验证 ===";
    //判断打出的牌合不合理
    Cards *temp = new Cards();

    for(auto i = _SelcetPanel.begin(); i != _SelcetPanel.end(); i++)
    {
        temp->add((*i)->getcard());
    }

    PlayHand playHand(temp);
    PlayHand::HandType type = playHand.Getplayhandtype();
    // qDebug() << "牌型识别:" << type;

    // 使用修复后的判断逻辑
    player* lastPlayer = _Gamecontrol->GetPendplayer();
    player* currentPlayer = _Gamecontrol->GetCurrentPlayer();

    // qDebug() << "上一个出牌者:" << lastPlayer;
    // qDebug() << "当前玩家:" << currentPlayer;

    // 情况1：首家出牌或自由出牌（上一轮也是自己出的牌）
    if(lastPlayer == nullptr || lastPlayer == currentPlayer)
    {
        // qDebug() << "自由出牌模式";
        if(type == PlayHand::Hand_Unknown) {
            // qDebug() << "牌型无效";
            delete temp;
            return;
        }
    }
    // 情况2：需要压其他玩家的牌
    else
    {
        // qDebug() << "压牌模式";
        Cards* lastCards = _Gamecontrol->GetCurrentCards();
        // qDebug() << "上家出牌数量:" << (lastCards ? lastCards->GetCardtotal() : 0);

        if(lastCards) {
            PlayHand lastPlayHand(lastCards);
            bool canBeat = playHand.CanBeat(lastPlayHand);
            // qDebug() << "能否压住上家:" << canBeat;

            if(!canBeat) {
                // qDebug() << "不能压住上家的牌";
                delete temp;
                return;
            }
        }
    }

    // qDebug() << "出牌有效，开始处理";


    //触发出牌
    _Gamecontrol->GetCurrentPlayer()->PlayHand(temp);

    HandleUserPlaySuccess();

    // 在UserPlayHand()最后加：
    ClearSelectedPanels();          // 必须清空选中
    PendCardpos(_Gamecontrol->GetUSer()); // 立即刷新手牌显示
}

void Maingame::UserNoPlayer()
{
    // qDebug() << "=== 玩家要不起 ===";

    // 判断当前玩家
    if(_Gamecontrol->GetCurrentPlayer() != _Gamecontrol->GetUSer())
    {
        // qDebug() << "不是当前玩家回合";
        return;
    }

    // 安全检查：不能首家要不起
    if(_Gamecontrol->GetPendplayer() == nullptr)
    {
        // qDebug() << "首家不能要不起";
        return;
    }

    // 安全检查：不能连续要不起
    if(_Gamecontrol->GetPendplayer() == _Gamecontrol->GetUSer())
    {
        // qDebug() << "不能连续要不起";
        return;
    }

    // qDebug() << "玩家选择要不起";

    // 清空所有选中的牌
    ClearSelectedPanels();

    // 创建一个空的Cards对象来表示"要不起"
    Cards* emptyCards = new Cards();  // 空的卡牌集合

    // 触发出牌（传递空的卡牌集合）
    _Gamecontrol->GetCurrentPlayer()->PlayHand(emptyCards);

    // 再次确保清空选中容器
    ClearSelectedPanels();

    // 更新手牌显示
    PendCardpos(_Gamecontrol->GetUSer());

    // qDebug() << "=== 要不起处理完成 ===";
    ui->widget->Setbtngroupstate(MybuttonGroup::Null);
}

void Maingame::AutoPlayFirstCard()
{
    if(_Gamecontrol->GetCurrentPlayer() != _Gamecontrol->GetUSer())
    {
        return;
    }

    Cards userCards = _Gamecontrol->GetUSer()->GetCards();
    QListcard list = userCards.Listcardssort();

    if(list.isEmpty())
    {
        // qDebug() << "自动出牌失败：手牌为空";
        return;
    }

    // qDebug() << "自由出牌超时，自动打出第一张牌";

    Cards *autoCards = new Cards();
    autoCards->add(list.first());

    _Gamecontrol->GetCurrentPlayer()->PlayHand(autoCards);

    ClearSelectedPanels();
    PendCardpos(_Gamecontrol->GetUSer());
}

void Maingame::HandleUserPlaySuccess()
{
    ResetCountdown();

    if(_IsUserFirstLordPlay)
    {
        _IsUserFirstLordPlay = false;
    }
}

void Maingame::ClearSelectedPanels()
{
    for (CardPanel* panel : _SelcetPanel)
    {
        if(panel)
        {
            panel->setselect(false);
        }
    }
    _SelcetPanel.clear();
}

CardPanel* Maingame::PanelFromPos(const QPoint &pos) const
{
    QList<CardPanel*> panels = _PanelPositon.keys();

    // 由于卡牌存在重叠，需要按照屏幕从左到右的顺序反向命中，
    // 以确保与视觉层级一致（右侧/后添加的牌位于最上层）。
    std::sort(panels.begin(), panels.end(), [this](CardPanel *lhs, CardPanel *rhs)
    {
        return _PanelPositon.value(lhs).left() < _PanelPositon.value(rhs).left();
    });

    for(auto it = panels.crbegin(); it != panels.crend(); ++it)
    {
        CardPanel *panel = *it;
        if(_PanelPositon.value(panel).contains(pos))
        {
            return panel;
        }
    }

    return nullptr;
}

void Maingame::ShowRobotHands()
{
    _RobotRevealPanels.clear();

    if(!_Gamecontrol)
    {
        return;
    }

    for(player* p : _Players)
    {
        if(!p || p == _Gamecontrol->GetUSer())
        {
            continue;
        }

        auto ctx = _Playercontexts.value(p, nullptr);
        if(!ctx)
        {
            continue;
        }

        ctx->Isfront = true;
        QListcard cards = p->GetCards().Listcardssort();
        for(const Card &c : cards)
        {
            CardPanel* panel = _CardPenelMap.value(c, nullptr);
            if(panel)
            {
                panel->setfront(true);
                panel->raise();
                panel->show();
                _RobotRevealPanels.append(panel);
            }
        }

        PendCardpos(p);
    }
}

void Maingame::ClearRobotHands()
{
    for(CardPanel* panel : _RobotRevealPanels)
    {
        if(panel)
        {
            panel->setfront(false);
            panel->hide();
        }
    }

    if(_Gamecontrol)
    {
        for(player* p : _Players)
        {
            if(p && p != _Gamecontrol->GetUSer())
            {
                auto ctx = _Playercontexts.value(p, nullptr);
                if(ctx)
                {
                    ctx->Isfront = false;
                }
            }
        }
    }

    _RobotRevealPanels.clear();
}
void Maingame::RePlayGame()
{
    PlayerStateChange(_Gamecontrol->GetUSer(),gamecontrol::USERGETLORD);

}

void Maingame::InitPlayerTimer()
{

    _Timecount=new Timecount(this);
    _Timecount->move((width()-_Timecount->width())/2,(height()-_Timecount->height())/2+100);
    //时间到了
    connect(_Timecount,&Timecount::S_TimeOUt,this,[=](){
        if(_Gamecontrol->GetCurrentPlayer() != _Gamecontrol->GetUSer())
        {
            ResetCountdown();
            return;
        }

        const bool isFreePlay = IsFreePlayStage();
        const bool isFirstLordTurn = _IsUserFirstLordPlay;

        if(isFreePlay || isFirstLordTurn)
        {
            AutoPlayFirstCard();
            if(isFirstLordTurn)
            {
                _IsUserFirstLordPlay = false;
            }
        }
        else
        {
            UserNoPlayer();
        }

        ResetCountdown();

    });
    //音乐提示了 - 修改这里添加倒计时提醒音效
    connect(_Timecount,&Timecount::S_Timemusic,this,[=]()
            {
                _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::ALERT);
            });
    //准备出牌触发信号
    connect(_Gamecontrol->GetUSer(),&player::notifyTime,this,[=](){
        if(_CurrentGameState != gamecontrol::GIVECARD)
        {
            return;
        }

        if(_Gamecontrol->GetUSer()==_Gamecontrol->GetCurrentPlayer())
        {
            // qDebug()<<"触发信号";
            ResetCountdown();
            _Timecount->Start();
            _Timecount->show();

        }


    });


}

void Maingame::Showanimation(PlayHand::HandType type)
{
    switch(type)
    {
    case(PlayHand::Hand_Seq_Pair):
    case(PlayHand::Hand_Seq_Sim):
        _MyAnmation->ShowSimsqe(type);
        _MyAnmation->setFixedSize(250,150);
        _MyAnmation->move((width()-_MyAnmation->width())/2,200);
        _MyAnmation->show();
        break;
    case(PlayHand::Hand_Bomb):
    case(PlayHand::Hand_Bomb_Jokers):
        _MyAnmation->ShowBom(type);
        _MyAnmation->setFixedSize(250,200);
        _MyAnmation->move((width()-_MyAnmation->width())/2,(height()-_MyAnmation->height())/2-70);
        _MyAnmation->show();

        // 添加炸弹音效
        _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::BOMB);
        break;
    case(PlayHand::Hand_Plane):
    case(PlayHand::Hand_Plane_Two_Single):
    case(PlayHand::Hand_Plane_Two_Pair):
        _MyAnmation->ShowPlane();
        _MyAnmation->setFixedSize(800,75);
        _MyAnmation->move((width()-(_MyAnmation->width())/2)/2,200);
        _MyAnmation->show();

        // 添加飞机音效
        _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::PLANE);
        break;
    default:
        break;
    }
    return;
}



//卡牌位置的显示
void Maingame::InitGameScene()
{
    _PendCards=new CardPanel(this);
    _PendCards->resize(_IMage_Card_Size.width(),_IMage_Card_Size.height());
    _PendCards->setimage(_Card_back,_Card_back);

    _MoveCards=new CardPanel(this);
    _MoveCards->setimage(_Card_back,_Card_back);
    _MoveCards->resize(_IMage_Card_Size.width(),_IMage_Card_Size.height());

    _PendCards->move(width()/2-_IMage_Card_Size.width()/2,height()/2-100);
    _MoveCards->move(width()/2-_IMage_Card_Size.width()/2,height()/2-100);
    _Base_point=QPoint(width()/2-_IMage_Card_Size.width()/2,height()/2-100);

    _PendCards->show();


}
Maingame::~Maingame()
{
    if(_Bgmcontrol) {
        _Bgmcontrol->StopBgm();
        _Bgmcontrol->StopOtherBgm();
    }
    delete ui;
}

void Maingame::paintEvent(QPaintEvent *event)
{

    QPainter p1(this);
    p1.drawPixmap(rect(),_IMage_Map);

}

void Maingame::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        // 只有在用户回合且处于出牌阶段才允许拖拽选牌，避免机器人回合产生音效
        if(!_CanSelectCards || _CurrentGameState != gamecontrol::GIVECARD ||
           !_Gamecontrol || _Gamecontrol->GetCurrentPlayer() != _Gamecontrol->GetUSer())
        {
            return;
        }

        // 放宽手牌区域的判定，轻微滑出不再立即清空选中
        QRect tolerantRect = _Mycardsrect.adjusted(-10, -10, 10, 10);
        if(!tolerantRect.contains(event->pos()))
        {
            _IsDraggingSelect = false;
            _CurrtPanel = nullptr;
            return;
        }

        if(!_IsDraggingSelect)
        {
            _IsDraggingSelect = true;
            _CurrtPanel = PanelFromPos(event->pos());
            return;
        }

        CardPanel *temp = PanelFromPos(event->pos());
        if(temp && _CurrtPanel != temp)
        {
            temp->Click();
            _CurrtPanel = temp;
        }
    }
    else if(event->buttons() & Qt::RightButton)
    {
        UserPlayHand();
    }

}

void Maingame::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        // 只有在当前轮到用户并且在出牌阶段时才允许开始拖拽选牌
        if(!_CanSelectCards || _CurrentGameState != gamecontrol::GIVECARD ||
           !_Gamecontrol || _Gamecontrol->GetCurrentPlayer() != _Gamecontrol->GetUSer())
        {
            _IsDraggingSelect = false;
            return;
        }

        QRect tolerantRect = _Mycardsrect.adjusted(-10, -10, 10, 10);
        if(!tolerantRect.contains(event->pos()))
        {
            _IsDraggingSelect = false;
            return;
        }

        _IsDraggingSelect = true;
        _CurrtPanel = PanelFromPos(event->pos());
    }
    else if(event->button() == Qt::RightButton)
    {
        UserPlayHand();
    }

    QMainWindow::mousePressEvent(event);
}

void Maingame::mouseReleaseEvent(QMouseEvent *event)
{
    _IsDraggingSelect = false;
    _CurrtPanel = nullptr;
    QMainWindow::mouseReleaseEvent(event);
}

void Maingame::closeEvent(QCloseEvent *event)
{
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("退出游戏"),
        tr("确认退出游戏？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
void Maingame::OnLordDetermined(player* lordPlayer)
{
    // qDebug() << "地主确定:" << lordPlayer;

    _IsUserFirstLordPlay = (lordPlayer == _Gamecontrol->GetUSer());

    // 更新所有玩家的角色头像
    for(int i = 0; i < 3; i++)
    {
        player* currentPlayer = _Players[i];
        QPixmap RolePix = currentPlayer->GetPlayerRolePixmap(
            currentPlayer->GetRole(),  // 使用玩家当前的真实角色
            currentPlayer->GetSex(),
            currentPlayer->GetLocation()
            );
        auto ctx = _Playercontexts.find(currentPlayer).value();
        ctx->_ROlelabel->setPixmap(RolePix);
        ctx->_ROlelabel->setFixedSize(RolePix.size());
        const QPoint targetPos = CalculateRoleLabelPos(ctx, RolePix.size());
        ctx->_ROlelabel->move(targetPos);
        ctx->_ROlelabel->raise();
        ctx->_ROlelabel->show();

        // qDebug() << "设置玩家角色头像 - 玩家:" << currentPlayer
                 //<< "角色:" << currentPlayer->GetRole();
    }
}
