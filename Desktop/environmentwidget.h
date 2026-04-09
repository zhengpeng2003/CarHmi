#pragma once
#include <QWidget>
#include <QPixmap>
#include <QPropertyAnimation>

class EnvironmentWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int carOffsetX READ carOffsetX WRITE setCarOffsetX)
public:
    explicit EnvironmentWidget(QWidget *parent = nullptr);

    void setTemperature(float temp) { m_temperature = temp; update(); }
    void setHumidity(float hum) { m_humidity = hum; update(); }
    void setBackgroundImage(const QString &path);
    int carOffsetX() const { return m_carOffsetX; }
    void setCarOffsetX(int offset);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void rebuildCache();
    QPoint baseCarTopLeft() const;
    QRect carRect() const;
    void startCarDriveAnimation();

    float m_temperature = 0;
    float m_humidity = 0;
    QPixmap m_bg;
    QPixmap m_cachedCarPixmap;
    int m_carOffsetX = 0;
    QPropertyAnimation *m_carDriveAnimation = nullptr;
};
