#include "endpanel.h"

#include <QPainterPath>

EndPanel::EndPanel(player *player, QWidget *parent) :QWidget{parent},_Player(player)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setStyleSheet(
        "background: rgba(7, 18, 22, 224);"
        "border: 1px solid rgba(255,255,255,58);"
        "border-radius: 14px;");

    _Gameoverpix.load(":/images/gameover.png");
    resize(236, 146);

    QString resultPath;
    if(player->GetRole()==player::LORD&&player->Getwin()==true)
    {
        resultPath = ":/images/lord_win.png";
    }
    if(player->GetRole()==player::LORD&&player->Getwin()==false)
    {
        resultPath = ":/images/lord_fail.png";
    }
    if(player->GetRole()==player::FORMAR&&player->Getwin()==true)
    {
        resultPath = ":/images/farmer_win.png";
    }
    if(player->GetRole()==player::FORMAR&&player->Getwin()==false)
    {
        resultPath = ":/images/farmer_fail.png";
    }

    _ResultLabel = new QLabel(this);
    _ResultLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QPixmap resultPixmap(resultPath);
    resultPixmap = resultPixmap.scaled(QSize(160, 44), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _ResultLabel->setPixmap(resultPixmap);
    _ResultLabel->setFixedSize(resultPixmap.size());
    _ResultLabel->move((width() - _ResultLabel->width()) / 2, 16);

    _SummaryLabel = new QLabel(this);
    _SummaryLabel->setAlignment(Qt::AlignCenter);
    _SummaryLabel->setStyleSheet("color: rgb(246,241,224); font-size: 12px; font-weight: 700; background: transparent;");
    _SummaryLabel->setGeometry(18, 62, width() - 36, 20);
    _SummaryLabel->setText(player->Getwin() ? QStringLiteral("本局获胜") : QStringLiteral("再来一把"));

    _ScoreLabel = new QLabel(this);
    _ScoreLabel->setAlignment(Qt::AlignCenter);
    _ScoreLabel->setStyleSheet("color: rgba(246,241,224,210); font-size: 10px; background: transparent;");
    _ScoreLabel->setGeometry(14, 84, width() - 28, 18);
    _ScoreLabel->setText(
        QStringLiteral("A:%1   B:%2   我:%3")
            .arg(player->GetPrePlayer()->GetScore())
            .arg(player->GetNextPlayer()->GetScore())
            .arg(player->GetScore()));

    _ContinueButton = new QPushButton(QStringLiteral("继续"), this);
    _ContinueButton->setFixedSize(96, 30);
    _ContinueButton->move((width() - _ContinueButton->width()) / 2, 106);
    _ContinueButton->setStyleSheet(
        "QPushButton {"
        "background: rgba(245, 182, 70, 215);"
        "color: rgb(18, 18, 18);"
        "border: none;"
        "border-radius: 15px;"
        "font-size: 12px;"
        "font-weight: 700;"
        "padding: 0 10px;"
        "}"
        "QPushButton:hover { background: rgba(255, 201, 98, 225); }"
        "QPushButton:pressed { background: rgba(228, 160, 55, 225); }");
    connect(_ContinueButton,&QPushButton::clicked,this,&EndPanel::S_Continue);
}

void EndPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);//启用图片平滑变换当图片需要缩放时，使用高质量的插值算法（如双线性插值）
    painter.setRenderHint(QPainter::Antialiasing, true);//启用抗锯齿对图形边缘进行平滑处理，消除锯齿

    QPainterPath path;
    path.addRoundedRect(rect().adjusted(0, 0, -1, -1), 14, 14);
    painter.fillPath(path, QColor(7, 18, 22, 224));
    painter.setPen(QColor(255, 255, 255, 58));
    painter.drawPath(path);

    if(!_Gameoverpix.isNull())
    {
        painter.save();
        painter.setOpacity(0.10);
        QPixmap watermark = _Gameoverpix.scaled(QSize(110, 110), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap((width() - watermark.width()) / 2,
                           (height() - watermark.height()) / 2 - 8,
                           watermark);
        painter.restore();
    }
}
