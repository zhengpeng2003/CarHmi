#ifndef PICSHOW_H
#define PICSHOW_H

#include <QDialog>
#include "QPropertyAnimation"
#include <QEvent>
namespace Ui {
class picshow;
}

class picshow : public QDialog
{
    Q_OBJECT

public:
    explicit picshow(QWidget *parent = nullptr);
    ~picshow();
    void showprenextbtn(bool b_show);
    void repic();
    QString _select_pic;
private:
    QPropertyAnimation * pre_btn_animation;
    QPropertyAnimation * next_btn_animation;
    QPixmap _pic;

    Ui::picshow *ui;
signals:
    void sigprebtn();
    void signextbtn();
protected:
    virtual bool event(QEvent *event) override;
public    slots:
    void slotpicshow(QString path);
    void slotprepicshow(const QString path);
    void slotnextpicshow(const QString path);
    void slotclosepic();
};

#endif // PICSHOW_H
