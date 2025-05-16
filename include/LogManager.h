// LogManager.h
#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QString>

class LogManager {
public:
    static void writeLog(const QString& pid, const QString& dllPath, int prediction, const QString& source);
};

#endif // LOGMANAGER_H
