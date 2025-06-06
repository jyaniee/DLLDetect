#include "LogManager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>

void LogManager::writeLog(const QString& dllPath,
                          int prediction,
                          const QString& source,
                          const std::vector<Result>& cachedResults,
                          const QString& methodName)
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString filePath = desktopPath + QString("/log_%1.csv").arg(methodName);

    QFile file(filePath);
    bool fileExists = file.exists();

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        if (!fileExists) {
            out << "timestamp,PID,dll_path,result\n";
        }

        QString pid = "Unknown";
        for (const Result& res : cachedResults) {
            if (res.dllList.contains(dllPath)) {
                pid = QString::number(res.pid);
                break;
            }
        }

        QString resultMsg;
        if (prediction == 0 && source == "whitelist") {
            resultMsg = "정상 DLL입니다 (화이트리스트)";
        } else if (prediction == 0) {
            resultMsg = "정상 DLL입니다";
        } else {
            resultMsg = "비정상 DLL입니다";
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString logLine = QString("%1,%2,%3,%4\n").arg(timestamp, pid, dllPath, resultMsg);
        out << logLine;
        file.close();

        qDebug() << "[로그 저장 완료]" << logLine.trimmed();
    } else {
        qWarning() << "[로그 저장 실패]" << filePath;
    }
}

void LogManager::writeBulkLog(const QStringList& dllList,
                              const QSet<QString>& suspiciousSet,
                              const std::vector<Result>& cachedResults,
                              const QString& methodName,
                              const QString& sourceTag)
{
    for (const QString& dllPath : dllList) {
        int prediction = suspiciousSet.contains(dllPath) ? 1 : 0;
        writeLog(dllPath, prediction, sourceTag, cachedResults, methodName);
    }
}
