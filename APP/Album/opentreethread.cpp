#include "opentreethread.h"
#include "protreeitem.h"
#include "protreewidget.h"
#include "type.h"

#include "qdir.h"

opentreethread::opentreethread(const QString scr_path, const QString name, int file_cout, protreewidget *self, QObject *parent)
    :QThread(parent),_scr_path(scr_path),_name(name),_file_cout(file_cout),_self(self),_bststop(false)
{

}

void opentreethread::openprotree(const QString scr_path, const QString name, int file_cout, protreewidget *self)
{

    auto *item =new protreeitem(self,name,scr_path,TreeItemPro);

    item->setData(0,Qt::ToolTipRole,scr_path);
    item->setData(0,Qt::DisplayRole,name);
    item->setData(0,Qt::DecorationRole,QIcon(":/icon/dir.png"));
    re_opentree(scr_path,name,file_cout,self,item,item,nullptr);

}

void opentreethread::re_opentree(const QString scr_path, const QString name, int file_cout, protreewidget *self,
                                 protreeitem *root, protreeitem *parent, protreeitem *previous)
{


    QDir scr_path_dir(scr_path);
    scr_path_dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Files);
    scr_path_dir.setSorting(QDir::Name);
    QFileInfoList fileinfolist=scr_path_dir.entryInfoList();
    for(int i=0;i<fileinfolist.size();i++)
    {
        if(_bststop)
            return;
        QFileInfo temp_file=fileinfolist.at(i);
        if(temp_file.isDir())
        {
            if(_bststop)
                return;
            file_cout++;
            emit progressupdate(file_cout);

            //protreeitem::protreeitem(QTreeWidgetItem *parent, const QString &name,
            //                         const QString &path, QTreeWidgetItem* root,int type):QTreeWidgetItem(parent,type),
            //    _path(path),_name(name),_root(root),_pre_item(nullptr),_next_item(nullptr)
            auto *item =new protreeitem(root,temp_file.fileName(),temp_file.filePath(),root,TreeItemDir);
            item->setData(0,Qt::ToolTipRole,temp_file.filePath());
            item->setData(0,Qt::DisplayRole,temp_file.fileName());
            item->setData(0,Qt::DecorationRole,QIcon(":/icon/dir.png"));
            re_opentree(temp_file.filePath(),temp_file.fileName(),file_cout,self,root,item,previous);

        }
        else
        {
            if(_bststop)
                return;
            QString suffix=temp_file.completeSuffix();
            if(suffix!="png"&&suffix!="jpeg"&&suffix!="jpg")//如果这些都不是
            {
                continue;
            }
            file_cout++;
            emit progressupdate(file_cout);
            if(_bststop)
                return;
            //protreeitem(QTreeWidgetItem *parent, const QString &name,const QString &path, QTreeWidgetItem* root,int type);
            auto *item =new protreeitem(parent,temp_file.fileName(),temp_file.filePath(),root,TreeItemPic);
            item->setData(0,Qt::ToolTipRole,temp_file.filePath());
            item->setData(0,Qt::DisplayRole,temp_file.fileName());
            item->setData(0,Qt::DecorationRole,QIcon(":/icon/bulm.ico"));
            if(previous)
            {
                auto* pre_proitem = dynamic_cast<protreeitem*>(previous);
                previous->SetNextItem(item);
            }
            item->SetPreItem(previous);
            previous=item;

        }
    }

}

void opentreethread::run()
{

    openprotree(_scr_path,_name,_file_cout,_self);


    emit progressfinish(_file_cout);
}

void opentreethread::slotprogresscancle()
{
    _bststop=true;

}

