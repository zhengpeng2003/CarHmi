#ifndef WIDGET_H
#define WIDGET_H

#include <QSettings>
#include <QWidget>
#include <QVector>
#include <QRect>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QProcess>
#include <QPoint>
#include <QElapsedTimer>

#include "appwidget.h"
#include "environmentwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

struct AppInfo
{
    quint8 Id;
    QString Name;
    QString Path;
    QString Log;

    AppInfo(){}
    AppInfo(quint8 id, const QString &name, const QString &path, const QString &log)
        : Id(id), Name(name), Path(path), Log(log){}
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QVector<QRect> calcAppRects(quint32 screenW, quint32 screenH);
    bool StartApp(const AppInfo &appinfo);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void InitTestApps();
    AppInfo createApp(quint8 id, const QString &exeName, const QRect &rect);
    EnvironmentWidget* createEnvironmentPanel(const QRect &rect, double temp, double hum, const QString &bg);
    bool handlePress(const QPoint &globalPos, AppWidget *appWidget);
    bool handleMove(const QPoint &globalPos);
    bool handleRelease(const QPoint &globalPos, AppWidget *appWidget);
    const AppInfo *findAppInfo(quint8 id) const;
    void snapToCurrentPage();
    bool isInLeftArea(const QPoint &localPos) const;

private:
    Ui::Widget *ui;

    QVector<AppInfo> _AppInfos;
    EnvironmentWidget *_envPanel = nullptr;

    QWidget *leftContainer = nullptr;
    QVector<QWidget*> pages;
    int currentPage = 0;

    QPoint pressPos;
    int startX = 0;
    bool isDragging = false;
    bool launchInProgress = false;
    AppWidget *pressedAppWidget = nullptr;
    QElapsedTimer launchCooldown;

    static constexpr int ClickThreshold = 16;
    static constexpr int DragThreshold = 12;
    static constexpr int PageSwitchThreshold = 60;
};

#endif
