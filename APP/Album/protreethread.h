#ifndef PROTREETHREAD_H
#define PROTREETHREAD_H

#include <QThread>
#include <QTreeWidget>

class protreethread : public QThread
{
    Q_OBJECT
public:
    protreethread(const QString &scr_path, const QString &dis_path,
                  QTreeWidgetItem* parent_item, int file_cout, QTreeWidget *self, QTreeWidgetItem *root);
    ~protreethread() override;

protected:
    void creatprotreethread(const QString & scr_path, const QString & dis_path, const QString & parentPath, int file_cout);
    void run() override;

signals:
    void signalupdateprogress(int);
    void signalfishprogress(int);
    void signalcancle(int);
    void signalDirectoryDiscovered(QString parentPath, QString name, QString fullPath);
    void signalFileDiscovered(QString parentPath, QString name, QString fullPath);
    void signalCancelledCleanup(QString rootPath);

public slots:
    void slotcancle();

private:
    QString  _scr_path;
    QString  _dis_path;
    QTreeWidgetItem* _parent_item;
    int _file_cout;
    QTreeWidget * _self;
    QTreeWidgetItem *_root;
    bool _bststop;
};

#endif // PROTREETHREAD_H
