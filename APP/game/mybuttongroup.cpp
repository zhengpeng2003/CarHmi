#include "mybuttongroup.h"
#include "ui_mybuttongroup.h"

MybuttonGroup::MybuttonGroup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MybuttonGroup)
{
    ui->setupUi(this);


}

void MybuttonGroup::Initbutton()
{
    //出牌

    ui->pushButton_no->InitMybutton(":/images/pass_btn-1.png",":/images/pass_btn-2.png",":/images/pass_btn-3.png");
    ui->pushButton_play->InitMybutton(":/images/chupai_btn-1.png",":/images/chupai_btn-2.png",":/images/chupai_btn-3.png");
    //开始
    ui->pushButton_start->InitMybutton(":/images/start-1.png",":/images/start-2.png",":/images/start-3.png");
    //第一次出牌
    ui->pushButton_playfirst->InitMybutton(":/images/chupai_btn-1.png",":/images/chupai_btn-2.png",":/images/chupai_btn-3.png");
    //抢地主
    ui->pushButton_one->InitMybutton(":/images/1fen-1.png",":/images/1fen-2.png",":/images/1fen-3.png");
    ui->pushButton_two->InitMybutton(":/images/2fen-1.png",":/images/2fen-2.png",":/images/2fen-3.png");
    ui->pushButton_three->InitMybutton(":/images/3fen-1.png",":/images/3fen-2.png",":/images/3fen-3.png");
    ui->pushButton_NO->InitMybutton(":/images/buqiang-1.png",":/images/buqiang-2.png",":/images/buqiang-3.png");

    _Pushbuttons<<ui->pushButton_no<<ui->pushButton_play<<ui->pushButton_start
                 <<ui->pushButton_playfirst<<ui->pushButton_one<<ui->pushButton_three
                 <<ui->pushButton_NO;
    for(int i=0;i<7;i++)
    {

        _Pushbuttons.at(i)->setFixedSize(90,45);
    }
    //出牌
    connect(ui->pushButton_no,&QPushButton::clicked,this,&MybuttonGroup::S_NoHand);
    connect(ui->pushButton_play,&QPushButton::clicked,this,&MybuttonGroup::S_PlayHand);
    //开始
    connect(ui->pushButton_start,&QPushButton::clicked,this,&MybuttonGroup::S_Start);
    //第一次出牌
    connect(ui->pushButton_playfirst,&QPushButton::clicked,this,&MybuttonGroup::S_PlayHand);
    //抢地主
    connect(ui->pushButton_one,&QPushButton::clicked,this,[this]()
            {
                emit S_Point(1);
            });
    connect(ui->pushButton_two,&QPushButton::clicked,this,[this]()
            {

                emit S_Point(2);
            });
    connect(ui->pushButton_three,&QPushButton::clicked,this,[this]()
            {
                emit S_Point(3);
            });
    connect(ui->pushButton_NO,&QPushButton::clicked,this,[this]()
            {

                emit S_Point(0);
            });


}

void MybuttonGroup::Setbtngroupstate(State state)
{
    ui->stackedWidget->setCurrentIndex(state);
}

void MybuttonGroup::SetStartButtonVisible(bool visible)
{
    if(ui && ui->pushButton_start)
    {
        ui->pushButton_start->setVisible(visible);
    }
}

MybuttonGroup::~MybuttonGroup()
{
    delete ui;
}
