#ifndef PICANIMATION_H
#define PICANIMATION_H

#include <QTimer>
#include <QTreeWidget>
#include <QWidget>

class picanimation : public QWidget
{
    Q_OBJECT
public:
    explicit picanimation(QWidget *parent = nullptr);
    ~picanimation();
    void setanimation(QTreeWidgetItem *first_chid);
    void timestart();
    void timeout();
    void stop();
private:
    QPixmap _pixmap1;
    QPixmap _pixmap2;
    QTreeWidgetItem * _current_item;
    double _float;//透明度
    bool _bst_stop;
    QTimer *_timer;
    QMap<QString,QTreeWidgetItem*> _map_info;
protected:
    virtual void paintEvent(QPaintEvent *event);
signals:
    void signalupdatelistitem(QTreeWidgetItem * item);
    void sigselectupdate(QTreeWidgetItem * item);
    void sigplaybtn();
    void sigstopbtn();
    void musicstart();
    void musicstop();
public slots:
    void slotpreshow();
    void slotnextshow();
    void slotupdatebtn();
    void slotselectupdatepic(QString path);


};

#endif // PICANIMATION_H
