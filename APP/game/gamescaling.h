#ifndef GAMESCALING_H
#define GAMESCALING_H

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QWidget>

class GameScaling
{
public:
    static QSize designSize();
    static void initialize(const QSize &actualSize);
    static double scaleX();
    static double scaleY();
    static int x(int value);
    static int y(int value);
    static QSize size(int w, int h);
    static QPoint point(int px, int py);
    static QRect rect(int px, int py, int w, int h);
    static QRect rect(const QRect &designRect);
    static void applyFixedSize(QWidget *widget, int w, int h);
    static void move(QWidget *widget, int x, int y);
};

#endif // GAMESCALING_H
