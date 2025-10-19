#include "DebugEventMonitor.h"
#include <softpub.h>
#include <wintrust.h>
#include <QMetaType>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Wintrust.lib")
#pragma comment(lib, "Crypt32.lib")

DebugEventMonitor::DebugEventMonitor(QObject* p): QThread(p) { qRegisterMetaType<quint64>("quint64");}
DebugEventMonitor::~DebugEventMonitor(){ stopMonitoring(); }

/*bool DebugEventMonitor::enableDebugPrivilege(){
    HANDLE tok{};
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok)) return false;
    LUID luid{}; if(!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)){ CloseHandle(tok); return false; }
    TOKEN_PRIVILEGES tp{}; tp.PrivilegeCount=1; tp.Privileges[0].Luid=luid; tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(tok, FALSE, &tp, sizeof(tp), nullptr, nullptr); CloseHandle(tok);
    return GetLastError()==ERROR_SUCCESS;
}*/
bool DebugEventMonitor::enableDebugPrivilege(){
    HANDLE tok = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok)) {
        emit logLine(QString("[!] OpenProcessToken failed err=%1").arg(GetLastError()));
        return false;
    }

    LUID luid{};
    if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) {
        DWORD err = GetLastError();
        CloseHandle(tok);
        emit logLine(QString("[!] LookupPrivilegeValueW failed err=%1").arg(err));
        return false;
    }

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(tok, FALSE, &tp, sizeof(tp), nullptr, nullptr)) {
        DWORD err = GetLastError();
        CloseHandle(tok);
        emit logLine(QString("[!] AdjustTokenPrivileges failed (call) err=%1").arg(err));
        return false;
    }

    DWORD adjustErr = GetLastError();
    CloseHandle(tok);
    if (adjustErr != ERROR_SUCCESS) {
        emit logLine(QString("[!] AdjustTokenPrivileges returned err=%1").arg(adjustErr));
        return false;
    }

    emit logLine("[*] SeDebugPrivilege successfully enabled (or already enabled)");
    return true;
}
//수정

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
    emit logLine("[*] Stop requested (waiting for thread exit");
    if(isRunning()){ wait(500); }
        wait(1000);


//    if(m_pid){
//    DebugActiveProcessStop(m_pid); // 디버깅 이벤트 해제
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
    ULONGLONG lastCreateMs=0; ULONG_PTR lastStart= 0;
/*
    while(m_running){
        if(!WaitForDebugEvent(&de, 200)) { continue; }
        DWORD cont = DBG_CONTINUE;

        switch(de.dwDebugEventCode){
            case CREATE_THREAD_DEBUG_EVENT:{  // 디버깅 이벤트 코드가 CREATE_THREAD_DEBUG_EVENT일 때
                lastStart = (ULONG_PTR)de.u.CreateThread.lpStartAddress;
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
*/
    while(m_running){
        if(!WaitForDebugEvent(&de, 200)) { continue; }
        DWORD cont = DBG_CONTINUE;

        switch(de.dwDebugEventCode){
        case CREATE_THREAD_DEBUG_EVENT: {
            lastStart = (ULONG_PTR)de.u.CreateThread.lpStartAddress;
            lastCreateMs = nowMs();
            emit logLine(QString("[THREAD] tid=%1 start=%2").arg(de.dwThreadId).arg((qulonglong)lastStart, 0, 16));
            break;
        }
        case LOAD_DLL_DEBUG_EVENT: {
            QString path = pathFromHandle(de.u.LoadDll.hFile);
            emit dllLoaded(path, (quint64)de.u.LoadDll.lpBaseOfDll);

            int pathRisk = scorePathRisk(path);
            int dt = (int)(nowMs() - lastCreateMs);
            bool linked = (lastCreateMs && dt>=0 && dt<=(int)WINDOW_MS);

            int score = (linked?60:20) + pathRisk;
            emit suspicionScore(score, path, (quint64)lastStart, linked?dt:-1);

            if(linked){
                emit logLine(QString("[LINK] new thread -> dll load in %1ms (start=%2)")
                                 .arg(dt).arg((qulonglong)lastStart, 0, 16));
            }

            // 의심 임계값
            if(score >= 80){
                emit alert(m_autoKill? "terminate" : "warn", score, path);

                if(m_autoKill){
                    // 가능한 PID 결정: de.dwProcessId 우선, 없으면 m_pid
                    DWORD targetPid = de.dwProcessId ? de.dwProcessId : m_pid;
                    emit logLine(QString("[*] Auto-kill enabled. Trying to terminate pid=%1 (score=%2) dll=%3")
                                     .arg((qulonglong)targetPid).arg(score).arg(path));

                    // 1) 우선 요청 권한은 PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION
                    DWORD desiredAccess = PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION;
                    HANDLE hp = OpenProcess(desiredAccess, FALSE, targetPid);
                    if(!hp){
                        DWORD err = GetLastError();
                        emit logLine(QString("[!] OpenProcess failed (pid=%1) err=%2 - trying PROCESS_ALL_ACCESS")
                                         .arg((qulonglong)targetPid).arg(err));

                        // 2) 테스트용으로 PROCESS_ALL_ACCESS 재시도 (주의: 관리자/SeDebug 필요)
                        hp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
                        if(!hp){
                            DWORD err2 = GetLastError();
                            emit logLine(QString("[!] OpenProcess(PROCESS_ALL_ACCESS) also failed (pid=%1) err=%2")
                                             .arg((qulonglong)targetPid).arg(err2));
                        } else {
                            emit logLine(QString("[*] OpenProcess(PROCESS_ALL_ACCESS) succeeded (pid=%1)").arg((qulonglong)targetPid));
                        }
                    }

                    if(hp){
                        // TerminateProcess 시도 및 결과 로깅
                        emit logLine(QString("[*] Calling TerminateProcess on pid=%1").arg((qulonglong)targetPid));

                        if(!TerminateProcess(hp, 1)){
                            DWORD terr = GetLastError();
                            emit logLine(QString("[!] TerminateProcess failed (pid=%1) err=%2").arg((qulonglong)targetPid).arg(terr));
                        } else {
                            emit logLine(QString("[+] TerminateProcess succeeded (pid=%1)").arg((qulonglong)targetPid));
                        }

                        CloseHandle(hp);
                    }

                    // 디버깅 세션 정리: DebugActiveProcessStop 시도
                    if(m_pid){
                        if(!DebugActiveProcessStop(m_pid)){
                            DWORD derr = GetLastError();
                            emit logLine(QString("[!] DebugActiveProcessStop failed (pid=%1) err=%2").arg((qulonglong)m_pid).arg(derr));
                        } else {
                            emit logLine(QString("[*] DebugActiveProcessStop called for pid=%1").arg((qulonglong)m_pid));
                        }
                    }

                    // 종료 시 루프 중지 (프로세스가 죽었건 안 죽었건 더이상 감시할 필요 없음)
                    m_running = false;
                } // if m_autoKill
            } // if score>=80

            if(de.u.LoadDll.hFile && de.u.LoadDll.hFile != INVALID_HANDLE_VALUE)
                CloseHandle(de.u.LoadDll.hFile);

            break;
        }
        case EXIT_PROCESS_DEBUG_EVENT:{
            emit logLine(QString("[EXIT] pid=%1").arg((qulonglong)de.dwProcessId));
            m_running = false;
            break;
        }
        } // switch

        // 이벤트 계속 처리
        ContinueDebugEvent(de.dwProcessId, de.dwThreadId, cont);
    } // while
    emit logLine("[*] Monitor loop ended - cleaning up DebugActiveProcessStop");
    if(m_pid) {
        if (!DebugActiveProcessStop(m_pid)){
            emit logLine(QString("[!] DebugActiveProcessStop failed err=%1").arg(GetLastError()));
        } else {
            emit logLine(QString("[*] DebugActiveProcessStop succesful for pid=%1").arg((qulonglong)m_pid));
        }
    }
}
