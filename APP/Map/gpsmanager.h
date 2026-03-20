#ifndef GPSMANAGER_H
#define GPSMANAGER_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QTimer>

extern "C" {
#include <nmea/nmea.h>
}

class GPSManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude NOTIFY positionChanged)
    Q_PROPERTY(double longitude READ longitude NOTIFY positionChanged)

public:
    explicit GPSManager(QObject *parent = nullptr);
    ~GPSManager();

    double latitude() const { return m_lat; }
    double longitude() const { return m_lng; }

signals:
    void positionChanged(double lat, double lng);

private slots:
    void readSerialData();
    void updateSystemGPS(const QGeoPositionInfo &info);
    void checkGpsSource();

private:
    int fd;

    nmeaPARSER parser;
    nmeaINFO info;

    QTimer *timer;
    QTimer *checkTimer;
    QGeoPositionInfoSource *sysGps;

    bool useSerial;
    qint64 lastSerialTime;

    double m_lat;
    double m_lng;
};

#endif
