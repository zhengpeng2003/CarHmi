#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    resize(480,272);
    InitTestApps();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::InitTestApps()
{
    _AppInfos.clear();

    QVector<QRect> rects = calcAppRects(width()/2, height());

    createApp(1, "game", rects[0]);
    createApp(2, "music", rects[1]);
    createApp(3, "album", rects[2]);
    createApp(4, "setting", rects[3]);
    createApp(5, "map", rects[0]);

    int rightX = width()/2;
    QRect rightRect(rightX + 5, 5, width() - rightX - 10, height() - 10);

    _envPanel = createEnvironmentPanel(rightRect, 26.3, 58, ":/images/car.png");
}

AppInfo Widget::createApp(quint8 id, const QString &exeName, const QRect &rect)
{
    QString name, log;
    QString exeFileName = exeName.toLower();

    if (exeFileName.contains("game")) {
        name = "游戏"; log = ":/images/game.jpg";
    } else if (exeFileName.contains("music")) {
        name = "音乐"; log = ":/images/music.png";
    } else if (exeFileName.contains("album")) {
        name = "相册"; log = ":/images/album.png";
    } else if (exeFileName.contains("map")) {
        name = "地图"; log = ":/images/map.png";
    } else {
        name = "设置"; log = ":/images/default.png";
    }

    AppInfo info(id, name, exeName, log);
    _AppInfos.append(info);

    int perPage = 4;
    int index = _AppInfos.size() - 1;
    int pageIndex = index / perPage;

    int leftW = width()/2;
    int leftH = height();

    if (!leftContainer)
    {
        leftContainer = new QWidget(this);
        leftContainer->setGeometry(0, 0, leftW, leftH);
    }

    if (pageIndex >= pages.size())
    {
        QWidget *page = new QWidget(leftContainer);
        page->setGeometry(pageIndex * leftW, 0, leftW, leftH);
        pages.append(page);

        leftContainer->resize(pages.size() * leftW, leftH);
    }

    QWidget *page = pages[pageIndex];

    AppWidget *w = new AppWidget(page);
    w->setAppInfo(info.Id, info.Name, info.Path, info.Log);
    w->setGeometry(rect);
    w->installEventFilter(this);
    w->show();

    return info;
}

bool Widget::handlePress(const QPoint &globalPos, AppWidget *appWidget)
{
    const QPoint localPos = mapFromGlobal(globalPos);
    if (!isInLeftArea(localPos)) {
        return false;
    }

    pressPos = localPos;
    startX = leftContainer ? leftContainer->x() : 0;
    isDragging = false;
    pressedAppWidget = appWidget;
    return true;
}

bool Widget::handleMove(const QPoint &globalPos)
{
    if (!leftContainer) {
        return false;
    }

    const QPoint localPos = mapFromGlobal(globalPos);
    if (!isInLeftArea(localPos) && !pressedAppWidget) {
        return false;
    }

    const QPoint delta = localPos - pressPos;
    if (!isDragging && delta.manhattanLength() >= DragThreshold) {
        isDragging = true;
    }

    if (!isDragging) {
        return pressedAppWidget != nullptr;
    }

    int newX = startX + delta.x();
    const int minX = qMin(0, -(leftContainer->width() - width()/2));

    if(newX > 0) newX = 0;
    if(newX < minX) newX = minX;

    leftContainer->move(newX, 0);
    return true;
}

bool Widget::handleRelease(const QPoint &globalPos, AppWidget *appWidget)
{
    if (!leftContainer) {
        return false;
    }

    const QPoint localPos = mapFromGlobal(globalPos);
    const QPoint delta = localPos - pressPos;
    const bool isClick = !isDragging && delta.manhattanLength() <= ClickThreshold;

    if (isClick && pressedAppWidget && appWidget == pressedAppWidget) {
        if (const AppInfo *info = findAppInfo(pressedAppWidget->appId())) {
            qDebug() << "Desktop click confirmed, start app:" << info->Path;
            pressedAppWidget = nullptr;
            return StartApp(*info);
        }
    }

    if (isDragging || qAbs(delta.x()) >= PageSwitchThreshold) {
        if(delta.x() > PageSwitchThreshold && currentPage > 0)
            currentPage--;
        else if(delta.x() < -PageSwitchThreshold && currentPage < pages.size()-1)
            currentPage++;
    }

    pressedAppWidget = nullptr;
    snapToCurrentPage();
    return isInLeftArea(localPos);
}

const AppInfo *Widget::findAppInfo(quint8 id) const
{
    for (const AppInfo &info : _AppInfos) {
        if (info.Id == id) {
            return &info;
        }
    }
    return nullptr;
}

void Widget::snapToCurrentPage()
{
    if (!leftContainer) {
        return;
    }

    int targetX = -currentPage * (width()/2);

    QPropertyAnimation *anim = new QPropertyAnimation(leftContainer, "pos");
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setEndValue(QPoint(targetX, 0));
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

bool Widget::isInLeftArea(const QPoint &localPos) const
{
    return localPos.x() >= 0 && localPos.x() <= width()/2;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    handlePress(event->globalPos(), nullptr);
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    handleMove(event->globalPos());
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    handleRelease(event->globalPos(), nullptr);
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    AppWidget *appWidget = qobject_cast<AppWidget *>(watched);
    if (!appWidget) {
        return QWidget::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return handlePress(static_cast<QMouseEvent *>(event)->globalPos(), appWidget);
    case QEvent::MouseMove:
        return handleMove(static_cast<QMouseEvent *>(event)->globalPos());
    case QEvent::MouseButtonRelease:
        return handleRelease(static_cast<QMouseEvent *>(event)->globalPos(), appWidget);
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

EnvironmentWidget* Widget::createEnvironmentPanel(const QRect &rect, double temp, double hum, const QString &bg)
{
    EnvironmentWidget *env = new EnvironmentWidget(this);
    env->setGeometry(rect);
    env->setTemperature(temp);
    env->setHumidity(hum);
    env->setBackgroundImage(bg);
    env->show();
    return env;
}

bool Widget::StartApp(const AppInfo &appinfo)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString scriptPath = appDir + "/app/run.sh";
    const QString stateFile = appDir + "/tmp/desktop_state";

    if (!QFile::exists(scriptPath)) {
        qWarning() << "App launcher script missing:" << scriptPath;
        return false;
    }

    QDir().mkpath(appDir + "/tmp");
    QFile state(stateFile);
    if (state.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        state.write("app\n");
        state.close();
    } else {
        qWarning() << "Unable to write desktop_state before starting app:" << stateFile;
        return false;
    }

    const QStringList args{appinfo.Path};
    qDebug() << "Desktop launching app:" << appinfo.Path << "via" << scriptPath;

    if (!QProcess::startDetached(scriptPath, args)) {
        qWarning() << "Failed to start app launcher for" << appinfo.Path;
        if (state.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            state.write("desktop\n");
            state.close();
        }
        return false;
    }

    qDebug() << "Desktop exiting after handoff to app:" << appinfo.Path;
    qApp->quit();
    return true;
}

QVector<QRect> Widget::calcAppRects(quint32 screenW, quint32 screenH)
{
    QVector<QRect> rects;

    int spacing = 5;
    int rows = 2, cols = 2;
    int squareW = (screenW - (cols + 1) * spacing) / cols;
    int squareH = (screenH - (rows + 1) * spacing) / rows;

    int size = qMin(squareW, squareH);

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            int x = spacing + c * (size + spacing);
            int y = spacing + r * (size + spacing) + 30;

            rects.append(QRect(x, y, size, size));
        }
    }

    return rects;
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}
