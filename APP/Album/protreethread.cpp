#include "protreethread.h"
#include "protreeitem.h"
#include <protreeitem.h>
#include <qapplication.h>
#include <qdir.h>
#include "type.h"
protreethread::protreethread(const QString &scr_path,const QString &dis_path,
                             QTreeWidgetItem* parent_item,int file_cout,QTreeWidget *self,QTreeWidgetItem *root)
    :QThread(nullptr),_scr_path(scr_path),_dis_path(dis_path),_file_cout(file_cout),_parent_item(parent_item),_self(self),_root(root),
    _bststop(false)
{

}

protreethread::~protreethread()
{

}
void protreethread::creatprotreethread(const QString & scr_path,const QString & dis_path,QTreeWidgetItem* paremt_item,int  file_cout,
                                       QTreeWidget * self,QTreeWidgetItem *root, QTreeWidgetItem* preItem)
{

    if(_bststop)
    {
        return;
    }
    bool needcopy = true;
    if(scr_path==dis_path)//不需要拷贝
    {
        needcopy=false;
    }
    QDir import_dir(scr_path);//导入路径
    QStringList namefiles;
    import_dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);//过滤剩下的
    import_dir.setSorting(QDir::Name);
    QFileInfoList list=import_dir.entryInfoList();//过滤完剩下的内容
    for(int i=0;i<list.size();i++)
    {
        if(_bststop)//随时进行判断
        {
            return;
        }
        QFileInfo fileinfo=list.at(i);
        bool bisDir=fileinfo.isDir();//判断当前文件是不是目录
        if(bisDir)
        {
            if(_bststop)

            return;
            else
            {
                file_cout++;
                emit signalupdateprogress(file_cout);
                QDir sub_dist(dis_path);
                QString sub_dist_path=sub_dist.absoluteFilePath(fileinfo.fileName());
                QDir sub_dist_dir(sub_dist_path);
                if(!sub_dist_dir.exists())
                {
                    bool OK=sub_dist_dir.mkpath(sub_dist_path);
                    if(!OK)
                        continue;
                }

                //protreeitem(QTreeWidgetItem *parent, const QString &name,const QString &path, QTreeWidgetItem* root,int type);
                //auto * item=new protreeitem(paremt_item,fileinfo.fileName(),dis_file_path,root,0);
                auto * item=new protreeitem(paremt_item,fileinfo.fileName(),sub_dist_path,root,TreeItemDir);
                item->setData(0,Qt::DisplayRole,fileinfo.fileName());//展示字符串

                item->setData(0,Qt::DecorationRole,QIcon(":/icon/dir.png"));//图片
                item->setData(0,Qt::ToolTipRole,sub_dist_path);//提示信息
                //creatprotreethread(const QString & scr_path,const QString & dis_path,QTreeWidgetItem* paremt_item,int  file_cout,
                //                   QTreeWidget * self,QTreeWidgetItem *root, QTreeWidgetItem* preItem)
                creatprotreethread(fileinfo.absoluteFilePath(),sub_dist_path,item,file_cout,self,root,preItem);
                //absoluteFilePath和absolutepath不一样 一个有文件名一个没有

            }

        }
        else
        {
            if(_bststop)
            {
                return;
            }
            const QString & suffix = fileinfo.completeSuffix();//取出后缀文件
            if(suffix!="png"&&suffix!="jpeg"&&suffix!="jpg")//如果这些都不是
            {
                continue;
            }
            file_cout++;
            emit signalupdateprogress(file_cout);
            if(!needcopy)
            {
                continue;
            }
            QDir dis_dir(dis_path);//拷贝文件
            QString dis_file_path=dis_dir.absoluteFilePath(fileinfo.fileName());
            if(!QFile::copy(fileinfo.absoluteFilePath(),dis_file_path))//原来文件路径的地址 copy到新地址
                continue;
            else
            {

                //protreeitem(QTreeWidget *view, const QString &name,const QString &path, int type);
                //protreeitem(QTreeWidgetItem *parent, const QString &name,const QString &path, QTreeWidgetItem* root,int type);

                auto * item=new protreeitem(paremt_item,fileinfo.fileName(),dis_file_path,root,TreeItemPic);


                item->setData(0,Qt::DisplayRole,fileinfo.fileName());//展示字符串
                item->setData(0,Qt::DecorationRole,QIcon(":/icon/pic.png"));//图片
                item->setData(0,Qt::ToolTipRole,dis_file_path);//提示信息

                if(preItem)
                {

                auto * pre_proitem=dynamic_cast<protreeitem*>(preItem);//开始没有前一个节点所以要设置
                pre_proitem->SetNextItem(item);
                }
                item->SetPreItem(preItem);
                preItem=item;
            }
        }

    }

}
void protreethread::run()
{

    creatprotreethread(_scr_path,_dis_path,_parent_item,_file_cout,_self,_root,nullptr);
    if(_bststop)//最好重新定义一个函数这个没有必要
    {
        auto path=dynamic_cast<protreeitem*>(_root)->GetPath();
        auto index=_self->indexOfTopLevelItem(_root);//返回给定顶级项的索引值，若未找到该项则返回 -1
        delete _self->takeTopLevelItem(index);
        QDir dir(path);
        dir.removeRecursively();
        return;
    }
    emit signalfishprogress(_file_cout);

}

void protreethread::slotcancle()
{

    _bststop=false;
}

