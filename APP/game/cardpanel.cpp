#include "cardpanel.h"

#include <QMouseEvent>

CardPanel::CardPanel(QWidget *parent)
    : QWidget{parent},is_front(0),_Card(nullptr),_Player(nullptr),is_select(false)
{
    this->setFixedSize(20,10);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
}

void CardPanel::setimage(const QPixmap &map_fornt, const QPixmap &map_back)
{

    this->_Map_front=map_fornt;
    this->_Map_back=map_back;
    if(!_Map_front.isNull())
    {
        setCardSize(_Map_front.size());
    }
    else if(!_Map_back.isNull())
    {
        setCardSize(_Map_back.size());
    }


}
void CardPanel::setPresentation(const QSize &panelSize, const QRect &drawRect)
{
    if(panelSize.isValid())
    {
        setFixedSize(panelSize);
    }

    const QRect fallback(QPoint(0, 0), size());
    _DrawRect = drawRect.isValid() ? drawRect : fallback;
    setMask(QRegion(_DrawRect));
    update();
}

void CardPanel::setCardSize(const QSize &size)
{
    if(size.isValid())
    {
        setPresentation(size, QRect(QPoint(0, 0), size));
    }
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
    Q_UNUSED(event);
    QPainter p1(this);
    p1.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QRect targetRect = _DrawRect.isValid() ? _DrawRect : rect();
    p1.drawPixmap(targetRect, getimage());

}
 void CardPanel::mousePressEvent(QMouseEvent *event)
 {
    if(_DrawRect.isValid() && !_DrawRect.contains(event->pos()))
    {
        event->ignore();
        return;
    }
    emit S_Cardsselect(event->button());
    event->accept();
 }


