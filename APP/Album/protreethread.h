#ifndef PROTREETHREAD_H
#define PROTREETHREAD_H

#include <QThread>
#include <qtreewidget.h>

class protreethread : public QThread
{
    Q_OBJECT
public:

    protreethread(const QString &scr_path,const QString& dis_path,
                  QTreeWidgetItem* parent_item,int file_cout,QTreeWidget *self,QTreeWidgetItem *root);

    ~protreethread();



protected:
    void creatprotreethread(const QString & scr_path,const QString & dis_path,QTreeWidgetItem* paremt_item,int  file_cout,
                        QTreeWidget * self,QTreeWidgetItem *root, QTreeWidgetItem* preItem = nullptr);
    virtual void run();
signals:
    void signalupdateprogress(int);//更新
    void signalfishprogress(int);//完成信号
    void signalcancle(int);

public slots:
    void slotcancle();
private:
    QString  _scr_path;
    QString  _dis_path;
    QTreeWidgetItem* _parent_item;
    int _file_cout;
    QTreeWidget * _self;
    QTreeWidgetItem *_root;

    bool _bststop;//设置停止的


};

#endif // PROTREETHREAD_H
