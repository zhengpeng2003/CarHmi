#ifndef GPSMANAGER_H
#define GPSMANAGER_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QNetworkAccessManager>
#include <QTimer>

extern "C" {
#include <nmea/nmea.h>
}

class GPSManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude NOTIFY positionChanged)
    Q_PROPERTY(double longitude READ longitude NOTIFY positionChanged)
    Q_PROPERTY(QString searchMessage READ searchMessage NOTIFY searchMessageChanged)

public:
    explicit GPSManager(QObject *parent = nullptr);
    ~GPSManager();

    double latitude() const { return m_lat; }
    double longitude() const { return m_lng; }
    QString searchMessage() const { return m_searchMessage; }

    Q_INVOKABLE void searchPlace(const QString &keyword);

signals:
    void positionChanged(double lat, double lng);
    void searchLocationFound(double lat, double lng, QString keyword);
    void searchFailed(QString keyword);
    void searchMessageChanged();

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
    QNetworkAccessManager *networkManager;

    bool useSerial;
    qint64 lastSerialTime;

    double m_lat;
    double m_lng;
    QString m_searchMessage;
};

#endif
