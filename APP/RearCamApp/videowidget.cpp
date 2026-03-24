#include "videowidget.h"

#include <QColor>
#include <QFont>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QVariantAnimation>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent),
      m_showGuideLine(true),
      m_nightMode(false),
      m_paused(false),
      m_distanceText("1.2m"),
      m_scanProgress(0.0),
      m_glowStrength(0.32)
{
    setFixedSize(332, 176);
    setAttribute(Qt::WA_OpaquePaintEvent);

    auto *scanAnimation = new QVariantAnimation(this);
    scanAnimation->setStartValue(0.0);
    scanAnimation->setEndValue(1.0);
    scanAnimation->setDuration(3200);
    scanAnimation->setLoopCount(-1);
    connect(scanAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_scanProgress = value.toReal();
        update();
    });
    scanAnimation->start();

    auto *glowAnimation = new QVariantAnimation(this);
    glowAnimation->setDuration(1800);
    glowAnimation->setLoopCount(-1);
    glowAnimation->setKeyValueAt(0.0, 0.24);
    glowAnimation->setKeyValueAt(0.5, 0.48);
    glowAnimation->setKeyValueAt(1.0, 0.24);
    connect(glowAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_glowStrength = value.toReal();
        update();
    });
    glowAnimation->start();
}

void VideoWidget::setFrame(const QImage &img)
{
    m_frame = img.convertToFormat(QImage::Format_RGB32);
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
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF outerRect = rect().adjusted(3, 3, -3, -3);
    const QRectF frameRect = outerRect.adjusted(6, 6, -6, -6);
    const QColor accent = m_nightMode ? QColor(112, 255, 142) : QColor(88, 214, 255);
    const QColor amber(255, 186, 76);
    const QColor red(255, 94, 94);

    QRadialGradient shadow(frameRect.center().x(), frameRect.bottom() + 20.0, width() * 0.55);
    shadow.setColorAt(0.0, QColor(0, 0, 0, 120));
    shadow.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(shadow);
    p.drawRoundedRect(outerRect.adjusted(8, 18, -8, 10), 18, 18);

    QLinearGradient shellGradient(0, outerRect.top(), 0, outerRect.bottom());
    shellGradient.setColorAt(0.0, QColor(20, 32, 45));
    shellGradient.setColorAt(0.5, QColor(7, 14, 23));
    shellGradient.setColorAt(1.0, QColor(10, 20, 31));
    p.setBrush(shellGradient);
    p.setPen(QPen(QColor(78, 109, 133, 190), 1.2));
    p.drawRoundedRect(outerRect, 18, 18);

    QPainterPath clipPath;
    clipPath.addRoundedRect(frameRect, 13, 13);

    p.save();
    p.setClipPath(clipPath);

    if (!m_frame.isNull()) {
        const QImage scaled = m_frame.scaled(frameRect.size().toSize(),
                                             Qt::KeepAspectRatioByExpanding,
                                             Qt::SmoothTransformation);
        const qreal x = frameRect.left() + (frameRect.width() - scaled.width()) / 2.0;
        const qreal y = frameRect.top() + (frameRect.height() - scaled.height()) / 2.0;
        p.drawImage(QPointF(x, y), scaled);

        QLinearGradient imageShade(frameRect.topLeft(), frameRect.bottomLeft());
        imageShade.setColorAt(0.0, QColor(2, 7, 13, 12));
        imageShade.setColorAt(0.6, QColor(2, 7, 13, 40));
        imageShade.setColorAt(1.0, QColor(2, 7, 13, 88));
        p.fillRect(frameRect, imageShade);

        if (m_nightMode) {
            p.fillRect(frameRect, QColor(5, 80, 28, 44));
        }
    } else {
        QLinearGradient emptyGradient(frameRect.topLeft(), frameRect.bottomLeft());
        emptyGradient.setColorAt(0.0, QColor(11, 23, 35));
        emptyGradient.setColorAt(1.0, QColor(4, 10, 17));
        p.fillRect(frameRect, emptyGradient);

        p.setPen(QColor(210, 228, 240));
        QFont titleFont("DejaVu Sans", 16, QFont::Bold);
        p.setFont(titleFont);
        p.drawText(frameRect.adjusted(0, -6, 0, -8), Qt::AlignCenter, "NO SIGNAL");

        p.setPen(QColor(128, 157, 178));
        QFont bodyFont("DejaVu Sans", 9);
        bodyFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
        p.setFont(bodyFont);
        p.drawText(frameRect.adjusted(0, 26, 0, 0), Qt::AlignCenter, "Awaiting rear camera feed");
    }

    const int scanY = static_cast<int>(frameRect.top() + m_scanProgress * (frameRect.height() + 24.0)) - 14;
    QLinearGradient scanGradient(frameRect.left(), scanY, frameRect.left(), scanY + 24);
    scanGradient.setColorAt(0.0, QColor(accent.red(), accent.green(), accent.blue(), 0));
    scanGradient.setColorAt(0.45, QColor(accent.red(), accent.green(), accent.blue(), 24));
    scanGradient.setColorAt(0.5, QColor(accent.red(), accent.green(), accent.blue(), 72));
    scanGradient.setColorAt(0.55, QColor(accent.red(), accent.green(), accent.blue(), 22));
    scanGradient.setColorAt(1.0, QColor(accent.red(), accent.green(), accent.blue(), 0));
    p.fillRect(QRectF(frameRect.left(), scanY, frameRect.width(), 24.0), scanGradient);

    for (int y = static_cast<int>(frameRect.top()) + 3; y < frameRect.bottom(); y += 6) {
        p.setPen(QPen(QColor(255, 255, 255, 10), 1));
        p.drawLine(QPointF(frameRect.left(), y), QPointF(frameRect.right(), y));
    }

    QLinearGradient hudGlow(frameRect.topLeft(), frameRect.topRight());
    hudGlow.setColorAt(0.0, QColor(accent.red(), accent.green(), accent.blue(),
                                   static_cast<int>(40 + m_glowStrength * 40)));
    hudGlow.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.fillRect(QRectF(frameRect.left(), frameRect.top(), frameRect.width(), 26), hudGlow);

    p.restore();

    p.setPen(QPen(QColor(0, 0, 0, 48), 4));
    p.drawRoundedRect(frameRect.adjusted(-1, -1, 1, 1), 14, 14);

    const qreal bracket = 18.0;
    QPen bracketPen(QColor(accent.red(), accent.green(), accent.blue(),
                           static_cast<int>(150 + m_glowStrength * 40)), 2.0);
    p.setPen(bracketPen);
    p.drawLine(QPointF(frameRect.left() + 8, frameRect.top() + 8),
               QPointF(frameRect.left() + 8 + bracket, frameRect.top() + 8));
    p.drawLine(QPointF(frameRect.left() + 8, frameRect.top() + 8),
               QPointF(frameRect.left() + 8, frameRect.top() + 8 + bracket));
    p.drawLine(QPointF(frameRect.right() - 8, frameRect.top() + 8),
               QPointF(frameRect.right() - 8 - bracket, frameRect.top() + 8));
    p.drawLine(QPointF(frameRect.right() - 8, frameRect.top() + 8),
               QPointF(frameRect.right() - 8, frameRect.top() + 8 + bracket));
    p.drawLine(QPointF(frameRect.left() + 8, frameRect.bottom() - 8),
               QPointF(frameRect.left() + 8 + bracket, frameRect.bottom() - 8));
    p.drawLine(QPointF(frameRect.left() + 8, frameRect.bottom() - 8),
               QPointF(frameRect.left() + 8, frameRect.bottom() - 8 - bracket));
    p.drawLine(QPointF(frameRect.right() - 8, frameRect.bottom() - 8),
               QPointF(frameRect.right() - 8 - bracket, frameRect.bottom() - 8));
    p.drawLine(QPointF(frameRect.right() - 8, frameRect.bottom() - 8),
               QPointF(frameRect.right() - 8, frameRect.bottom() - 8 - bracket));

    QRectF statusChip(frameRect.left() + 14, frameRect.top() + 12, 122, 24);
    p.setPen(QPen(QColor(133, 164, 186, 180), 1));
    p.setBrush(QColor(5, 13, 20, 168));
    p.drawRoundedRect(statusChip, 11, 11);
    p.setPen(QColor(220, 234, 245));
    QFont chipFont("DejaVu Sans", 8, QFont::Bold);
    chipFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.1);
    p.setFont(chipFont);
    p.drawText(statusChip.adjusted(10, 0, -10, 0), Qt::AlignVCenter | Qt::AlignLeft, "LIVE REAR FEED");

    if (m_showGuideLine) {
        QPen guidePen(accent, 2.2);
        QPen warnPen(amber, 2.1);
        QPen alertPen(red, 2.0);

        const qreal cx = frameRect.center().x();
        const qreal bottomY = frameRect.bottom() - 12.0;
        const qreal topY = frameRect.top() + 78.0;

        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(guidePen);
        p.drawLine(QPointF(cx - 88, bottomY), QPointF(cx - 44, topY));
        p.drawLine(QPointF(cx + 88, bottomY), QPointF(cx + 44, topY));
        p.drawLine(QPointF(cx, bottomY - 1), QPointF(cx, topY - 4));
        p.drawLine(QPointF(cx - 70, frameRect.bottom() - 42), QPointF(cx + 70, frameRect.bottom() - 42));

        p.setPen(warnPen);
        p.drawLine(QPointF(cx - 54, frameRect.bottom() - 67), QPointF(cx + 54, frameRect.bottom() - 67));
        p.drawLine(QPointF(cx - 48, bottomY - 6), QPointF(cx + 48, bottomY - 6));

        p.setPen(alertPen);
        p.drawLine(QPointF(cx - 38, frameRect.bottom() - 90), QPointF(cx + 38, frameRect.bottom() - 90));

        QFont guideFont("DejaVu Sans", 8, QFont::Bold);
        p.setFont(guideFont);
        p.setPen(QColor(230, 239, 247));
        p.drawText(QRectF(cx - 42, frameRect.bottom() - 80, 32, 16), "0.5m");
        p.drawText(QRectF(cx + 12, frameRect.bottom() - 55, 34, 16), "1.0m");
    }

    QRectF distanceChip(frameRect.left() + 116, frameRect.bottom() - 30, 100, 22);
    p.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), 170), 1));
    p.setBrush(QColor(4, 10, 18, 176));
    p.drawRoundedRect(distanceChip, 11, 11);
    QFont distanceFont("DejaVu Sans", 9, QFont::Bold);
    distanceFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    p.setFont(distanceFont);
    p.setPen(QColor(235, 244, 250));
    p.drawText(distanceChip, Qt::AlignCenter, m_distanceText);

    if (m_paused) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(3, 7, 12, 170));
        p.drawRoundedRect(frameRect, 13, 13);

        QRectF pausedChip(frameRect.center().x() - 72, frameRect.center().y() - 22, 144, 44);
        p.setPen(QPen(QColor(255, 203, 84, 210), 1));
        p.setBrush(QColor(11, 15, 20, 196));
        p.drawRoundedRect(pausedChip, 14, 14);

        QFont pausedFont("DejaVu Sans", 14, QFont::Bold);
        pausedFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
        p.setFont(pausedFont);
        p.setPen(QColor(255, 226, 142));
        p.drawText(pausedChip, Qt::AlignCenter, "PAUSED");
    }

    p.setPen(QPen(QColor(132, 164, 186, 172), 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(frameRect, 13, 13);
}
