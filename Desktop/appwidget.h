#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class AppWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppWidget(QWidget *parent = nullptr);

    void setAppInfo(quint8 id, const QString &name, const QString &exePath, const QString &iconPath);
    quint8 appId() const { return m_id; }

private:
    quint8 m_id = 0;
    QString m_exePath;

    QLabel *iconLabel;
    QLabel *nameLabel;
};
