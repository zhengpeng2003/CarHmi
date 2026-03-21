#include "opentreethread.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>

opentreethread::opentreethread(const QString scr_path, const QString name, int file_cout, protreewidget *self, QObject *parent)
    : QThread(parent), _scr_path(scr_path), _name(name), _file_cout(file_cout), _self(self), _bststop(false)
{
}

void opentreethread::openprotree(const QString scr_path, const QString name, int file_cout, protreewidget *self)
{
    Q_UNUSED(file_cout)
    Q_UNUSED(self)
    emit rootItemReady(name, scr_path);
    re_opentree(scr_path, _file_cout, scr_path, scr_path);
}

void opentreethread::re_opentree(const QString scr_path, int file_cout, const QString &rootPath, const QString &parentPath)
{
    QDir scr_path_dir(scr_path);
    scr_path_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files);
    scr_path_dir.setSorting(QDir::Name);
    const QFileInfoList fileinfolist = scr_path_dir.entryInfoList();
    for (const QFileInfo &temp_file : fileinfolist) {
        if (_bststop) {
            return;
        }
        if (temp_file.isDir()) {
            ++file_cout;
            emit progressupdate(file_cout);
            emit directoryDiscovered(parentPath, temp_file.fileName(), temp_file.filePath());
            re_opentree(temp_file.filePath(), file_cout, rootPath, temp_file.filePath());
            continue;
        }

        const QString suffix = temp_file.completeSuffix().toLower();
        if (suffix != "png" && suffix != "jpeg" && suffix != "jpg") {
            continue;
        }
        ++file_cout;
        emit progressupdate(file_cout);
        emit fileDiscovered(parentPath, temp_file.fileName(), temp_file.filePath());
    }
}

void opentreethread::run()
{
    openprotree(_scr_path, _name, _file_cout, _self);
    if (!_bststop) {
        emit progressfinish(_file_cout);
    }
}

void opentreethread::slotprogresscancle()
{
    _bststop = true;
}
