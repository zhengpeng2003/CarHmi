#include "slideshowdialog.h"

#include "piclistwidget.h"
#include "protreewidget.h"
#include "ui_slideshowdialog.h"

slideshowdialog::slideshowdialog(QWidget *parent, protreeitem *firstchild, protreeitem *lastchild)
    : QDialog(parent),_firstchild(firstchild),_lastchild(lastchild)
    , ui(new Ui::slideshowdialog)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);//设置无边框
    ui->next_btn1->creatpicbutton(":/icon/next.png",":/icon/next_hover.png",":/icon/next_press.png");
    ui->pre_btn1->creatpicbutton(":/icon/previous.png",":/icon/previous_hover.png",":/icon/previous_press.png");
    ui->close_slide->creatpicbutton(":/icon/closeshow.png",":/icon/closeshow_hover.png",":/icon/closeshow_press.png");
    ui->play_slide->seticons(":/icon/play.png",":/icon/play_hover.png",":/icon/play_press.png",
                             ":/icon/pause.png",":/icon/pause_hover.png",":/icon/pause_press.png");
    auto * piclist=dynamic_cast<piclistwidget*>(ui->pre_show_list);
    connect(ui->animation_widget,&picanimation::signalupdatelistitem,piclist,&piclistwidget::slotupdatelistitem);//更新下面的item
    connect(ui->animation_widget,&picanimation::sigselectupdate,piclist,&piclistwidget::updateselect);//更新每次更新是鼠标的位置
    connect(ui->close_slide,&QPushButton::clicked,this,&QDialog::close);//关闭
    connect(ui->pre_btn1,&QPushButton::clicked,ui->animation_widget,&picanimation::slotpreshow);//向前
    connect(ui->next_btn1,&QPushButton::clicked,ui->animation_widget,&picanimation::slotnextshow);//向后


    connect(piclist,&QListWidget::itemPressed,piclist,&piclistwidget::sloselectupdatepic);
    connect(piclist,&piclistwidget::sigselectupdatepic,ui->animation_widget,&picanimation::slotselectupdatepic);//实现点击换图
    connect(ui->play_slide,&QPushButton::clicked,ui->animation_widget,&picanimation::slotupdatebtn);//实现点击换图
    ui->animation_widget->setanimation(firstchild);
    ui->play_slide->setnormal2();
    connect(ui->pre_btn1,&QPushButton::clicked,ui->play_slide,&slidebutton::slplaybtn);//向前 点击时暂停
    connect(ui->next_btn1,&QPushButton::clicked,ui->play_slide,&slidebutton::slplaybtn);//向后 点击时暂停 设置暂停按钮

    connect(ui->animation_widget,&picanimation::sigplaybtn,ui->play_slide,&slidebutton::slplaybtn);//图案设置
    connect(ui->animation_widget,&picanimation::sigstopbtn,ui->play_slide,&slidebutton::slstopbtn);//图案设置
    auto * pro_widget=dynamic_cast<protreewidget*>(parent);
    connect(ui->animation_widget,&picanimation::musicstart,pro_widget,&protreewidget::startmusic);//设置播放音乐
    connect(ui->animation_widget,&picanimation::musicstop,pro_widget,&protreewidget::stopmusic);//设置停止
    connect(ui->close_slide,&QPushButton::clicked,this,[=](){
        this->close();
        pro_widget->stopmusic();
    });//关闭
    ui->animation_widget->timestart();
}
slideshowdialog::~slideshowdialog()
{
    delete ui;
}


