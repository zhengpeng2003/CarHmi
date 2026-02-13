#ifndef MYBUTTONGROUP_H
#define MYBUTTONGROUP_H

#include "qpushbutton.h"
#include <QWidget>

namespace Ui {
class MybuttonGroup;

}

class MybuttonGroup : public QWidget
{
    Q_OBJECT

public:
    enum State
    {Start,PlayCardfirst,Getloard,Null,PlayCard};//当前在哪个窗口
    explicit MybuttonGroup(QWidget *parent = nullptr);
    void Initbutton();
    void Setbtngroupstate(MybuttonGroup::State state);
    void SetStartButtonVisible(bool visible);
    ~MybuttonGroup();
signals:
    void S_Point(int point);
    void S_PlayHand();
    void S_NoHand();
    void S_GetLord();
    void S_NoLord();
    void S_Start();

private:
    QVector<QPushButton*> _Pushbuttons;
    Ui::MybuttonGroup *ui;
};

#endif // MYBUTTONGROUP_H
