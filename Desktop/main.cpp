#include "widget.h"

#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.show();

    const QString screenshotPath = qEnvironmentVariable("DESKTOP_AUTO_SHOT");
    if (!screenshotPath.isEmpty()) {
        QTimer::singleShot(500, [&w, screenshotPath]() {
            w.grab().save(screenshotPath);
            qApp->quit();
        });
    }

    return a.exec();
}
