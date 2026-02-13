
#include "protreewidget.h"
#include "removeprotree.h"
#include <QAudioOutput>
#include <QGuiApplication>
#include <protreeitem.h>
#include <protreethread.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qmenu.h>
#include "qlayoutitem.h"
#include "type.h"
#include <QHeaderView>
#include "opentreethread.h"
#include "slideshowdialog.h"
protreewidget::protreewidget(QWidget *parent)
    : QTreeWidget (parent),action_item(nullptr),right_btn_item(nullptr),select_item(nullptr),_mediaplayer(nullptr)
{
    this->header()->hide();
    _open_file =new QAction(this);
    _open_file->setText("导入文件");
    _open_file->setIcon(QIcon(":/icon/openpro.png"));

    _set_action=new QAction(this);
    _set_action->setText("设置活动项目");
    _set_action->setIcon(QIcon(":/icon/core.png"));

    _close_action=new QAction(this);
    _close_action->setText("关闭项目");
    _close_action->setIcon(QIcon(":/icon/close.png"));

    _slide_player=new QAction(this);
    _slide_player->setText("轮播图播放");
    _slide_player->setIcon(QIcon(":/icon/slideshow.png"));

    connect(this,&protreewidget::itemPressed,this,&protreewidget::add_tree_son);
    connect(this,&protreewidget::itemDoubleClicked,this,&protreewidget::slotdobleclick);
    connect(_set_action,&QAction::triggered,this,&protreewidget::set_action_slots);
    connect(_open_file,&QAction::triggered,this,&protreewidget::open_file_slots);
    connect(_close_action,&QAction::triggered,this,&protreewidget::close_action_slots);
    connect(_slide_player,&QAction::triggered,this,&protreewidget::slide_player_slots);

    // Qt5 版本：QMediaPlayer 不需要 QAudioOutput
    _mediaplayer = new QMediaPlayer(this);

    // Qt5 使用 setMedia() 而不是 setSource()
    // _mediaplayer->setLoops(1);  // Qt5 没有 setLoops，用信号槽循环播放
}

void protreewidget::add_tree(const QString &name,const QString &path)//添加文件 创建第一个文件夹当前文件没有内容
{


    QDir dir(path);//相当于存着
    QString file_path=dir.absoluteFilePath(name);
    if(current_tree_name.find(file_path)!=current_tree_name.end())
    {
        return;
    }
    else
    {
        QDir pro_dir(file_path);
        if(!pro_dir.exists())
        {
            bool temp=pro_dir.mkdir(file_path);
            if(!temp)
            {

                return;
            }

        }
    }
    _set_path.insert(file_path);
    //protreeitem(QTreeWidget *view, const QString &name,const QString &path, int type):QTreeWidgetItem()
    auto *item=new  protreeitem(this,name,file_path,0);


    //auto *item=new  QTreeWidgetItem(this);
    item->setData(0,Qt::DisplayRole,name);
    item->setData(0,Qt::ToolTipRole,file_path);
    item->setData(0,Qt::DecorationRole,QIcon(":/icon/dir.png"));
    this->addTopLevelItem(item);

    // 确保项目可见
    this->setCurrentItem(item);
    this->scrollToItem(item);
    this->viewport()->update();

    // 调试输出


    QApplication::processEvents(); // 处理所有待处理事件，包括重绘

    // 或者只更新特定窗口
    if (QWidget *topLevel = QApplication::activeWindow()) {
        topLevel->update();
        topLevel->repaint();
    }
}

 void protreewidget::add_tree_son(QTreeWidgetItem *item, int column)//右键点击项目 添加项目
{

    if(QApplication::mouseButtons()==Qt::RightButton)
    {
        _menu=new QMenu(this);
        _menu->addAction(_open_file);
        _menu->addAction(_close_action);
        _menu->addAction(_set_action);
        _menu->addAction(_slide_player);
        right_btn_item=item;//当前的右键
        _menu->exec(QCursor::pos());

    }

}
//导入文件
void protreewidget::open_file_slots()
{

    QFileDialog _open_file_Dialog;
    _open_file_Dialog.setFileMode(QFileDialog::Directory);
    _open_file_Dialog.setWindowTitle("导入文件");
    QString path = "";
    if(!right_btn_item)
    {

        return;
    }
    path = dynamic_cast<protreeitem*>(right_btn_item)->GetPath();
    _open_file_Dialog.setDirectory(path);
    _open_file_Dialog.setWindowIcon(QIcon(":/icon/dir.png"));
    _open_file_Dialog.setViewMode(QFileDialog::Detail);//显示目录中每个项目的图标、名称以及详细信息。
    if(_open_file_Dialog.exec())
    {
        filenames=_open_file_Dialog.selectedFiles();

    }
    if(filenames.length()<=0)
    {
        return;
    }

    QString filepath=filenames.at(0);//打开文件的路径

    _dialog_progess = new QProgressDialog(this);//设置进度条

    int file_cout=0;
    //创建线程
    _thread_create_pro=std::make_shared<protreethread>(filepath,path,right_btn_item,file_cout,this,right_btn_item);

    //绑定添加文件与进度条的关系随时更新
    connect(_thread_create_pro.get(),&protreethread::signalupdateprogress,this,&protreewidget::progressupdate);
    connect(_thread_create_pro.get(),&protreethread::signalfishprogress,this,&protreewidget::progressfinish);

    //当progress点击取消触发信号 然后将消息框删除目的为了释放
    connect(_dialog_progess,&QProgressDialog::canceled,this,&protreewidget::progresscancel);
    //再次触发一个信号 然后跳转到thread的槽实现
    connect(this,&protreewidget::signalotcancle,_thread_create_pro.get(),&protreethread::slotcancle);

    _thread_create_pro->start();
    _dialog_progess->setWindowTitle("place wait");
    _dialog_progess->setFixedWidth(PROGRESS_WIDTH);
    _dialog_progess->setRange(0,PROGRESS_WIDTH);


}
//字体加粗
void protreewidget::set_action_slots()
{
    if(!right_btn_item)
        return;
    QFont font;
    font.setBold(false);
    if(action_item)//已经点击了
    {
        action_item->setFont(0,font);//点击过了我如果再点击别的就会先判断这个 然后下面再设置新的
    }

        //下面这个是一直都会走的必须有一个加粗
        action_item=right_btn_item;
        font.setBold(true);
        action_item->setFont(0,font);


}
//关闭项目
void protreewidget::close_action_slots()
{
    removeprotree remove_dialog;
    auto res=remove_dialog.exec();
    if(res==QDialog::Rejected)
        return;
    bool is_delete_file=remove_dialog.isremoved();//判断是否删除文件
    //获取路径

    auto  index_right_btn=this->indexOfTopLevelItem(right_btn_item);//返回当前item的索引值
    auto * Protreeitem=dynamic_cast<protreeitem*>(right_btn_item);//右键
    auto * tempselect=dynamic_cast<protreeitem*>(select_item);
    auto delete_path=Protreeitem->GetPath();
    _set_path.remove(delete_path);




    if(is_delete_file)
    {
        QDir delet_dir(delete_path);
        delet_dir.removeRecursively();//删除多个文件递归
    }
    if(Protreeitem==action_item)//删除之后要置空如果不用的话 因为它本身是指向同一个地方
    {
        action_item=nullptr;
    }
    if(tempselect&&Protreeitem==tempselect->GetrootItem())//我只判断了一个
    {

        select_item=nullptr;
        tempselect=nullptr;
        emit sigcolse();
    }
    delete this->takeTopLevelItem(index_right_btn);//删除索引的项
    right_btn_item=nullptr;

}
//幻灯片播放
void protreewidget::slide_player_slots()
{

    if(!right_btn_item)
    {
        return;
    }
    auto *first_child=dynamic_cast<protreeitem*>(right_btn_item)->Getfirstchild();
    if(!first_child)
    {


        return ;
    }


    auto* last_child=dynamic_cast<protreeitem*>(right_btn_item)->Getlastchild();


    _slide_show_dlg = std::make_shared<slideshowdialog>(this, first_child, last_child);
    _slide_show_dlg->setModal(true);//设置阻塞必须在这个界面完成才可以进行下一步
    _slide_show_dlg->showMaximized();
}


//打开文件
void protreewidget::open_tree(const QString &path)
{
    if(_set_path.find(path)==_set_path.end())//判断当前有没有这个文件
    {
        QDir open_path(path);
        _set_path.insert(path);
        int file_cout=0;
        QString open_path_name=open_path.dirName();
        //opentreethread::opentreethread(const QString scr_path, const QString name, int file_cout,protreewidget *self, QObject *parent)
        //    : QThread(parent),_scr_path(scr_path),_bststop(false),_name(name),_file_cout(file_cout)
       _thread_open_pro=std::make_shared<opentreethread>(path,open_path_name,file_cout,this,nullptr);
        _dialog_progess = new QProgressDialog(this);

        connect(_thread_open_pro.get(),&opentreethread::progressupdate,this,&protreewidget::progressupdate);
        connect(_thread_open_pro.get(),&opentreethread::progressfinish,this,&protreewidget::progressfinish);
        connect(_dialog_progess,&QProgressDialog::canceled,this,&protreewidget::progresscancel);
        connect(this,&protreewidget::signalotcancle,_thread_open_pro.get(),&opentreethread::slotprogresscancle);



        _thread_open_pro->start();
        _dialog_progess->setWindowTitle("place wait");//设置主题
        _dialog_progess->setFixedWidth(PROGRESS_WIDTH);//设置大小
        _dialog_progess->setRange(0,PROGRESS_WIDTH);


    }
    else
        return;


}
//进度条更新
void protreewidget::progressupdate(int cout)
{


    if(!_dialog_progess)
        return;
    if(cout>PROGRESS_MAX)
        _dialog_progess->setValue(cout%PROGRESS_MAX);
    else
        _dialog_progess->setValue(cout);
}
//进度条完成
void protreewidget::progressfinish(int cout)
{

    _dialog_progess->setValue(PROGRESS_MAX);
    _dialog_progess->deleteLater();//“嘿，Qt，等当前这一轮事件处理完了，你找个合适的时机，帮我把这个对象安全地删掉。
    //”Qt 会把这个请求放入事件队列中，​等控制权重新回到该线程的事件循环时，它才会真正调用 delete来销毁这个对象。
}
//进度条取消
void protreewidget::progresscancel()
{
    emit signalotcancle();
    delete _dialog_progess;
    _dialog_progess =nullptr;

}

void protreewidget::slpreupdate()
{
    if(!select_item)
        return;

    auto * current_item=dynamic_cast<protreeitem*>(select_item)->GetpreItem();
    if(!current_item)
    {
    return;
    }
    select_item=current_item;
    emit  signal_preshowpic(current_item->GetPath());
    this->setCurrentItem(select_item);
}

void protreewidget::slnextupdate()
{
    if(!select_item)
        return;

    auto * current_item=dynamic_cast<protreeitem*>(select_item)->GetNextItem();
    if(!current_item)
    {
        return;
    }
    select_item=current_item;
    emit  signal_nextshowpic(current_item->GetPath());
    this->setCurrentItem(select_item);
}

void protreewidget::set_music()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setDirectory(QDir::current());
    dialog.setWindowTitle("设置播放音乐");
    dialog.setNameFilter("*.mp3");

    QStringList filenames;
    if(dialog.exec())
    {
        filenames=dialog.selectedFiles();
    }
    else
        return;

    if(filenames.length()<=0)
        return;

    _mediaplerlist.clear();
    _mediaplerlist.append(QUrl::fromLocalFile(filenames.at(0)));

    // Qt5: setMedia() 而不是 setSource()
    _mediaplayer->setMedia(_mediaplerlist.at(0));
}
void protreewidget::startmusic()
{
    _mediaplayer->play();

}

void protreewidget::stopmusic()
{
    _mediaplayer->stop();
}


void protreewidget::slotdobleclick(QTreeWidgetItem *item, int column)
{
    if(QGuiApplication::mouseButtons()==Qt::LeftButton)
    {
        select_item=item;

        auto tree_item=dynamic_cast<protreeitem*>(item);

        if(!tree_item)
            return;

        else
        {
            QString path=tree_item->GetPath();
            emit signal_showpic(path);
        }
    }

}
