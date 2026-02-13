#ifndef PROTREE_H
#define PROTREE_H

#include <QDialog>
#include <QTreeWidget>

namespace Ui {
class protree;
}

class protree : public QDialog
{
    Q_OBJECT

public:
    explicit protree(QWidget *parent = nullptr);
    ~protree();
    QTreeWidget * getwidget();
public slots:
    void addtreewidget(const QString name,QString path);

private:
    Ui::protree *ui;
};

#endif // PROTREE_H
