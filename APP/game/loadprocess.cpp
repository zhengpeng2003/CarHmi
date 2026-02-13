#include "loadprocess.h"

#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <maingame.h>

#define TARGET_W 480
#define TARGET_H 272

Loadprocess::Loadprocess(QWidget *parent)
    : QWidget{parent}
{
    setFixedSize(TARGET_W, TARGET_H);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // ===== 原始资源 =====
    QPixmap mainSrc(":/images/loading.png");
    QPixmap progressSrc(":/images/progress.png");

    // ⭐ 统一缩放比例（关键）
    double scaleW = TARGET_W * 1.0 / mainSrc.width();
    double scaleH = TARGET_H * 1.0 / mainSrc.height();
    double scale = qMin(scaleW, scaleH);

    // 缩放后的背景（大熊）
    _MainPix = mainSrc.scaled(
        mainSrc.size() * scale,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    _Timer = new QTimer(this);
    connect(_Timer, &QTimer::timeout, this, [=]() {

        _Rate += 5;
        if (_Rate > progressSrc.width())
            _Rate = progressSrc.width();

        // 原图裁剪（保持设计尺寸）
        QPixmap cut = progressSrc.copy(0, 0, _Rate, progressSrc.height());

        // ⭐ 只按同一个 scale 缩放一次
        _Process = cut.scaled(
            cut.size() * scale,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );

        update();

        if (_Rate >= progressSrc.width()) {
            _Timer->stop();
            _Timer->deleteLater();
            Maingame *maingame = new Maingame();

            maingame->show();

            close();
        }
    });

    _Timer->start(15);
}

void Loadprocess::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 背景居中
    int bgX = (width() - _MainPix.width()) / 2;
    int bgY = (height() - _MainPix.height()) / 2;
    painter.drawPixmap(bgX, bgY, _MainPix);

    // ⭐ 进度条：相对背景底部位置（不会超）
    int px = bgX + ( _MainPix.width() - _Process.width() ) / 2;
    int py = bgY + _MainPix.height() - _Process.height() - int(20 * (_MainPix.width() * 1.0));

    painter.drawPixmap(px, py, _Process);
}
