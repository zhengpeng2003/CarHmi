#ifndef LOADPROCESS_H
#define LOADPROCESS_H

#include <QWidget>

class Loadprocess : public QWidget
{
    Q_OBJECT
public:
    explicit Loadprocess(QWidget *parent = nullptr);
protected:
    virtual void paintEvent(QPaintEvent *event)override;
signals:
private:
    QPixmap _MainPix;
    QPixmap _Process;
    QTimer *_Timer;
    int _Rate=0;
};

#endif // LOADPROCESS_H
