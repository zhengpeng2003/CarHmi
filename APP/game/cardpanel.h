#ifndef CARDPANEL_H
#define CARDPANEL_H
#include <card.h>
#include <QWidget>
#include <player.h>
#include <QPixmap>
#include <QPainter>
class CardPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CardPanel(QWidget *parent = nullptr);
    void setimage(const QPixmap& map_fornt, const QPixmap& map_back);

    QPixmap getimage();
    QPixmap Getimagefont();
    QPixmap Getimageback();
    player* getowner();
    void setowner(player &play);

    Card *getcard();
    void setcard(Card *card);

    void setselect(bool is_select);
    void Click();
    void setfront(bool is_front);
    bool getfront();
    bool GetSelect();
protected :
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
signals:
    void S_Cardsselect(Qt::MouseButton event);
private:
    Card *_Card;
    player *_Player;
    QPixmap _Map_front;
    QPixmap _Map_back;
    bool is_select;
    bool is_front;

};

#endif // CARDPANEL_H
