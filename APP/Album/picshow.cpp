#include "picshow.h"
#include "ui_picshow.h"
#include "QGraphicsOpacityEffect"
#include "QPropertyAnimation"
picshow::picshow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::picshow)
{
    ui->setupUi(this);
    ui->previous_btn->creatpicbutton(":/icon/previous.png",":/icon/previous_hover.png",":/icon/previous_press.png");
    ui->next_btn->creatpicbutton(":/icon/next.png",":/icon/next_hover.png",":/icon/next_press.png");

    QGraphicsOpacityEffect  * pre_btn_opacity=new QGraphicsOpacityEffect(this);
    pre_btn_opacity->setOpacity(0);
    ui->previous_btn->setGraphicsEffect(pre_btn_opacity);

    QGraphicsOpacityEffect * next_btn_opacity=new QGraphicsOpacityEffect(this);
    next_btn_opacity->setOpacity(0);
    ui->next_btn->setGraphicsEffect(next_btn_opacity);

    pre_btn_animation=new QPropertyAnimation(pre_btn_opacity,"opacity",this);
    pre_btn_animation->setEasingCurve(QEasingCurve::InExpo);
    pre_btn_animation->setDuration(500);

    next_btn_animation=new QPropertyAnimation(next_btn_opacity,"opacity",this);
    next_btn_animation->setEasingCurve(QEasingCurve::InExpo);
    next_btn_animation->setDuration(500);

    //前进后退按钮的连接
    connect(ui->previous_btn,&QPushButton::clicked,this,&picshow::sigprebtn);
    connect(ui->next_btn,&QPushButton::clicked,this,&picshow::signextbtn);

}

picshow::~picshow()
{
    delete ui;
}

void picshow::showprenextbtn(bool b_show)
{
    if(b_show)
    {
        pre_btn_animation->stop();
        pre_btn_animation->setStartValue(0);//透明
        pre_btn_animation->setEndValue(1);
        pre_btn_animation->start();


        next_btn_animation->stop();
        next_btn_animation->setStartValue(0);//透明
        next_btn_animation->setEndValue(1);
        next_btn_animation->start();
    }
    else
    {

        pre_btn_animation->stop();
        pre_btn_animation->setStartValue(1);//透明
        pre_btn_animation->setEndValue(0);
        pre_btn_animation->start();


        next_btn_animation->stop();
        next_btn_animation->setStartValue(1);//透明
        next_btn_animation->setEndValue(0);
        next_btn_animation->start();
    }
}

void picshow::repic()
{
    if(_select_pic=="")
        return;
    _pic.load(_select_pic);
    auto const height=ui->gridLayout->geometry().height();
    auto const  width=ui->gridLayout->geometry().width();
    _pic=_pic.scaled(height,width,Qt::KeepAspectRatio);
    ui->pic_show_label->setPixmap(_pic);
}

bool picshow::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::Enter:
        showprenextbtn(true);break;
    case QEvent::Leave:
        showprenextbtn(false);break;
    default:
        break;
    }
   return  QDialog::event(event);
}

void picshow::slotpicshow(QString path)
{
    _select_pic=path;
    if(_select_pic=="")
        return;
    _pic.load(_select_pic);
    int height=this->height()+200;
    int width=this->width()+20;

    // 确保 QLabel 会自动缩放其内容以填充整个标签区域
    ui->pic_show_label->setScaledContents(true);
    _pic=_pic.scaled(height,width,Qt::KeepAspectRatio);//缩放
    ui->pic_show_label->setPixmap(_pic);
}

void picshow::slotprepicshow(const QString path)
{
    _select_pic=path;
    if(_select_pic=="")
        return;
    _pic.load(path);
    auto const height=ui->gridLayout->geometry().height();
    auto const  width=ui->gridLayout->geometry().width();
    _pic=_pic.scaled(height,width,Qt::KeepAspectRatio);
    ui->pic_show_label->setPixmap(_pic);
}

void picshow::slotnextpicshow(const QString path)
{
    _select_pic=path;
    if(_select_pic=="")
        return;
    _pic.load(path);
    auto const height=ui->gridLayout->geometry().height();
    auto const  width=ui->gridLayout->geometry().width();
    _pic=_pic.scaled(height,width,Qt::KeepAspectRatio);
    ui->pic_show_label->setPixmap(_pic);
}

void picshow::slotclosepic()
{
    _select_pic=="";
    ui->pic_show_label->clear();
}
