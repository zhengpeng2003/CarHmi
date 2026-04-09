#ifndef WIDGET_H
#define WIDGET_H

#include "appinfo.h"

#include <QElapsedTimer>
#include <QQuickWidget>
#include <QVariantMap>
#include <QVector>
#include <QWidget>

class QResizeEvent;
class QTimer;

class Widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(int selectedAppId READ selectedAppId NOTIFY selectedAppIdChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(double humidity READ humidity NOTIFY humidityChanged)
    Q_PROPERTY(bool loadingVisible READ loadingVisible NOTIFY loadingVisibleChanged)
    Q_PROPERTY(QString loadingAppName READ loadingAppName NOTIFY loadingAppNameChanged)
    Q_PROPERTY(QString carImageSource READ carImageSource CONSTANT)

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override = default;

    bool StartApp(const AppInfo &appinfo);

    int pageCount() const;
    int currentPage() const;
    int selectedAppId() const;
    double temperature() const;
    double humidity() const;
    bool loadingVisible() const;
    QString loadingAppName() const;
    QString carImageSource() const;

    Q_INVOKABLE int appCount() const;
    Q_INVOKABLE QVariantMap appAt(int index) const;
    Q_INVOKABLE void appClicked(int appId);
    Q_INVOKABLE void pageChanged(int pageIndex);
    Q_INVOKABLE void clearSelectionFromQml();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void pageCountChanged();
    void currentPageChanged();
    void selectedAppIdChanged();
    void temperatureChanged();
    void humidityChanged();
    void loadingVisibleChanged();
    void loadingAppNameChanged();

private:
    void initQuickUi();
    void layoutQuickUi();
    void initTestApps();
    AppInfo createAppInfo(quint8 id, const QString &exeName) const;
    const AppInfo *findAppInfo(int appId) const;
    void selectAppById(int appId);
    void clearSelectedApp();
    void setCurrentPageInternal(int pageIndex);
    void setSelectedAppIdInternal(int appId);
    void setLoadingVisible(bool visible);
    void setLoadingAppName(const QString &appName);
    void startPendingLaunch();
    void checkLaunchReady();
    void finishLaunchHandoff();
    QString runtimeTmpDir() const;
    QString iconSourceForQml(const QString &resourcePath, quint8 appId) const;
    QString prepareCarImage() const;

private:
    QVector<AppInfo> m_appInfos;
    QQuickWidget *m_quickWidget = nullptr;

    int m_currentPage = 0;
    int m_selectedAppId = -1;
    double m_temperature = 26.3;
    double m_humidity = 58.0;
    bool m_loadingVisible = false;
    QString m_loadingAppName;
    QString m_carImageSource;

    bool m_launchInProgress = false;
    bool m_launchQuitScheduled = false;
    QElapsedTimer m_launchCooldown;
    QElapsedTimer m_launchWaitTimer;
    QTimer *m_selectionResetTimer = nullptr;
    QTimer *m_launchReadyPollTimer = nullptr;
    AppInfo m_pendingLaunchAppInfo;
    QString m_launcherScriptPath;
    QString m_launchReadyFilePath;
    QString m_launchFailFilePath;

    static constexpr int AppsPerPage = 4;
    static constexpr int SelectionConfirmTimeoutMs = 1800;
    static constexpr int LaunchReadyPollIntervalMs = 80;
    static constexpr int LaunchReadyTimeoutMs = 3500;
    static constexpr int MinLoadingDisplayMs = 450;
};

#endif
