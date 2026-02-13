#ifndef ROBOTGETLOARD_H
#define ROBOTGETLOARD_H

#include <QObject>
#include <QThread>
#include "player.h"

class Robotgetloard : public QThread
{
    Q_OBJECT
public:
    explicit Robotgetloard(player * player,QObject * parent);

protected:
    virtual void run()override;
private:
    player * _Player;
};

#endif // ROBOTGETLOARD_H
