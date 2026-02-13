#include "endpanel.h"
EndPanel::EndPanel(player *player, QWidget *parent) :QWidget{parent},_Player(player)
{

    // qDebug()<<player;
    _Gameoverpix.load(":/images/gameover.png");
    this->resize(_Gameoverpix.size());
    QLabel * Fontpix=new QLabel(this);
    if(player->GetRole()==player::LORD&&player->Getwin()==true)
    {
        Fontpix->setPixmap(QPixmap(":/images/lord_win.png"));
    }
    if(player->GetRole()==player::LORD&&player->Getwin()==false)
    {
        Fontpix->setPixmap(QPixmap(":/images/lord_fail.png"));
    }
    if(player->GetRole()==player::FORMAR&&player->Getwin()==true)
    {
        Fontpix->setPixmap(QPixmap(":/images/farmer_win.png"));
    }
    if(player->GetRole()==player::FORMAR&&player->Getwin()==false)
    {
        Fontpix->setPixmap(QPixmap(":/images/farmer_fail.png"));
    }
    ShowScore * WidgetSCore=new ShowScore(this);

    WidgetSCore->InitScore(player->GetPrePlayer()->GetScore(),player->GetNextPlayer()->GetScore(),player->GetScore());
    QString stylebtn=R"(
    QPushButton {
        border: none;
        background: transparent;
        border-image: url(:/images/button_normal.png);
    }
    QPushButton:hover {
        border-image: url(:/images/button_hover.png);
    }
    QPushButton:pressed {
        border-image: url(:/images/button_pressed.png);
    }

    )";
    QPushButton * btncontinue=new QPushButton(this);
    btncontinue->setStyleSheet(stylebtn);
    btncontinue->move(84,429);
    btncontinue->setFixedSize(231,48);
    WidgetSCore->move(75,230);
    WidgetSCore->setFixedSize(260,160);
    Fontpix->move(125,125);
    connect(btncontinue,&QPushButton::clicked,this,&EndPanel::S_Continue);
}

void EndPanel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);//启用图片平滑变换当图片需要缩放时，使用高质量的插值算法（如双线性插值）
    painter.setRenderHint(QPainter::Antialiasing, true);//启用抗锯齿对图形边缘进行平滑处理，消除锯齿
    painter.drawPixmap(0,0,_Gameoverpix);
}
