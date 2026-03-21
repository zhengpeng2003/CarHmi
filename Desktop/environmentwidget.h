#pragma once
#include <QWidget>
#include <QPixmap>

class EnvironmentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnvironmentWidget(QWidget *parent = nullptr);

    void setTemperature(float temp) { m_temperature = temp; update(); }
    void setHumidity(float hum) { m_humidity = hum; update(); }
    void setBackgroundImage(const QString &path) { m_bg.load(path); update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float m_temperature = 0;
    float m_humidity = 0;
    QPixmap m_bg;
};
