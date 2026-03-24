#ifndef MYBUTTON_H
#define MYBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QEvent>
#include <QMouseEvent>
#include <QResizeEvent>
class Mybutton : public QPushButton
{
    Q_OBJECT
public:
    explicit Mybutton(QWidget *parent = nullptr);
    void InitMybutton(QString Norlmal,QString Hover,QString Click);
    void SetNormal();
    void SetHover();
    void SetClick();
protected:
    virtual bool event(QEvent *e) override;
    virtual void resizeEvent(QResizeEvent *event) override;
private:
    QPixmap _Pixmap;
    QString _Norlmal;
    QString _Hover;
    QString _Click;

};

#endif // MYBUTTON_H
