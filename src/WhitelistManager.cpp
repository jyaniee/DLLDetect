#include "WhitelistManager.h"
#include <QFile>
#include <QTextStream>

bool WhitelistManager::loadWhitelist(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed().toLower();
        if (!line.isEmpty())
            whitelist.insert(line);
    }
    return true;
}

bool WhitelistManager::isWhitelisted(const QString& dllName) const {
    return whitelist.contains(dllName.toLower());
}
