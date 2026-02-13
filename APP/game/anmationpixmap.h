#ifndef ANMATIONPIXMAP_H
#define ANMATIONPIXMAP_H

#include <QObject>
#include <QWidget>
#include "qpixmap.h"
#include "QTimer"
#include "QPainter"
#include "playhand.h"
class AnmationPixmap : public QWidget
{
    Q_OBJECT
public:
    explicit AnmationPixmap(QWidget *parent = nullptr);
    void ShowBet(int Bet);
    void ShowSimsqe(PlayHand::HandType type);
    void ShowBom(PlayHand::HandType type);
    void ShowPlane();
protected:
    virtual void paintEvent(QPaintEvent *event)override;
private:
    QPixmap _Mypix;
    QTimer *_Timer;
    int _count=0;
    int _Mx=0;
};

#endif // ANMATIONPIXMAP_H
