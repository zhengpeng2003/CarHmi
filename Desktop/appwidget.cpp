#include "appwidget.h"
#include <QVBoxLayout>
#include <QPainter>

AppWidget::AppWidget(QWidget *parent) : QWidget(parent)
{
    iconLabel = new QLabel(this);
    nameLabel = new QLabel(this);
    iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    nameLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(nameLabel);

    setLayout(layout);
    updateAppearance();
}

void AppWidget::setAppInfo(quint8 id, const QString &name, const QString &exePath, const QString &iconPath)
{
    m_id = id;
    m_exePath = exePath;
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

void AppWidget::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }

    m_selected = selected;
    updateAppearance();
}

void AppWidget::updateAppearance()
{
    if (m_selected) {
        setStyleSheet(
            "background:rgba(30,144,255,90);"
            "border-radius:10px;"
            "border:2px solid rgba(255,255,255,210);"
            "color:white;"
        );
        return;
    }

    setStyleSheet(
        "background:rgba(0,0,0,60);"
        "border-radius:10px;"
        "border:1px solid rgba(255,255,255,45);"
        "color:white;"
    );
}
