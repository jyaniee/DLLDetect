#include "LogManager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>
#include "Result.h"

namespace {
QString buildLogFilePath(const QString& pid, const QString& processName, const QString& methodName) {
    QString fileName = QString("%1-%2-%3.csv").arg(pid, processName, methodName);
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    return desktopPath + "/" + fileName;
}

QString generateResultMessage(int prediction, const QString& source) {
    if (prediction == 0 && source == "whitelist") {
        return "정상 DLL입니다 (화이트리스트)";
    } else if (prediction == 0) {
        return "정상 DLL입니다";
    } else {
        return "비정상 DLL입니다";
    }
}
}

void LogManager::writeLog(const QString& dllPath,
                          int prediction,
                          const QString& source,
                          const std::vector<Result>& cachedResults,
                          const QString& methodName,
                          const QString& targetPid)
{
    if (dllPath.isEmpty() || methodName.isEmpty() || targetPid.isEmpty()) {
        qDebug() << "[로그 스킵됨] 조건 미충족:" << dllPath;
        return;
    }

    QString processName = "unknown";
    for (const Result& res : cachedResults) {
        if (res.dllList.contains(dllPath) && QString::number(res.pid) == targetPid) {
            processName = res.processName;
            break;
        }
    }

    QString filePath = buildLogFilePath(targetPid, processName, methodName);
    QFile file(filePath);
    bool fileExists = file.exists();

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        if (!fileExists) {
            out << "timestamp,PID,dll_path,result\n";
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString resultMsg = generateResultMessage(prediction, source);
        QString logLine = QString("%1,%2,%3,%4\n").arg(timestamp, targetPid, dllPath, resultMsg);
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
                              const QString& sourceTag,
                              const QString& targetPid)
{
    for (const QString& dllPath : dllList) {
        int prediction = suspiciousSet.contains(dllPath) ? 1 : 0;
        writeLog(dllPath, prediction, sourceTag, cachedResults, methodName, targetPid);
    }
}
