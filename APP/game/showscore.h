#ifndef SHOWSCORE_H
#define SHOWSCORE_H

#include <QWidget>
#include <QLabel>
namespace Ui {
class ShowScore;
}

class ShowScore : public QWidget
{
    Q_OBJECT

public:
    explicit ShowScore(QWidget *parent = nullptr);
    void InitScore(int a,int b,int c);
    void SetPhaseText(const QString &text);
    void SetMultiplier(int value);
    void SetLordText(const QString &text);
    ~ShowScore();

private:
    QList<QLabel*> myLable;
    Ui::ShowScore *ui;
};

#endif // SHOWSCORE_H
