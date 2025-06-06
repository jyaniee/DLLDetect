#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "Result.h"
#include <QString>
#include <vector>
#include <QSet>
#include <QStringList>

class LogManager {
public:
    // 단일 DLL 로그 저장
    static void writeLog(const QString& dllPath,
                         int prediction,
                         const QString& source,
                         const std::vector<Result>& cachedResults,
                         const QString& methodName);

    // 전체 DLL 결과 일괄 저장
    static void writeBulkLog(const QStringList& dllList,
                             const QSet<QString>& suspiciousSet,
                             const std::vector<Result>& cachedResults,
                             const QString& methodName,
                             const QString& sourceTag);
};

#endif // LOGMANAGER_H
