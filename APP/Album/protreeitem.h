#ifndef PROTREEITEM_H
#define PROTREEITEM_H

#include <QTreeWidgetItem>
#include <QWidget>

class protreeitem : public QTreeWidgetItem
{
public:
    protreeitem(QTreeWidget *view, const QString &name,const QString &path, int type);
    protreeitem(QTreeWidgetItem *parent, const QString &name,const QString &path, QTreeWidgetItem* root,int type);

    void SetPreItem(QTreeWidgetItem *item);
    void SetNextItem(QTreeWidgetItem *item);
    protreeitem * GetNextItem();
    protreeitem * GetpreItem();
    protreeitem * GetrootItem();
    protreeitem * Getfirstchild();
    protreeitem * Getlastchild();
    protreeitem * Getallchild();
    QString GetPath();
private:
    QString    _path;
    QString    _name;
    QTreeWidgetItem*   _root;
    QTreeWidgetItem*   _pre_item;
    QTreeWidgetItem*   _next_item;


};

#endif // PROTREEITEM_H
