#include "widget.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QQmlContext>
#include <QResizeEvent>
#include <QTimer>
#include <QUrl>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    resize(480, 272);
    setAutoFillBackground(true);
    setStyleSheet(QStringLiteral("background:white;"));

    m_selectionResetTimer = new QTimer(this);
    m_selectionResetTimer->setSingleShot(true);
    connect(m_selectionResetTimer, &QTimer::timeout, this, &Widget::clearSelectedApp);

    m_launchReadyPollTimer = new QTimer(this);
    m_launchReadyPollTimer->setInterval(LaunchReadyPollIntervalMs);
    connect(m_launchReadyPollTimer, &QTimer::timeout, this, &Widget::checkLaunchReady);

    QDir().mkpath(runtimeTmpDir());
    m_carImageSource = prepareCarImage();

    initTestApps();
    initQuickUi();
}

int Widget::pageCount() const
{
    return (m_appInfos.size() + AppsPerPage - 1) / AppsPerPage;
}

int Widget::currentPage() const
{
    return m_currentPage;
}

int Widget::selectedAppId() const
{
    return m_selectedAppId;
}

double Widget::temperature() const
{
    return m_temperature;
}

double Widget::humidity() const
{
    return m_humidity;
}

bool Widget::loadingVisible() const
{
    return m_loadingVisible;
}

QString Widget::loadingAppName() const
{
    return m_loadingAppName;
}

QString Widget::carImageSource() const
{
    return m_carImageSource;
}

int Widget::appCount() const
{
    return m_appInfos.size();
}

QVariantMap Widget::appAt(int index) const
{
    if (index < 0 || index >= m_appInfos.size()) {
        return {
            {QStringLiteral("valid"), false}
        };
    }

    const AppInfo &info = m_appInfos.at(index);
    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("id"), info.Id},
        {QStringLiteral("name"), info.Name},
        {QStringLiteral("path"), info.Path},
        {QStringLiteral("iconPath"), info.Log}
    };
}

void Widget::appClicked(int appId)
{
    if (m_launchInProgress || appId < 0) {
        return;
    }

    if (m_selectedAppId == appId && m_selectionResetTimer->isActive()) {
        if (const AppInfo *info = findAppInfo(appId)) {
            clearSelectedApp();
            StartApp(*info);
        }
        return;
    }

    selectAppById(appId);
}

void Widget::pageChanged(int pageIndex)
{
    clearSelectedApp();
    setCurrentPageInternal(pageIndex);
}

void Widget::clearSelectionFromQml()
{
    clearSelectedApp();
}

void Widget::initQuickUi()
{
    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setClearColor(Qt::white);
    m_quickWidget->rootContext()->setContextProperty(QStringLiteral("widgetController"), this);
    m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    layoutQuickUi();
}

void Widget::layoutQuickUi()
{
    if (m_quickWidget) {
        m_quickWidget->setGeometry(rect());
    }
}

void Widget::initTestApps()
{
    m_appInfos.clear();
    m_appInfos
        << createAppInfo(1, QStringLiteral("game"))
        << createAppInfo(2, QStringLiteral("music"))
        << createAppInfo(3, QStringLiteral("album"))
        << createAppInfo(4, QStringLiteral("setting"))
        << createAppInfo(5, QStringLiteral("map"))
        << createAppInfo(6, QStringLiteral("RearCam"));

    emit pageCountChanged();
}

AppInfo Widget::createAppInfo(quint8 id, const QString &exeName) const
{
    QString name;
    QString iconResourcePath;
    const QString exeFileName = exeName.toLower();

    if (exeFileName.contains(QStringLiteral("game"))) {
        name = QStringLiteral("游戏");
        iconResourcePath = QStringLiteral(":/images/game.jpg");
    } else if (exeFileName.contains(QStringLiteral("music"))) {
        name = QStringLiteral("音乐");
        iconResourcePath = QStringLiteral(":/images/music.png");
    } else if (exeFileName.contains(QStringLiteral("album"))) {
        name = QStringLiteral("相册");
        iconResourcePath = QStringLiteral(":/images/album.png");
    } else if (exeFileName.contains(QStringLiteral("map"))) {
        name = QStringLiteral("地图");
        iconResourcePath = QStringLiteral(":/images/map.png");
    } else if (exeFileName.contains(QStringLiteral("rearcam"))) {
        name = QStringLiteral("摄像");
        iconResourcePath = QStringLiteral(":/images/RearCam.png");
    } else {
        name = QStringLiteral("设置");
        iconResourcePath = QStringLiteral(":/images/default.png");
    }

    return AppInfo(id, name, exeName, iconSourceForQml(iconResourcePath, id));
}

const AppInfo *Widget::findAppInfo(int appId) const
{
    for (const AppInfo &info : m_appInfos) {
        if (info.Id == appId) {
            return &info;
        }
    }

    return nullptr;
}

void Widget::selectAppById(int appId)
{
    setSelectedAppIdInternal(appId);
    m_selectionResetTimer->start(SelectionConfirmTimeoutMs);
}

void Widget::clearSelectedApp()
{
    m_selectionResetTimer->stop();
    setSelectedAppIdInternal(-1);
}

void Widget::setCurrentPageInternal(int pageIndex)
{
    const int bounded = qBound(0, pageIndex, qMax(0, pageCount() - 1));
    if (m_currentPage == bounded) {
        return;
    }

    m_currentPage = bounded;
    emit currentPageChanged();
}

void Widget::setSelectedAppIdInternal(int appId)
{
    if (m_selectedAppId == appId) {
        return;
    }

    m_selectedAppId = appId;
    emit selectedAppIdChanged();
}

void Widget::setLoadingVisible(bool visible)
{
    if (m_loadingVisible == visible) {
        return;
    }

    m_loadingVisible = visible;
    emit loadingVisibleChanged();
}

void Widget::setLoadingAppName(const QString &appName)
{
    if (m_loadingAppName == appName) {
        return;
    }

    m_loadingAppName = appName;
    emit loadingAppNameChanged();
}

bool Widget::StartApp(const AppInfo &appinfo)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString scriptPath = appDir + QStringLiteral("/app/run.sh");

    if (m_launchInProgress) {
        return false;
    }

    if (m_launchCooldown.isValid() && m_launchCooldown.elapsed() < 800) {
        return false;
    }

    if (!QFile::exists(scriptPath)) {
        return false;
    }

    m_launchInProgress = true;
    m_launchQuitScheduled = false;
    clearSelectedApp();
    m_pendingLaunchAppInfo = appinfo;
    m_launcherScriptPath = scriptPath;
    m_launchReadyFilePath = runtimeTmpDir() + QStringLiteral("/app_launch_ready");
    m_launchFailFilePath = runtimeTmpDir() + QStringLiteral("/app_launch_failed");
    QFile::remove(m_launchReadyFilePath);
    QFile::remove(m_launchFailFilePath);

    setLoadingAppName(appinfo.Name);
    setLoadingVisible(true);
    QTimer::singleShot(30, this, &Widget::startPendingLaunch);
    return true;
}

void Widget::startPendingLaunch()
{
    if (!m_launchInProgress || m_launchQuitScheduled) {
        return;
    }

    const QStringList args{m_pendingLaunchAppInfo.Path};
    if (!QProcess::startDetached(m_launcherScriptPath, args)) {
        m_launchInProgress = false;
        setLoadingVisible(false);
        return;
    }

    m_launchCooldown.restart();
    m_launchWaitTimer.start();
    m_launchReadyPollTimer->start();
}

void Widget::checkLaunchReady()
{
    if (!m_launchInProgress || m_launchQuitScheduled) {
        m_launchReadyPollTimer->stop();
        return;
    }

    if (QFile::exists(m_launchFailFilePath)) {
        QFile::remove(m_launchFailFilePath);
        m_launchReadyPollTimer->stop();
        m_launchInProgress = false;
        setLoadingVisible(false);
        return;
    }

    const bool isReady = QFile::exists(m_launchReadyFilePath);
    if (isReady && m_launchWaitTimer.elapsed() < MinLoadingDisplayMs) {
        return;
    }

    if (isReady || m_launchWaitTimer.elapsed() >= LaunchReadyTimeoutMs) {
        finishLaunchHandoff();
    }
}

void Widget::finishLaunchHandoff()
{
    if (m_launchQuitScheduled) {
        return;
    }

    m_launchQuitScheduled = true;
    m_launchReadyPollTimer->stop();
    QTimer::singleShot(0, qApp, &QCoreApplication::quit);
}

QString Widget::runtimeTmpDir() const
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/tmp");
}

QString Widget::iconSourceForQml(const QString &resourcePath, quint8 appId) const
{
    QPixmap src(resourcePath);
    if (src.isNull()) {
        return resourcePath.startsWith(QStringLiteral(":/"))
            ? QStringLiteral("qrc%1").arg(resourcePath)
            : resourcePath;
    }

    QPixmap rounded(80, 80);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath clipPath;
    clipPath.addRoundedRect(0, 0, 80, 80, 20, 20);
    painter.setClipPath(clipPath);
    painter.drawPixmap(0, 0, src.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    const QString outputPath = runtimeTmpDir() + QStringLiteral("/qml_icon_%1.png").arg(appId);
    rounded.save(outputPath, "PNG");
    return QUrl::fromLocalFile(outputPath).toString();
}

QString Widget::prepareCarImage() const
{
    QImage image(QStringLiteral(":/images/car.png"));
    if (image.isNull()) {
        return QStringLiteral("qrc:/images/car.png");
    }

    QImage argb = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < argb.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(argb.scanLine(y));
        for (int x = 0; x < argb.width(); ++x) {
            const QColor color(line[x]);
            if (color.red() > 240 && color.green() > 240 && color.blue() > 240) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    int left = argb.width();
    int top = argb.height();
    int right = -1;
    int bottom = -1;

    for (int y = 0; y < argb.height(); ++y) {
        for (int x = 0; x < argb.width(); ++x) {
            if (qAlpha(argb.pixel(x, y)) > 0) {
                left = qMin(left, x);
                top = qMin(top, y);
                right = qMax(right, x);
                bottom = qMax(bottom, y);
            }
        }
    }

    if (right >= left && bottom >= top) {
        const int margin = 8;
        const QRect cropRect(qMax(0, left - margin),
                             qMax(0, top - margin),
                             qMin(argb.width() - 1, right + margin) - qMax(0, left - margin) + 1,
                             qMin(argb.height() - 1, bottom + margin) - qMax(0, top - margin) + 1);
        argb = argb.copy(cropRect);
    }

    const QString outputPath = runtimeTmpDir() + QStringLiteral("/qml_car.png");
    argb.save(outputPath, "PNG");
    return QUrl::fromLocalFile(outputPath).toString();
}

void Widget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutQuickUi();
}
