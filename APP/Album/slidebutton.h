#ifndef SLIDEBUTTON_H
#define SLIDEBUTTON_H

#include <QPushButton>

class slidebutton : public QPushButton
{
    Q_OBJECT
public:
    slidebutton(QWidget *parent = nullptr);
    void seticons(const QString normal,const QString hover,const QString pressed,
                const QString normal2,const QString hover2,const QString pressed2);
    void setnormal();
    void sethover();
    void setpressed();

    void setnormal2();
    void sethover2();
    void setpressed2();

protected:
    virtual bool event(QEvent *event) override;
private:
    QString _normal;
    QString _hover;
    QString _pressed;
    QString _normal2;
    QString _hover2;
    QString _pressed2;
    int _state;
public slots:
    void slplaybtn();
    void slstopbtn();
};

#endif // SLIDEBUTTON_H
