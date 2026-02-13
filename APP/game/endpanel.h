#ifndef ENDPANEL_H
#define ENDPANEL_H
#include <player.h>
#include <QWidget>
#include <QPushButton>
#include <showscore.h>
#include <QLabel>
#include <qpainter.h>
class EndPanel : public QWidget
{
    Q_OBJECT
public:
    explicit EndPanel(player * player,QWidget *parent = nullptr);
protected:
    virtual void paintEvent(QPaintEvent *event)override;
signals:
    void S_Continue();
private:
    player * _Player;//通过穿进来的用户获取分数
    QPixmap _Gameoverpix;
};

#endif // ENDPANEL_H
