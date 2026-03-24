#include "anmationpixmap.h"

AnmationPixmap::AnmationPixmap(QWidget *parent)
    :QWidget{parent}, _Timer(new QTimer(this)), _count(0), _Mx(0)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void AnmationPixmap::ShowBet(int Bet)
{
    _Mx = 0;
    if(Bet <= 0)
    {
        _Mypix.load(":/images/buqinag.png");
    }
    else if(Bet==1)
    {
        _Mypix.load(":/images/score1.png");
    }
    else if(Bet==2)
    {
        _Mypix.load(":/images/score2.png");
    }
    else
    {
        _Mypix.load(":/images/score3.png");

    }
    FitPixmapToWidget();

    update();
}

void AnmationPixmap::ShowSimsqe(PlayHand::HandType type)
{

    _Mx=0;
    QString name= type==PlayHand::Hand_Seq_Pair ?":/images/liandui.png":":/images/shunzi.png";
    _Mypix.load(name);
    FitPixmapToWidget();
    // qDebug()<<name;
    update();
    QTimer::singleShot(2000,this,&AnmationPixmap::hide);

}

void AnmationPixmap::ShowBom(PlayHand::HandType type)
{
    _Mx=0;
    _count = 0;
    int Number= type==PlayHand::Hand_Bomb ?12:8;
    disconnect(_Timer, &QTimer::timeout, this, nullptr);
    connect(_Timer,&QTimer::timeout,this,[=](){
        _count++;
        if(Number==12&&_count<Number)
        {
            QString name=QString(":/images/bomb_%1.png").arg(_count);
            _Mypix.load(name);
            FitPixmapToWidget();
        }
        else if(Number==8&&_count<Number)
        {
            QString name=QString(":/images/joker_bomb_%1.png").arg(_count);
            _Mypix.load(name);
            FitPixmapToWidget();
        }
        else
        {
            _Mypix=QPixmap();
            _Timer->stop();
            _count=0;
        }
        update();
    });
    _Timer->start(200);
    QTimer::singleShot(2000,this,&AnmationPixmap::hide);
}

void AnmationPixmap::ShowPlane()
{
    _Mx = width();  // 从右侧开始
    _count = 1;
    _Timer->stop();
    disconnect(_Timer, &QTimer::timeout, this, nullptr);
    // 立即显示第一张图片
    QString name = QString(":/images/plane_%1.png").arg(_count);
    _Mypix.load(name);
    FitPixmapToWidget();
    update();
    connect(_Timer, &QTimer::timeout, this, [=]() mutable {
        _Mx -= 8;  // 移动速度

        // 每移动100像素切换一张图片
        int distanceMoved = width() - _Mx;
        int frameToShow = (distanceMoved / 100) % 5 + 1;  // 1-5循环

        if (frameToShow != _count)
        {
            _count = frameToShow;
            QString name = QString(":/images/plane_%1.png").arg(_count);
            _Mypix.load(name);
            FitPixmapToWidget();
        }

        // 飞出屏幕后停止
        if (_Mx + _Mypix.width() < 0)
        {
            _Timer->stop();
            _Mypix = QPixmap();
            _count = 1;
        }

        update();
    });

    _Timer->start(50);  // 20fps
    QTimer::singleShot(5000, this, &AnmationPixmap::hide);
}

void AnmationPixmap::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter Painter(this);
    if(_Mypix.isNull())
    {
        return;
    }

    const int drawY = (height() - _Mypix.height()) / 2;
    if(_Mx == 0)
    {
        const int drawX = (width() - _Mypix.width()) / 2;
        Painter.drawPixmap(drawX, drawY, _Mypix);
    }
    else
    {
        Painter.drawPixmap(_Mx, drawY, _Mypix);
    }
}

void AnmationPixmap::FitPixmapToWidget()
{
    if(_Mypix.isNull() || width() <= 0 || height() <= 0)
    {
        return;
    }
    _Mypix = _Mypix.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
