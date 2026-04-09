#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void setFrame(const QImage &img);
    void setShowGuideLine(bool on);
    void setNightMode(bool on);
    void setPaused(bool on);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage m_frame;
    bool m_showGuideLine;
    bool m_nightMode;
    bool m_paused;
    qreal m_scanPos;
};

#endif
