#include "piclistitem.h"



piclistitem::piclistitem(const QIcon &icon, const QString &text, QListWidget *listview, int type, int index)
    :QListWidgetItem(icon," ",listview,type),_index(index),_icon(icon),_text(text),_path(text)
{

}
int piclistitem::getindex()
{
    return _index;
}
QString piclistitem::getpath()
{
    return _path;
}
