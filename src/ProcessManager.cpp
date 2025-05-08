#include "ProcessManager.h"
#include "DLLAnalyzer.h"
#include <QString>  // QString 헤더 추가
#include <windows.h>
#include <tlhelp32.h>
#include "Result.h"  // Result.h 파일 포함

ProcessManager::ProcessManager() {}

std::vector<Result> ProcessManager::getProcessList()
{
    std::vector<Result> processList;
    DLLAnalyzer analyzer;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return processList;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe)) {
        do {

            QString processName = QString::fromWCharArray(pe.szExeFile);
            DWORD pid = pe.th32ProcessID;

            std::vector<std::string> dllList = analyzer.GetLoadedModules(pid);
            int dllCount = static_cast<int>(dllList.size());

            Result result(pe.th32ProcessID, processName, {}, false);
            processList.push_back(result);

        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return processList;
}
