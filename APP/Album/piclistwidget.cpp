#include "piclistwidget.h"
#include "type.h"
#include "protreeitem.h"
#include "piclistitem.h"
#include <QScrollBar>
#include <QGuiApplication>
#include <QPainter>
#include <QWidgetItem>

piclistwidget::piclistwidget(QWidget *parent):QListWidget(parent),_count(0), _last_index(15)
{
    this->setViewMode(QListView::IconMode);
    this->setFlow(QListView::LeftToRight);
    //this->setWrapping(false);//warapp要关闭某人不换行
    //this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//影藏滑动组件

    this->setIconSize(QSize(PREICON_SIZE,PREICON_SIZE));
    this->setSpacing(10);
}

void piclistwidget::updatelistpic(QString path)
{
    if(path=="")
        return;
    QPixmap sor_map(path);
    QPixmap des_map(PREICON_SIZE,PREICON_SIZE);

    sor_map=sor_map.scaled(PREICON_SIZE,PREICON_SIZE,Qt::KeepAspectRatio);
    int s_width=sor_map.width();
    int s_height=sor_map.height();

    des_map.fill(QColor(220,220,220, 50));
    QPainter p(&des_map);
    int x=(PREICON_SIZE-s_width)/2;
    int y=(PREICON_SIZE-s_height)/2;
    p.drawPixmap(x,y,sor_map);
    _count++;
    //piclistitem(const QIcon &icon,const QString &text, QListWidget *listview, int type,int index);
    piclistitem * item=new piclistitem(QIcon(des_map),path,this,TreeItemPic,_count);
    _set_pic[path]=item;
    item->setSizeHint(QSize(PREICON_SIZE,PREICON_SIZE));
    if(_count==1)
    {
        _point=this->pos();
    }
    this->addItem(item);

}

void piclistwidget::updateselect(QTreeWidgetItem *item)
{

    if(!item)
        return;
    auto * current_item=dynamic_cast<protreeitem*>(item);
    QString path=current_item->GetPath();
    auto index=_set_pic.find(path);

    auto listitem=dynamic_cast<piclistitem*>(index.value());

    if(index==_set_pic.end())
        return;
    int  index1=listitem->getindex();

    if(index1>_last_index)
    {

        auto current_pos=this->pos();
        //this->move(current_pos.x()-(index1-_last_index)*100,current_pos.y());
        horizontalScrollBar()->setValue(current_pos.x()-(index1-_last_index)*100);
        _last_index=index1;
    }

    this->setCurrentItem(listitem);



}

void piclistwidget::slotupdatelistitem(QTreeWidgetItem *item)
{
    if(!item)
        return;

    auto * current_item=dynamic_cast<protreeitem*>(item);
    QString path=current_item->GetPath();
    if(_set_pic.find(path)==_set_pic.end())
    {

        updatelistpic(path);
    }
}

void piclistwidget::sloselectupdatepic(QListWidgetItem *item)
{
    if(!item)
        return;
    if(QGuiApplication::mouseButtons()!=Qt::LeftButton)
        return;
    auto cur_item=dynamic_cast<piclistitem*>(item);

    auto path=cur_item->getpath();
    emit sigselectupdatepic(path);

}

