#include "gpsmanager.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

GPSManager::GPSManager(QObject *parent)
    : QObject(parent),
    useSerial(false),
    m_lat(39.9),
    m_lng(116.3),
    networkManager(new QNetworkAccessManager(this))
{
    // 打开串口
    fd = open("/dev/ttymxc2", O_RDONLY | O_NOCTTY);

    if (fd >= 0) {
        struct termios opt;
        tcflush(fd, TCIOFLUSH);
        tcgetattr(fd, &opt);

        cfsetispeed(&opt, B9600);
        cfsetospeed(&opt, B9600);

        opt.c_cflag &= ~CSIZE;
        opt.c_cflag |= CS8;
        opt.c_cflag &= ~PARENB;
        opt.c_cflag &= ~CSTOPB;

        tcsetattr(fd, TCSANOW, &opt);

        qDebug() << "serial open ok";
    } else {
        qDebug() << "serial open failed";
    }

    // 初始化nmea
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    // 定时读取
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout,
            this, &GPSManager::readSerialData);
    timer->start(1000);

    // 检测切换
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout,
            this, &GPSManager::checkGpsSource);
    checkTimer->start(1000);

    // 系统GPS
    sysGps = QGeoPositionInfoSource::createDefaultSource(this);
    if (sysGps) {
        connect(sysGps, &QGeoPositionInfoSource::positionUpdated,
                this, &GPSManager::updateSystemGPS);
        sysGps->startUpdates();
    }

    lastSerialTime = 0;
}

GPSManager::~GPSManager()
{
    nmea_parser_destroy(&parser);
    if (fd > 0) close(fd);
}

void GPSManager::readSerialData()
{
    if (fd < 0) return;

    char buff[4096] = {0};
    char tmp[100] = {0};
    int len = 0;

    for (int i = 0; i < 8; i++) {
        int size = read(fd, tmp, 100);
        if (size > 0) {
            memcpy(buff + len, tmp, size);
            len += size;
        }
    }

    if (len <= 0) return;

    lastSerialTime = QDateTime::currentMSecsSinceEpoch();

    nmea_parse(&parser, buff, len, &info);

    if (info.sig > 0) {
        double lat = nmea_ndeg2degree(info.lat);
        double lng = nmea_ndeg2degree(info.lon);

        if (qAbs(lat - m_lat) > 0.00001 ||
            qAbs(lng - m_lng) > 0.00001)
        {
            m_lat = lat;
            m_lng = lng;
            useSerial = true;

            emit positionChanged(m_lat, m_lng);
        }
    }
}

void GPSManager::updateSystemGPS(const QGeoPositionInfo &pos)
{
    if (useSerial) return;

    m_lat = pos.coordinate().latitude();
    m_lng = pos.coordinate().longitude();

    emit positionChanged(m_lat, m_lng);
}

void GPSManager::checkGpsSource()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    if (useSerial && (now - lastSerialTime > 3000)) {
        useSerial = false;
        qDebug() << "switch to system gps";
    }
}

void GPSManager::searchPlace(const QString &keyword)
{
    const QString trimmedKeyword = keyword.trimmed();
    if (trimmedKeyword.isEmpty()) {
        m_searchMessage = QStringLiteral("请输入城市拼音");
        emit searchMessageChanged();
        emit searchFailed(trimmedKeyword);
        return;
    }

    const QString apiKey = qEnvironmentVariable("AMAP_WEB_API_KEY");
    if (apiKey.isEmpty()) {
        m_searchMessage = QStringLiteral("当前搜索不到");
        emit searchMessageChanged();
        emit searchFailed(trimmedKeyword);
        return;
    }

    QUrl url(QStringLiteral("https://restapi.amap.com/v3/geocode/geo"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("address"), trimmedKeyword);
    query.addQueryItem(QStringLiteral("key"), apiKey);
    url.setQuery(query);

    m_searchMessage = QStringLiteral("搜索中...");
    emit searchMessageChanged();

    QNetworkReply *reply = networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmedKeyword]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            m_searchMessage = QStringLiteral("当前搜索不到");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject object = document.object();
        if (object.value(QStringLiteral("status")).toString() != QStringLiteral("1")) {
            m_searchMessage = QStringLiteral("当前搜索不到");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        const QJsonArray geocodes = object.value(QStringLiteral("geocodes")).toArray();

        if (geocodes.isEmpty()) {
            m_searchMessage = QStringLiteral("当前搜索不到");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        const QString location = geocodes.first().toObject().value(QStringLiteral("location")).toString();
        const QStringList parts = location.split(",");
        if (parts.size() != 2) {
            m_searchMessage = QStringLiteral("当前搜索不到");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        bool lngOk = false;
        bool latOk = false;
        const double lng = parts.at(0).toDouble(&lngOk);
        const double lat = parts.at(1).toDouble(&latOk);
        if (!latOk || !lngOk) {
            m_searchMessage = QStringLiteral("当前搜索不到");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        m_searchMessage = QStringLiteral("已定位到：") + trimmedKeyword;
        emit searchMessageChanged();
        emit searchLocationFound(lat, lng, trimmedKeyword);
    });
}
