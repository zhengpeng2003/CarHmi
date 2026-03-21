#include "appwidget.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>

AppWidget::AppWidget(QWidget *parent) : QWidget(parent)
{
    iconLabel = new QLabel(this);
    nameLabel = new QLabel(this);

    nameLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(nameLabel);

    setLayout(layout);
    setStyleSheet("background:rgba(0,0,0,60);border-radius:10px;");
}

void AppWidget::setAppInfo(quint8 id, const QString &name, const QString &, const QString &iconPath)
{
    m_id = id;
    nameLabel->setText(name);

    QPixmap src(iconPath);
    QPixmap rounded(80,80);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(0,0,80,80,20,20);
    painter.setClipPath(path);

    painter.drawPixmap(0,0,src.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation));

    iconLabel->setPixmap(rounded);
}

void AppWidget::mousePressEvent(QMouseEvent *)
{
    emit clicked(m_id);
}
