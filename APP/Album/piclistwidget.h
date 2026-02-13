#ifndef PICLISTWIDGET_H
#define PICLISTWIDGET_H

#include <QListWidget>
#include <QTreeWidgetItem>
#include <QWidget>

class piclistwidget : public QListWidget
{
    Q_OBJECT
public:
    piclistwidget(QWidget *parent = nullptr);
    void updatelistpic(QString path);
    void updateselect(QTreeWidgetItem *item);
private:
    QPoint _point;
    QMap<QString,QListWidgetItem*> _set_pic;
    int _count;
    int _last_index;
signals:
    void sigselectupdatepic(QString path);
public slots:
    void slotupdatelistitem(QTreeWidgetItem *item);
    void sloselectupdatepic(QListWidgetItem *item);
};

#endif // PICLISTWIDGET_H
