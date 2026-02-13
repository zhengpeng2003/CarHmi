#include "maingame.h"
#include <loadprocess.h>
#include <QApplication>
#include <gamecontrol.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Loadprocess w;

    w.show();



    return a.exec();
}
