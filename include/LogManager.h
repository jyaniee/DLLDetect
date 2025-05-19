// LogManager.h
#ifndef LOGMANAGER_H
#define LOGMANAGER_H
#include "Result.h"
#include <QString>

class LogManager {
public:
    // LogManager.h
    static void writeLog(const QString& dllPath, int prediction, const QString& source,
                         const std::vector<Result>& cachedResults);


};

#endif // LOGMANAGER_H
