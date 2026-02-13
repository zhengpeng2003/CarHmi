#include "mainwindow.h"
#include "picshow.h"
#include "ui_mainwindow.h"
#include "wizard.h";
#include <protree.h>
//1
#include <qfiledialog.h>
#include "mainwindow.h"
#include "picshow.h"
#include "ui_mainwindow.h"
#include "wizard.h"
#include <protree.h>
#include <QFileDialog>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), _protree(nullptr), _pic_show(nullptr)
{
    this->resize(480, 272);
    ui->setupUi(this);

    // 左侧项目树
    _protree = new protree;
    _protree->setFixedWidth(160);
    ui->verticalLayout->addWidget(_protree);

    // 右侧图片显示
    _pic_show = new picshow;
    ui->verticalLayout_2->addWidget(_pic_show);

    // ========== 菜单栏 ==========
    // 文件菜单
    QMenu *menu_file = menuBar()->addMenu("文件(F)");

    QAction *menu_file_create = new QAction(QIcon(":/icon/createpro.png"), tr("创建项目"), this);
    menu_file_create->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));

    QAction *menu_file_open = new QAction(QIcon(":/icon/openpro.png"), tr("打开项目"), this);
    menu_file_open->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));

    // ⭐ 退出按钮
    QAction *menu_exit = new QAction(QIcon(":/icon/exit.png"), tr("退出"), this);
    menu_exit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    menu_file->addAction(menu_file_create);
    menu_file->addAction(menu_file_open);
    menu_file->addSeparator();
    menu_file->addAction(menu_exit);

    // 设置菜单
    QMenu *menu_set = menuBar()->addMenu("设置(S)");

    QAction *menu_set_music = new QAction(QIcon(":/icon/music.png"), tr("设置背景"), this);
    menu_set_music->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));

    menu_set->addAction(menu_set_music);

    // ========== 信号槽连接 ==========
    QTreeWidget *pro_tree = dynamic_cast<protree*>(_protree)->getwidget();
    auto *_protreewidget = dynamic_cast<protreewidget*>(pro_tree);

    // 文件菜单
    connect(menu_file_create, &QAction::triggered, this, &MainWindow::create_project);
    connect(menu_file_open, &QAction::triggered, this, &MainWindow::open_project);
    connect(menu_exit, &QAction::triggered, qApp, &QApplication::quit);  // ⭐ 退出应用

    // 设置菜单
    connect(menu_set_music, &QAction::triggered, _protreewidget, &protreewidget::set_music);

    // 打开项目信号
    connect(this, &MainWindow::signal_oepn, _protreewidget, &protreewidget::open_tree);

    // 图片显示
    auto *_pic_show_pro = dynamic_cast<picshow*>(_pic_show);
    connect(_protreewidget, &protreewidget::signal_showpic, _pic_show_pro, &picshow::slotpicshow);

    // 前后翻页
    connect(_pic_show_pro, &picshow::sigprebtn, _protreewidget, &protreewidget::slpreupdate);
    connect(_protreewidget, &protreewidget::signal_preshowpic, _pic_show_pro, &picshow::slotprepicshow);
    connect(_pic_show_pro, &picshow::signextbtn, _protreewidget, &protreewidget::slnextupdate);
    connect(_protreewidget, &protreewidget::signal_nextshowpic, _pic_show_pro, &picshow::slotnextpicshow);

    // 关闭项目
    connect(_protreewidget, &protreewidget::sigcolse, _pic_show_pro, &picshow::slotclosepic);
}
void MainWindow::open_project()
{



    QFileDialog open_dialog(this);
    open_dialog.setMinimumSize(480, 272);
    open_dialog.setFileMode(QFileDialog::Directory);//一个目录
    open_dialog.setViewMode(QFileDialog::Detail);
    open_dialog.setWindowTitle("打开文件");
    open_dialog.setDirectory(QDir::current());
    open_dialog.resize(480,272);
    open_dialog.setMinimumSize(480, 272);


    if(open_dialog.exec()==QDialog::Rejected)
    return;
    QStringList filename;
    filename=open_dialog.selectedFiles();
    if(filename.size()<=0)
        return;
    else
    {

        //connect(this,&MainWindow::signal_oepn,dynamic_cast<protree*>(_protree),&protree::opentreewidget);
        emit signal_oepn(filename.at(0));


        QString current_path=filename.at(0);
    }
}
 void MainWindow::create_project()
    {


        // 创建向导对象（堆分配）
        Wizard *wizard = new Wizard(this);

        // 设置窗口标题
        wizard->setWindowTitle("创建项目");
        connect(wizard,&Wizard::wizard_signal,dynamic_cast<protree*>(_protree),&protree::addtreewidget);//与protreewidget连接
        // 模态显示对话框（阻塞直到关闭
        if (wizard->exec() == QDialog::Accepted) {
            // 获取用户输入的数据
            QString projectName = wizard->field("projectName").toString();
            QString projectPath = wizard->field("projectPath").toString();



            // 实际创建项目的代码
            // createProject(projectName, projectPath);


        }
        else
        {

            delete wizard;
        }

    }



void MainWindow::resizeEvent(QResizeEvent *event)
{
    auto * _pic_show_pro=dynamic_cast<picshow*>(_pic_show);
    _pic_show_pro->repic();
    return QWidget::resizeEvent(event);
}


MainWindow::~MainWindow()
{
    delete ui;
}
