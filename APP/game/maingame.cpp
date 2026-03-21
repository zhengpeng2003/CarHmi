#include "maingame.h"
#include "qpainter.h"
#include "ui_maingame.h"
#include <QCloseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSequentialAnimationGroup>
#include <QPainter>
#include <QTimer>
#include <algorithm>
#include "gameresourcecache.h"
#include "gamescaling.h"

namespace {
struct LayoutMetrics {
    QSize playerCardSize = QSize(36, 48);
    QSize tableCardSize = QSize(36, 48);
    QSize lordCardSize = QSize(32, 42);
    QSize deckCardSize = QSize(32, 42);
    int handOverlap = 20;
    int verticalOverlap = 12;
    int tableOverlap = 24;
    int lordOverlap = 20;
    QSize buttonSize = QSize(80, 40);
    int buttonGap = 10;
    int bottomHandY = 210;
    int topOpponentY = 10;
    int deckX = 224;
    int deckY = 115;
    int playAreaY = 120;
    int leftRobotX = 8;
    int rightRobotX = 440;
    int avatarMain = 56;
    int avatarOpp = 42;
    int roleMaxHeight = 60;
};

LayoutMetrics metrics()
{
    return LayoutMetrics{};
}

QPoint scaledPoint(int x, int y)
{
    return GameScaling::point(x, y);
}
}

Maingame::Maingame(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Maingame)
{
    initializeWindowChrome();
    ui->setupUi(this);
    applyDesignLayout();
    initializeExitButton();
}

void Maingame::initializeWindowChrome()
{
    GameScaling::initialize(size().isValid() ? size() : GameScaling::designSize());
    setFixedSize(GameScaling::designSize());
    setWindowIcon(QIcon(":/images/logo.ico"));
    setWindowTitle("欢乐斗地主");
}

void Maingame::initializeExitButton()
{
    QWidget *cw = centralWidget();
    _ExitButton = new QPushButton(cw);
    _ExitButton->setText("×");
    GameScaling::applyFixedSize(_ExitButton, 32, 32);
    GameScaling::move(_ExitButton, 448, 0);
    _ExitButton->setStyleSheet(
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
    connect(_ExitButton, &QPushButton::clicked, this, &QWidget::close);
}

void Maingame::applyDesignLayout()
{
    const LayoutMetrics m = metrics();
    if (ui->widget) {
        ui->widget->setGeometry(GameScaling::rect(120, 220, m.buttonSize.width() * 4 + m.buttonGap * 3, m.buttonSize.height()));
    }
    if (ui->widget_showscore) {
        ui->widget_showscore->setGeometry(GameScaling::rect(150, 0, 180, 28));
    }
}

int Maingame::setupStageCount() const
{
    return 9;
}

QString Maingame::setupStageLabel(int stage) const
{
    switch (stage) {
    case SetupResources: return QStringLiteral("加载背景与基础资源");
    case SetupGameControl: return QStringLiteral("初始化逻辑控制器");
    case SetupScoreboard: return QStringLiteral("初始化积分面板");
    case SetupCardPanels: return QStringLiteral("构建卡牌面板");
    case SetupScene: return QStringLiteral("初始化场景元素");
    case SetupButtons: return QStringLiteral("初始化游戏按钮");
    case SetupMovePoints: return QStringLiteral("计算布局锚点");
    case SetupTimers: return QStringLiteral("初始化计时器");
    case SetupAudio: return QStringLiteral("初始化音效系统");
    default: return QStringLiteral("初始化中");
    }
}

void Maingame::initializeStage(int stage)
{
    if (_CompletedStages.contains(stage)) {
        return;
    }

    switch (stage) {
    case SetupResources: {
        const int rand = QRandomGenerator::global()->bounded(0, 10) + 1;
        const QString imgPath = QString(":/images/background-%1.png").arg(rand);
        _IMage_Map = GameResourceCache::pixmap(imgPath);
        _MyAnmation = new AnmationPixmap(this);
        break;
    }
    case SetupGameControl:
        InitGamecontrol();
        break;
    case SetupScoreboard:
        InitScore();
        break;
    case SetupCardPanels:
        InitCardpanelMap();
        break;
    case SetupScene:
        InitGameScene();
        break;
    case SetupButtons:
        InitGroupbtn();
        break;
    case SetupMovePoints:
        InitMovepoint();
        break;
    case SetupTimers:
        if (!_Timer_PlayHand) {
            _Timer_PlayHand = new QTimer(this);
            _Movetime = 0;
            connect(_Timer_PlayHand, &QTimer::timeout, this, [=]() {
                if (!_Gamecontrol) {
                    return;
                }
                _Movetime++;
                PlayHandtimer(_Gamecontrol->GetCurrentPlayer(), _Movetime);
            });
        }
        InitPlayerTimer();
        break;
    case SetupAudio:
        if (!_Bgmcontrol) {
            _Bgmcontrol = new Bgmcontrol(this);
            _Bgmcontrol->InitMusicPlayer();
            _Bgmcontrol->StartBgm();
        }
        break;
    default:
        break;
    }

    _CompletedStages.insert(stage);
    emit setupProgressChanged(_CompletedStages.size(), setupStageCount(), setupStageLabel(stage));
    if (_CompletedStages.size() == setupStageCount()) {
        _SetupCompleted = true;
        emit setupFinished();
    }
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
    prepareCardPixmaps();
}
void Maingame::InitCardImage(QPixmap Card_front,QPixmap Card_back,Card *card)
{
    if (!card) {
        return;
    }
    _CardPixmapMap.insert(*card, qMakePair(Card_front, Card_back));
}

void Maingame::InitGroupbtn()
{
    const LayoutMetrics m = metrics();
    ui->widget->Initbutton();
    ui->widget->Setbtngroupstate(MybuttonGroup::Start);
    const QRect cardsRect[] = {
        GameScaling::rect(m.leftRobotX, 0, m.lordCardSize.width(), 272),
        GameScaling::rect(m.rightRobotX, 0, m.lordCardSize.width(), 272),
        GameScaling::rect(0, m.bottomHandY, 480, m.playerCardSize.height())
    };
    const QRect playHandRect[] = {
        GameScaling::rect(30, m.playAreaY, 120, m.tableCardSize.height()),
        GameScaling::rect(330, m.playAreaY, 120, m.tableCardSize.height()),
        GameScaling::rect(96, m.playAreaY, 288, m.tableCardSize.height())
    };
    const QPoint roleImgPos[] = {
        GameScaling::point(52, 218),
        GameScaling::point(428, 218),
        GameScaling::point(120, 188)
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
        tempcontext->_NOCardlabel->setGeometry(playHandRect[i]);
        tempcontext->_NOCardlabel->setScaledContents(true);
        tempcontext->_NOCardlabel->hide();

        tempcontext->_ROlelabel = new QLabel(this);
        tempcontext->_ROlelabel->hide();
        tempcontext->_ROlelabel->resize(GameScaling::size(i == index ? m.avatarMain : m.avatarOpp, m.roleMaxHeight));
        tempcontext->_ROlelabel->move(CalculateCenteredPos(roleImgPos[i], tempcontext->_ROlelabel->size()));

        tempcontext->_CountLabel = new QLabel(this);
        tempcontext->_CountLabel->setAlignment(Qt::AlignCenter);
        tempcontext->_CountLabel->setStyleSheet("QLabel { color: white; background: rgba(0,0,0,120); border-radius: 4px; font: 12px 'Microsoft YaHei'; padding: 1px 4px; }");
        tempcontext->_CountLabel->hide();

        tempcontext->_Mycards=new Cards();
        _Playercontexts.insert(_Players.at(i), tempcontext);  // 存储指针

    }
    //开始按钮
    connect(ui->widget,&MybuttonGroup::S_Start,this,[this](){
        if (_GameLaunchPending || !_SetupCompleted) {
            return;
        }
        ui->widget->SetStartButtonVisible(false);
        ui->widget->Setbtngroupstate(MybuttonGroup::Null);
        QTimer::singleShot(0, this, [this]() { beginGameStartSequence(); });
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

void Maingame::beginGameStartSequence()
{
    if (_GameLaunchPending || !_Gamecontrol) {
        return;
    }

    _GameLaunchPending = true;
    ++_GameStartSequenceId;
    ensureGraphicsView();
    QTimer::singleShot(0, this, [this, sequenceId = _GameStartSequenceId]() {
        if (sequenceId == _GameStartSequenceId) runGameStartStep(0);
    });
    QTimer::singleShot(50, this, [this, sequenceId = _GameStartSequenceId]() {
        if (sequenceId == _GameStartSequenceId) runGameStartStep(1);
    });
    QTimer::singleShot(100, this, [this, sequenceId = _GameStartSequenceId]() {
        if (sequenceId == _GameStartSequenceId) runGameStartStep(2);
    });
}
void Maingame::runGameStartStep(int step)
{
    switch (step) {
    case 0:
        ResetCountdown();
        _CanSelectCards = false;
        _IsUserFirstLordPlay = false;
        ClearSelectedPanels();
        if (_MyAnmation) {
            _MyAnmation->hide();
        }
        ui->widget->Setbtngroupstate(MybuttonGroup::Null);
        _Gamecontrol->RetCardDate();
        _Gamecontrol->SetCurrentPlayer(_Gamecontrol->GetUSer());
        break;
    case 1:
        createCardItemsInBatches();
        break;
    case 2:
        beginSerializedDealing();
        break;
    default:
        break;
    }
}
void Maingame::SatrtPend()
{
    beginSerializedDealing();
}
void Maingame::PlayHandtimer(player * Player,int Movetime)
{
    Q_UNUSED(Player)
    Q_UNUSED(Movetime)
}
void Maingame::ensureLordCardPanels()
{
    const LayoutMetrics m = metrics();
    ensureGraphicsView();
    for (int i = 0; i < 3; ++i) {
        if (_LordCards[i]) {
            continue;
        }
        _LordCards[i] = new CardPanel();
        _CardScene->addItem(_LordCards[i]);
        _LordCards[i]->setGeometry(GameScaling::rect(0, 0, m.lordCardSize.width(), m.lordCardSize.height()));
        _LordCards[i]->setimage(_Card_back, _Card_back);
        _LordCards[i]->setfront(false);
        _LordCards[i]->setVisible(false);
        _LordCards[i]->setZValue(50 + i);
    }
}

void Maingame::SetCurrentGameStatue(gamecontrol::GameState state)
{
    _CurrentGameState = state;
    switch(state)
    {
    case gamecontrol::PENDCARD:
        beginGameStartSequence();
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

void Maingame::PendCardplayer(player *player)
{
    Cards cards=player->GetCards();
    QListcard Listcard=cards.Listcardssort();

    for(auto i=0;i<Listcard.size();i++)
    {
        CardPanel* cardpenl=_CardPenelMap.value(Listcard[i], nullptr);
        if(cardpenl)
        {
            cardpenl->setowner(*player);
            cardpenl->setselect(false);
        }
    }
    if (!_GameLaunchPending || _PendingDealCount <= 0) {
        PendCardpos(player);
    }
}

void Maingame::PendCardpos(player* player) {
    if (!player || !_Playercontexts.contains(player)) {
        return;
    }

    const LayoutMetrics m = metrics();
    Cards cards = player->GetCards();
    QListcard sortedCards = cards.Listcardssort(Cards::ASC);
    _Playercontext* context = _Playercontexts[player];
    const QRect rect = context->_PLayerCardsRect;
    const int opponentCardSpace = GameScaling::y(m.verticalOverlap);

    if(context->_Align == Horizontal)
    {
        _PanelPositon.clear();
        _Mycardsrect = QRect();
    }

    for(int i = 0; i < sortedCards.size(); i++) {
        CardPanel* panel = _CardPenelMap.value(sortedCards[i], nullptr);
        if(!panel) continue;

        if(context->_Align == Horizontal) {
            panel->resize(GameScaling::size(m.playerCardSize.width(), m.playerCardSize.height()));
            const QPoint basePos = handBasePos(i, sortedCards.size(), false);
            const QRect temp(basePos.x(), basePos.y() - GameScaling::y(12), panel->width(), panel->height() + GameScaling::y(12));
            _PanelPositon.insert(panel, temp);
            _Mycardsrect = _Mycardsrect.united(temp);

            auto *posAnim = new QPropertyAnimation(panel, "pos");
            posAnim->setDuration(180);
            posAnim->setStartValue(panel->pos());
            posAnim->setEndValue(handBasePos(i, sortedCards.size(), panel->GetSelect()));
            posAnim->setEasingCurve(QEasingCurve::OutCubic);
            posAnim->start(QAbstractAnimation::DeleteWhenStopped);

            auto *rotAnim = new QPropertyAnimation(panel, "rotation");
            rotAnim->setDuration(180);
            rotAnim->setStartValue(panel->rotation());
            rotAnim->setEndValue(handRotation(i, sortedCards.size()));
            rotAnim->setEasingCurve(QEasingCurve::OutCubic);
            rotAnim->start(QAbstractAnimation::DeleteWhenStopped);

            panel->setfront(true);
            panel->setZValue(i);
        }
        else {
            const bool isLeft = (player == _Gamecontrol->GetLeftroot());
            panel->resize(GameScaling::size(m.lordCardSize.width(), m.lordCardSize.height()));
            panel->setfront(false);
            const int topY = GameScaling::y((272 - (m.lordCardSize.height() + qMax(0, sortedCards.size() - 1) * m.verticalOverlap)) / 2.0);
            const QPoint pos(rect.left(), topY + i * opponentCardSpace);
            auto *posAnim = new QPropertyAnimation(panel, "pos");
            posAnim->setDuration(150);
            posAnim->setStartValue(panel->pos());
            posAnim->setEndValue(pos);
            posAnim->setEasingCurve(QEasingCurve::OutCubic);
            posAnim->start(QAbstractAnimation::DeleteWhenStopped);
            panel->setRotation(0);
            panel->setZValue(isLeft ? i : 100 + i);
        }

        panel->setVisible(true);
    }

    if (context->_CountLabel) {
        if (context->_Align == Horizontal) {
            context->_CountLabel->hide();
        } else {
            context->_CountLabel->setText(QString::number(sortedCards.size()));
            context->_CountLabel->adjustSize();
            context->_CountLabel->move(rect.left(), rect.center().y() + GameScaling::y(36));
            context->_CountLabel->show();
            context->_CountLabel->raise();
        }
    }

    if(context->_Last_Cards != nullptr && !context->_Last_Cards->isempty()) {
        QRect location = context->_PlayerHandRect;
        QListcard list = context->_Last_Cards->Listcardssort();
        const int startX = location.left() + (location.width() - (m.tableCardSize.width() + qMax(0, list.size() - 1) * m.tableOverlap)) / 2;
        for(int i = 0; i < list.size(); i++) {
            CardPanel *tempPanel = _CardPenelMap.value(list.at(i), nullptr);
            if(!tempPanel) continue;
            tempPanel->resize(GameScaling::size(m.tableCardSize.width(), m.tableCardSize.height()));
            tempPanel->setfront(true);
            const QPoint targetPos(startX + i * GameScaling::x(m.tableOverlap), GameScaling::y(m.playAreaY));
            auto *playAnim = new QPropertyAnimation(tempPanel, "pos");
            playAnim->setDuration(220);
            playAnim->setStartValue(tempPanel->pos());
            playAnim->setEndValue(targetPos);
            playAnim->setEasingCurve(QEasingCurve::OutBack);
            playAnim->start(QAbstractAnimation::DeleteWhenStopped);
            tempPanel->setRotation(0);
            tempPanel->setZValue(300 + i);
            tempPanel->setVisible(true);
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
        const player *owner = temp->getowner();
        if (!owner) {
            return;
        }
        const Cards ownerCards = temp->getowner()->GetCards();
        const QListcard sorted = ownerCards.Listcardssort(Cards::ASC);
        const int index = sorted.indexOf(*temp->getcard());
        const bool willSelect = !temp->GetSelect();
        temp->animateSelection(willSelect, handBasePos(qMax(0, index), sorted.size(), false), GameScaling::y(12), 150);

        auto it = _SelcetPanel.find(temp);
        if (it == _SelcetPanel.end()) {
            _SelcetPanel.insert(temp);
            _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::SELECT_CARD);
        } else {
            _SelcetPanel.erase(it);
            temp->setselect(false);
        }
        QTimer::singleShot(160, this, [this, ownerPtr = temp->getowner()]() {
            if (ownerPtr) {
                PendCardpos(ownerPtr);
            }
        });
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
    if (_Timecount) {
        return;
    }

    _Timecount=new Timecount(this);
    _Timecount->move((width()-_Timecount->width())/2,(height()-_Timecount->height())/2 + GameScaling::y(100));
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
        _MyAnmation->setFixedSize(GameScaling::size(250,150));
        _MyAnmation->move((width()-_MyAnmation->width())/2,200);
        _MyAnmation->show();
        break;
    case(PlayHand::Hand_Bomb):
    case(PlayHand::Hand_Bomb_Jokers):
        _MyAnmation->ShowBom(type);
        _MyAnmation->setFixedSize(GameScaling::size(250,200));
        _MyAnmation->move((width()-_MyAnmation->width())/2,(height()-_MyAnmation->height())/2-70);
        _MyAnmation->show();

        // 添加炸弹音效
        _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::BOMB);
        break;
    case(PlayHand::Hand_Plane):
    case(PlayHand::Hand_Plane_Two_Single):
    case(PlayHand::Hand_Plane_Two_Pair):
        _MyAnmation->ShowPlane();
        _MyAnmation->setFixedSize(GameScaling::size(800,75));
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
    ensureGraphicsView();
    const LayoutMetrics m = metrics();
    if (!_PendCards) {
        _PendCards = new CardPanel();
        _CardScene->addItem(_PendCards);
    }
    if (!_MoveCards) {
        _MoveCards = new CardPanel();
        _CardScene->addItem(_MoveCards);
    }
    _PendCards->resize(GameScaling::size(m.deckCardSize.width(), m.deckCardSize.height()));
    _PendCards->setimage(_Card_back, _Card_back);
    _PendCards->setfront(false);
    _PendCards->setVisible(false);
    _MoveCards->resize(GameScaling::size(m.deckCardSize.width(), m.deckCardSize.height()));
    _MoveCards->setimage(_Card_back, _Card_back);
    _MoveCards->setfront(false);
    _MoveCards->setVisible(false);
    _Base_point = GameScaling::point(m.deckX, m.deckY);
    _PendCards->setPos(_Base_point);
    _MoveCards->setPos(_Base_point);
    ensureLordCardPanels();
}
void Maingame::ensureGraphicsView()
{
    if (_CardView && _CardScene) {
        return;
    }

    _CardScene = new QGraphicsScene(this);
    _CardScene->setSceneRect(0, 0, GameScaling::x(480), GameScaling::y(272));

    _CardView = new QGraphicsView(ui->centralwidget);
    _CardView->setGeometry(GameScaling::rect(0, 0, 480, 272));
    _CardView->setFrameShape(QFrame::NoFrame);
    _CardView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _CardView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _CardView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    _CardView->setStyleSheet("background: transparent;");
    _CardView->setScene(_CardScene);
    _CardView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    _CardView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    _CardView->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    _CardView->lower();
}

void Maingame::prepareCardPixmaps()
{
    _CardPixmapMap.clear();
    _IMage_Cards = GameResourceCache::pixmap(":/images/card.png");
    if (_IMage_Cards.isNull()) {
        qWarning() << "card.png 资源加载失败";
        return;
    }

    _IMage_Card_Size = GameScaling::size(metrics().playerCardSize.width(), metrics().playerCardSize.height());
    const int sheetCardWidth = _IMage_Cards.width() / 13;
    const int sheetCardHeight = _IMage_Cards.height() / 5;
    _Card_back = _IMage_Cards.copy(sheetCardWidth * 2, sheetCardHeight * 4, sheetCardWidth, sheetCardHeight);

    for (int i = 0; i < 13; ++i) {
        for (int j = 0; j < 4; ++j) {
            Card::cardpoint point = static_cast<Card::cardpoint>(i + 1);
            Card::cardsuit suit = static_cast<Card::cardsuit>(j + 1);
            Card temp(suit, point);
            QPixmap face = _IMage_Cards.copy(sheetCardWidth * i, sheetCardHeight * j, sheetCardWidth, sheetCardHeight);
            _CardPixmapMap.insert(temp, qMakePair(face, _Card_back));
        }
    }

    _CardPixmapMap.insert(Card(Card::Suit_Begin, Card::Card_SJ), qMakePair(_IMage_Cards.copy(0, sheetCardHeight * 4, sheetCardWidth, sheetCardHeight), _Card_back));
    _CardPixmapMap.insert(Card(Card::Suit_Begin, Card::Card_BJ), qMakePair(_IMage_Cards.copy(sheetCardWidth, sheetCardHeight * 4, sheetCardWidth, sheetCardHeight), _Card_back));
}

void Maingame::createCardItemsInBatches()
{
    ensureGraphicsView();
    if (_CardCreationRunning || !_CardPenelMap.isEmpty()) {
        finalizeCardItemCreation();
        return;
    }

    _CardCreationQueue = _CardPixmapMap.keys().toVector();
    _CardCreationIndex = 0;
    _CardCreationRunning = true;

    auto *batchTimer = new QTimer(this);
    batchTimer->setSingleShot(false);
    connect(batchTimer, &QTimer::timeout, this, [this, batchTimer]() {
        const int batchSize = 6;
        const LayoutMetrics m = metrics();
        for (int count = 0; count < batchSize && _CardCreationIndex < _CardCreationQueue.size(); ++count, ++_CardCreationIndex) {
            const Card key = _CardCreationQueue.at(_CardCreationIndex);
            if (_CardPenelMap.contains(key)) {
                continue;
            }
            CardPanel *item = new CardPanel();
            _CardScene->addItem(item);
            item->resize(GameScaling::size(m.playerCardSize.width(), m.playerCardSize.height()));
            const auto pix = _CardPixmapMap.value(key);
            item->setimage(pix.first, pix.second);
            item->setcard(new Card(key));
            item->setfront(false);
            item->setVisible(false);
            item->setPos(_Base_point);
            connect(item, &CardPanel::S_Cardsselect, this, &Maingame::Cardpanel, Qt::UniqueConnection);
            _CardPenelMap.insert(key, item);
        }
        if (_CardCreationIndex < _CardCreationQueue.size()) {
            return;
        }
        batchTimer->stop();
        batchTimer->deleteLater();
        _CardCreationRunning = false;
        finalizeCardItemCreation();
    });
    batchTimer->start(0);
}

void Maingame::finalizeCardItemCreation()
{
    for (auto it = _CardPenelMap.begin(); it != _CardPenelMap.end(); ++it) {
        it.value()->setVisible(false);
        it.value()->setPos(_Base_point);
        it.value()->setRotation(0);
        it.value()->setZValue(0);
        it.value()->setfront(false);
        it.value()->setselect(false);
    }
}

int Maingame::handSpacing(int cardCount) const
{
    return GameScaling::x(cardCount <= 18 ? 20 : 16);
}

QPoint Maingame::handBasePos(int index, int cardCount, bool selected) const
{
    const int rawSpacing = cardCount <= 18 ? 20 : 16;
    const int spacing = GameScaling::x(rawSpacing);
    const int startX = GameScaling::x((480 - (36 + qMax(0, cardCount - 1) * rawSpacing)) / 2.0);
    const qreal center = (cardCount - 1) / 2.0;
    const int y = GameScaling::y(215 + qRound(qAbs(index - center) * 2.0) - (selected ? 12 : 0));
    return QPoint(startX + index * spacing, y);
}

qreal Maingame::handRotation(int index, int cardCount) const
{
    const qreal center = (cardCount - 1) / 2.0;
    return (index - center) * 2.0;
}

void Maingame::beginSerializedDealing()
{
    if (_CardCreationRunning) {
        QTimer::singleShot(20, this, &Maingame::beginSerializedDealing);
        return;
    }
    if (!_Gamecontrol || !_Bgmcontrol || !_MyAnmation) {
        return;
    }

    ui->widget->SetStartButtonVisible(false);
    _CanSelectCards = false;
    ClearSelectedPanels();
    _PendingDealCount = 51;

    for (auto it = _Playercontexts.begin(); it != _Playercontexts.end(); ++it) {
        it.value()->_Mycards->clearcards();
        it.value()->Isfront = false;
        if (it.value()->_Last_Cards) {
            it.value()->_Last_Cards->clearcards();
        }
        it.value()->_NOCardlabel->hide();
        if (it.value()->_CountLabel) {
            it.value()->_CountLabel->hide();
        }
    }

    _MyAnmation->hide();
    _PendCards->setPos(_Base_point);
    _MoveCards->setPos(_Base_point);
    _PendCards->setVisible(true);
    _MoveCards->setVisible(false);

    for (auto it = _CardPenelMap.begin(); it != _CardPenelMap.end(); ++it) {
        it.value()->setVisible(false);
        it.value()->setPos(_Base_point);
        it.value()->setRotation(0);
        it.value()->setselect(false);
        it.value()->setfront(false);
        it.value()->setZValue(0);
    }

    _Gamecontrol->SetCurrentPlayer(_Gamecontrol->GetUSer());
    _Bgmcontrol->OtherBgm(Bgmcontrol::OtherSound::DISPATCH);
    scheduleDealStep();
}

void Maingame::scheduleDealStep()
{
    if (_PendingDealCount <= 0) {
        _Bgmcontrol->StopOtherBgm();
        _MoveCards->setVisible(false);
        _PendCards->setVisible(false);

        QVector<Card*> lordCards;
        for (int i = 0; i < 3; ++i) {
            Card* card = _Gamecontrol->TakeOneCard();
            if (card) lordCards.append(card);
        }
        std::sort(lordCards.begin(), lordCards.end(), [](Card *a, Card *b) {
            return a && b ? *a < *b : a < b;
        });
        const LayoutMetrics m = metrics();
        const int totalWidth = m.lordCardSize.width() + qMax(0, lordCards.size() - 1) * m.lordOverlap;
        const int baseX = GameScaling::x((480 - totalWidth) / 2.0);
        for (int i = 0; i < lordCards.size() && i < 3; ++i) {
            CardPanel *lordPanel = _LordCards[i];
            auto source = _CardPenelMap.value(*lordCards[i], nullptr);
            if (!lordPanel || !source) continue;
            lordPanel->resize(GameScaling::size(m.lordCardSize.width(), m.lordCardSize.height()));
            lordPanel->setimage(source->Getimagefont(), _Card_back);
            lordPanel->setfront(false);
            lordPanel->setPos(baseX + GameScaling::x(i * m.lordOverlap), GameScaling::y(m.topOpponentY));
            lordPanel->setVisible(true);
        }
        for (Card *card : lordCards) {
            _Gamecontrol->GetAllCards()->add(card);
        }
        _GameLaunchPending = false;
        SetCurrentGameStatue(gamecontrol::GETLORD);
        return;
    }

    player *targetPlayer = _Gamecontrol->GetCurrentPlayer();
    if (!targetPlayer) {
        targetPlayer = _Gamecontrol->GetUSer();
        _Gamecontrol->SetCurrentPlayer(targetPlayer);
    }

    Card *drawnCard = _Gamecontrol->TakeOneCard();
    if (!drawnCard) {
        _GameLaunchPending = false;
        return;
    }

    Card key(drawnCard->getcardsuit(), drawnCard->getcardpoint());
    CardPanel *panel = _CardPenelMap.value(key, nullptr);
    if (!panel) {
        _GameLaunchPending = false;
        return;
    }

    targetPlayer->StoreGetCard(drawnCard);
    Cards cards = targetPlayer->GetCards();
    const int count = cards.GetCardtotal();
    const int index = qMax(0, count - 1);
    QPoint targetPos;
    qreal targetRotation = 0;
    const LayoutMetrics m = metrics();
    if (targetPlayer == _Gamecontrol->GetUSer()) {
        targetPos = handBasePos(index, count, false);
        targetRotation = handRotation(index, count);
        panel->resize(GameScaling::size(m.playerCardSize.width(), m.playerCardSize.height()));
    } else if (targetPlayer == _Gamecontrol->GetLeftroot()) {
        panel->resize(GameScaling::size(m.lordCardSize.width(), m.lordCardSize.height()));
        targetPos = scaledPoint(m.leftRobotX, 40 + (qMin(count, 10) - 1) * m.verticalOverlap);
    } else {
        panel->resize(GameScaling::size(m.lordCardSize.width(), m.lordCardSize.height()));
        targetPos = scaledPoint(m.rightRobotX, 40 + (qMin(count, 10) - 1) * m.verticalOverlap);
    }

    panel->setPos(_Base_point);
    panel->setRotation(0);
    panel->setVisible(true);
    panel->setZValue(999);
    panel->setfront(false);

    auto *moveAnim = new QPropertyAnimation(panel, "pos");
    moveAnim->setDuration(260);
    moveAnim->setStartValue(_Base_point);
    moveAnim->setEndValue(targetPos);
    moveAnim->setEasingCurve(QEasingCurve::OutQuint);
    moveAnim->start(QAbstractAnimation::DeleteWhenStopped);

    auto *rotAnim = new QPropertyAnimation(panel, "rotation");
    rotAnim->setDuration(260);
    rotAnim->setStartValue(0);
    rotAnim->setEndValue(targetRotation);
    rotAnim->setEasingCurve(QEasingCurve::OutQuint);
    rotAnim->start(QAbstractAnimation::DeleteWhenStopped);

    panel->animateFlip(targetPlayer == _Gamecontrol->GetUSer());

    QTimer::singleShot(220, this, [this, targetPlayer]() {
        PendCardpos(targetPlayer);
    });

    _PendingDealCount -= 1;
    _Gamecontrol->SetCurrentPlayer(targetPlayer->GetNextPlayer());
    QTimer::singleShot(60, this, &Maingame::scheduleDealStep);
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