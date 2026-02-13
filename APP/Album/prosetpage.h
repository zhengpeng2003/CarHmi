#ifndef PROSETPAGE_H
#define PROSETPAGE_H

#include <QWizardPage>

namespace Ui {
class Prosetpage;
}

class Prosetpage : public QWizardPage
{
    Q_OBJECT

public:
    explicit Prosetpage(QWidget *parent = nullptr);
    void get_path_name(QString &name,QString& path);
    ~Prosetpage();
protected:
    virtual bool isComplete()const;
    //bool isComplete();  // 错误写法（对Qt向导无效）不要这么写
private slots:
    void on_pushButton_browse_clicked();

private:
    Ui::Prosetpage *ui;
};

#endif // PROSETPAGE_H
