#include "cardpanel.h"

#include <QMouseEvent>

CardPanel::CardPanel(QWidget *parent)
    : QWidget{parent},is_front(0),_Card(nullptr),_Player(nullptr),is_select(false)
{
    this->setFixedSize(65,100);
}

void CardPanel::setimage(const QPixmap &map_fornt, const QPixmap &map_back)
{

    this->_Map_front=map_fornt;
    this->_Map_back=map_back;


}
QPixmap CardPanel::getimage()
{

    if(is_front)
        return _Map_front;
    else
        return _Map_back;
}

QPixmap CardPanel::Getimagefont()
{
    return _Map_front;
}

QPixmap CardPanel::Getimageback()
{
    return _Map_back;
}

player *CardPanel::getowner()
{
    return _Player;
}
void CardPanel::setowner(player& player)
{
    this->_Player=&player;
}

Card *CardPanel::getcard()
{
    return _Card;
};
void CardPanel::setcard(Card *card)
{
    this->_Card=card;
}
void CardPanel::setselect(bool is_select)
{
    this->is_select=is_select;
}
void CardPanel::Click()
{
    emit S_Cardsselect(Qt::LeftButton);
}

void CardPanel::setfront(bool is_front)
{

    this->is_front=is_front;
    update();
}
bool CardPanel::getfront()
{
    return is_front;
}

bool CardPanel::GetSelect()
{
    return is_select;
}
void CardPanel::paintEvent(QPaintEvent *event)
{
    QPainter p1(this);
    p1.drawPixmap(rect(),getimage());

}
 void CardPanel::mousePressEvent(QMouseEvent *event)
 {

    emit S_Cardsselect(event->button());
    return QWidget::mousePressEvent(event);
 }


