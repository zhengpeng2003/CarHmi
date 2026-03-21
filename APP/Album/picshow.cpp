#include "picshow.h"
#include "ui_picshow.h"
#include "QGraphicsOpacityEffect"
#include "QPropertyAnimation"
#include <QtConcurrent>
#include <QImageReader>

picshow::picshow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::picshow)
{
    ui->setupUi(this);
    ui->previous_btn->creatpicbutton(":/icon/previous.png",":/icon/previous_hover.png",":/icon/previous_press.png");
    ui->next_btn->creatpicbutton(":/icon/next.png",":/icon/next_hover.png",":/icon/next_press.png");

    auto *pre_btn_opacity = new QGraphicsOpacityEffect(this);
    pre_btn_opacity->setOpacity(0);
    ui->previous_btn->setGraphicsEffect(pre_btn_opacity);

    auto *next_btn_opacity = new QGraphicsOpacityEffect(this);
    next_btn_opacity->setOpacity(0);
    ui->next_btn->setGraphicsEffect(next_btn_opacity);

    pre_btn_animation = new QPropertyAnimation(pre_btn_opacity,"opacity",this);
    pre_btn_animation->setEasingCurve(QEasingCurve::InExpo);
    pre_btn_animation->setDuration(500);

    next_btn_animation = new QPropertyAnimation(next_btn_opacity,"opacity",this);
    next_btn_animation->setEasingCurve(QEasingCurve::InExpo);
    next_btn_animation->setDuration(500);

    connect(ui->previous_btn,&QPushButton::clicked,this,&picshow::sigprebtn);
    connect(ui->next_btn,&QPushButton::clicked,this,&picshow::signextbtn);
    connect(&_imageWatcher, &QFutureWatcher<QImage>::finished, this, [this]() {
        if (_pendingPath != _select_pic) {
            return;
        }
        const QImage image = _imageWatcher.result();
        if (image.isNull()) {
            ui->pic_show_label->clear();
            return;
        }
        _pic = QPixmap::fromImage(image);
        ui->pic_show_label->setScaledContents(true);
        ui->pic_show_label->setPixmap(scaledPixmapForCurrentView(_pic));
    });
}

picshow::~picshow()
{
    _imageWatcher.cancel();
    _imageWatcher.waitForFinished();
    delete ui;
}

QPixmap picshow::scaledPixmapForCurrentView(const QPixmap &pixmap) const
{
    const QSize targetSize = ui->pic_show_label->size().boundedTo(QSize(1920, 1080));
    if (targetSize.isEmpty()) {
        return pixmap;
    }
    return pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void picshow::requestPixmap(const QString &path)
{
    _select_pic = path;
    _pendingPath = path;
    if (_select_pic.isEmpty()) {
        ui->pic_show_label->clear();
        return;
    }

    if (_imageWatcher.isRunning()) {
        _imageWatcher.cancel();
        _imageWatcher.waitForFinished();
    }

    _imageWatcher.setFuture(QtConcurrent::run([path]() {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        const QSize maxSize(1920, 1080);
        if (reader.size().isValid()) {
            QSize scaled = reader.size();
            scaled.scale(maxSize, Qt::KeepAspectRatio);
            reader.setScaledSize(scaled);
        }
        return reader.read();
    }));
}

void picshow::showprenextbtn(bool b_show)
{
    if(b_show)
    {
        pre_btn_animation->stop();
        pre_btn_animation->setStartValue(0);
        pre_btn_animation->setEndValue(1);
        pre_btn_animation->start();

        next_btn_animation->stop();
        next_btn_animation->setStartValue(0);
        next_btn_animation->setEndValue(1);
        next_btn_animation->start();
    }
    else
    {
        pre_btn_animation->stop();
        pre_btn_animation->setStartValue(1);
        pre_btn_animation->setEndValue(0);
        pre_btn_animation->start();

        next_btn_animation->stop();
        next_btn_animation->setStartValue(1);
        next_btn_animation->setEndValue(0);
        next_btn_animation->start();
    }
}

void picshow::repic()
{
    if(_select_pic.isEmpty())
        return;
    if (!_pic.isNull()) {
        ui->pic_show_label->setPixmap(scaledPixmapForCurrentView(_pic));
    }
}

bool picshow::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::Enter:
        showprenextbtn(true);break;
    case QEvent::Leave:
        showprenextbtn(false);break;
    default:
        break;
    }
   return  QDialog::event(event);
}

void picshow::slotpicshow(QString path)
{
    requestPixmap(path);
}

void picshow::slotprepicshow(const QString path)
{
    requestPixmap(path);
}

void picshow::slotnextpicshow(const QString path)
{
    requestPixmap(path);
}

void picshow::slotclosepic()
{
    _select_pic = "";
    _pendingPath.clear();
    _pic = QPixmap();
    if (_imageWatcher.isRunning()) {
        _imageWatcher.cancel();
        _imageWatcher.waitForFinished();
    }
    ui->pic_show_label->clear();
}
