#include "showscore.h"
#include "ui_showscore.h"

ShowScore::ShowScore(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowScore)
{
    ui->setupUi(this);
    myLable<<ui->label<<ui->label_2<<ui->label_3<<
    ui->label_7<<ui->label_8<<ui->label_9<<
        ui->label_A<<ui->label_B<<ui->label_Me;
    QString scoreStyle = R"(
        QLabel {
            color: red;
            font-size: 18px;
            font-weight: bold;
            background: transparent;
        }
    )";
    for(int i=0;i<myLable.size();i++)
    {
        myLable.at(i)->setStyleSheet(scoreStyle);
    }
}

void ShowScore::InitScore(int a, int b, int c)
{

    ui->label_A->setText(QString::number(a));
    ui->label_B->setText(QString::number(b));
    ui->label_Me->setText(QString::number(c));

}

ShowScore::~ShowScore()
{
    delete ui;
}
