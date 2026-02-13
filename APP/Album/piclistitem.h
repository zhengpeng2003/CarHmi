#ifndef PICLISTITEM_H
#define PICLISTITEM_H

#include <QListWidgetItem>
#include <QWidget>

class piclistitem : public QListWidgetItem
{

public:

    piclistitem(const QIcon &icon,const QString &text, QListWidget *listview, int type,int index);
    int  getindex();
    QString getpath();
private:
    int _index;
    int _type;
    QIcon _icon;
    QString _text;
    QString _path;
};

#endif // PICLISTITEM_H
