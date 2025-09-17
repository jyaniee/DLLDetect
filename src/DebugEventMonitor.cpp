#include "DebugEventMonitor.h"
#include <softpub.h>
#include <wintrust.h>
#include <QMetaType>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "Crypt32.lib")

DebugEventMonitor::DebugEventMonitor(QObject* p): QThread(p) { qRegisterMetaType<quint64>("quint64");}
DebugEventMonitor::~DebugEventMonitor(){ stopMonitoring(); }

bool DebugEventMonitor::enableDebugPrivilege(){
    HANDLE tok{};
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok)) return false;
    LUID luid{}; if(!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)){ CloseHandle(tok); return false; }
    TOKEN_PRIVILEGES tp{}; tp.PrivilegeCount=1; tp.Privileges[0].Luid=luid; tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(tok, FALSE, &tp, sizeof(tp), nullptr, nullptr); CloseHandle(tok);
    return GetLastError()==ERROR_SUCCESS;
}

QString DebugEventMonitor::pathFromHandle(HANDLE hFile){
    if(!hFile || hFile==INVALID_HANDLE_VALUE) return {};
    wchar_t buf[32768]; DWORD n=GetFinalPathNameByHandleW(hFile, buf, 32768, FILE_NAME_NORMALIZED);
    if(!n) return{};
    QString s = QString::fromWCharArray(buf, int(n));
    if(s.startsWith(u"\\\\?\\")) s = s.mid(4);
    return s;
}

int DebugEventMonitor::scorePathRisk(const QString& p){
    int score=0; const QString L = p.toLower();
    if(L.isEmpty()) score += 50;
    if(L.contains("\\users\\")) score+=20;
    if(L.contains("\\downloads\\")) score+=30;
    if(L.contains("\\appdata\\local\\temp\\")) score+=40;
    if(L.endsWith(".dll")==false) score+=10;
    if(score>100) score=100; if(score<0) score=0;
    return score;
}

ULONGLONG DebugEventMonitor::nowMs(){
    FILETIME ft; GetSystemTimePreciseAsFileTime(&ft);
    ULARGE_INTEGER u; u.LowPart=ft.dwLowDateTime; u.HighPart=ft.dwHighDateTime;
    return u.QuadPart/10000ULL;
}

void DebugEventMonitor::startMonitoring(DWORD pid, bool autoKill){
    if(isRunning()) stopMonitoring();
    m_pid = pid; m_autoKill = autoKill; m_running = true; start();
}

void DebugEventMonitor::stopMonitoring(){
    m_running = false;
    if(isRunning()){ wait(500); }
    if(m_pid){
        DebugActiveProcessStop(m_pid); // 디버깅 이벤트 해제
    }
}

void DebugEventMonitor::run(){
    if(!m_pid) return;
    enableDebugPrivilege();
    if(!DebugActiveProcess(m_pid)){
        emit logLine(QString("[!] DebugActiveProcess failed: %1").arg(GetLastError()));
        return;
    }
    emit logLine(QString("[+] monitoring pid %1").arg(m_pid));

    DEBUG_EVENT de{};
    const ULONGLONG WINDOW_MS = 300;
    ULONGLONG lastCreateMs=0; LPVOID lastStart=nullptr;

    while(m_running){
        if(!WaitForDebugEvent(&de, 200)) { continue; }
        DWORD cont = DBG_CONTINUE;

        switch(de.dwDebugEventCode){
            case CREATE_THREAD_DEBUG_EVENT:{  // 디버깅 이벤트 코드가 CREATE_THREAD_DEBUG_EVENT일 때
                lastStart = de.u.CreateThread.lpStartAddress;
                lastCreateMs = nowMs();
                emit logLine(QString("[THREAD] tid=%1 start=%2").arg(de.dwThreadId).arg((quint64)lastStart, 0, 16));
                break;
            }
            case LOAD_DLL_DEBUG_EVENT:{ // 디버깅 이벤트 코드가 LOAD_DLL_DEBUG_EVENT일 때
                QString path = pathFromHandle(de.u.LoadDll.hFile);
                emit dllLoaded(path, (quint64)de.u.LoadDll.lpBaseOfDll);

                int pathRisk = scorePathRisk(path);
                int dt = (int)(nowMs() - lastCreateMs);
                bool linked = (lastCreateMs && dt>=0 && dt<=(int)WINDOW_MS);

                int score = (linked?60:20) + pathRisk;
                emit suspicionScore(score, path, (quint64)lastStart, linked?dt:-1);

                if(linked){
                    emit logLine(QString("[LINK] new thread -> dll load in %1ms (start=%2)")
                                .arg(dt).arg((quint64)lastStart, 0, 16));
                }
                if(score>=80){
                    emit alert(m_autoKill? "terminate" : "warn", score, path);
                    if(m_autoKill){
                        if(HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, de.dwProcessId)){  // 프로세스 종료
                            TerminateProcess(hp, 0); CloseHandle(hp);
                        }
                    }
                }
                if(de.u.LoadDll.hFile && de.u.LoadDll.hFile!=INVALID_HANDLE_VALUE)
                    CloseHandle(de.u.LoadDll.hFile);
                break;
            }
            case EXIT_PROCESS_DEBUG_EVENT:{
                emit logLine(QString("[EXIT] pid=%1").arg(de.dwProcessId));
                m_running=false;
                break;
            }
        }
            ContinueDebugEvent(de.dwProcessId, de.dwThreadId, cont);
    }
}
