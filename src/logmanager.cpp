// LogManager.cpp
#include "LogManager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>
#include "Result.h"

void LogManager::writeLog(const QString& dllPath, int prediction, const QString& source,
                          const std::vector<Result>& cachedResults) {
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString filePath = desktopPath + "/log.csv";

    QFile file(filePath);
    bool fileExists = file.exists();

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        if (!fileExists) {
            out << "timestamp,PID,dll_path,result\n";
        }

        QString pid = "Unknown";
        for (const Result &res : cachedResults) {
            if (res.dllList.contains(dllPath)) {
                pid = QString::number(res.pid);
                break;
            }
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString resultMsg;

        if (prediction == 0 && source == "whitelist") {
            resultMsg = "정상 DLL입니다 (화이트리스트)";
        } else if (prediction == 0) {
            resultMsg = "정상 DLL입니다";
        } else {
            resultMsg = "비정상 DLL입니다";
        }

        out << QString("%1,%2,%3,%4\n").arg(timestamp, pid, dllPath, resultMsg);
        file.close();
    }
}
