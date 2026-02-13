#include "wizard.h"
#include "ui_wizard.h"

Wizard::Wizard(QWidget *parent)
    : QWizard(parent)
    , ui(new Ui::Wizard)
{
    ui->setupUi(this);


}

Wizard::~Wizard()
{
    delete ui;
}

void Wizard::done(int result)
{
    if(result==QDialog::Rejected)
        return QWizard::done(result);//如果要继续使用这个函数就返回必须要返回而且必须是QWizard

    else
    {

        QString name,path;
        ui->wizardPage1->get_path_name(name,path);
        emit wizard_signal(name,path);
        QWizard::done(result);

    }

}
