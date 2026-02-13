#ifndef CONFIRMPAGE_H
#define CONFIRMPAGE_H

#include <QWizardPage>

namespace Ui {
class confirmpage;
}

class confirmpage : public QWizardPage
{
    Q_OBJECT

public:
    explicit confirmpage(QWidget *parent = nullptr);
    ~confirmpage();

private:
    Ui::confirmpage *ui;
};

#endif // CONFIRMPAGE_H
