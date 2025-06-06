#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "Result.h"
#include <QString>
#include <vector>
#include <QSet>
#include <QStringList>

class LogManager {
public:
    static void writeLog(const QString& dllPath, int prediction, const QString& source,
                         const std::vector<Result>& cachedResults, const QString& methodName, const QString& pid);

    static void writeBulkLog(const QStringList& dllList, const QSet<QString>& suspiciousSet,
                             const std::vector<Result>& cachedResults, const QString& methodName,
                             const QString& sourceTag, const QString& targetPid);

private:
    static QString generateLogFilePath(const QString& pid, const QString& processName, const QString& methodName);
    static QString formatLogLine(const QString& timestamp, const QString& pid, const QString& dllPath, const QString& resultMsg);
};


#endif // LOGMANAGER_H
