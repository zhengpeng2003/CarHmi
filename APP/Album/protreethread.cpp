#include "protreethread.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include "protreeitem.h"

protreethread::protreethread(const QString &scr_path, const QString &dis_path,
                             QTreeWidgetItem* parent_item, int file_cout, QTreeWidget *self, QTreeWidgetItem *root)
    : QThread(nullptr),
      _scr_path(scr_path),
      _dis_path(dis_path),
      _parent_item(parent_item),
      _file_cout(file_cout),
      _self(self),
      _root(root),
      _bststop(false)
{
}

protreethread::~protreethread() = default;

void protreethread::creatprotreethread(const QString &scr_path, const QString &dis_path, const QString &parentPath, int file_cout)
{
    if (_bststop) {
        return;
    }

    const bool needcopy = (scr_path != dis_path);
    QDir import_dir(scr_path);
    import_dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    import_dir.setSorting(QDir::Name);
    const QFileInfoList list = import_dir.entryInfoList();

    for (const QFileInfo &fileinfo : list) {
        if (_bststop) {
            return;
        }

        if (fileinfo.isDir()) {
            ++file_cout;
            emit signalupdateprogress(file_cout);
            QDir subDist(dis_path);
            const QString subDistPath = subDist.absoluteFilePath(fileinfo.fileName());
            QDir subDistDir(subDistPath);
            if (!subDistDir.exists() && !subDistDir.mkpath(subDistPath)) {
                continue;
            }
            emit signalDirectoryDiscovered(parentPath, fileinfo.fileName(), subDistPath);
            creatprotreethread(fileinfo.absoluteFilePath(), subDistPath, subDistPath, file_cout);
            continue;
        }

        const QString suffix = fileinfo.completeSuffix().toLower();
        if (suffix != "png" && suffix != "jpeg" && suffix != "jpg") {
            continue;
        }

        ++file_cout;
        emit signalupdateprogress(file_cout);
        QString disFilePath = fileinfo.absoluteFilePath();
        if (needcopy) {
            QDir dis_dir(dis_path);
            disFilePath = dis_dir.absoluteFilePath(fileinfo.fileName());
            if (!QFile::copy(fileinfo.absoluteFilePath(), disFilePath)) {
                continue;
            }
        }
        emit signalFileDiscovered(parentPath, fileinfo.fileName(), disFilePath);
    }
}

void protreethread::run()
{
    QString rootPath;
    if (auto *item = dynamic_cast<protreeitem*>(_root)) {
        rootPath = item->GetPath();
    }
    creatprotreethread(_scr_path, _dis_path, rootPath, _file_cout);
    if (_bststop) {
        emit signalCancelledCleanup(rootPath);
        return;
    }
    emit signalfishprogress(_file_cout);
}

void protreethread::slotcancle()
{
    _bststop = true;
}
