#include "apptilequickitem.h"

#include <QApplication>
#include <QPainter>
#include <QQuickWindow>

#include "appwidget.h"

AppTileQuickItem::AppTileQuickItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
}

void AppTileQuickItem::paint(QPainter *painter)
{
    updateCache();

    if (!m_cache.isNull()) {
        painter->drawPixmap(0, 0, m_cache);
    }
}

int AppTileQuickItem::appId() const
{
    return m_appId;
}

QString AppTileQuickItem::appName() const
{
    return m_appName;
}

QString AppTileQuickItem::appPath() const
{
    return m_appPath;
}

QString AppTileQuickItem::iconPath() const
{
    return m_iconPath;
}

bool AppTileQuickItem::isSelected() const
{
    return m_selected;
}

void AppTileQuickItem::setAppId(int appId)
{
    if (m_appId == appId) {
        return;
    }

    m_appId = appId;
    markDirty();
    emit appIdChanged();
}

void AppTileQuickItem::setAppName(const QString &appName)
{
    if (m_appName == appName) {
        return;
    }

    m_appName = appName;
    markDirty();
    emit appNameChanged();
}

void AppTileQuickItem::setAppPath(const QString &appPath)
{
    if (m_appPath == appPath) {
        return;
    }

    m_appPath = appPath;
    markDirty();
    emit appPathChanged();
}

void AppTileQuickItem::setIconPath(const QString &iconPath)
{
    if (m_iconPath == iconPath) {
        return;
    }

    m_iconPath = iconPath;
    markDirty();
    emit iconPathChanged();
}

void AppTileQuickItem::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }

    m_selected = selected;
    markDirty();
    emit selectedChanged();
}

void AppTileQuickItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        markDirty();
    }
}

void AppTileQuickItem::updateCache()
{
    if (!m_cacheDirty) {
        return;
    }

    const QSize size(qMax(0, int(width())), qMax(0, int(height())));
    if (size.isEmpty()) {
        m_cache = QPixmap();
        m_cacheDirty = false;
        return;
    }

    if (m_appId < 0 || (m_appName.isEmpty() && m_iconPath.isEmpty())) {
        m_cache = QPixmap(size);
        m_cache.fill(Qt::transparent);
        m_cacheDirty = false;
        return;
    }

    const qreal devicePixelRatio = window() ? window()->devicePixelRatio() : qApp->devicePixelRatio();
    const QSize bufferSize = QSize(qMax(1, int(size.width() * devicePixelRatio)),
                                   qMax(1, int(size.height() * devicePixelRatio)));

    AppWidget preview;
    preview.resize(size);
    preview.setAttribute(Qt::WA_DontShowOnScreen, true);
    preview.setAppInfo(static_cast<quint8>(qMax(0, m_appId)), m_appName, m_appPath, m_iconPath);
    preview.setSelected(m_selected);
    preview.ensurePolished();

    QPixmap buffer(bufferSize);
    buffer.setDevicePixelRatio(devicePixelRatio);
    buffer.fill(Qt::transparent);
    preview.render(&buffer);

    m_cache = buffer;
    m_cacheDirty = false;
}

void AppTileQuickItem::markDirty()
{
    m_cacheDirty = true;
    update();
}
