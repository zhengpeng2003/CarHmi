#include "protreeitem.h"
#include "type.h"
protreeitem::protreeitem(QTreeWidget *view, const QString &name,
                         const QString &path, int type):QTreeWidgetItem (view, type),
    _path(path),_name(name),_root(this),_pre_item(nullptr),_next_item(nullptr)
{
}
protreeitem::protreeitem(QTreeWidgetItem *parent, const QString &name,
                         const QString &path, QTreeWidgetItem* root,int type):QTreeWidgetItem(parent,type),
    _path(path),_name(name),_root(root),_pre_item(nullptr),_next_item(nullptr)
{
}
void protreeitem::SetPreItem(QTreeWidgetItem *item)
{
    _pre_item = item;
}

void protreeitem::SetNextItem(QTreeWidgetItem *item)
{
    _next_item = item;

}

protreeitem *protreeitem::GetNextItem()
{
    return dynamic_cast<protreeitem*>(_next_item);


}

protreeitem *protreeitem::GetpreItem()
{
    return dynamic_cast<protreeitem*>(_pre_item);
}

protreeitem *protreeitem::GetrootItem()
{
    return dynamic_cast<protreeitem*>(_root);
}

protreeitem *protreeitem::Getfirstchild()
{

    auto file_cout=this->childCount();

    if(!file_cout)
        return nullptr;
    for(int i=0;i<file_cout;i++)
    {

        auto first_child=this->child(i);
        auto first_tree_child=dynamic_cast<protreeitem*>(first_child);
        if(first_tree_child->type()==TreeItemPic)
            return first_tree_child;
        first_child=dynamic_cast<protreeitem*>(first_tree_child)->Getfirstchild();
        if(!first_child)
            continue;

        first_tree_child=dynamic_cast<protreeitem*>(first_child);
        return first_tree_child;
    }
    return nullptr;
}

protreeitem *protreeitem::Getlastchild()
{
    auto file_cout=this->childCount();
    if(!file_cout)
    return nullptr;
    for(int i=file_cout-1;i>0;i++)
    {

        auto last_child=this->child(i);
        auto last_tree_child=dynamic_cast<protreeitem*>(last_child);
        if(last_tree_child->type()==TreeItemPic)
            return last_tree_child;
        last_child=last_tree_child->Getlastchild();
        if(!last_child)
            continue;
        last_tree_child=dynamic_cast<protreeitem*>(last_tree_child);
        return last_tree_child;
    }
    return nullptr;
}

protreeitem *protreeitem::Getallchild()
{
    return this->GetNextItem();
}


QString protreeitem::GetPath()
{
    return _path;
}

