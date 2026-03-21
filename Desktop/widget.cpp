#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QApplication>

// ================= 构造 =================
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

// ================= 测试数据 =================
void Widget::InitTestApps()
{
    _AppInfos.clear();

    // ⭐ 左半屏 rect（关键！！！）
    QVector<QRect> rects = calcAppRects(width()/2, height());

    createApp(1, "game", rects[0]);
    createApp(2, "music", rects[1]);
    createApp(3, "album", rects[2]);
    createApp(4, "setting", rects[3]);

    // ⭐ 测试分页（一定要加）
    createApp(5, "map", rects[0]);

    // ⭐ 右侧必须单独算（不能复用 rects）
    int rightX = width()/2;
    QRect rightRect(rightX + 5, 5, width() - rightX - 10, height() - 10);

    _envPanel = createEnvironmentPanel(rightRect, 26.3, 58, ":/images/car.png");
}

// ================= createApp（核心） =================
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
    }
     else if (exeFileName.contains("map")) {
    name = "地图"; log = ":/images/map.png";
    }
    else {
        name = "设置"; log = ":/images/default.png";
    }

    AppInfo info(id, name, exeName, log);
    _AppInfos.append(info);

    // ⭐分页逻辑
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
    w->show();

    connect(w, &AppWidget::clicked, this, [=](quint8){
        StartApp(info);
    });

    return info;
}

// ================= 滑动 =================
void Widget::mousePressEvent(QMouseEvent *event)
{
    if(event->x() > width()/2) return;

    pressPos = event->pos();
    startX = leftContainer ? leftContainer->x() : 0;
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if(!leftContainer) return;
    if(event->x() > width()/2) return;

    int dx = event->pos().x() - pressPos.x();
    int newX = startX + dx;

    int minX = -(leftContainer->width() - width()/2);

    if(newX > 0) newX = 0;
    if(newX < minX) newX = minX;

    leftContainer->move(newX, 0);
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if(!leftContainer) return;
    if(event->x() > width()/2) return;

    int dx = event->pos().x() - pressPos.x();

    if(dx > 60 && currentPage > 0)
        currentPage--;
    else if(dx < -60 && currentPage < pages.size()-1)
        currentPage++;

    int targetX = -currentPage * (width()/2);

    QPropertyAnimation *anim = new QPropertyAnimation(leftContainer, "pos");
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setEndValue(QPoint(targetX, 0));
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ================= 环境面板 =================
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

// ================= 启动 =================
bool Widget::StartApp(const AppInfo &appinfo)
{
    QString scriptPath = QCoreApplication::applicationDirPath() + "/app/run.sh";

    if (!QFile::exists(scriptPath))
        return false;

    QStringList args;
    args << appinfo.Path;

    QProcess::startDetached(scriptPath, args);

    qApp->quit();
    return true;
}

// ================= 布局 =================
QVector<QRect> Widget::calcAppRects(quint32 screenW, quint32 screenH)
{
    QVector<QRect> rects;

    int spacing = 5;
    int rows = 2, cols = 2;

    // ⭐ 这里直接用 screenW（不要再 /2）
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

// ================= paint =================
void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}
