#ifndef CARDPANEL_H
#define CARDPANEL_H

#include <QGraphicsDropShadowEffect>
#include <QGraphicsObject>
#include <QPixmap>
#include <QPointer>
#include <QPropertyAnimation>

#include "card.h"
#include "player.h"

class CardPanel : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
    Q_PROPERTY(qreal flipScale READ flipScale WRITE setFlipScale)
public:
    explicit CardPanel(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setimage(const QPixmap &map_fornt, const QPixmap &map_back);
    QPixmap getimage() const;
    QPixmap Getimagefont() const;
    QPixmap Getimageback() const;

    player *getowner() const;
    void setowner(player &play);

    Card *getcard() const;
    void setcard(Card *card);

    void setselect(bool is_select);
    void Click();
    void setfront(bool is_front);
    bool getfront() const;
    bool GetSelect() const;

    void resize(const QSize &size);
    void resize(int w, int h);
    void setGeometry(const QRect &rect);
    void move(const QPoint &point);
    void move(int x, int y);
    int width() const;
    int height() const;
    QRect rect() const;
    void raise();

    void setFlipScale(qreal scale);
    qreal flipScale() const;
    void animateSelection(bool selected, const QPointF &basePos, int raiseOffset = 12, int durationMs = 150);
    void animateFlip(bool showFrontAfterFlip, int durationMs = 120);

signals:
    void S_Cardsselect(Qt::MouseButton event);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QSizeF _size;
    Card *_Card = nullptr;
    player *_Player = nullptr;
    QPixmap _Map_front;
    QPixmap _Map_back;
    bool is_select = false;
    bool is_front = false;
    qreal _flipScale = 1.0;
    QPointer<QGraphicsDropShadowEffect> _shadowEffect;
};

#endif // CARDPANEL_H
