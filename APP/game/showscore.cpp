#include "showscore.h"
#include "ui_showscore.h"

ShowScore::ShowScore(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowScore)
{
    ui->setupUi(this);
    myLable << ui->label << ui->label_2 << ui->label_3
            << ui->label_7 << ui->label_8 << ui->label_9
            << ui->label_A << ui->label_B << ui->label_Me
            << ui->label_phase << ui->label_multiplier << ui->label_lord;
    QString scoreStyle = R"(
        QLabel {
            color: rgb(245, 244, 232);
            font-size: 11px;
            font-weight: 600;
            background: transparent;
        }
    )";
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background: rgba(12, 28, 24, 145); border: 1px solid rgba(255,255,255,35); border-radius: 8px;");
    for(int i = 0; i < myLable.size(); i++)
    {
        myLable.at(i)->setStyleSheet(scoreStyle);
    }
    ui->label_A->setMinimumWidth(20);
    ui->label_B->setMinimumWidth(20);
    ui->label_Me->setMinimumWidth(20);
    ui->label_phase->setMinimumWidth(44);
    ui->label_multiplier->setMinimumWidth(24);
    ui->label_lord->setMinimumWidth(36);
}

void ShowScore::InitScore(int a, int b, int c)
{

    ui->label_A->setText(QString::number(a));
    ui->label_B->setText(QString::number(b));
    ui->label_Me->setText(QString::number(c));

}

void ShowScore::SetPhaseText(const QString &text)
{
    ui->label_phase->setText(text);
}

void ShowScore::SetMultiplier(int value)
{
    const int safeValue = value <= 0 ? 1 : value;
    ui->label_multiplier->setText(QString("x%1").arg(safeValue));
}

void ShowScore::SetLordText(const QString &text)
{
    ui->label_lord->setText(text);
}

ShowScore::~ShowScore()
{
    delete ui;
}
