#include "mainwindow.h"

#include <QApplication>
#include <qfile.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFile styleFile(":/qss/style.qss"); // 使用资源路径
    //QFile是文件内容的操作者，QDir是文件路径的组织者

    if (styleFile.open(QIODevice::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet); // 为整个应用程序设置样式
        styleFile.close();
    }
    w.showMaximized();
    w.setWindowTitle("Album");
    return a.exec();


}
