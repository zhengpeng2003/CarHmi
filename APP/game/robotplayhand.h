#ifndef ROBOTPLAYHAND_H
#define ROBOTPLAYHAND_H

#include <QObject>
#include <QThread>
#include "player.h"
class RobotPlayhand : public QThread
{
    Q_OBJECT
public:
    explicit RobotPlayhand(player *player,QObject *parent);
protected:
    virtual void run()override;
private:
    player * _Player;
};


#endif // ROBOTPLAYHAND_H
