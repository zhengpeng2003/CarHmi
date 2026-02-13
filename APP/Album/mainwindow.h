#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "protreewidget.h"
#include "wizard.h"
#include <QMainWindow>
#include <protree.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void open_project();
    void create_project();

signals:
    void signal_oepn(const QString& path);
protected:
    virtual void resizeEvent(QResizeEvent *event);
private:
    Ui::MainWindow *ui;
    QWidget * _protree;

    QWidget * _pic_show;
};
#endif // MAINWINDOW_H
