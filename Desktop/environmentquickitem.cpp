#include "environmentquickitem.h"

#include <QColor>
#include <QImage>
#include <QPainter>

EnvironmentQuickItem::EnvironmentQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);

    m_carDriveAnimation = new QPropertyAnimation(this, "carOffsetX", this);
    m_carDriveAnimation->setDuration(900);
    m_carDriveAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void EnvironmentQuickItem::paint(QPainter *painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QRectF fullRect(0, 0, width(), height());
    painter->setBrush(QColor(60, 60, 60));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(fullRect, 15, 15);

    if (!m_cachedCarPixmap.isNull()) {
        painter->drawPixmap(baseCarTopLeft() + QPointF(m_carOffsetX, 0), m_cachedCarPixmap);
    }

    const QString tempText = QStringLiteral("温度: %1°C").arg(m_temperature, 0, 'f', 1);
    const QString humText = QStringLiteral("湿度: %1%").arg(m_humidity, 0, 'f', 0);

    QFont font = painter->font();
    font.setPointSize(16);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::white);

    const qreal yOffset = height() / 2.0 + 20.0;
    painter->drawText(QRectF(0, yOffset, width(), 30), Qt::AlignCenter, tempText);
    painter->drawText(QRectF(0, yOffset + 35, width(), 30), Qt::AlignCenter, humText);
}

double EnvironmentQuickItem::temperature() const
{
    return m_temperature;
}

double EnvironmentQuickItem::humidity() const
{
    return m_humidity;
}

QString EnvironmentQuickItem::backgroundImage() const
{
    return m_backgroundImage;
}

int EnvironmentQuickItem::carOffsetX() const
{
    return m_carOffsetX;
}

void EnvironmentQuickItem::startCarDriveAnimation()
{
    if (m_cachedCarPixmap.isNull() || !m_carDriveAnimation) {
        return;
    }

    const int startOffset = int(width()) + 20 - int(baseCarTopLeft().x());
    m_carDriveAnimation->stop();
    setCarOffsetX(startOffset);
    m_carDriveAnimation->setStartValue(startOffset);
    m_carDriveAnimation->setEndValue(0);
    m_carDriveAnimation->start();
}

bool EnvironmentQuickItem::hitCar(qreal x, qreal y) const
{
    return carRect().contains(QPointF(x, y));
}

void EnvironmentQuickItem::setTemperature(double temperature)
{
    if (qFuzzyCompare(m_temperature, temperature)) {
        return;
    }

    m_temperature = temperature;
    update();
    emit temperatureChanged();
}

void EnvironmentQuickItem::setHumidity(double humidity)
{
    if (qFuzzyCompare(m_humidity, humidity)) {
        return;
    }

    m_humidity = humidity;
    update();
    emit humidityChanged();
}

void EnvironmentQuickItem::setBackgroundImage(const QString &backgroundImage)
{
    if (m_backgroundImage == backgroundImage) {
        return;
    }

    m_backgroundImage = backgroundImage;
    m_backgroundPixmap.load(backgroundImage);
    rebuildCache();
    update();
    emit backgroundImageChanged();
}

void EnvironmentQuickItem::setCarOffsetX(int carOffsetX)
{
    if (m_carOffsetX == carOffsetX) {
        return;
    }

    m_carOffsetX = carOffsetX;
    update();
    emit carOffsetXChanged();
}

void EnvironmentQuickItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        rebuildCache();
        update();
    }
}

void EnvironmentQuickItem::rebuildCache()
{
    if (m_backgroundPixmap.isNull() || width() <= 0 || height() <= 0) {
        m_cachedCarPixmap = QPixmap();
        return;
    }

    QImage img = m_backgroundPixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const QColor c(line[x]);
            if (c.red() > 240 && c.green() > 240 && c.blue() > 240) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    const int iconWidth = qMin(200, qMax(120, int(width()) - 40));
    const int iconHeight = qMin(200, qMax(120, int(height()) - 80));
    m_cachedCarPixmap = QPixmap::fromImage(img).scaled(iconWidth,
                                                       iconHeight,
                                                       Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation);
}

QPointF EnvironmentQuickItem::baseCarTopLeft() const
{
    return QPointF(width() / 2.0 - m_cachedCarPixmap.width() / 2.0,
                   height() / 2.0 - m_cachedCarPixmap.height() / 2.0 - 30.0);
}

QRectF EnvironmentQuickItem::carRect() const
{
    if (m_cachedCarPixmap.isNull()) {
        return QRectF();
    }

    return QRectF(baseCarTopLeft() + QPointF(m_carOffsetX, 0), m_cachedCarPixmap.size());
}
