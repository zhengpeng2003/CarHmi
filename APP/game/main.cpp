#include "maingame.h"
#include <loadprocess.h>
#include <QApplication>
#include <QLockFile>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, false);
    QApplication a(argc, argv);

    const QString lockDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(lockDir);
    QLockFile lockFile(lockDir + QStringLiteral("/carhmi_landfarmer.lock"));
    lockFile.setStaleLockTime(0);
    if (!lockFile.tryLock()) {
        QMessageBox::information(nullptr, QObject::tr("欢乐斗地主"), QObject::tr("应用正在启动或已经运行。"));
        return 0;
    }

    Loadprocess w;
    w.show();

    const int code = a.exec();
    lockFile.unlock();
    return code;
}
