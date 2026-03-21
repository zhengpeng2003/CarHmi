#include "loadprocess.h"

#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QTimer>
#include "gameresourcecache.h"
#include "gamescaling.h"
#include "maingame.h"

Loadprocess::Loadprocess(QWidget *parent)
    : QWidget{parent}
{
    GameScaling::initialize(GameScaling::designSize());
    setFixedSize(GameScaling::designSize());
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    _MainPix = GameResourceCache::pixmap(":/images/loading.png");
    _StageLabel = QStringLiteral("等待初始化...");
}

void Loadprocess::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!_Started) {
        _Started = true;
        QTimer::singleShot(0, this, &Loadprocess::startBootstrap);
    }
}

void Loadprocess::startBootstrap()
{
    _Maingame = new Maingame();
    connect(_Maingame, &Maingame::setupProgressChanged, this, &Loadprocess::updateProgress);
    connect(_Maingame, &Maingame::setupFinished, this, [this]() {
        if (_Maingame) {
            _Maingame->show();
        }
        close();
    });

    const int total = _Maingame->setupStageCount();
    for (int stage = 0; stage < total; ++stage) {
        QTimer::singleShot(stage * 10, this, [this, stage]() {
            if (_Maingame) {
                _Maingame->initializeStage(stage);
            }
        });
    }
}

void Loadprocess::updateProgress(int current, int total, const QString &label)
{
    _ProgressMax = qMax(total, 1);
    _ProgressValue = qBound(0, current, _ProgressMax);
    _StageLabel = label;

    const QPixmap progressSrc = GameResourceCache::pixmap(":/images/progress.png");
    const int cutWidth = qRound(progressSrc.width() * (_ProgressValue / static_cast<double>(_ProgressMax)));
    if (!progressSrc.isNull() && cutWidth > 0) {
        _Process = progressSrc.copy(0, 0, cutWidth, progressSrc.height());
    } else {
        _Process = QPixmap();
    }
    update();
}

void Loadprocess::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPixmap(rect(), _MainPix);

    if (!_Process.isNull()) {
        const QSize progressSize(GameScaling::x(_Process.width()), GameScaling::y(_Process.height()));
        const QPixmap progressPixmap = _Process.scaled(progressSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        const int px = (width() - progressPixmap.width()) / 2;
        const int py = height() - progressPixmap.height() - GameScaling::y(28);
        painter.drawPixmap(px, py, progressPixmap);
    }

    painter.setPen(Qt::white);
    painter.drawText(QRect(GameScaling::x(40), height() - GameScaling::y(48), width() - GameScaling::x(80), GameScaling::y(20)),
                     Qt::AlignCenter,
                     _StageLabel);
}
