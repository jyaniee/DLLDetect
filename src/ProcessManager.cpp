#include "ProcessManager.h"
#include "DLLAnalyzer.h"
#include <QString>  // QString 헤더 추가
#include <windows.h>
#include <tlhelp32.h>
#include "Result.h"  // Result.h 파일 포함

ProcessManager::ProcessManager(QObject* parent) : QObject(parent) {}

void ProcessManager::runScan()
{
    std::vector<Result> processList;
    DLLAnalyzer analyzer;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE){
        emit scanFinished(processList);
            return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {

            QString processName = QString::fromWCharArray(pe.szExeFile);
            DWORD pid = pe.th32ProcessID;

            std::vector<std::string> dllListRaw = analyzer.GetLoadedModules(pid);
            //int dllCount = static_cast<int>(dllList.size());
            QStringList dllList;
            for(const std::string& s : dllListRaw){
                dllList.append(QString::fromStdString(s));
            }
            Result result(pe.th32ProcessID, processName, dllList, false);
            processList.push_back(result);

        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    emit scanFinished(processList);
}
