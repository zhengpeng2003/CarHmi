// appwidget.h
#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>

class AppWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppWidget(QWidget *parent = nullptr);

    void setAppInfo(quint8 id, const QString &name, const QString &exePath, const QString &iconPath);

signals:
    void clicked(quint8 id); // 点击信号，传递 APP ID

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    quint8 m_id;
    QString m_exePath;

    QLabel *iconLabel;
    QLabel *nameLabel;
};
