#pragma once

#include <QPixmap>
#include <QQuickPaintedItem>

class AppTileQuickItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int appId READ appId WRITE setAppId NOTIFY appIdChanged)
    Q_PROPERTY(QString appName READ appName WRITE setAppName NOTIFY appNameChanged)
    Q_PROPERTY(QString appPath READ appPath WRITE setAppPath NOTIFY appPathChanged)
    Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath NOTIFY iconPathChanged)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected NOTIFY selectedChanged)

public:
    explicit AppTileQuickItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    int appId() const;
    QString appName() const;
    QString appPath() const;
    QString iconPath() const;
    bool isSelected() const;

public slots:
    void setAppId(int appId);
    void setAppName(const QString &appName);
    void setAppPath(const QString &appPath);
    void setIconPath(const QString &iconPath);
    void setSelected(bool selected);

signals:
    void appIdChanged();
    void appNameChanged();
    void appPathChanged();
    void iconPathChanged();
    void selectedChanged();

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void updateCache();
    void markDirty();

private:
    int m_appId = -1;
    QString m_appName;
    QString m_appPath;
    QString m_iconPath;
    bool m_selected = false;
    bool m_cacheDirty = true;
    QPixmap m_cache;
};
