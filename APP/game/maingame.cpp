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
#include <QVBoxLayout>
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
    InitUiLayers();

    /* ===============================
     * 3️⃣ 右上角退出按钮（关键）
     * =============================== */
    QWidget *cw = ui->centralwidget;

    QPushButton *exitBtn = new QPushButton(cw);
    exitBtn->setText("×");
    exitBtn->setFixedSize(24, 24);
    exitBtn->move(width() - exitBtn->width() - 8, 0);

    // 样式
    exitBtn->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(0,0,0,120);"
        "color: white;"
        "border: 1px solid white;"
        "border-radius: 12px;"
        "font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(255,0,0,160);"
        "}"
        );
    exitBtn->raise();

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
    _MyAnmation = new AnmationPixmap(ui->overlayLayer);
    _MyAnmation->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    InitGamecontrol();
    InitScore();
    InitCardpanelMap();
    ApplyLayoutZones();
    InitGameScene();
    InitGroupbtn();
    InitMovepoint();

    /* ===============================
     * 6️⃣ 出牌计时器
     * =============================== */
    _Timer_PlayHand = new QTimer(this);
    _DispatchLayoutTimer = new QTimer(this);
    _DispatchLayoutTimer->setSingleShot(true);
    _Movetime = 0;

    connect(_Timer_PlayHand, &QTimer::timeout, this, [=]() {
        _Movetime++;
        PlayHandtimer(_Gamecontrol->GetCurrentPlayer(), _Movetime);
    });
    connect(_DispatchLayoutTimer, &QTimer::timeout, this, [this]() {
        FlushPendingLayouts();
    });

    InitPlayerTimer();

    /* ===============================
     * 7️⃣ 背景音乐
     * =============================== */
    _Bgmcontrol = new Bgmcontrol(this);
    _Bgmcontrol->InitMusicPlayer();
    _Bgmcontrol->StartBgm();
    UpdateHudState(QStringLiteral("待开始"));
    ui->widget->Setbtngroupstate(MybuttonGroup::Null);
    ShowStartOverlay(true);
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
    ui->widget_showscore->SetMultiplier(1);
    ui->widget_showscore->SetLordText(QStringLiteral("未定"));
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

void Maingame::InitUiLayers()
{
    ui->centralwidget->setAttribute(Qt::WA_StyledBackground, true);
    ui->centralwidget->setStyleSheet("background: transparent;");
    ui->tableRoot->setAttribute(Qt::WA_StyledBackground, true);
    ui->tableRoot->setStyleSheet("background: transparent;");
    ui->tableLayer->setAttribute(Qt::WA_StyledBackground, true);
    ui->tableLayer->setStyleSheet("background: transparent;");
    ui->overlayLayer->setAttribute(Qt::WA_StyledBackground, true);
    ui->overlayLayer->setStyleSheet("background: transparent;");
    ui->overlayLayer->setMouseTracking(true);

    ui->widget->setParent(ui->overlayLayer);
    ui->widget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    ui->widget->hide();
    ui->tableLayer->lower();
    ui->overlayLayer->raise();
    ui->widget_showscore->raise();

    InitStartOverlay();
}

void Maingame::InitStartOverlay()
{
    _StartOverlay = new QWidget(ui->overlayLayer);
    _StartOverlay->setAttribute(Qt::WA_StyledBackground, true);
    _StartOverlay->setStyleSheet("background: rgba(4, 18, 16, 105);");

    _StartPanel = new QWidget(_StartOverlay);
    _StartPanel->setAttribute(Qt::WA_StyledBackground, true);
    _StartPanel->setStyleSheet(
        "background: rgba(8, 28, 24, 190);"
        "border: 1px solid rgba(255,255,255,55);"
        "border-radius: 12px;");

    _StartTitle = new QLabel(QStringLiteral("欢乐斗地主"), _StartPanel);
    _StartTitle->setAlignment(Qt::AlignCenter);
    _StartTitle->setStyleSheet("color: rgb(244, 241, 226); font-size: 18px; font-weight: 700; background: transparent;");

    _StartSubtitle = new QLabel(QStringLiteral("小屏牌桌模式"), _StartPanel);
    _StartSubtitle->setAlignment(Qt::AlignCenter);
    _StartSubtitle->setStyleSheet("color: rgba(244, 241, 226, 180); font-size: 10px; background: transparent;");

    _StartButton = new Mybutton(_StartPanel);
    _StartButton->InitMybutton(":/images/start-1.png",":/images/start-2.png",":/images/start-3.png");
    _StartButton->setFixedSize(86, 34);
    connect(_StartButton, &QPushButton::clicked, this, [this]() {
        ShowStartOverlay(false);
        SetCurrentGameStatue(gamecontrol::PENDCARD);
    });
}

void Maingame::ShowStartOverlay(bool visible)
{
    if(!_StartOverlay)
    {
        return;
    }

    _StartOverlay->setVisible(visible);
    if(visible)
    {
        _StartOverlay->raise();
        if(ui && ui->widget)
        {
            ui->widget->hide();
        }
    }
}

void Maingame::UpdateHudState(const QString &phaseText)
{
    if(!ui || !ui->widget_showscore || !_Gamecontrol)
    {
        return;
    }

    ui->widget_showscore->SetPhaseText(phaseText);
    ui->widget_showscore->SetMultiplier(_Gamecontrol->GetCurrentBet());

    player *lordPlayer = nullptr;
    for(player *p : _Players)
    {
        if(p && p->GetRole() == player::LORD)
        {
            lordPlayer = p;
            break;
        }
    }
    ui->widget_showscore->SetLordText(lordPlayer ? PlayerShortName(lordPlayer) : QStringLiteral("未定"));
}

QString Maingame::PlayerShortName(player *player) const
{
    if(!_Gamecontrol || !player)
    {
        return QStringLiteral("-");
    }
    if(player == _Gamecontrol->GetUSer())
    {
        return QStringLiteral("我");
    }
    if(player == _Gamecontrol->GetLeftroot())
    {
        return QStringLiteral("A");
    }
    if(player == _Gamecontrol->GetRightroot())
    {
        return QStringLiteral("B");
    }
    return QStringLiteral("?");
}

QPixmap Maingame::ScalePixmapToFit(const QPixmap &pixmap, const QSize &maxSize) const
{
    if(pixmap.isNull() || !maxSize.isValid())
    {
        return pixmap;
    }
    return pixmap.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void Maingame::UpdateUserActionUi()
{
    if(!ui || !ui->widget || !_Gamecontrol)
    {
        return;
    }

    const bool isUserTurn = (_CurrentGameState == gamecontrol::GIVECARD &&
                             _Gamecontrol->GetCurrentPlayer() == _Gamecontrol->GetUSer());
    const bool canPlay = (!_SelcetPanel.isEmpty() || IsFreePlayStage());
    ui->widget->SetPlayButtonEnabled(isUserTurn && canPlay);
}

Maingame::LayoutZones Maingame::BuildLayoutZones() const
{
    LayoutZones zones;
    zones.tableRect = ui->tableRoot->rect();

    // ========== 基础区域定义（480x272 小屏坐标方案）==========
    // 注意：所有坐标相对于 tableRoot，tableRoot 从 y=24 开始
    
    // 中央牌桌区域（底牌/公共牌区）
    zones.centerBoardRect = QRect(152, 42, 176, 40);
    
    // 玩家手牌区（本地玩家手牌）
    zones.userHandRect = QRect(18, 176, 444, 68);
    
    // 动作按钮栏（出牌、过牌、抢地主等按钮）
    zones.actionBarRect = QRect(130, 142, 220, 34);
    
    // 开始面板（开始游戏、设置等）
    zones.startPanelRect = QRect(140, 68, 200, 80);
    
    // 地主三张底牌区域
    zones.lordCardsRect = QRect(174, 4, 132, 40);
    
    // 倒计时区域
    zones.countdownRect = QRect(218, 144, 44, 44);
    
    // 左侧紧凑信息区（玩家简要信息）
    zones.leftCompactRect = QRect(16+40, 20, 26, 130);
    
    // 右侧紧凑信息区（玩家简要信息）
    zones.rightCompactRect = QRect(438-40, 20, 26, 130);
    
    // 左侧出牌区（左边机器人打出的牌）
    zones.leftPlayRect = QRect(8+80, 62, 106, 42);
    
    // 右侧出牌区（右边机器人打出的牌）
    zones.rightPlayRect = QRect(366-80, 62, 106, 42);
    
    // 中央出牌区（当前轮次打出的牌）
    zones.centerPlayRect = QRect(98, 98, 284, 40);

    // ========== 角色锚点（玩家头像/名称位置）==========
    // roleAnchors[0] - 左侧机器人（农民/地主）
    zones.roleAnchors[0] = QPoint(20, 92);
    
    // roleAnchors[1] - 右侧机器人（农民/地主）
    zones.roleAnchors[1] = QPoint(460, 92);
    
    // roleAnchors[2] - 本地玩家（左下角）
    zones.roleAnchors[2] = QPoint(56, 200);

    // ========== 信息锚点（积分、身份等信息位置）==========
    // infoAnchors[0] - 左侧机器人的信息（分数、地主标识等）
    zones.infoAnchors[0] = QPoint(130, 102);
    
    // infoAnchors[1] - 右侧机器人的信息
    zones.infoAnchors[1] = QPoint(350, 102);
    
    // infoAnchors[2] - 本地玩家的信息
    zones.infoAnchors[2] = QPoint(240, 88);

    return zones;
}

void Maingame::ApplyLayoutZones()
{
    if(!ui)
    {
        return;
    }

    _LayoutZones = BuildLayoutZones();
    ui->widget_showscore->setGeometry(0, 0, width(), 24);
    ui->tableRoot->setGeometry(0, 24, width(), height() - 24);
    ui->tableLayer->setGeometry(ui->tableRoot->rect());
    ui->overlayLayer->setGeometry(ui->tableRoot->rect());
    ui->widget->setFixedSize(_LayoutZones.actionBarRect.size());
    ui->widget->move(_LayoutZones.actionBarRect.topLeft());
    ui->widget->raise();

    if(_StartOverlay)
    {
        _StartOverlay->setGeometry(ui->overlayLayer->rect());
    }
    if(_StartPanel)
    {
        _StartPanel->setGeometry(_LayoutZones.startPanelRect);
    }
    if(_StartTitle && _StartSubtitle && _StartButton)
    {
        _StartTitle->setGeometry(0, 8, _StartPanel->width(), 24);
        _StartSubtitle->setGeometry(0, 34, _StartPanel->width(), 14);
        _StartButton->move((_StartPanel->width() - _StartButton->width()) / 2, _StartPanel->height() - _StartButton->height() - 8);
    }
    if(_Timecount)
    {
        _Timecount->move(_LayoutZones.countdownRect.topLeft());
    }
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
    const QSize bubbleMaxSize(84, 22);
    const QPixmap scaled = ScalePixmapToFit(pixmap, bubbleMaxSize);
    ctx->_NOCardlabel->setText(QString());
    ctx->_NOCardlabel->setPixmap(scaled);
    ctx->_NOCardlabel->setFixedSize(scaled.size());

    const QPoint targetPos = CalculateLabelPosAbovePlayArea(ctx, scaled.size(), 0);
    ctx->_NOCardlabel->move(targetPos);
    ctx->_NOCardlabel->raise();
    ctx->_NOCardlabel->show();
}
//初始化卡牌
void Maingame::InitCardpanelMap()
{
    _IMage_Cards.load(":/images/card.png");
    const QSize sourceCardSize(_IMage_Cards.width() / 13, _IMage_Cards.height() / 5);
    QSize displayCardSize = sourceCardSize;
    displayCardSize.scale(QSize(38, 52), Qt::KeepAspectRatio);
    _IMage_Card_Size = displayCardSize;

    _Card_back = _IMage_Cards.copy(sourceCardSize.width() * 2,
                                   sourceCardSize.height() * 4,
                                   sourceCardSize.width(),
                                   sourceCardSize.height())
                     .scaled(_IMage_Card_Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);//背面图像
    for(int i=0;i<13;i++)//点数
    {
        for(int j=0;j<4;j++)//花色
        {
            Card::cardpoint point=(Card::cardpoint)(i+1);
            Card::cardsuit suit=(Card::cardsuit)(j+1);
            Card *TempCard=new Card(suit,point);
            QPixmap Temppixmap = _IMage_Cards.copy(sourceCardSize.width() * i,
                                                   sourceCardSize.height() * j,
                                                   sourceCardSize.width(),
                                                   sourceCardSize.height())
                                     .scaled(_IMage_Card_Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            InitCardImage(Temppixmap,_Card_back,TempCard);//插入卡牌 图片显示问题

        }
    }
    QPixmap TemppixmapSJ = _IMage_Cards.copy(0,
                                             sourceCardSize.height() * 4,
                                             sourceCardSize.width(),
                                             sourceCardSize.height())
                               .scaled(_IMage_Card_Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);//大王牌
    QPixmap TemppixmapBJ = _IMage_Cards.copy(sourceCardSize.width(),
                                             sourceCardSize.height() * 4,
                                             sourceCardSize.width(),
                                             sourceCardSize.height())
                               .scaled(_IMage_Card_Size, Qt::KeepAspectRatio, Qt::SmoothTransformation);//小王牌
    Card *CardSJ=new Card(Card::Suit_Begin,Card::Card_SJ);
    Card *CardBJ=new Card(Card::Suit_Begin,Card::Card_BJ);
    InitCardImage(TemppixmapSJ,_Card_back,CardSJ);
    InitCardImage(TemppixmapBJ,_Card_back,CardBJ);

}

void Maingame::InitCardImage(QPixmap Card_front,QPixmap Card_back,Card *card)//设置卡牌图片初始化
{
    CardPanel * cardpenel=new CardPanel(ui->overlayLayer);
    cardpenel->setimage(Card_front,Card_back);
    cardpenel->setCardSize(_IMage_Card_Size);
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
    ui->widget->Setbtngroupstate(MybuttonGroup::Null);
    ApplyLayoutZones();
    const QRect cardsRect[] =
    {
        _LayoutZones.leftCompactRect,
        _LayoutZones.rightCompactRect,
        _LayoutZones.userHandRect
    };
    const QRect playHandRect[] =
    {
        _LayoutZones.leftPlayRect,
        _LayoutZones.rightPlayRect,
        _LayoutZones.centerPlayRect
    };
    const QPoint roleImgPos[] =
    {
        _LayoutZones.roleAnchors[0],
        _LayoutZones.roleAnchors[1],
        _LayoutZones.roleAnchors[2]
    };
    const QPoint info[] =
    {
        _LayoutZones.infoAnchors[0],
        _LayoutZones.infoAnchors[1],
        _LayoutZones.infoAnchors[2]
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

        tempcontext->_NOCardlabel = new QLabel(ui->overlayLayer);
        tempcontext->_NOCardlabel->setFixedSize(84, 22);
        tempcontext->_NOCardlabel->move(info[i].x() - 42, info[i].y() - 11);
        tempcontext->_NOCardlabel->setAlignment(Qt::AlignCenter);
        tempcontext->_NOCardlabel->setScaledContents(true);
        tempcontext->_NOCardlabel->setStyleSheet(
            "background: rgba(8,18,20,170);"
            "color: rgb(245,244,232);"
            "border: 1px solid rgba(255,255,255,50);"
            "border-radius: 10px;");
        tempcontext->_NOCardlabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        tempcontext->_NOCardlabel->hide();

        tempcontext->_ROlelabel = new QLabel(ui->overlayLayer);
        tempcontext->_ROlelabel->move(roleImgPos[i].x() - 15, roleImgPos[i].y() - 21);
        tempcontext->_ROlelabel->hide();
        tempcontext->_ROlelabel->resize(60, 70);
        tempcontext->_ROlelabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        tempcontext->_CountLabel = new QLabel(ui->overlayLayer);
        tempcontext->_CountLabel->setAlignment(Qt::AlignCenter);
        tempcontext->_CountLabel->setFixedSize(26, 16);
        tempcontext->_CountLabel->setStyleSheet(
            "background: rgba(8, 18, 20, 180);"
            "color: rgb(246, 239, 214);"
            "border: 1px solid rgba(255,255,255,45);"
            "border-radius: 8px;"
            "font-size: 10px;"
            "font-weight: 600;");
        tempcontext->_CountLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        tempcontext->_CountLabel->hide();

        tempcontext->_Mycards=new Cards();
        _Playercontexts.insert(_Players.at(i), tempcontext);  // 存储指针

    }
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

    UpdateUserActionUi();

}
void Maingame::SatrtPend()
{
    if(_Timer_PlayHand->isActive())
    {
        _Timer_PlayHand->stop();
    }
    if(_DispatchLayoutTimer->isActive())
    {
        _DispatchLayoutTimer->stop();
    }

    ui->widget->SetStartButtonVisible(false);
    _CanSelectCards = false;
    _IsDispatchBatching = true;
    _DispatchCardsSinceLastLayout = 0;
    _PendingLayoutPlayers.clear();
    _Movetime = 0;
    ClearSelectedPanels();
    UpdateUserActionUi();
    ResetCountdown();
    _Bgmcontrol->StopOtherBgm();

    // 每局开始时重置抢分/动画控件到固定位置，避免上一局动画改变坐标
    _MyAnmation->setFixedSize(132, 42);
    _MyAnmation->move(_LayoutZones.centerBoardRect.center().x() - _MyAnmation->width() / 2,
                      _LayoutZones.centerBoardRect.center().y() - _MyAnmation->height() / 2);
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

    // 2. 把所有卡片初始化
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
    for(int i = 0; i < 3; ++i)
    {
        if(_LordCards[i])
        {
            _LordCards[i]->hide();
            _LordCards[i]->setfront(false);
        }
    }

    //开始发牌定时器启动
    _Timer_PlayHand->start(25);
    QTimer::singleShot(0, this, [this]() {
        if(_CurrentGameState == gamecontrol::PENDCARD)
        {
            _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::DISPATCH);
        }
    });
}

void Maingame::PlayHandtimer(player * Player,int Movetime)
{
    if(!_Gamecontrol || !Player)
    {
        _Timer_PlayHand->stop();
        return;
    }

    auto playerIt = _Playercontexts.find(Player);
    if(playerIt == _Playercontexts.end())
    {
        _Timer_PlayHand->stop();
        return;
    }

    QRect TempRect=playerIt.value()->_PLayerCardsRect;
    auto x=TempRect.x();
    auto y=TempRect.y();
    int Cardpool=_Gamecontrol->GetCardcount();
    if(Cardpool<=3)
    {


        // 发牌结束音乐关闭
        _Bgmcontrol->StopOtherBgm();
        _Timer_PlayHand->stop();
        _IsDispatchBatching = false;
        if(_DispatchLayoutTimer->isActive())
        {
            _DispatchLayoutTimer->stop();
        }
        FlushPendingLayouts();
        _MoveCards->hide();
        _PendCards->hide();
        // 1. 取出三张地主牌
        QVector<Card*> lordCards;
        lordCards.reserve(3);
        for (int i = 0; i < 3 && _Gamecontrol->GetCardcount() > 0; ++i) {
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

        const QSize lordCardSize(30, 40);
        const int lordSpacing = 6;
        const int lordStartX = _LayoutZones.lordCardsRect.left();
        const int lordTopY = _LayoutZones.lordCardsRect.top() +
                             std::max(0, (_LayoutZones.lordCardsRect.height() - lordCardSize.height()) / 2);

        for (int i = 0; i < lordCards.size(); ++i) {
            Card* card = lordCards[i];
            if(_LordCards[i])
            {
                _LordCards[i]->deleteLater();
                _LordCards[i] = nullptr;
            }
            CardPanel* lordPanel = new CardPanel(ui->overlayLayer);

            if (_CardPenelMap.contains(*card)) {
                lordPanel->setimage(_CardPenelMap[*card]->Getimagefont(), _Card_back);
            }

            lordPanel->setCardSize(lordCardSize);
            lordPanel->move(lordStartX + i * (lordCardSize.width() + lordSpacing), lordTopY);
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
        return;

    }
    if(Movetime<5)//移动的动画
    {

        const int move_size=30;//小屏收紧发牌动画轨迹
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
        if(!drawnCard)
        {
            _Timer_PlayHand->stop();
            return;
        }
        Card tempCard(drawnCard->getcardsuit(), drawnCard->getcardpoint());  // 创建临时对象
        if(!_CardPenelMap.contains(tempCard)) {
            return;
        }
        Player->StoreGetCard(drawnCard);
        _DispatchCardsSinceLastLayout++;
        if(_DispatchCardsSinceLastLayout >= 3)
        {
            _DispatchCardsSinceLastLayout = 0;
            _DispatchLayoutTimer->start(0);
        }
        _Movetime=0;
        _Gamecontrol->SetCurrentPlayer(Player->GetNextPlayer());
    }
}

void Maingame::InitMovepoint()
{
    for(int i = 0; i < _Players.size() && i < 3; i++)
    {
        auto ctx = _Playercontexts.value(_Players.at(i), nullptr);
        if(ctx)
        {
            _xy[i] = QPoint(ctx->_PLayerCardsRect.x(), ctx->_PLayerCardsRect.y());
        }
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
        UpdateHudState(QStringLiteral("发牌中"));
        UpdateUserActionUi();
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
            if(_LordCards[i])
            {
                _LordCards[i]->setfront(true);
            }
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
        UpdateHudState(QStringLiteral("出牌中"));
        UpdateUserActionUi();
        break;
    }

    case gamecontrol::GETLORD://叫地主
    {    ResetCountdown();
        _CanSelectCards = false;
        ClearSelectedPanels();
        _MyAnmation->hide();
        for(int i = 0; i < 3; ++i)
        {
            if(_LordCards[i])
            {
                _LordCards[i]->show();
            }
        }
        ui->widget->Setbtngroupstate(MybuttonGroup::Getloard);
        UpdateHudState(QStringLiteral("叫地主"));
        UpdateUserActionUi();
        break;
    }
}
}

void Maingame::PendCardplayer(player *player)
{
    if(!player)
    {
        return;
    }

    if(_IsDispatchBatching && _CurrentGameState == gamecontrol::PENDCARD)
    {
        _PendingLayoutPlayers.insert(player);
        return;
    }

    RefreshPlayerPanels(player);
}

void Maingame::PendCardpos(player* player) {
    if(!player || !_Playercontexts.contains(player))
    {
        return;
    }

    Cards cards = player->GetCards();
    QListcard sortedCards = cards.Listcardssort(Cards::ASC);
    _Playercontext* context = _Playercontexts[player];
    const QRect cardsRect = context->_PLayerCardsRect;
    const QSize handCardSize(38, 52);
    const QSize robotStackSize(26, 36);
    const QSize userPlaySize(32, 44);
    const QSize robotPlaySize(25, 35);

    const bool isUserArea = (context->_Align == Horizontal);
    if(isUserArea)
    {
        _PanelPositon.clear();
        if(context->_CountLabel)
        {
            context->_CountLabel->hide();
        }
    }
    else if(context->_CountLabel)
    {
        context->_CountLabel->setText(QString::number(sortedCards.size()));
        context->_CountLabel->move(cardsRect.center().x() - context->_CountLabel->width() / 2,
                                   cardsRect.bottom() + 4);
        context->_CountLabel->show();
    }

    const QSize stackCardSize = isUserArea ? handCardSize : robotStackSize;
    const int stackCardWidth = stackCardSize.width();
    const int stackCardHeight = stackCardSize.height();
    const int maxRobotVisible = isUserArea ? sortedCards.size() : sortedCards.size();  // 全部显示
    const int hiddenPrefix = 0;  // 不隐藏

    for(int i = 0; i < sortedCards.size(); i++)
    {
        CardPanel* panel = _CardPenelMap.value(sortedCards[i], nullptr);
        if(!panel)
        {
            continue;
        }
        panel->setCardSize(stackCardSize);

        if(isUserArea)
        {
            const int raiseOffset = 8;
            const QSize handPanelSize(stackCardWidth, stackCardHeight + raiseOffset);
            const int preferredSpacing = sortedCards.size() > 15 ? 16 : (sortedCards.size() > 12 ? 17 : 18);
            const int cardSpace = std::max(11, CalculateStackSpacing(cardsRect.width(), stackCardWidth, sortedCards.size(), preferredSpacing));
            const int stackWidth = stackCardWidth + std::max(0, sortedCards.size() - 1) * cardSpace;
            const int leftX = cardsRect.left() + std::max(0, (cardsRect.width() - stackWidth) / 2);
            const int baseTop = cardsRect.top() + std::max(0, (cardsRect.height() - handPanelSize.height()) / 2);

            _Mycardsrect = QRect(leftX, baseTop - raiseOffset, stackWidth, stackCardHeight + raiseOffset);
            const int drawTop = panel->GetSelect() ? 0 : raiseOffset;
            const QPoint targetPos(leftX + i * cardSpace, baseTop);
            panel->setPresentation(handPanelSize, QRect(0, drawTop, stackCardWidth, stackCardHeight));
            if(panel->pos() != targetPos)
            {
                panel->move(targetPos);
                panel->raise();
            }
            if(!panel->getfront())
            {
                panel->setfront(true);
            }
            if(!panel->isVisible())
            {
                panel->show();
                panel->raise();
            }
            _PanelPositon.insert(panel, QRect(targetPos + QPoint(0, drawTop), stackCardSize));
            continue;
        }

        // ========== 机器人手牌：堆叠效果，全部显示 ==========
        if(panel->getfront() != context->Isfront)
        {
            panel->setfront(context->Isfront);
        }

        const int visibleIndex = i;  // 全部显示，不隐藏
        const int stackOffset = 8;   // 堆叠偏移量（像素），每张牌露出8像素
        const int stackHeight = stackCardHeight + std::max(0, maxRobotVisible - 1) * stackOffset;
        const int leftX = cardsRect.left() + std::max(0, (cardsRect.width() - stackCardWidth) / 2);
        const int topY = cardsRect.top() + std::max(0, (cardsRect.height() - stackHeight) / 2);
        const QPoint targetPos(leftX, topY + visibleIndex * stackOffset);  // 垂直堆叠，每张牌向下偏移
        
        panel->setPresentation(stackCardSize, QRect(QPoint(0, 0), stackCardSize));
        
        if(panel->pos() != targetPos)
        {
            panel->move(targetPos);
            panel->raise();
        }
        if(!panel->isVisible())
        {
            panel->show();
            panel->raise();
        }
    }

    if(context->_Last_Cards && !context->_Last_Cards->isempty())
    {
        const QRect playRect = context->_PlayerHandRect;
        QListcard list = context->_Last_Cards->Listcardssort();
        const QSize playCardSize = isUserArea ? userPlaySize : robotPlaySize;
        const int playCardWidth = playCardSize.width();
        const int playCardHeight = playCardSize.height();
        int visibleCount = list.size();
        int hidePrefix = 0;
        if(!isUserArea && list.size() > 6)
        {
            visibleCount = 6;
            hidePrefix = list.size() - visibleCount;
        }

        const int playSpacing = std::max(isUserArea ? 12 : 8,
                                         CalculateStackSpacing(playRect.width(), playCardWidth, visibleCount, isUserArea ? 16 : 10));
        const int stackWidth = playCardWidth + std::max(0, visibleCount - 1) * playSpacing;
        const int startX = playRect.left() + std::max(0, (playRect.width() - stackWidth) / 2);
        const int topY = playRect.top() + std::max(0, (playRect.height() - playCardHeight) / 2);

        for(int i = 0; i < list.size(); i++)
        {
            CardPanel *tempPanel = _CardPenelMap.value(list.at(i), nullptr);
            if(!tempPanel)
            {
                continue;
            }
            if(i < hidePrefix)
            {
                tempPanel->hide();
                continue;
            }

            const int displayIndex = i - hidePrefix;
            tempPanel->setPresentation(playCardSize, QRect(QPoint(0, 0), playCardSize));
            tempPanel->setfront(true);
            const QPoint targetPos(startX + displayIndex * playSpacing, topY);
            if(tempPanel->pos() != targetPos)
            {
                tempPanel->move(targetPos);
                tempPanel->raise();
            }
            if(!tempPanel->isVisible())
            {
                tempPanel->show();
                tempPanel->raise();
            }
        }
    }
}

void Maingame::RefreshPlayerPanels(player *player)
{
    Cards cards = player->GetCards();
    QListcard listcard = cards.Listcardssort();

    for(int i = 0; i < listcard.size(); i++)
    {
        CardPanel* cardPanel = _CardPenelMap.value(listcard[i], nullptr);
        if(cardPanel)
        {
            cardPanel->setowner(*player);
            if(cardPanel->GetSelect())
            {
                cardPanel->setselect(false);
            }
        }
    }
    PendCardpos(player);
}

void Maingame::FlushPendingLayouts()
{
    if(_PendingLayoutPlayers.isEmpty())
    {
        return;
    }

    const QSet<player*> pendingPlayers = _PendingLayoutPlayers;
    _PendingLayoutPlayers.clear();

    for(player* player : pendingPlayers)
    {
        if(player)
        {
            RefreshPlayerPanels(player);
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
            const QPixmap scaledPass = ScalePixmapToFit(passPixmap, QSize(84, 22));
            noCardLabel->setPixmap(scaledPass);
            noCardLabel->setFixedSize(scaledPass.size());
            const QPoint targetPos = CalculateLabelPosAbovePlayArea(_Playercontexts.find(player).value(), scaledPass.size(), 0);
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
        UpdateHudState(QStringLiteral("叫地主"));
        UpdateUserActionUi();
        if(player == _Gamecontrol->GetUSer())
        {
            _Gamecontrol->StartPrepareLord();//开始抢地主并且发送信号
        }
        break;

    case gamecontrol::USERPEND:
        UpdateHudState(QStringLiteral("出牌中"));
        _CanSelectCards = (player == _Gamecontrol->GetUSer());
        if(!_CanSelectCards)
        {
            ClearSelectedPanels();
        }
        if(_Timecount)
        {
            QPoint timerPos = _LayoutZones.countdownRect.topLeft();
            if(player == _Gamecontrol->GetUSer())
            {
                timerPos = QPoint(_LayoutZones.actionBarRect.center().x() - _Timecount->width() / 2,
                                  _LayoutZones.actionBarRect.top() - _Timecount->height() - 2);
            }
            else if(player == _Gamecontrol->GetLeftroot())
            {
                timerPos = QPoint(_LayoutZones.roleAnchors[0].x() + 10,
                                  _LayoutZones.roleAnchors[0].y() - _Timecount->height() / 2 + 8);
                _Timecount->hide();
            }
            else
            {
                timerPos = QPoint(_LayoutZones.roleAnchors[1].x() - _Timecount->width() - 10,
                                  _LayoutZones.roleAnchors[1].y() - _Timecount->height() / 2 + 8);
                _Timecount->hide();
            }
            _Timecount->move(timerPos);
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
        UpdateUserActionUi();
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
        UpdateHudState(QStringLiteral("结算中"));
        UpdateUserActionUi();
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
        _MyAnmation->setFixedSize(160, 52);
        _MyAnmation->move(_LayoutZones.centerBoardRect.center().x() - _MyAnmation->width() / 2,
                          _LayoutZones.centerBoardRect.center().y() - _MyAnmation->height() / 2);
        _MyAnmation->show();
    }
    else
    {
        _MyAnmation->hide();
    }

    ui->widget->Setbtngroupstate(MybuttonGroup::Null);
    UpdateHudState(QStringLiteral("抢地主"));

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
    if(temp->getowner() != _Gamecontrol->GetUSer())
    {
        return;
    }

    if(event != Qt::LeftButton)
    {
        return;
    }

    const bool selected = !temp->GetSelect();
    temp->setselect(selected);
    if(selected)
    {
        _SelcetPanel.insert(temp);
        _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::SELECT_CARD);
    }
    else
    {
        _SelcetPanel.remove(temp);
    }
    PendCardpos(temp->getowner());
    UpdateUserActionUi();
}

void Maingame::InitEndPanel(player *player)
{
    EndPanel *E1=new EndPanel(player,ui->overlayLayer);
    E1->move((ui->overlayLayer->width()-E1->width())/2,-E1->height());
    E1->show();//先显示出来 动画效果更好
    QPropertyAnimation *anim = new QPropertyAnimation(E1, "pos", this);
        anim->setDuration(1500);
        anim->setStartValue(QPoint((ui->overlayLayer->width()-E1->width())/2,-E1->height()));
        anim->setEndValue(QPoint((ui->overlayLayer->width()-E1->width())/2,(ui->overlayLayer->height()-E1->height())/2));
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
        ui->widget->Setbtngroupstate(MybuttonGroup::Null);
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
                    UpdateUserActionUi();
                }
            }
        }

        if(_SelcetPanel.isEmpty())
        {
            UpdateUserActionUi();
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
    UpdateUserActionUi();
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
    UpdateUserActionUi();
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
    UpdateUserActionUi();
}

CardPanel* Maingame::PanelFromPos(const QPoint &pos) const
{
    const QPoint tablePos = (ui && ui->overlayLayer) ? ui->overlayLayer->mapFrom(this, pos) : pos;
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
        if(_PanelPositon.value(panel).contains(tablePos))
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

int Maingame::CalculateStackSpacing(int availableSpan, int cardSpan, int cardCount, int preferredSpacing) const
{
    if(cardCount <= 1)
    {
        return 0;
    }

    const int safePreferred = std::max(1, preferredSpacing);
    const int remaining = std::max(0, availableSpan - cardSpan);
    const int spacing = remaining / (cardCount - 1);
    return std::max(1, std::min(safePreferred, spacing));
}

QRect Maingame::ClampRectToWindow(const QRect &rect) const
{
    const QRect windowRect = this->rect();
    const int maxWidth = std::max(1, windowRect.width());
    const int maxHeight = std::max(1, windowRect.height());

    QRect bounded = rect;
    bounded.setWidth(std::min(rect.width(), maxWidth));
    bounded.setHeight(std::min(rect.height(), maxHeight));
    // //bounded.moveLeft(std::clamp(bounded.left(), windowRect.left(), windowRect.right() - bounded.width() + 1));c10没有这个
    // Maingame::ClampRectToWindow(const QRect&) const’:
    //                                                         ../../maingame.cpp:1364:27: error: ‘clamp’ is not a member of ‘std’
    //                                         bounded.moveLeft(std::clamp(bounded.left(), windowRect.left(), windowRect.right() - bounded.width() + 1));
    // ^~~~~
    //bounded.moveTop(std::clamp(bounded.top(), windowRect.top(), windowRect.bottom() - bounded.height() + 1));
    return bounded;
}
void Maingame::RePlayGame()
{
    PlayerStateChange(_Gamecontrol->GetUSer(),gamecontrol::USERGETLORD);

}

void Maingame::InitPlayerTimer()
{

    _Timecount=new Timecount(ui->overlayLayer);
    _Timecount->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    _Timecount->move(_LayoutZones.countdownRect.topLeft());
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
            _Timecount->move(_LayoutZones.actionBarRect.center().x() - _Timecount->width() / 2,
                             _LayoutZones.actionBarRect.top() - _Timecount->height() - 2);
            _Timecount->Start();
            _Timecount->show();

        }


    });


}

void Maingame::Showanimation(PlayHand::HandType type)
{
    const QRect effectRect = _LayoutZones.centerBoardRect;
    switch(type)
    {
    case(PlayHand::Hand_Seq_Pair):
    case(PlayHand::Hand_Seq_Sim):
        _MyAnmation->ShowSimsqe(type);
        _MyAnmation->setFixedSize(132, 42);
        _MyAnmation->move(effectRect.center().x() - _MyAnmation->width() / 2,
                          effectRect.center().y() - _MyAnmation->height() / 2);
        _MyAnmation->show();
        break;
    case(PlayHand::Hand_Bomb):
    case(PlayHand::Hand_Bomb_Jokers):
        _MyAnmation->ShowBom(type);
        _MyAnmation->setFixedSize(132, 42);
        _MyAnmation->move(effectRect.center().x() - _MyAnmation->width() / 2,
                          effectRect.center().y() - _MyAnmation->height() / 2);
        _MyAnmation->show();

        // 添加炸弹音效
        _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::BOMB);
        break;
    case(PlayHand::Hand_Plane):
    case(PlayHand::Hand_Plane_Two_Single):
    case(PlayHand::Hand_Plane_Two_Pair):
        _MyAnmation->ShowPlane();
        _MyAnmation->setFixedSize(132, 42);
        _MyAnmation->move(effectRect.center().x() - _MyAnmation->width() / 2,
                          effectRect.center().y() - _MyAnmation->height() / 2);
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
    _PendCards=new CardPanel(ui->overlayLayer);
    _PendCards->resize(_IMage_Card_Size.width(),_IMage_Card_Size.height());
    _PendCards->setimage(_Card_back,_Card_back);

    _MoveCards=new CardPanel(ui->overlayLayer);
    _MoveCards->setimage(_Card_back,_Card_back);
    _MoveCards->resize(_IMage_Card_Size.width(),_IMage_Card_Size.height());

    _Base_point = QPoint(_LayoutZones.lordCardsRect.center().x() - _IMage_Card_Size.width() / 2,
                         _LayoutZones.lordCardsRect.bottom() + 2);
    _PendCards->move(_Base_point);
    _MoveCards->move(_Base_point);

    _PendCards->hide();
    _MoveCards->hide();


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
    Q_UNUSED(event);
    QPainter p1(this);
    if(_IMage_Map.isNull())
    {
        return;
    }
    QPixmap scaled = _IMage_Map.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    const QRect sourceRect((scaled.width() - width()) / 2,
                           (scaled.height() - height()) / 2,
                           width(), height());
    p1.drawPixmap(rect(), scaled, sourceRect);

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
        const QPoint tablePos = (ui && ui->overlayLayer) ? ui->overlayLayer->mapFrom(this, event->pos()) : event->pos();
        QRect tolerantRect = _Mycardsrect.adjusted(-10, -10, 10, 10);
        if(!tolerantRect.contains(tablePos))
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

        const QPoint tablePos = (ui && ui->overlayLayer) ? ui->overlayLayer->mapFrom(this, event->pos()) : event->pos();
        QRect tolerantRect = _Mycardsrect.adjusted(-10, -10, 10, 10);
        if(!tolerantRect.contains(tablePos))
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
        auto ctx = _Playercontexts.find(currentPlayer).value();
        if(currentPlayer == _Gamecontrol->GetUSer())
        {
            ctx->_ROlelabel->hide();
            continue;
        }
        QPixmap RolePix = currentPlayer->GetPlayerRolePixmap(
            currentPlayer->GetRole(),  // 使用玩家当前的真实角色
            currentPlayer->GetSex(),
            currentPlayer->GetLocation()
            );
        RolePix = RolePix.scaled(QSize(30, 42), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ctx->_ROlelabel->setPixmap(RolePix);
        ctx->_ROlelabel->setFixedSize(RolePix.size());
        const QPoint targetPos = CalculateRoleLabelPos(ctx, RolePix.size());
        ctx->_ROlelabel->move(targetPos);
        ctx->_ROlelabel->raise();
        ctx->_ROlelabel->show();

        // qDebug() << "设置玩家角色头像 - 玩家:" << currentPlayer
                 //<< "角色:" << currentPlayer->GetRole();
    }
    UpdateHudState(QStringLiteral("出牌中"));
}
