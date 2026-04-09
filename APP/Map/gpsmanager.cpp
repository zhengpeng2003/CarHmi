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
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

namespace {

constexpr double kEarthA = 6378245.0;
constexpr double kEarthEe = 0.00669342162296594323;
constexpr double kPi = 3.14159265358979323846;

bool outOfChina(double lat, double lng)
{
    return lng < 72.004 || lng > 137.8347 || lat < 0.8293 || lat > 55.8271;
}

double transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * qSqrt(qAbs(x));
    ret += (20.0 * qSin(6.0 * x * kPi) + 20.0 * qSin(2.0 * x * kPi)) * 2.0 / 3.0;
    ret += (20.0 * qSin(y * kPi) + 40.0 * qSin(y / 3.0 * kPi)) * 2.0 / 3.0;
    ret += (160.0 * qSin(y / 12.0 * kPi) + 320.0 * qSin(y * kPi / 30.0)) * 2.0 / 3.0;
    return ret;
}

double transformLng(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * qSqrt(qAbs(x));
    ret += (20.0 * qSin(6.0 * x * kPi) + 20.0 * qSin(2.0 * x * kPi)) * 2.0 / 3.0;
    ret += (20.0 * qSin(x * kPi) + 40.0 * qSin(x / 3.0 * kPi)) * 2.0 / 3.0;
    ret += (150.0 * qSin(x / 12.0 * kPi) + 300.0 * qSin(x / 30.0 * kPi)) * 2.0 / 3.0;
    return ret;
}

QPair<double, double> wgs84ToGcj02(double lat, double lng)
{
    if (outOfChina(lat, lng)) {
        return qMakePair(lat, lng);
    }

    double dLat = transformLat(lng - 105.0, lat - 35.0);
    double dLng = transformLng(lng - 105.0, lat - 35.0);
    const double radLat = lat / 180.0 * kPi;
    double magic = qSin(radLat);
    magic = 1.0 - kEarthEe * magic * magic;
    const double sqrtMagic = qSqrt(magic);
    dLat = (dLat * 180.0) / ((kEarthA * (1.0 - kEarthEe)) / (magic * sqrtMagic) * kPi);
    dLng = (dLng * 180.0) / (kEarthA / sqrtMagic * qCos(radLat) * kPi);
    return qMakePair(lat + dLat, lng + dLng);
}

bool shouldConvertGpsCoordinate(const QString &source)
{
    return source == QStringLiteral("serial-gps") || source == QStringLiteral("system-gps");
}

QByteArray normalizeNmeaLineEndings(const char *data, int len)
{
    QByteArray normalized;
    normalized.reserve(len * 2);

    for (int i = 0; i < len; ++i) {
        const char ch = data[i];
        if (ch == '\n') {
            if (i == 0 || data[i - 1] != '\r') {
                normalized.append('\r');
            }
            normalized.append('\n');
            continue;
        }
        normalized.append(ch);
    }

    return normalized;
}

}

GPSManager::GPSManager(QObject *parent)
    : QObject(parent),
    networkManager(new QNetworkAccessManager(this))
{
    loadConfig();
    detectEnvironment();

    fd = open(m_config.serialPort.toLocal8Bit().constData(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd >= 0) {
        struct termios opt;
        tcflush(fd, TCIOFLUSH);
        tcgetattr(fd, &opt);
        cfmakeraw(&opt);
        cfsetispeed(&opt, B9600);
        cfsetospeed(&opt, B9600);
        opt.c_cflag |= (CLOCAL | CREAD);
        opt.c_cflag &= ~CSIZE;
        opt.c_cflag |= CS8;
        opt.c_cflag &= ~PARENB;
        opt.c_cflag &= ~CSTOPB;
        opt.c_cc[VMIN] = 0;
        opt.c_cc[VTIME] = 0;
        tcsetattr(fd, TCSANOW, &opt);
        qInfo() << "serial open ok:" << m_config.serialPort;
    } else {
        qWarning() << "serial open failed:" << m_config.serialPort << strerror(errno);
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
    const QString configuredPlugin = settings.value(QStringLiteral("map/plugin"), m_config.pluginName).toString().trimmed().toLower();
    if (!configuredPlugin.isEmpty() && configuredPlugin != QStringLiteral("amap")) {
        qWarning() << "map/plugin is" << configuredPlugin
                   << "but this app currently uses the amap QtLocation plugin, forcing amap";
    }
    m_config.pluginName = QStringLiteral("amap");
    m_config.apiKey = settings.value(QStringLiteral("map/api_key")).toString();
    m_config.searchTimeoutMs = settings.value(QStringLiteral("network/search_timeout_ms"), m_config.searchTimeoutMs).toInt();
    m_config.retryCount = settings.value(QStringLiteral("network/retry_count"), m_config.retryCount).toInt();
}

void GPSManager::detectEnvironment()
{
    m_pluginAvailable = QGeoServiceProvider::availableServiceProviders().contains(m_config.pluginName);
    QNetworkConfigurationManager networkManager;
    m_networkAvailable = networkManager.isOnline();
    m_sslAvailable = true;

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
    if (m_config.pluginName == QStringLiteral("amap") && shouldConvertGpsCoordinate(source)) {
        const auto converted = wgs84ToGcj02(lat, lng);
        qDebug() << "WGS84 -> GCJ02" << source << "raw" << lat << lng
                 << "converted" << converted.first << converted.second;
        lat = converted.first;
        lng = converted.second;
    }

    qDebug() << "handlePositionUpdate from" << source << "lat=" << lat << "lng=" << lng;
    const bool changed = qAbs(lat - m_lat) > 0.00001 || qAbs(lng - m_lng) > 0.00001;
    m_lat = lat;
    m_lng = lng;
    if (changed) {
        emit positionChanged();
        qDebug() << "position changed, emitted positionChanged";
    }
    if (m_mapState == GpsFollowState) {
        emit centerRequested(m_lat, m_lng, static_cast<int>(m_mapState), source);
        qDebug() << "follow state, emitted centerRequested";
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
    const int ready = select(fd + 1, &readSet, nullptr, nullptr, &timeout);
    if (ready < 0) {
        if (errno != EINTR) {
            qWarning() << "select serial failed:" << m_config.serialPort << strerror(errno);
        }
        return;
    }
    if (ready == 0) {
        ++m_emptyReadCount;
        if (m_emptyReadCount >= 3 && !m_serialSilenceReported) {
            m_serialSilenceReported = true;
            qWarning() << "serial opened but no NMEA data received for"
                       << m_emptyReadCount
                       << "poll cycles, device may be disconnected or occupied by another process:"
                       << m_config.serialPort;
        }
        return;
    }

    char buff[4096] = {0};
    int len = 0;
    while (len < static_cast<int>(sizeof(buff)) - 1) {
        const int size = read(fd, buff + len, sizeof(buff) - 1 - len);
        if (size > 0) {
            len += size;
            continue;
        }

        if (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            qWarning() << "read serial failed:" << m_config.serialPort << strerror(errno);
        }
        if (size <= 0) {
            break;
        }
    }
    if (len <= 0) {
        return;
    }

    m_emptyReadCount = 0;
    if (m_serialSilenceReported) {
        m_serialSilenceReported = false;
        qInfo() << "serial data resumed:" << m_config.serialPort;
    }

    const QByteArray normalizedBatch = normalizeNmeaLineEndings(buff, len);

    // 打印原始数据（前80字符）
    QString rawPreview = QString::fromLatin1(normalizedBatch.constData(), qMin(normalizedBatch.size(), 80));
    qDebug() << "Raw NMEA:" << rawPreview;

    if (!normalizedBatch.contains('\r') && normalizedBatch.contains('\n')) {
        qWarning() << "NMEA stream has LF but no CR, normalized to CRLF before parsing";
    }

    lastSerialTime = QDateTime::currentMSecsSinceEpoch();
    const int parsedCount = nmea_parse(&parser, normalizedBatch.constData(), normalizedBatch.size(), &info);

    qDebug() << "nmea_parse result: parsed=" << parsedCount
             << "smask=" << info.smask
             << "sig=" << info.sig
             << "lat=" << info.lat << "lon=" << info.lon
             << "fix=" << info.fix;

    if (info.sig > 0 && !qFuzzyIsNull(info.lat) && !qFuzzyIsNull(info.lon)) {
        double latDeg = nmea_ndeg2degree(info.lat);
        double lonDeg = nmea_ndeg2degree(info.lon);
        qDebug() << "GPS fixed! lat=" << latDeg << "lon=" << lonDeg;
        useSerial = true;
        handlePositionUpdate(latDeg, lonDeg, QStringLiteral("serial-gps"));
    } else {
        qDebug() << "GPS no fix or incomplete sentence set, sig=" << info.sig
                 << "fix=" << info.fix
                 << "parsed=" << parsedCount;
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
    if (keyword.isEmpty()) return;
    m_searchHistory.removeAll(keyword);
    m_searchHistory.prepend(keyword);
    while (m_searchHistory.size() > 10) m_searchHistory.removeLast();
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
        if (reply->isRunning()) reply->abort();
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, trimmedKeyword]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("搜索失败，可检查网络后重试"));
            return;
        }

        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (document.isNull()) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("响应数据格式错误"));
            return;
        }

        QJsonObject object = document.object();
        if (object.value(QStringLiteral("status")).toString() != QStringLiteral("1")) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("搜索失败: %1").arg(object.value(QStringLiteral("info")).toString()));
            return;
        }

        QJsonArray geocodes = object.value(QStringLiteral("geocodes")).toArray();
        if (geocodes.isEmpty()) {
            finishSearchFailure(trimmedKeyword, QStringLiteral("无匹配结果，请调整关键词后重试"));
            return;
        }

        QVariantList results;
        for (const QJsonValue &value : geocodes) {
            QJsonObject item = value.toObject();
            QString location = item.value(QStringLiteral("location")).toString();
            QStringList parts = location.split(',');
            if (parts.size() != 2) continue;
            bool lngOk, latOk;
            double lng = parts.at(0).toDouble(&lngOk);
            double lat = parts.at(1).toDouble(&latOk);
            if (!latOk || !lngOk) continue;
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
    if (!m_lastSearchKeyword.isEmpty()) searchPlace(m_lastSearchKeyword);
}

void GPSManager::confirmSearchResult(int index)
{
    if (index < 0 || index >= m_searchResults.size()) return;
    QVariantMap result = m_searchResults.at(index).toMap();
    double lat = result.value(QStringLiteral("lat")).toDouble();
    double lng = result.value(QStringLiteral("lng")).toDouble();
    QString title = result.value(QStringLiteral("title")).toString();
    setMapState(SearchLocatedState);
    m_searchMessage = QStringLiteral("已定位到：%1").arg(title);
    emit searchMessageChanged();
    emit searchLocationFound(lat, lng, title);
    emit centerRequested(lat, lng, static_cast<int>(m_mapState), QStringLiteral("search-selection"));
    clearSearchResults();
}

void GPSManager::clearSearchResults()
{
    if (m_searchResults.isEmpty()) return;
    m_searchResults.clear();
    emit searchResultsChanged();
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
    qDebug() << "resumeGpsFollow called, current mapState=" << m_mapState
             << " lat=" << m_lat << " lng=" << m_lng;
    setMapState(GpsFollowState);
    emit centerRequested(m_lat, m_lng, static_cast<int>(m_mapState), QStringLiteral("resume-follow"));
    qDebug() << "centerRequested emitted";
}
