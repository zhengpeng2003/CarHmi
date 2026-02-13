#ifndef USER_H
#define USER_H

#include <QObject>
#include <player.h>
class user:public player
{
    Q_OBJECT
public:
    using player::player;
    virtual void PrepareGetLord() override;//抢地主
    virtual void PreparePlayCard() override;//出牌

    user();
};

#endif // USER_H
