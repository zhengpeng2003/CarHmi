#include "player.h"
#include "qdebug.h"

player::player(QObject *parent)
    : QObject{parent}
{}

player::player(QString Name, PlayerType playerType, Location location, QObject *parent)
    : QObject(parent),           // 先初始化基类
    _Isfront(false),
    _Name(Name),
    _Score(0),   // 参数名避免与成员变量相同
    _PlayerType(playerType),
    _Location(location),      // 添加缺失的初始化
    _Role(UN_KONWROLE),        // 根据你的枚举类型设置默认值
    _Sex(UN_KONWSEX),
    _Preplayer(nullptr),
    _Nextplayer(nullptr),
    _PendPlayer(nullptr),
    _PendCard(nullptr),           // 添加缺失的初始化
    _Win(false)                // 添加缺失的初始化
{
    // 构造函数体
}
int  player::GetScore()
{
    return _Score;
}

void player::SetScore(int score)
{
    this->_Score=score;
}

player *player::GetPrePlayer()
{
    return this->_Preplayer;
}

player *player::GetNextPlayer()
{
    return this->_Nextplayer;
}

void player::StoreGetCard(Card *card)
{
    this->_Cards.add(card);
    notifystorecard(this);
}

void player::StoreGetCards(Cards *cards)
{
    this->_Cards.add(cards);
    notifystorecard(this);
}

Cards player::GetCards()
{
    return _Cards;
}

void player::RemoveCards(Cards *cards)
{
    if (cards)
    {
        _Cards.remove(cards);
    }
}

void player::ClearCards()
{
    this->_Cards.clearcards();
}

void player::PlayHand(Cards *cards)
{
    emit notifyplayhand(this,cards);
}

void player::SetPendinfo(player *PendPlayer, Cards *PendCard)
{
    this->_PendPlayer=PendPlayer;

    this->_PendCard=PendCard;
}

player *player::GetPendPlayer()
{
    return this->_PendPlayer;
}

Cards *player::GetPendCard()
{
    return this->_PendCard;
}

void player::Setwin(bool is_win)
{
    _Win=is_win;

}

bool player::Getwin()
{
    return _Win;
}

QPixmap player::GetPlayerRolePixmap(Role Role, Sex Sex, Location Location)
{
    int temp = QRandomGenerator::global()->bounded(2);
    QPixmap res;
    QImage Image;

    QVector<QString> farmman;
    QVector<QString> farmwoman;
    QVector<QString> lordman;
    QVector<QString> lordwoman;

    farmman << ":/images/farmer_man_1.png" << ":/images/farmer_man_2.png";
    farmwoman << ":/images/farmer_woman_1.png" << ":/images/farmer_woman_2.png";
    lordman << ":/images/lord_man_1.png" << ":/images/lord_man_2.png";
    lordwoman << ":/images/lord_woman_1.png" << ":/images/lord_woman_2.png";

    // 修正逻辑：先判断角色，再判断性别
    if(Role == player::FORMAR)  // 农民角色
    {
        if(Sex == player::MAN)  // 男性农民
        {
            Image.load(farmman[temp]);
        }
        else if(Sex == player::WOMAN)  // 女性农民
        {
            Image.load(farmwoman[temp]);
        }
    }
    else if(Role == player::LORD)  // 地主角色
    {
        if(Sex == player::MAN)  // 男性地主
        {
            Image.load(lordman[temp]);
        }
        else if(Sex == player::WOMAN)  // 女性地主
        {
            Image.load(lordwoman[temp]);
        }
    }

    // 确保图片加载成功
    if(Image.isNull()) {
        qWarning() << "玩家角色图片加载失败，使用默认图片";
        // 可以在这里加载一个默认图片
        Image.load(":/images/default_player.png");
    }

    // 根据位置决定是否镜像
    if(Location == LEFT) {
        res = QPixmap::fromImage(Image.mirrored(true, false));
    } else {
        res = QPixmap::fromImage(Image);
    }

    return res;
}

void player::PrepareGetLord()
{

}

void player::PreparePlayCard()
{

}

void player::RobotGetLard()
{

}

void player::RobotThinkPlayhand()
{

}

player::Location player::GetLocation()
{
    return _Location;
}

player::Sex player::GetSex()
{
    return _Sex;

}

player::Role player::GetRole()
{
    return _Role;
}



void player::SetPrePlayer(player *a)
{
    this->_Preplayer=a;
}

void player::SetNextPlayer(player *a)
{
    this->_Nextplayer=a;
}

void player::SetSex(Sex a)
{
    this->_Sex=a;
}

void player::setRole(Role a)
{
    this->_Role=a;
}

void player::Setfront(bool Isfront)
{
    this->_Isfront=Isfront;
}

bool player::GetIsfront()
{
    return _Isfront;
}

void player::ResetForNewGame()
{
    _Role = UN_KONWROLE;
    _PendPlayer = nullptr;
    _PendCard = nullptr;
    _Win = false;
    _Isfront = false;
}

void player::grablordbet(int bet)
{
    emit notifygrablordbet(this,bet);
}

