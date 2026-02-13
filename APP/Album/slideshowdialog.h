#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include <QDialog>
#include <protreeitem.h>
namespace Ui {
class slideshowdialog;
}

class slideshowdialog : public QDialog
{
    Q_OBJECT

public:
    slideshowdialog(QWidget *parent,protreeitem *firstchild,protreeitem * lastchild);
    ~slideshowdialog();

private:
    protreeitem *_firstchild;
    protreeitem * _lastchild;
    Ui::slideshowdialog *ui;
};

#endif // SLIDESHOWDIALOG_H
