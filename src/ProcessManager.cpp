#include "ProcessManager.h"
#include "DLLAnalyzer.h"
#include <QString>
#include <windows.h>
#include <tlhelp32.h>
#include "Result.h"

// 생성자: QObject를 부모로 갖도록 초기화
ProcessManager::ProcessManager(QObject* parent)
    : QObject(parent) {}

// runScan(): 실행 중인 프로세스를 스캔하여 DLL 목록과 함께 결과를 시그널로 전송

void ProcessManager::runScan()
{
    std::vector<Result> processList;
    DLLAnalyzer analyzer;

    // 시스템의 실행 중인 프로세스 목록 스냅샷 생성
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        emit scanFinished(processList);  // 실패했더라도 빈 리스트 emit
        return;

    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    // 첫 번째 프로세스부터 순회
    if (Process32First(hSnapshot, &pe)) {
        do {
            QString processName = QString::fromWCharArray(pe.szExeFile);
            DWORD pid = pe.th32ProcessID;

            // DLLAnalyzer를 이용해 해당 프로세스의 DLL 목록을 가져옴
            std::vector<std::string> dllListRaw = analyzer.GetLoadedModules(pid);
            QStringList dllList;
            for (const std::string& s : dllListRaw) {
                dllList.append(QString::fromStdString(s));
            }

            // Result 객체 생성 후 리스트에 추가
            Result result(pe.th32ProcessID, processName, dllList, false);
            processList.push_back(result);

        } while (Process32Next(hSnapshot, &pe));
    }

    // 핸들 닫기
    CloseHandle(hSnapshot);
    // 모든 결과를 시그널로 전송
    emit scanFinished(processList);
}
