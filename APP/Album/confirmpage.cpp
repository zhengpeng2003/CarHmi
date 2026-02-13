#include "confirmpage.h"
#include "ui_confirmpage.h"

confirmpage::confirmpage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::confirmpage)
{
    ui->setupUi(this);
}

confirmpage::~confirmpage()
{
    delete ui;
}
