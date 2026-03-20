#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    w.show();
    return a.exec();
}
