#ifndef PICBUTTON_H
#define PICBUTTON_H

#include <QPushButton>
#include <QPixmap>
#include <QEvent>
class picbutton : public QPushButton
{
public:
    picbutton(QWidget *parent = nullptr);
    void creatpicbutton(QString normal,QString hover,QString pressed);

    void hoverbutton();
    void pressedbutton();
    void normalbutton();
private:
    QString _normal;
    QString _hover;
    QString _pressed;

protected:
    bool event(QEvent *e) override;
};

#endif // PICBUTTON_H
