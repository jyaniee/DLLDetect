#ifndef DEBUGEVENTMONITOR_H
#define DEBUGEVENTMONITOR_H

#pragma once
#include <QObject>
#include <QThread>
#include <QString>
#include <windows.h>

class DebugEventMonitor : public QThread {
    Q_OBJECT
public:
    explicit DebugEventMonitor(QObject* parent=nullptr);
    ~DebugEventMonitor() override;

    void startMonitoring(DWORD pid, bool autoKill=false);
    void stopMonitoring();

signals:
    void logLine(const QString& line); // 콘솔/로그 뿌리기
    void dllLoaded(const QString& path, quint64 base); // DLL 로드 이벤트
    void suspicionScore(int score, const QString& path, quint64 startAddr, int dtMs);
    void alert(const QString& action, int score, const QString& path); // 차단/경고

protected:
    void run() override;

private:
    bool enableDebugPrivilege();
    QString pathFromHandle(HANDLE hFile);
    int scorePathRisk(const QString& pathLower);
    ULONGLONG nowMs();

    volatile bool m_running{false};
    DWORD m_pid{0};
    bool m_autoKill{false};
};


#endif // DEBUGEVENTMONITOR_H
