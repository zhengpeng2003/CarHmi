#include "cardpanel.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>

CardPanel::CardPanel(QGraphicsItem *parent)
    : QGraphicsObject(parent), _size(36, 48)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setTransformOriginPoint(_size.width() / 2.0, _size.height() / 2.0);
}

QRectF CardPanel::boundingRect() const
{
    return QRectF(0, 0, _size.width(), _size.height());
}

void CardPanel::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF bounds = boundingRect();
    QPainterPath clipPath;
    clipPath.addRoundedRect(bounds.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
    painter->setClipPath(clipPath);

    const QPixmap pix = getimage();
    if (!pix.isNull()) {
        painter->save();
        painter->translate(bounds.center());
        painter->scale(_flipScale, 1.0);
        painter->translate(-bounds.center());
        painter->drawPixmap(bounds.toRect(), pix);
        painter->restore();
    } else {
        painter->fillPath(clipPath, Qt::darkGreen);
    }
}

void CardPanel::setimage(const QPixmap &map_fornt, const QPixmap &map_back)
{
    _Map_front = map_fornt;
    _Map_back = map_back;
    update();
}

QPixmap CardPanel::getimage() const { return is_front ? _Map_front : _Map_back; }
QPixmap CardPanel::Getimagefont() const { return _Map_front; }
QPixmap CardPanel::Getimageback() const { return _Map_back; }
player *CardPanel::getowner() const { return _Player; }
void CardPanel::setowner(player &play) { _Player = &play; }
Card *CardPanel::getcard() const { return _Card; }
void CardPanel::setcard(Card *card) { _Card = card; }

void CardPanel::setselect(bool selected)
{
    is_select = selected;
}

void CardPanel::Click()
{
    emit S_Cardsselect(Qt::LeftButton);
}

void CardPanel::setfront(bool front)
{
    if (is_front == front) {
        return;
    }
    is_front = front;
    update();
}

bool CardPanel::getfront() const { return is_front; }
bool CardPanel::GetSelect() const { return is_select; }

void CardPanel::resize(const QSize &size)
{
    prepareGeometryChange();
    _size = size;
    setTransformOriginPoint(_size.width() / 2.0, _size.height() / 2.0);
    update();
}

void CardPanel::resize(int w, int h) { resize(QSize(w, h)); }

void CardPanel::setGeometry(const QRect &rect)
{
    resize(rect.size());
    setPos(rect.topLeft());
}

void CardPanel::move(const QPoint &point) { setPos(point); }
void CardPanel::move(int x, int y) { setPos(x, y); }
int CardPanel::width() const { return qRound(_size.width()); }
int CardPanel::height() const { return qRound(_size.height()); }
QRect CardPanel::rect() const { return QRect(0, 0, width(), height()); }
void CardPanel::raise() { setZValue(zValue() + 1.0); }

void CardPanel::setFlipScale(qreal scale)
{
    _flipScale = scale;
    update();
}

qreal CardPanel::flipScale() const
{
    return _flipScale;
}

void CardPanel::animateSelection(bool selected, const QPointF &basePos, int raiseOffset, int durationMs)
{
    setselect(selected);
    const QPointF endPos = basePos + QPointF(0, selected ? -raiseOffset : 0);

    auto *posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(durationMs);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);
    posAnim->setStartValue(pos());
    posAnim->setEndValue(endPos);
    posAnim->start(QAbstractAnimation::DeleteWhenStopped);

    if (selected) {
        if (!_shadowEffect) {
            _shadowEffect = new QGraphicsDropShadowEffect();
            _shadowEffect->setBlurRadius(18.0);
            _shadowEffect->setOffset(0, 4);
            _shadowEffect->setColor(QColor(0, 0, 0, 180));
        }
        setGraphicsEffect(_shadowEffect);
    } else {
        setGraphicsEffect(nullptr);
    }
}

void CardPanel::animateFlip(bool showFrontAfterFlip, int durationMs)
{
    auto *shrink = new QPropertyAnimation(this, "flipScale");
    shrink->setDuration(durationMs / 2);
    shrink->setStartValue(1.0);
    shrink->setEndValue(0.0);
    connect(shrink, &QPropertyAnimation::finished, this, [this, showFrontAfterFlip, durationMs]() {
        setfront(showFrontAfterFlip);
        auto *expand = new QPropertyAnimation(this, "flipScale");
        expand->setDuration(durationMs / 2);
        expand->setStartValue(0.0);
        expand->setEndValue(1.0);
        expand->start(QAbstractAnimation::DeleteWhenStopped);
    });
    shrink->start(QAbstractAnimation::DeleteWhenStopped);
}

void CardPanel::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit S_Cardsselect(event->button());
    QGraphicsObject::mousePressEvent(event);
}
