#include "gameresourcecache.h"

#include <QHash>

namespace {
QHash<QString, QPixmap> &cache()
{
    static QHash<QString, QPixmap> s_cache;
    return s_cache;
}
}

QPixmap GameResourceCache::pixmap(const QString &path)
{
    auto &c = cache();
    const auto it = c.constFind(path);
    if (it != c.constEnd()) {
        return it.value();
    }
    QPixmap pixmap(path);
    c.insert(path, pixmap);
    return pixmap;
}

void GameResourceCache::preload(const QStringList &paths)
{
    for (const QString &path : paths) {
        pixmap(path);
    }
}

void GameResourceCache::clear()
{
    cache().clear();
}
