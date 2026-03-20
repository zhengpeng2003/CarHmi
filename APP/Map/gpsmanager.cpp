#include "gpsmanager.h"
#include <QDebug>
#include <QDateTime>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

GPSManager::GPSManager(QObject *parent)
    : QObject(parent),
    useSerial(false),
    m_lat(39.9),
    m_lng(116.3)
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
    timer->start(200);

    // 检测切换
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout,
            this, &GPSManager::checkGpsSource);
    checkTimer->start(2000);

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
