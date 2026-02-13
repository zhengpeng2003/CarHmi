#include "prosetpage.h"
#include "ui_prosetpage.h"

#include <QDir>
#include <qfiledialog.h>

Prosetpage::Prosetpage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::Prosetpage)
{
    ui->setupUi(this);
    //设置一个向导
    registerField("projectName",ui->lineEdit_name);//这目的是与向导框连接 wizard 将输入框注册为向导页面的必填字段

    //有*代表必须要填写
    registerField("projectPath*",ui->lineEdit_path);



    //绑定信号与槽
    connect(ui->lineEdit_name,&QLineEdit::textChanged,this,&Prosetpage::completeChanged);//每当页面值发生变化都会触发这个信号
    connect(ui->lineEdit_path,&QLineEdit::textChanged,this,&Prosetpage::completeChanged);

    //将当前文件路径保存
    QString currentpath=QDir::currentPath();//要包含头文件
    ui->lineEdit_path->setText(currentpath);

    //将鼠标的位置移动到当前地址
    ui->lineEdit_path->setCursorPosition(currentpath.size());

    //设置删除按钮
    ui->lineEdit_path->setClearButtonEnabled(true);
    ui->lineEdit_name->setClearButtonEnabled(true);




}
/*graph LR
    A[用户修改文本框] --> B[textChanged信号]
    B --> C[completeChanged信号]
    C --> D[向导调用isComplete]
    D --> E{返回true/false?}
    E -->|true| F[启用Next按钮]
    E -->|false| G[禁用Next按钮]
*/
Prosetpage::~Prosetpage()
{
    delete ui;
}

bool Prosetpage::isComplete() const//重写事件
{


    QDir path_dir(ui->lineEdit_path->text());

    //QDir dir("C:/MyFolder");
    //QString absPath = dir.absoluteFilePath("data.txt");
    // absPath = "C:/MyFolder/data.txt"
    //这个abs相当于是拼接
    QString abspath=path_dir.absoluteFilePath(ui->lineEdit_name->text());
    QDir abs(abspath);
    if(ui->lineEdit_path->text().isEmpty()||ui->lineEdit_name->text().isEmpty())
    {
        return false;
    }
    if(!path_dir.exists())
    {

        ui->label_tips->setText("project path is not exitsts");
        return false;
    }
    if(abs.exists())
    {
        ui->label_tips->setText("project has exists, change path or name!");
        return false;
    }


    return true;
}


void Prosetpage::on_pushButton_browse_clicked()
{
    QFileDialog filedialog;
    //设置文件主题
    filedialog.setWindowTitle("打开文件");
    //设置接收的文件为目录还是为文件
    filedialog.setFileMode(QFileDialog::Directory);
    //设置文件的当前位置
    filedialog.setDirectory(ui->lineEdit_path->text());
    //设置视图的文件模式
    filedialog.setViewMode(QFileDialog::Detail);

    QStringList filename;
    if(filedialog.exec())
    {
    filename=filedialog.selectedFiles();
    }
    if(filename.size()<=0)
    {
    return;
    }
    ui->lineEdit_path->setText(filename.at(0));
}

void Prosetpage::get_path_name(QString& name,QString& path)
{
    name=ui->lineEdit_name->text();
    path=ui->lineEdit_path->text();
}

