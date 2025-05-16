#pragma once
#include <QString>
#include <QSet>

class WhitelistManager {
public:
    bool loadWhitelist(const QString& filePath);
    bool isWhitelisted(const QString& dllName) const;

private:
    QSet<QString> whitelist;
};
