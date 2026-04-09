#include "videowidget.h"
#include <QPainter>
#include <QVariantAnimation>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , m_showGuideLine(true)
    , m_nightMode(false)
    , m_paused(false)
    , m_scanPos(0.0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *anim = new QVariantAnimation(this);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setDuration(3000);
    anim->setLoopCount(-1);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v){
        m_scanPos = v.toReal();
        update();
    });
    anim->start();
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

void VideoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 黑色背景
    p.fillRect(rect(), Qt::black);

    QRect videoRect = rect().adjusted(10, 10, -10, -10);
    if (!m_frame.isNull()) {
        QImage scaled = m_frame.scaled(videoRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = videoRect.left() + (videoRect.width() - scaled.width()) / 2;
        int y = videoRect.top() + (videoRect.height() - scaled.height()) / 2;
        p.drawImage(x, y, scaled);
    }
    if (!m_frame.isNull()) {
        QImage scaled = m_frame.scaled(videoRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int x = videoRect.left() + (videoRect.width() - scaled.width()) / 2;
        int y = videoRect.top() + (videoRect.height() - scaled.height()) / 2;
        p.drawImage(x, y, scaled);
    } else {
        p.setPen(QColor(200, 200, 200));
        p.drawText(videoRect, Qt::AlignCenter, "无视频信号");
    }

    // 扫描线
    int scanY = videoRect.top() + m_scanPos * videoRect.height();
    p.fillRect(QRect(videoRect.left(), scanY - 1, videoRect.width(), 2), QColor(180, 180, 180, 80));

    // 引导线（工业黄）
    if (m_showGuideLine) {
        QPen pen(m_nightMode ? QColor(150, 200, 150) : QColor(200, 170, 120), 1.5);
        p.setPen(pen);
        int cx = videoRect.center().x();
        int bottom = videoRect.bottom() - 8;
        int top = videoRect.top() + 60;
        p.drawLine(cx - 60, bottom, cx - 30, top);
        p.drawLine(cx + 60, bottom, cx + 30, top);
        p.drawLine(cx, bottom - 2, cx, top - 4);
        p.drawLine(cx - 50, bottom - 30, cx + 50, bottom - 30);
        p.drawLine(cx - 35, bottom - 60, cx + 35, bottom - 60);
    }

    // 暂停遮罩
    if (m_paused) {
        p.fillRect(videoRect, QColor(0, 0, 0, 180));
        p.setPen(Qt::white);
        p.drawText(videoRect, Qt::AlignCenter, "已暂停");
    }

    // 简单边框
    p.setPen(QPen(QColor(100, 110, 120), 1));
    p.drawRoundedRect(videoRect.adjusted(0, 0, -1, -1), 4, 4);
}
