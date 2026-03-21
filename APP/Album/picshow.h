#ifndef PICSHOW_H
#define PICSHOW_H

#include <QDialog>
#include <QEvent>
#include <QFutureWatcher>
#include <QPixmap>
#include <QPropertyAnimation>

namespace Ui {
class picshow;
}

class picshow : public QDialog
{
    Q_OBJECT

public:
    explicit picshow(QWidget *parent = nullptr);
    ~picshow() override;
    void showprenextbtn(bool b_show);
    void repic();
    QString _select_pic;
private:
    void requestPixmap(const QString &path);
    QPixmap scaledPixmapForCurrentView(const QPixmap &pixmap) const;

    QPropertyAnimation * pre_btn_animation;
    QPropertyAnimation * next_btn_animation;
    QPixmap _pic;
    QFutureWatcher<QImage> _imageWatcher;
    QString _pendingPath;

    Ui::picshow *ui;
signals:
    void sigprebtn();
    void signextbtn();
protected:
    bool event(QEvent *event) override;
public slots:
    void slotpicshow(QString path);
    void slotprepicshow(const QString path);
    void slotnextpicshow(const QString path);
    void slotclosepic();
};

#endif // PICSHOW_H
