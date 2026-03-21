#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H
#include<QString>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <cstdio>
#include <QDebug>
class Screen
{
public:
    Screen(){};
    quint32 Screen_Height;
    quint32 Screen_Width;

};
class AppInfo
{
public:
    quint8 Id;
    QString Name;
    QString Path;
    QString Log;
    AppInfo() {}   // ★ 必须有

    AppInfo(quint8 _id,
            const QString& _name,
            const QString& _path,
            const QString& _log)
        : Id(_id), Name(_name), Path(_path), Log(_log)
    {}

};


#endif // GLOBALCONFIG_H
