#include "widget.h"
#include "qtimer.h"
#include "widget.h"
#include "ui_widget.h"
#include <QMediaPlayer>
#include <QDebug>
#include "widget.h"
#include "ui_widget.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDebug>
#include <QFileInfoList>
#include <QDir>
#include <QTime>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_background.load(":/images/background1.jpg");  // 加载背景

    m_player = new QMediaPlayer(this);              // 创建播放器
    m_player->setVolume(50);                        // 设置默认音量 50%

    // ✅ 初始化音量滑块和显示
    ui->horizontalSlider_sound->setRange(0, 100);  // 范围 0~100
    ui->horizontalSlider_sound->setValue(m_player->volume());
    ui->label_sondsize->setText(QString::number(m_player->volume()));

    m_timer = new QTimer(this);                     // 进度定时器
    connect(m_timer, &QTimer::timeout, this, &Widget::updateProgress);
    m_timer->start(500);

    connect(ui->horizontalSlider_proce, &QSlider::sliderMoved,
            this, &Widget::on_horizontalSlider_proce_sliderMoved);
    connect(ui->horizontalSlider_sound, &QSlider::valueChanged,
            this, &Widget::on_horizontalSlider_sound_valueChanged);

    // 播放完成自动下一首
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status){
        if(status == QMediaPlayer::EndOfMedia){
            on_pushButton_next_clicked();
        }
    });
    // 左边的 item 点击切换歌曲
    connect(ui->listWidget, &QListWidget::itemClicked,
            this, &Widget::on_listWidget_itemClicked);

    initMusicList();    // 加载音乐列表
    if(!m_musicList.isEmpty()){
        m_currentIndex = 0;
        playCurrentMusic();
    }
    initmusicground();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initMusicList()
{
    ui->listWidget->clear();
    m_musicList.clear();

    QString musicDirPath = QCoreApplication::applicationDirPath() + "/musics";
    QDir musicDir(musicDirPath);
    if (!musicDir.exists()) {
        qWarning() << "music dir not exist:" << musicDirPath;
        return;
    }

    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.ogg" << "*.flac" << "*.aac";
    QFileInfoList files = musicDir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QFileInfo &info : files) {
        MusicItem item;
        item.title = info.completeBaseName();
        item.path  = info.absoluteFilePath();
        m_musicList.append(item);
        ui->listWidget->addItem(item.title);
    }
    qDebug() << "load music count:" << m_musicList.size();
    if(!m_musicList.isEmpty()){
        m_currentIndex = 0;
        ui->listWidget->setCurrentRow(0);
        playCurrentMusic();
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.drawPixmap(rect(), m_background);
}

void Widget::on_pushButton_about_clicked()   { /* TODO */ }
void Widget::on_pushButton_skin_clicked()    { /* TODO */ }
void Widget::on_pushButton_close_clicked()   { close(); }
void Widget::mousePressEvent(QMouseEvent *event)   { /* TODO */ }
void Widget::mouseReleaseEvent(QMouseEvent *event) { mousestate = false; }
void Widget::mouseMoveEvent(QMouseEvent *event)    { /* TODO */ }
void Widget::update_lyric()                        { /* TODO */ }

void Widget::initmusicground()
{
    // 1️⃣ 设置原始碟片图片并缩放
    QPixmap originalPixmap(":/images/musicground.png");
    originalPixmap = originalPixmap.scaled(120, 120, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // 2️⃣ 创建圆形遮罩
    QPixmap circularPixmap(120, 120);
    circularPixmap.fill(Qt::transparent); // 背景透明
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addEllipse(0, 0, 120, 120); // 圆形
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, originalPixmap);
    painter.end();

    // 3️⃣ 保存圆形碟片并设置到 QLabel
    m_discPixmap = circularPixmap;
    ui->music_images->setPixmap(m_discPixmap);
    ui->music_images->setFixedSize(120, 120);
    ui->music_images->setAlignment(Qt::AlignCenter);

    // 4️⃣ 初始化旋转角度
    m_angle = 0;

    // 5️⃣ 创建旋转定时器
    m_discTimer = new QTimer(this);
    connect(m_discTimer, &QTimer::timeout, this, [=](){
        m_angle += 2; // 每次旋转 2 度
        if(m_angle >= 360) m_angle -= 360;

        int size = m_discPixmap.width();
        QPixmap rotated(size, size);
        rotated.fill(Qt::transparent); // 背景透明

        QPainter p(&rotated);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);

        // 中心旋转
        p.translate(size/2.0, size/2.0);
        p.rotate(m_angle);
        p.translate(-size/2.0, -size/2.0);

        p.drawPixmap(0, 0, m_discPixmap);
        p.end();

        ui->music_images->setPixmap(rotated);
    });

    m_discTimer->start(50); // 50ms刷新 → 顺滑旋转
}


void Widget::playCurrentMusic()
{
    if(m_currentIndex < 0 || m_currentIndex >= m_musicList.size()) return;
    MusicItem item = m_musicList[m_currentIndex];

    // 播放音乐
    m_player->setMedia(QUrl::fromLocalFile(item.path));
    m_player->play();

    // 播放按钮图标
    ui->pushButton_play->setIcon(QIcon(":/images/pase-hover.png"));

    // ✅ 更新左侧列表选中项
    ui->listWidget->blockSignals(true); // 阻止信号触发
    ui->listWidget->setCurrentRow(m_currentIndex);
    ui->listWidget->blockSignals(false);
}

void Widget::on_pushButton_play_clicked()
{
    if(m_player->state() == QMediaPlayer::PlayingState){
        m_player->pause();
        ui->pushButton_play->setIcon(QIcon(":/images/play-hover.png"));
        m_discTimer->stop(); // 暂停旋转
    } else {
        m_player->play();
        ui->pushButton_play->setIcon(QIcon(":/images/pase-hover.png"));
        m_discTimer->start(50); // 继续旋转
    }

}

void Widget::on_horizontalSlider_sound_valueChanged(int value)
{
    m_player->setVolume(value);
    ui->label_sondsize->setText(QString::number(value));
}

void Widget::on_pushButton_next_clicked()
{
    if(m_musicList.isEmpty()) return;
    m_currentIndex = (m_currentIndex + 1) % m_musicList.size();
    playCurrentMusic();
}

void Widget::on_pushButton_pre_clicked()
{
    if(m_musicList.isEmpty()) return;
    m_currentIndex = (m_currentIndex - 1 + m_musicList.size()) % m_musicList.size();
    playCurrentMusic();
}

void Widget::updateProgress()
{
    if(m_player->duration() <= 0) return;
    int pos = m_player->position();
    int dur = m_player->duration();

    ui->horizontalSlider_proce->setMaximum(dur);
    ui->horizontalSlider_proce->setValue(pos);

    QTime current(0,0,0);
    current = current.addMSecs(pos);
    QTime total(0,0,0);
    total = total.addMSecs(dur);

    ui->label_proce->setText(current.toString("m:ss"));
    ui->label_procesize->setText(total.toString("m:ss"));
}

void Widget::on_horizontalSlider_proce_sliderMoved(int value)
{
    m_player->setPosition(value);
}
void Widget::on_listWidget_itemClicked(QListWidgetItem *item)
{
    int row = ui->listWidget->row(item);
    if(row >= 0 && row < m_musicList.size()) {
        m_currentIndex = row;
        playCurrentMusic();
    }
}
