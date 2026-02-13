#ifndef PROTREEWIDGET_H
#define PROTREEWIDGET_H

#include <QTreeWidget>
#include <QWidget>
#include <slideshowdialog.h>

#include <QMediaPlayer>
#include <protreethread.h>
#include <qprogressdialog.h>

// Qt5 不需要 QAudioOutput，删除这行
// #include <QAudioOutput>

class opentreethread;
class protreewidget : public QTreeWidget
{
    Q_OBJECT
public:
    QSet<QString> current_tree_name;
    protreewidget(QWidget *parent=nullptr);
    void forceUpdateWindow();
signals:
    void itempressed(const QString name,const QString path);
    void signalotcancle();
    void signal_showpic(const QString path);
    void signal_preshowpic(const QString path);
    void signal_nextshowpic(const QString path);
    void sigcolse();
public slots:
    void open_tree(const QString &path);
    void add_tree(const QString& name,const QString& path);
    void add_tree_son(QTreeWidgetItem *item, int column);
    void open_file_slots();
    void set_action_slots();
    void close_action_slots();
    void slide_player_slots();
    void slotdobleclick(QTreeWidgetItem *item, int column);
    void progressupdate(int count);
    void progressfinish(int count);
    void progresscancel();
    void slpreupdate();
    void slnextupdate();
    void set_music();
    void startmusic();
    void stopmusic();
private:
    QAction * _open_file;
    QAction * _set_action;
    QAction * _close_action;
    QAction * _slide_player;
    QMenu *_menu;

    QList<QString> filenames;
    QTreeWidgetItem * right_btn_item;
    QTreeWidgetItem * action_item;
    QTreeWidgetItem * select_item;
    QTreeWidgetItem * rootitem;
    QTreeWidgetItem * pro_item;
    QTreeWidgetItem * next_item;

    QProgressDialog* _open_progressdlg;
    QProgressDialog *_dialog_progess;
    QString current_path;
    QString current_name;
    std::shared_ptr<protreethread>  _thread_create_pro;
    std::shared_ptr<opentreethread> _thread_open_pro;
    std::shared_ptr<slideshowdialog> _slide_show_dlg;
    QMediaPlayer *_mediaplayer;

    // Qt5 删除 QAudioOutput 成员
    // QAudioOutput * _audioOutput;  ← 删除这行

    QList<QUrl> _mediaplerlist;
    QSet<QString> _set_path;
};

#endif // PROTREEWIDGET_H
