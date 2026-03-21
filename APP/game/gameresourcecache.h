#ifndef GAMERESOURCECACHE_H
#define GAMERESOURCECACHE_H

#include <QPixmap>
#include <QStringList>

class GameResourceCache
{
public:
    static QPixmap pixmap(const QString &path);
    static void preload(const QStringList &paths);
    static void clear();
};

#endif // GAMERESOURCECACHE_H
