#ifndef WIZARD_H
#define WIZARD_H
#include "protreewidget.h"
#include <QWizard>

namespace Ui {
class Wizard;
}

class Wizard : public QWizard
{
    Q_OBJECT

public:
    explicit Wizard(QWidget *parent = nullptr);
    ~Wizard();
protected :
    virtual void done(int result) override;
signals:
    void wizard_signal(const QString &name,const QString & path);

private:
    Ui::Wizard *ui;


};

#endif // WIZARD_H
