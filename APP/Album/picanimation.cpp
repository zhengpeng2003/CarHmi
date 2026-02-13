#include "picanimation.h"

#include "protreeitem.h"

#include <QMouseEvent>
#include <QPainter>
int a=0;
picanimation::picanimation(QWidget *parent)
    : QWidget{parent},_bst_stop(true),_float(0.0),_current_item(nullptr)
{
    _timer=new QTimer(this);
    connect(_timer,&QTimer::timeout,this,&picanimation::timeout);
}

picanimation::~picanimation()
{

}

void picanimation::setanimation(QTreeWidgetItem *first_chid)
{



    if(!first_chid)
        return;
    auto * pre_item=dynamic_cast<protreeitem*>(first_chid);


    QString path1=pre_item->GetPath();

    _pixmap1.load(path1);
    _current_item=first_chid;
    if(_map_info.find(path1)==_map_info.end())
    {
        _map_info[path1]=first_chid;

        emit signalupdatelistitem(pre_item);


    }

    emit sigselectupdate(first_chid);
    auto *  next_item=pre_item->GetNextItem();

    if(!next_item)
    {
        return;

    }
    QString path2=next_item->GetPath();
    _pixmap2.load(path2);

    /*if(_map_info.find(path2)==_map_info.end())
    {

        _map_info[path2]=next_item;
        emit signalupdatelistitem(pre_item);
    }*/
     update();

}

void picanimation::timestart()
{
    _timer->start(35);
    _bst_stop=true;
    emit musicstart();
}

void picanimation::timeout()
{
    if(!_current_item)
    {
        stop();
        update();
        return;

    }
    _float+=0.01;

    if(_float>=1)
    {

        _float=0;
        auto * pre_item=dynamic_cast<protreeitem*>(_current_item);
        auto * next_item=pre_item->GetNextItem();
        if(!pre_item)
        {
            stop();
            update();
            return;

        }

        setanimation(next_item);
        update();
        return;


    }
    update();

}

void picanimation::stop()
{
    _float=0;
    _bst_stop=false;
    _timer->stop();
    emit musicstop();
//要加一个float
}

void picanimation::paintEvent(QPaintEvent *event)
{
    if(_pixmap1.isNull())
    {
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing,true);
    QRect rect=geometry();
    int w=rect.width();
    int h=rect.height();
    _pixmap1=_pixmap1.scaled(w,h,Qt::KeepAspectRatio);
    int clarity=255*(1.0f-_float);

    QPixmap alphaPixmap(_pixmap1.size());
    alphaPixmap.fill(Qt::transparent);



    QPainter p1(&alphaPixmap);
    p1.setCompositionMode(QPainter::CompositionMode_Source);
    p1.drawPixmap(0,0,_pixmap1);
    p1.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p1.fillRect(alphaPixmap.rect(),QColor(0,0,0,clarity));
    p1.end();

    int x=(w-_pixmap1.width())/2;
    int y=(h-_pixmap1.height())/2;
    p.drawPixmap(x,y,alphaPixmap);
    update();
    if(_pixmap2.isNull())
    {
        stop();        return;
    }
    int clarity2=255*_float;
    _pixmap2=_pixmap2.scaled(w,h,Qt::KeepAspectRatio);
    QPixmap alphaPixmap2(_pixmap2.size());
    alphaPixmap2.fill(Qt::transparent);

    QPainter p2(&alphaPixmap2);
    p2.setCompositionMode(QPainter::CompositionMode_Source);
    p2.drawPixmap(0,0,_pixmap2);
    p2.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p2.fillRect(alphaPixmap2.rect(),QColor(0,0,0,clarity2));
    p2.end();
    int x1=(w-_pixmap2.width())/2;
    int y1=(h-_pixmap2.height())/2;
    p.drawPixmap(x1,y1,alphaPixmap2);

    p.end();

}

void picanimation::slotpreshow()
{
    _float=1;
    stop();

    if(!_current_item)
        return;
    auto cur_item=dynamic_cast<protreeitem*>(_current_item);
    auto pre_item=cur_item->GetpreItem();
    if(!pre_item)
        return;
    setanimation(pre_item);
    update();

}

void picanimation::slotnextshow()
{
    _float=1;
    stop();

    if(!_current_item)
        return;
    auto cur_item=dynamic_cast<protreeitem*>(_current_item);
    auto next_item=cur_item->GetNextItem();
    if(!next_item)
        return;
    setanimation(next_item);
    update();
}

void picanimation::slotupdatebtn()
{
    if(_bst_stop)//当前为播放  设置暂停按钮
    {
        _float=0;
        update();
        stop();
        emit sigplaybtn();

    }
    else//当前为暂停
    {
        _float=0;
        update();
        timestart();
        emit sigstopbtn();
    }

}

void picanimation::slotselectupdatepic(QString path)
{

    _float=1;
    stop();
    if(path=="")
        return;

    auto is_have=_map_info.find(path);
    if(is_have==_map_info.end())
        return;
    auto cur_item=dynamic_cast<protreeitem*>(is_have.value());

    setanimation(cur_item);


}
