#include "widget.h"
#include "ui_widget.h"
#include <QQmlContext>
#include <QSslSocket>
#include <QDebug>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    resize(480, 272);

    gps = new GPSManager(this);

    quickWidget = new QQuickWidget(this);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    // 传给QML
    quickWidget->rootContext()->setContextProperty("gpsManager", gps);
    quickWidget->rootContext()->setContextProperty("appQuitter", this);
    quickWidget->setSource(QUrl("qrc:/main.qml"));
    quickWidget->setGeometry(0, 0, width(), height());


}
void Widget::quitApp()
{
    QApplication::quit();
}
Widget::~Widget()
{
    delete ui;
}
