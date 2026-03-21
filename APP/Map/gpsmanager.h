#ifndef GPSMANAGER_H
#define GPSMANAGER_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QVariantList>
#include <QStringList>

extern "C" {
#include <nmea/nmea.h>
}

class GPSManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude NOTIFY positionChanged)
    Q_PROPERTY(double longitude READ longitude NOTIFY positionChanged)
    Q_PROPERTY(QString searchMessage READ searchMessage NOTIFY searchMessageChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults NOTIFY searchResultsChanged)
    Q_PROPERTY(QStringList searchHistory READ searchHistory NOTIFY searchHistoryChanged)
    Q_PROPERTY(QString environmentMessage READ environmentMessage NOTIFY environmentMessageChanged)
    Q_PROPERTY(int mapState READ mapState NOTIFY mapStateChanged)

public:
    enum MapState {
        GpsFollowState = 0,
        SearchLocatedState = 1,
        ManualBrowseState = 2
    };
    Q_ENUM(MapState)

    explicit GPSManager(QObject *parent = nullptr);
    ~GPSManager();

    double latitude() const { return m_lat; }
    double longitude() const { return m_lng; }
    QString searchMessage() const { return m_searchMessage; }
    QVariantList searchResults() const { return m_searchResults; }
    QStringList searchHistory() const { return m_searchHistory; }
    QString environmentMessage() const { return m_environmentMessage; }
    int mapState() const { return static_cast<int>(m_mapState); }

    Q_INVOKABLE void searchPlace(const QString &keyword);
    Q_INVOKABLE void retryLastSearch();
    Q_INVOKABLE void confirmSearchResult(int index);
    Q_INVOKABLE void clearSearchResults();
    Q_INVOKABLE void clearSearchHistory();
    Q_INVOKABLE void enterManualBrowse();
    Q_INVOKABLE void resumeGpsFollow();

signals:
    void positionChanged(double lat, double lng);
    void searchLocationFound(double lat, double lng, QString keyword);
    void searchFailed(QString keyword);
    void searchMessageChanged();
    void searchResultsChanged();
    void searchHistoryChanged();
    void environmentMessageChanged();
    void mapStateChanged();
    void centerRequested(double lat, double lng, int state, QString reason);

private slots:
    void readSerialData();
    void updateSystemGPS(const QGeoPositionInfo &info);
    void checkGpsSource();

private:
    struct MapConfig {
        QString serialPort = QStringLiteral("/dev/ttymxc2");
        QString pluginName = QStringLiteral("amap");
        QString apiKey;
        int searchTimeoutMs = 5000;
        int retryCount = 1;
    };

    void loadConfig();
    void detectEnvironment();
    void handlePositionUpdate(double lat, double lng, const QString &source);
    void setMapState(MapState state);
    void pushSearchHistory(const QString &keyword);
    bool environmentReadyForSearch(QString *errorMessage = nullptr) const;
    void finishSearchFailure(const QString &keyword, const QString &message);

    int fd = -1;
    nmeaPARSER parser;
    nmeaINFO info;
    QTimer *timer = nullptr;
    QTimer *checkTimer = nullptr;
    QGeoPositionInfoSource *sysGps = nullptr;
    QNetworkAccessManager *networkManager = nullptr;
    bool useSerial = false;
    qint64 lastSerialTime = 0;
    double m_lat = 39.9;
    double m_lng = 116.3;
    QString m_searchMessage;
    QVariantList m_searchResults;
    QStringList m_searchHistory;
    QString m_environmentMessage;
    QString m_lastSearchKeyword;
    MapState m_mapState = GpsFollowState;
    MapConfig m_config;
    bool m_pluginAvailable = false;
    bool m_networkAvailable = true;
    bool m_sslAvailable = false;
};

#endif
