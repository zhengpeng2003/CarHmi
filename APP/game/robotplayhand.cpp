#include "robotplayhand.h"

RobotPlayhand::RobotPlayhand(player *player,QObject *parent): QThread(parent)
{
    this->_Player=player;
}

void RobotPlayhand::run()
{
    msleep(1700);
    _Player->RobotThinkPlayhand();
}
