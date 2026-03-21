#ifndef LOADPROCESS_H
#define LOADPROCESS_H

#include <QPointer>
#include <QWidget>

class Maingame;

class Loadprocess : public QWidget
{
    Q_OBJECT
public:
    explicit Loadprocess(QWidget *parent = nullptr);
protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void startBootstrap();
    void updateProgress(int current, int total, const QString &label);

    QPixmap _MainPix;
    QPixmap _Process;
    QPointer<Maingame> _Maingame;
    int _ProgressValue = 0;
    int _ProgressMax = 100;
    bool _Started = false;
    QString _StageLabel;
};

#endif // LOADPROCESS_H
