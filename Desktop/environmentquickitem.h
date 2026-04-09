#pragma once

#include <QPixmap>
#include <QPropertyAnimation>
#include <QQuickPaintedItem>

class EnvironmentQuickItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(double humidity READ humidity WRITE setHumidity NOTIFY humidityChanged)
    Q_PROPERTY(QString backgroundImage READ backgroundImage WRITE setBackgroundImage NOTIFY backgroundImageChanged)
    Q_PROPERTY(int carOffsetX READ carOffsetX WRITE setCarOffsetX NOTIFY carOffsetXChanged)

public:
    explicit EnvironmentQuickItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    double temperature() const;
    double humidity() const;
    QString backgroundImage() const;
    int carOffsetX() const;

    Q_INVOKABLE void startCarDriveAnimation();
    Q_INVOKABLE bool hitCar(qreal x, qreal y) const;

public slots:
    void setTemperature(double temperature);
    void setHumidity(double humidity);
    void setBackgroundImage(const QString &backgroundImage);
    void setCarOffsetX(int carOffsetX);

signals:
    void temperatureChanged();
    void humidityChanged();
    void backgroundImageChanged();
    void carOffsetXChanged();

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void rebuildCache();
    QPointF baseCarTopLeft() const;
    QRectF carRect() const;

private:
    double m_temperature = 0.0;
    double m_humidity = 0.0;
    QString m_backgroundImage;
    QPixmap m_backgroundPixmap;
    QPixmap m_cachedCarPixmap;
    int m_carOffsetX = 0;
    QPropertyAnimation *m_carDriveAnimation = nullptr;
};
