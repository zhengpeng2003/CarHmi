#include "videowidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QFont>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent),
    m_showGuideLine(true),
    m_nightMode(false),
    m_paused(false),
    m_distanceText("1.2m")
{
    setFixedSize(320, 180);
}

void VideoWidget::setFrame(const QImage &img)
{
    m_frame = img;
    update();
}

void VideoWidget::setShowGuideLine(bool on)
{
    m_showGuideLine = on;
    update();
}

void VideoWidget::setNightMode(bool on)
{
    m_nightMode = on;
    update();
}

void VideoWidget::setPaused(bool on)
{
    m_paused = on;
    update();
}

void VideoWidget::setDistanceText(const QString &text)
{
    m_distanceText = text;
    update();
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rect(), Qt::black);

    if (!m_frame.isNull()) {
        QImage scaled = m_frame.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        p.drawImage(x, y, scaled);

        if (m_nightMode) {
            p.fillRect(rect(), QColor(0, 80, 0, 50));
        }
    } else {
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, "NO SIGNAL");
    }

    if (m_paused) {
        p.fillRect(rect(), QColor(0, 0, 0, 120));
        p.setPen(Qt::yellow);
        QFont f = p.font();
        f.setPointSize(18);
        f.setBold(true);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "PAUSED");
    }

    if (m_showGuideLine) {
        // 车后辅助线示意
        p.setRenderHint(QPainter::Antialiasing, true);

        QPen greenPen(QColor(0, 255, 0), 2);
        QPen yellowPen(QColor(255, 220, 0), 2);
        QPen redPen(QColor(255, 0, 0), 2);

        int cx = width() / 2;
        int bottomY = height() - 10;

        // 左右动态轨迹线
        p.setPen(greenPen);
        p.drawLine(cx - 95, bottomY, cx - 45, 95);
        p.drawLine(cx + 95, bottomY, cx + 45, 95);

        // 中间底线
        p.setPen(yellowPen);
        p.drawLine(cx - 55, bottomY - 5, cx + 55, bottomY - 5);

        // 距离辅助横线
        p.setPen(greenPen);
        p.drawLine(cx - 70, 135, cx + 70, 135);

        p.setPen(yellowPen);
        p.drawLine(cx - 55, 110, cx + 55, 110);

        p.setPen(redPen);
        p.drawLine(cx - 40, 88, cx + 40, 88);

        // 0.5 / 1.0 标签
        QFont f = p.font();
        f.setPointSize(8);
        f.setBold(true);
        p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(cx - 32, 106, "0.5");
        p.drawText(cx + 12, 132, "1.0");

        // 中间距离显示
        p.setPen(Qt::white);
        p.drawText(QRect(0, height() - 28, width(), 20), Qt::AlignCenter, m_distanceText);
    }

    // 边框
    p.setPen(QPen(QColor(180,180,180), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}
