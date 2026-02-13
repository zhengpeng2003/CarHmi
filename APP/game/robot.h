#pragma once
#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <player.h>
#include <strategy.h>
#include <QVector>
#include <mybuttongroup.h>
#include <robotgetloard.h>
#include <robotplayhand.h>
#include "strategy.h"
class gamecontrol;
class robot:public player
{
    Q_OBJECT
public:
    using player::player;//继承他们的父类
    virtual void PrepareGetLord() override;//抢地主 这个overr可以检查重写的名字是否正确
    virtual void PreparePlayCard() override;//出牌
    virtual void RobotGetLard()override;//考虑抢地主
    virtual void RobotThinkPlayhand() override;

    robot();

};

#endif // ROBOT_H
