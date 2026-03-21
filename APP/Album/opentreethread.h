#ifndef OPENTREETHREAD_H
#define OPENTREETHREAD_H

#include <QThread>

class protreewidget;
class opentreethread : public QThread
{
    Q_OBJECT
public:
    opentreethread(const QString scr_path, const QString name, int file_cout, protreewidget *self, QObject *parent = nullptr);
    void openprotree(const QString scr_path, const QString name, int file_cout, protreewidget *self);
    void re_opentree(const QString scr_path, int file_cout, const QString &rootPath, const QString &parentPath);
protected:
    void run() override;
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
    void rootItemReady(QString name, QString path);
    void directoryDiscovered(QString parentPath, QString name, QString fullPath);
    void fileDiscovered(QString parentPath, QString name, QString fullPath);
public slots:
    void slotprogresscancle();
};

#endif // OPENTREETHREAD_H
