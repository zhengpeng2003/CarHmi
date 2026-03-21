#include "gamescaling.h"

#include <QtMath>

namespace {
QSize g_actualSize = QSize(480, 272);
}

QSize GameScaling::designSize()
{
    return QSize(480, 272);
}

void GameScaling::initialize(const QSize &actualSize)
{
    if (actualSize.isValid()) {
        g_actualSize = actualSize;
    }
}

double GameScaling::scaleX()
{
    return g_actualSize.width() / static_cast<double>(designSize().width());
}

double GameScaling::scaleY()
{
    return g_actualSize.height() / static_cast<double>(designSize().height());
}

int GameScaling::x(int value)
{
    return qRound(value * scaleX());
}

int GameScaling::y(int value)
{
    return qRound(value * scaleY());
}

QSize GameScaling::size(int w, int h)
{
    return QSize(x(w), y(h));
}

QPoint GameScaling::point(int px, int py)
{
    return QPoint(x(px), y(py));
}

QRect GameScaling::rect(int px, int py, int w, int h)
{
    return QRect(x(px), y(py), x(w), y(h));
}

QRect GameScaling::rect(const QRect &designRect)
{
    return rect(designRect.x(), designRect.y(), designRect.width(), designRect.height());
}

void GameScaling::applyFixedSize(QWidget *widget, int w, int h)
{
    if (widget) {
        widget->setFixedSize(size(w, h));
    }
}

void GameScaling::move(QWidget *widget, int px, int py)
{
    if (widget) {
        widget->move(point(px, py));
    }
}
