#include "removeprotree.h"
#include "ui_removeprotree.h"

removeprotree::removeprotree(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::removeprotree)
{
    ui->setupUi(this);
    ui->checkBox_2->setCheckState(Qt::Unchecked);
}

removeprotree::~removeprotree()
{
    delete ui;
}

bool removeprotree::isremoved()
{
    bool state =ui->checkBox_2->checkState();
    return state;
}
