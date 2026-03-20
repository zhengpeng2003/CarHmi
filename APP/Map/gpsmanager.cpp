#include "gpsmanager.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>
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
    fd = open("/dev/ttymxc2", O_RDONLY | O_NOCTTY| O_NONBLOCK);

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


        opt.c_cc[VMIN] = 0;   // 不等待最少字符
        opt.c_cc[VTIME] = 0;  // 不等待超时

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

    // ========== 添加：先检查是否有数据可读，避免阻塞 ==========
    fd_set readSet;
    struct timeval timeout;

    FD_ZERO(&readSet);
    FD_SET(fd, &readSet);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;  // 100ms超时

    if (select(fd + 1, &readSet, NULL, NULL, &timeout) <= 0) {
        return;  // 没有数据可读，直接返回
    }
    // ========== 添加结束 ==========

    char buff[4096] = {0};
    char tmp[100] = {0};
    int len = 0;

    for (int i = 0; i < 8; i++) {
        int size = read(fd, tmp, 100);
        if (size > 0) {
            memcpy(buff + len, tmp, size);
            len += size;
        } else {
            break;  // 没有更多数据，退出循环
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
    qDebug() << "=== searchPlace called ===";
    qDebug() << "Keyword:" << trimmedKeyword;

    if (trimmedKeyword.isEmpty()) {
        m_searchMessage = QStringLiteral("请输入城市拼音");
        emit searchMessageChanged();
        emit searchFailed(trimmedKeyword);
        return;
    }

    // 直接硬编码API密钥
    const QString apiKey = "d3bdf063df89186ca9440197cd54c39f";
    qDebug() << "API Key:" << apiKey;

    // 可选：检查密钥格式是否正确
    if (apiKey.isEmpty() || apiKey.length() < 10) {
        m_searchMessage = QStringLiteral("API密钥无效");
        emit searchMessageChanged();
        emit searchFailed(trimmedKeyword);
        return;
    }

    QUrl url(QStringLiteral("https://restapi.amap.com/v3/geocode/geo"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("address"), trimmedKeyword);
    query.addQueryItem(QStringLiteral("key"), apiKey);  // 参数名是"key"

    url.setQuery(query);

    qDebug() << "Request URL:" << url.toString();

    m_searchMessage = QStringLiteral("搜索中...");
    emit searchMessageChanged();

    QNetworkReply *reply = networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmedKeyword]() {
        qDebug() << "=== Request finished ===";
        qDebug() << "Error code:" << reply->error();

        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            m_searchMessage = QStringLiteral("网络错误，请检查网络连接");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        QByteArray responseData = reply->readAll();
        qDebug() << "Response data:" << responseData;

        const QJsonDocument document = QJsonDocument::fromJson(responseData);
        if (document.isNull()) {
            qDebug() << "Invalid JSON response";
            m_searchMessage = QStringLiteral("响应数据格式错误");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        const QJsonObject object = document.object();
        QString status = object.value(QStringLiteral("status")).toString();
        QString info = object.value(QStringLiteral("info")).toString();

        qDebug() << "Status:" << status;
        qDebug() << "Info:" << info;

        if (status != QStringLiteral("1")) {
            QString errorMsg;
            if (info == "INVALID_USER_KEY") {
                errorMsg = "API密钥无效";
            } else if (info == "DAILY_QUERY_OVER_LIMIT") {
                errorMsg = "今日查询次数已达上限";
            } else {
                errorMsg = QStringLiteral("搜索失败: ") + info;
            }
            m_searchMessage = errorMsg;
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        const QJsonArray geocodes = object.value(QStringLiteral("geocodes")).toArray();

        if (geocodes.isEmpty()) {
            m_searchMessage = QStringLiteral("未找到该地点");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        const QString location = geocodes.first().toObject().value(QStringLiteral("location")).toString();
        const QStringList parts = location.split(",");
        if (parts.size() != 2) {
            m_searchMessage = QStringLiteral("坐标数据格式错误");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        bool lngOk = false;
        bool latOk = false;
        const double lng = parts.at(0).toDouble(&lngOk);
        const double lat = parts.at(1).toDouble(&latOk);
        if (!latOk || !lngOk) {
            m_searchMessage = QStringLiteral("坐标转换失败");
            emit searchMessageChanged();
            emit searchFailed(trimmedKeyword);
            return;
        }

        m_searchMessage = QStringLiteral("已定位到：") + trimmedKeyword;
        emit searchMessageChanged();
        emit searchLocationFound(lat, lng, trimmedKeyword);

        qDebug() << "Search success! Location:" << trimmedKeyword
                 << "Lat:" << lat << "Lng:" << lng;
    });
}
