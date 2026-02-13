#include "picbutton.h"

picbutton::picbutton(QWidget *parent):QPushButton(parent)
{

}

void picbutton::creatpicbutton(QString normal, QString hover, QString pressed)
{
    _normal=normal;
    _hover=hover;
    _pressed=pressed;
    QPixmap tempic;
    tempic.load(_normal);//load可以减少压力

    this->resize(tempic.size());
    this->setIcon(tempic);
    this->setIconSize(tempic.size());//让其更美观
}

void picbutton::normalbutton()
{
    QPixmap tempic;
    tempic.load(_normal);
    this->setIcon(tempic);
}

void picbutton::hoverbutton()
{
    QPixmap tempic;
    tempic.load(_hover);
    this->setIcon(tempic);
}
void picbutton::pressedbutton()
{
    QPixmap tempic;
    tempic.load(_pressed);
    this->setIcon(tempic);
}
bool picbutton::event(QEvent *e)
{
    switch(e->type())
    {
        case QEvent::Enter:
            hoverbutton();break;
        case QEvent::Leave:
            normalbutton();break;
        case QEvent::MouseButtonPress:
            pressedbutton();break;
        case QEvent::MouseButtonRelease:
            normalbutton();break;
        default:
            break;
    }
        return QPushButton::event(e);
}

