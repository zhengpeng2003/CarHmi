#pragma once

#include <QString>
#include <QtGlobal>

struct AppInfo
{
    quint8 Id = 0;
    QString Name;
    QString Path;
    QString Log;

    AppInfo() = default;
    AppInfo(quint8 id, const QString &name, const QString &path, const QString &log)
        : Id(id), Name(name), Path(path), Log(log)
    {
    }
};
