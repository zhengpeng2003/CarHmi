#include "environmentwidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QImage>
#include <QMouseEvent>

EnvironmentWidget::EnvironmentWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_carDriveAnimation = new QPropertyAnimation(this, "carOffsetX", this);
    m_carDriveAnimation->setDuration(900);
    m_carDriveAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void EnvironmentWidget::setBackgroundImage(const QString &path)
{
    m_bg.load(path);
    rebuildCache();
    update();
}

void EnvironmentWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    rebuildCache();
}

void EnvironmentWidget::setCarOffsetX(int offset)
{
    if (m_carOffsetX == offset) {
        return;
    }

    m_carOffsetX = offset;
    update();
}

void EnvironmentWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && carRect().contains(event->pos())) {
        startCarDriveAnimation();
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void EnvironmentWidget::rebuildCache()
{
    if (m_bg.isNull() || width() <= 0 || height() <= 0) {
        m_cachedCarPixmap = QPixmap();
        return;
    }

    QImage img = m_bg.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const QColor c(line[x]);
            if (c.red() > 240 && c.green() > 240 && c.blue() > 240) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    const int iconWidth = qMin(200, qMax(120, width() - 40));
    const int iconHeight = qMin(200, qMax(120, height() - 80));
    m_cachedCarPixmap = QPixmap::fromImage(img).scaled(iconWidth,
                                                       iconHeight,
                                                       Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation);
}

QPoint EnvironmentWidget::baseCarTopLeft() const
{
    return rect().center() - QPoint(m_cachedCarPixmap.width() / 2,
                                    m_cachedCarPixmap.height() / 2 + 30);
}

QRect EnvironmentWidget::carRect() const
{
    if (m_cachedCarPixmap.isNull()) {
        return QRect();
    }

    return QRect(baseCarTopLeft() + QPoint(m_carOffsetX, 0), m_cachedCarPixmap.size());
}

void EnvironmentWidget::startCarDriveAnimation()
{
    if (m_cachedCarPixmap.isNull() || !m_carDriveAnimation) {
        return;
    }

    const int startOffset = width() + 20 - baseCarTopLeft().x();
    m_carDriveAnimation->stop();
    setCarOffsetX(startOffset);
    m_carDriveAnimation->setStartValue(startOffset);
    m_carDriveAnimation->setEndValue(0);
    m_carDriveAnimation->start();
}

void EnvironmentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int radius = 15;
    painter.setBrush(QColor(60, 60, 60));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), radius, radius);

    if (!m_cachedCarPixmap.isNull()) {
        painter.drawPixmap(baseCarTopLeft() + QPoint(m_carOffsetX, 0), m_cachedCarPixmap);
    }

    const QString tempText = QStringLiteral("温度: %1°C").arg(m_temperature, 0, 'f', 1);
    const QString humText  = QStringLiteral("湿度: %1%").arg(m_humidity, 0, 'f', 0);

    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);

    const int yOffset = rect().height() / 2 + 20;
    painter.drawText(QRect(0, yOffset, rect().width(), 30), Qt::AlignCenter, tempText);
    painter.drawText(QRect(0, yOffset + 35, rect().width(), 30), Qt::AlignCenter, humText);
}
