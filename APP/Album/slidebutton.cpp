#include "slidebutton.h"
#include "QPixmap"
#include"type.h"

#include <QEvent>
slidebutton::slidebutton(QWidget *parent)
{

}

void slidebutton::seticons(const QString normal, const QString hover, const QString pressed,
                         const QString normal2, const QString hover2, const QString pressed2)//1 播放 2暂停
{
    _normal=normal;
    _hover=hover;
    _pressed=pressed;
    _normal2=normal2;
    _hover2=hover2;
    _pressed2=pressed2;
    QPixmap map;
    map.load(_normal);
    this->resize(map.size());
    this->setIcon(map);
    this->setIconSize(map.size());//为了高精度
    _state=PicBtnStateNormal;
}

bool slidebutton::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::Enter:
    {
        if(PicBtnState2Normal<=_state)
        {
            sethover2();
            break;

        }    else
        {
            sethover();
            break;
        }
    }
    case QEvent::Leave:
    {
        if(PicBtnState2Normal<=_state)
        {
            setnormal2();
            break;

        }    else
        {
            setnormal();
            break;
        }
    }
    case QEvent::MouseButtonPress:
    {
        if(PicBtnState2Normal<=_state)
        {
            setpressed2();break;
            break;

        }    else
        {
            setpressed();break;
            break;
        }
    }
    case QEvent::MouseButtonRelease:
    {
        if(PicBtnState2Normal<=_state)
        {
            setnormal();break;
            break;

        }    else
        {
            setnormal2();break;
            break;
        }
    }
    }
    return QPushButton::event(event);
}

void slidebutton::slplaybtn()
{
    setnormal();
}

void slidebutton::slstopbtn()
{
    setnormal2();
}
void slidebutton::setnormal()
{
    QPixmap tempico;
    tempico.load(_normal);
    this->setIcon(tempico);
    _state=PicBtnStateNormal;
}
void slidebutton::sethover()
{    QPixmap tempico;
    tempico.load(_hover);
    this->setIcon(tempico);
    _state=PicBtnStateHover;}
void slidebutton::setpressed()
{    QPixmap tempico;
    tempico.load(_pressed);
    this->setIcon(tempico);
    _state=PicBtnStatePress;
}

void slidebutton::setnormal2()
{    QPixmap tempico;
    tempico.load(_normal2);
    this->setIcon(tempico);
    _state=PicBtnState2Normal;}
void slidebutton::sethover2()
{    QPixmap tempico;
    tempico.load(_hover2);
    this->setIcon(tempico);
    _state=PicBtnState2Hover;}
void slidebutton::setpressed2()
{    QPixmap tempico;
    tempico.load(_pressed2);
    this->setIcon(tempico);
    _state=PicBtnState2Press;
}
