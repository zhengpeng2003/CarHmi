#include "mybutton.h"


Mybutton::Mybutton(QWidget *parent):QPushButton(parent)
{

}

void Mybutton::InitMybutton(QString Norlmal, QString Hover, QString Click)
{

    _Norlmal=Norlmal;
    _Hover=Hover;
    _Click=Click;
    _Pixmap.load(_Norlmal);
    this->setIconSize(QSize(90,45));
    this->setIcon(_Pixmap);

    // 关键：设置透明背景和无边框
    this->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"  // 完全透明背景
        "   border: none;"             // 无边框
        "}"
        );
}

void Mybutton::SetNormal()
{
    _Pixmap.load(_Norlmal);
    this->setIconSize(QSize(90,45));
    this->setIcon(_Pixmap);


}

void Mybutton::SetHover()
{
     _Pixmap.load(_Hover);
    this->setIconSize(QSize(90,45));
    this->setIcon(_Pixmap);


}

void Mybutton::SetClick()
{
    _Pixmap.load(_Click);
    this->setIconSize(QSize(90,45));
    this->setIcon(_Pixmap);


}

bool Mybutton::event(QEvent *e)
{
    switch(e->type())
    {
        case QEvent::MouseButtonPress:
        if(dynamic_cast<QMouseEvent*>(e)->button() == Qt::LeftButton)//进行强转
        {
         SetClick();break;
        }
        else
        {
        SetNormal();
        }
        case QEvent::Enter:
        SetHover();break;
        case QEvent::Leave:
        SetNormal();break;
        default://默认直接退出就行要不然
        break;
    }

    return   QPushButton::event(e);
}
