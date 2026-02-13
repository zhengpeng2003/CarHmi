#ifndef OPENTREETHREAD_H
#define OPENTREETHREAD_H


#include "protreeitem.h"
#include <QThread>
#include<protreewidget.h>
class opentreethread : public QThread
{
    Q_OBJECT
public://哪个widget self 其实就是root
    opentreethread(const QString scr_path,const QString name,int file_cout,protreewidget *self,QObject *parent = nullptr);
    void openprotree(const QString scr_path,const QString name,int file_cout,protreewidget *self);
    void re_opentree(const QString scr_path,const QString name,int file_cout,protreewidget *self
                     ,protreeitem * root,protreeitem *parent,protreeitem * previous);
protected:
    void run();
private:
    QString _scr_path;
    QString _name;
    int _file_cout;
    protreewidget *_self;
    bool _bststop;
signals:
    void progressupdate(int);
    void progressfinish(int);
    void progresscancle();
public slots:
    void slotprogresscancle();
};

#endif // OPENTREETHREAD_H
