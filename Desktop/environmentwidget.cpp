#include "environmentwidget.h"
#include <QPainter>

EnvironmentWidget::EnvironmentWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
}


void EnvironmentWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 1️⃣ 灰色底板，带圆角
    int radius = 15; // 圆角半径，可根据需要调整
    QRectF backgroundRect = rect();
    painter.setBrush(QColor(60, 60, 60)); // 深灰色背景
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(backgroundRect, radius, radius);

    // 2️⃣ 车图标
    QPixmap carIcon(":/images/car.png"); // 你的车图标资源
    if (!carIcon.isNull())
    {
        // 去掉白色背景
        QImage img = carIcon.toImage().convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < img.height(); ++y)
        {
            QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x)
            {
                QColor c(line[x]);
                if (c.red() > 240 && c.green() > 240 && c.blue() > 240)
                    line[x] = qRgba(0, 0, 0, 0); // 白色透明化
            }
        }
        carIcon = QPixmap::fromImage(img);

        // 缩放保持清晰
        const int iconWidth = 200;
        const int iconHeight = 200;
        carIcon = carIcon.scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 放置在上半部，稍微上移，不挡温湿度
        QPoint center = rect().center() - QPoint(carIcon.width() / 2, carIcon.height() / 2 + 30);
        painter.drawPixmap(center, carIcon);
    }

    // 3️⃣ 温度/湿度显示
    QString tempText = QString("温度: %1°C").arg(m_temperature, 0, 'f', 1);
    QString humText  = QString("湿度: %1%").arg(m_humidity, 0, 'f', 0);

    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);

    // 放在车图标下方
    int yOffset = rect().height() / 2 + 20;
    painter.drawText(QRect(0, yOffset, rect().width(), 30),
                     Qt::AlignCenter, tempText);
    painter.drawText(QRect(0, yOffset + 35, rect().width(), 30),
                     Qt::AlignCenter, humText);
}

