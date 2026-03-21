#include "mybuttongroup.h"
#include "ui_mybuttongroup.h"
#include "gamescaling.h"

MybuttonGroup::MybuttonGroup(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MybuttonGroup)
{
    ui->setupUi(this);
    setFixedSize(GameScaling::size(316, 36));
}

void MybuttonGroup::Initbutton()
{
    const QSize btnSize = GameScaling::size(70, 36);
    const QString style =
        "QPushButton {"
        "background-color: rgba(245,245,245,230);"
        "border: 1px solid rgba(0,0,0,80);"
        "border-radius: 4px;"
        "padding: 0px;"
        "font: 13px 'Microsoft YaHei';"
        "}"
        "QPushButton:pressed { background-color: rgba(220,220,220,230); }";

    ui->pushButton_no->setText(QStringLiteral("不要"));
    ui->pushButton_play->setText(QStringLiteral("出牌"));
    ui->pushButton_start->setText(QStringLiteral("开始游戏"));
    ui->pushButton_playfirst->setText(QStringLiteral("出牌"));
    ui->pushButton_one->setText(QStringLiteral("1分"));
    ui->pushButton_two->setText(QStringLiteral("2分"));
    ui->pushButton_three->setText(QStringLiteral("3分"));
    ui->pushButton_NO->setText(QStringLiteral("不抢"));

    const QList<QWidget*> pages = {
        ui->page_Start,
        ui->page_PlayCardfirst,
        ui->page_Getloard,
        ui->page_PlayCard
    };
    for (QWidget *page : pages) {
        if (page && page->layout()) {
            delete page->layout();
        }
    }

    _Pushbuttons<<ui->pushButton_no<<ui->pushButton_play<<ui->pushButton_start
                 <<ui->pushButton_playfirst<<ui->pushButton_one<<ui->pushButton_two
                 <<ui->pushButton_three<<ui->pushButton_NO;
    for(QPushButton *button : _Pushbuttons)
    {
        GameScaling::applyFixedSize(button, btnSize.width(), btnSize.height());
        button->setStyleSheet(style);
    }
    ui->pushButton_start->setParent(ui->page_Start);
    ui->pushButton_start->setGeometry(GameScaling::rect(160, 0, 70, 36));
    ui->pushButton_playfirst->setParent(ui->page_PlayCardfirst);
    ui->pushButton_playfirst->setGeometry(GameScaling::rect(160, 0, 70, 36));
    ui->pushButton_play->setParent(ui->page_PlayCard);
    ui->pushButton_play->setGeometry(GameScaling::rect(160, 0, 70, 36));
    ui->pushButton_no->setParent(ui->page_PlayCard);
    ui->pushButton_no->setGeometry(GameScaling::rect(80, 0, 70, 36));
    ui->pushButton_NO->setParent(ui->page_Getloard);
    ui->pushButton_NO->setGeometry(GameScaling::rect(0, 0, 70, 36));
    ui->pushButton_one->setParent(ui->page_Getloard);
    ui->pushButton_one->setGeometry(GameScaling::rect(80, 0, 70, 36));
    ui->pushButton_two->setParent(ui->page_Getloard);
    ui->pushButton_two->setGeometry(GameScaling::rect(160, 0, 70, 36));
    ui->pushButton_three->setParent(ui->page_Getloard);
    ui->pushButton_three->setGeometry(GameScaling::rect(240, 0, 70, 36));
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
    ui->stackedWidget->setGeometry(rect());
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
