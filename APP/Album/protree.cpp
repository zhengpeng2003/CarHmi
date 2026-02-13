#include "protree.h"
#include "ui_protree.h"

protree::protree(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::protree)
{
    ui->setupUi(this);

    this->setMinimumWidth(378);
    this->setMaximumWidth(378);
}

protree::~protree()
{
    delete ui;
}

 QTreeWidget* protree::getwidget()
{
     return ui->treeWidget;
}

void protree::addtreewidget(const QString name, QString path)
{
    ui->treeWidget->add_tree(name,path);
}

