#include "gpsmanager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QGeoServiceProvider>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkConfigurationManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QSslSocket>

#include <QUrl>
#include <QUrlQuery>
#include <QtMath>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
GPSManager::GPSManager(QObject *parent)
    : QObject(parent),
      networkManager(new QNetworkAccessManager(this))
{
    loadConfig();
    detectEnvironment();

    fd = open(m_config.serialPort.toLocal8Bit().constData(), O_RDONLY | O_NOCTTY | O_NONBLOCK);
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
        opt.c_cc[VMIN] = 0;
        opt.c_cc[VTIME] = 0;
        tcsetattr(fd, TCSANOW, &opt);
        qDebug() << "serial open ok:" << m_config.serialPort;
    } else {
        qDebug() << "serial open failed:" << m_config.serialPort;
    }

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GPSManager::readSerialData);
    timer->start(1000);

    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &GPSManager::checkGpsSource);
    checkTimer->start(2000);

    sysGps = QGeoPositionInfoSource::createDefaultSource(this);
    if (sysGps) {
        connect(sysGps, &QGeoPositionInfoSource::positionUpdated, this, &GPSManager::updateSystemGPS);
        sysGps->startUpdates();
    }
}

GPSManager::~GPSManager()
{
    nmea_parser_destroy(&parser);
    if (fd >= 0) {
        close(fd);
    }
}

void GPSManager::loadConfig()
{
    const QString configPath = QCoreApplication::applicationDirPath() + QStringLiteral("/map.ini");
    QSettings settings(configPath, QSettings::IniFormat);
    m_config.serialPort = settings.value(QStringLiteral("map/serial_port"), m_config.serialPort).toString();
    m_config.pluginName = settings.value(QStringLiteral("map/plugin"), m_config.pluginName).toString();
    m_config.apiKey = settings.value(QStringLiteral("map/api_key")).toString();
    m_config.searchTimeoutMs = settings.value(QStringLiteral("network/search_timeout_ms"), m_config.searchTimeoutMs).toInt();
    m_config.retryCount = settings.value(QStringLiteral("network/retry_count"), m_config.retryCount).toInt();
}

void GPSManager::detectEnvironment()
{
    m_pluginAvailable = QGeoServiceProvider::availableServiceProviders().contains(m_config.pluginName);
    QNetworkConfigurationManager networkManager;
    m_networkAvailable = networkManager.isOnline();
    // 移除 SSL 检测，直接设为可用
    m_sslAvailable = true;  // 强制设为 true，不使用 SSL 检测

    QStringList issues;
    if (!m_pluginAvailable) {
        qDebug() << QStringLiteral("地图插件不可用: %1").arg(m_config.pluginName);
        issues << QStringLiteral("地图插件不可用: %1").arg(m_config.pluginName);
    }
    if (!m_networkAvailable) {
        qDebug() << QStringLiteral("网络未连接");
        issues << QStringLiteral("网络未连接");
    }
    if (m_config.apiKey.trimmed().isEmpty()) {
        qDebug() << QStringLiteral("缺少 API Key 配置");
        issues << QStringLiteral("缺少 API Key 配置");
    }

    m_environmentMessage = issues.isEmpty()
                               ? QStringLiteral("环境检测通过")
                               : issues.join(QStringLiteral("；"));
    emit environmentMessageChanged();
}
bool GPSManager::environmentReadyForSearch(QString *errorMessage) const
{
    if (!m_networkAvailable) {
        if (errorMessage) *errorMessage = QStringLiteral("当前无网络，无法执行地点搜索");
        return false;
    }
    if (!m_sslAvailable) {
        if (errorMessage) *errorMessage = QStringLiteral("SSL 环境不可用，无法访问 HTTPS 搜索接口");
        return false;
    }
    if (m_config.apiKey.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("请在 map.ini 中配置 API Key");
        return false;
    }
    return true;
}

void GPSManager::setMapState(MapState state)
{
    if (m_mapState == state) {
        return;
    }
    m_mapState = state;
    emit mapStateChanged();
}

void GPSManager::handlePositionUpdate(double lat, double lng, const QString &source)
{
    const bool changed = qAbs(lat - m_lat) > 0.00001 || qAbs(lng - m_lng) > 0.00001;
    m_lat = lat;
    m_lng = lng;
    if (changed) {
        emit positionChanged(m_lat, m_lng);
    }
    if (m_mapState == GpsFollowState) {
        emit centerRequested(m_lat, m_lng, static_cast<int>(m_mapState), source);
    }
}

void GPSManager::readSerialData()
{
    if (fd < 0) return;
    fd_set readSet;
    struct timeval timeout;
    FD_ZERO(&readSet);
    FD_SET(fd, &readSet);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    if (select(fd + 1, &readSet, nullptr, nullptr, &timeout) <= 0) {
        return;
    }

    char buff[4096] = {0};
    char tmp[100] = {0};
    int len = 0;
    for (int i = 0; i < 8; ++i) {
        const int size = read(fd, tmp, 100);
        if (size > 0) {
            memcpy(buff + len, tmp, size);
            len += size;
        } else {
            break;
        }
    }
    if (len <= 0) return;

    lastSerialTime = QDateTime::currentMSecsSinceEpoch();
    nmea_parse(&parser, buff, len, &info);
    if (info.sig > 0) {
        useSerial = true;
        handlePositionUpdate(nmea_ndeg2degree(info.lat), nmea_ndeg2degree(info.lon), QStringLiteral("serial-gps"));
    }
}

void GPSManager::updateSystemGPS(const QGeoPositionInfo &pos)
{
    if (useSerial) return;
    handlePositionUpdate(pos.coordinate().latitude(), pos.coordinate().longitude(), QStringLiteral("system-gps"));
}

void GPSManager::checkGpsSource()
{
    QNetworkConfigurationManager networkManager;
    const bool online = networkManager.isOnline();
    if (m_networkAvailable != online) {
        m_networkAvailable = online;
        detectEnvironment();
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (useSerial && (now - lastSerialTime > 3000)) {
        useSerial = false;
        qDebug() << "switch to system gps";
    }
}

void GPSManager::pushSearchHistory(const QString &keyword)
{
    if (keyword.isEmpty()) {
        return;
    }
    m_searchHistory.removeAll(keyword);
    m_searchHistory.prepend(keyword);
    while (m_searchHistory.size() > 10) {
        m_searchHistory.removeLast();
    }
    emit searchHistoryChanged();
}

void GPSManager::finishSearchFailure(const QString &keyword, const QString &message)
{
    m_searchMessage = message;
    m_searchResults.clear();
    emit searchResultsChanged();
    emit searchMessageChanged();
    emit searchFailed(keyword);
}

void GPSManager::searchPlace(const QString &keyword)
{
    const QString trimmedKeyword = keyword.trimmed();
    m_lastSearchKeyword = trimmedKeyword;
    if (trimmedKeyword.isEmpty()) {
        finishSearchFailure(trimmedKeyword, QStringLiteral("请输入地点关键词"));
        return;
    }

    QString envError;
    if (!environmentReadyForSearch(&envError)) {
        finishSearchFailure(trimmedKeyword, envError);
        return;
    }

    m_searchMessage = QStringLiteral("搜索中...");
    emit searchMessageChanged();

    QUrl url(QStringLiteral("https://restapi.amap.com/v3/geocode/geo"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("address"), trimmedKeyword);
    query.addQueryItem(QStringLiteral("key"), m_config.apiKey);
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);
    QTimer::singleShot(m_config.searchTimeoutMs, reply, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmedKeyword]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("搜索失败，可检查网络后重试"));
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isNull()) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("响应数据格式错误"));
            return;
        }

        const QJsonObject object = document.object();
        if (object.value(QStringLiteral("status")).toString() != QStringLiteral("1")) {
            finishSearchFailure(trimmedKeyword,
                                QStringLiteral("搜索失败: %1").arg(object.value(QStringLiteral("info")).toString()));
            return;
        }

        const QJsonArray geocodes = object.value(QStringLiteral("geocodes")).toArray();
        if (geocodes.isEmpty()) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("无匹配结果，请调整关键词后重试"));
            return;
        }

        QVariantList results;
        for (const QJsonValue &value : geocodes) {
            const QJsonObject item = value.toObject();
            const QString location = item.value(QStringLiteral("location")).toString();
            const QStringList parts = location.split(',');
            if (parts.size() != 2) {
                continue;
            }
            bool lngOk = false;
            bool latOk = false;
            const double lng = parts.at(0).toDouble(&lngOk);
            const double lat = parts.at(1).toDouble(&latOk);
            if (!latOk || !lngOk) {
                continue;
            }
            QVariantMap result;
            result.insert(QStringLiteral("title"), item.value(QStringLiteral("formatted_address")).toString());
            result.insert(QStringLiteral("district"), item.value(QStringLiteral("district")).toString());
            result.insert(QStringLiteral("city"), item.value(QStringLiteral("city")).toVariant().toString());
            result.insert(QStringLiteral("lat"), lat);
            result.insert(QStringLiteral("lng"), lng);
            results.append(result);
        }

        if (results.isEmpty()) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("结果解析失败，请重试"));
            return;
        }

        m_searchResults = results;
        m_searchMessage = QStringLiteral("找到 %1 个候选结果，请确认").arg(results.size());
        pushSearchHistory(trimmedKeyword);
        emit searchResultsChanged();
        emit searchMessageChanged();
    });
}

void GPSManager::retryLastSearch()
{
    if (!m_lastSearchKeyword.isEmpty()) {
        searchPlace(m_lastSearchKeyword);
    }
}

void GPSManager::confirmSearchResult(int index)
{
    if (index < 0 || index >= m_searchResults.size()) {
        return;
    }
    const QVariantMap result = m_searchResults.at(index).toMap();
    const double lat = result.value(QStringLiteral("lat")).toDouble();
    const double lng = result.value(QStringLiteral("lng")).toDouble();
    const QString title = result.value(QStringLiteral("title")).toString();
    setMapState(SearchLocatedState);
    m_searchMessage = QStringLiteral("已定位到：%1").arg(title);
    emit searchMessageChanged();
    emit searchLocationFound(lat, lng, title);
    emit centerRequested(lat, lng, static_cast<int>(m_mapState), QStringLiteral("search-selection"));
}

void GPSManager::clearSearchHistory()
{
    m_searchHistory.clear();
    emit searchHistoryChanged();
}

void GPSManager::enterManualBrowse()
{
    setMapState(ManualBrowseState);
}

void GPSManager::resumeGpsFollow()
{
    setMapState(GpsFollowState);
    emit centerRequested(m_lat, m_lng, static_cast<int>(m_mapState), QStringLiteral("resume-follow"));
}
