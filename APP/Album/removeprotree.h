#ifndef REMOVEPROTREE_H
#define REMOVEPROTREE_H

#include <QDialog>

namespace Ui {
class removeprotree;
}

class removeprotree : public QDialog
{
    Q_OBJECT

public:
    explicit removeprotree(QWidget *parent = nullptr);
    ~removeprotree();
    bool isremoved();


private:
    Ui::removeprotree *ui;

};

#endif // REMOVEPROTREE_H
