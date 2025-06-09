#include <windows.h>
#include "DLLAnalyzer.h"
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <vector>
#include <string>

#define PSAPI_VERSION 2

DLLAnalyzer::DLLAnalyzer() {
    // 알려진 DLL 목록 초기화
    knownDLLs = {
        "kernel32.dll",
        "user32.dll",
        "gdi32.dll",
        // 추가적인 시스템 DLL들
    };
}

DLLAnalyzer::~DLLAnalyzer() {}

std::vector<DWORD> DLLAnalyzer::GetRunningProcesses() {
    std::vector<DWORD> processIDs;
    DWORD processes[1024], cbNeeded;

    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        int count = cbNeeded / sizeof(DWORD);
        for (int i = 0; i < count; i++) {
            if (processes[i] != 0) {
                processIDs.push_back(processes[i]);
            }
        }
    }
    return processIDs;
}

std::vector<std::string> DLLAnalyzer::GetLoadedModules(DWORD processID) {
    std::vector<std::string> modules;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess) {
        HMODULE hMods[1024];
        DWORD cbNeeded;
        if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                char szModName[MAX_PATH];
                if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
                    modules.push_back(std::string(szModName));
                }
            }
        }
        CloseHandle(hProcess);
    }
    return modules;
}

bool DLLAnalyzer::DetectSuspiciousDLLs(DWORD processID) {
    auto modules = GetLoadedModules(processID);
    for (const auto& mod : modules) {
        std::string dllName = mod.substr(mod.find_last_of("\\") + 1);
        if (!IsKnownDLL(dllName)) {
            std::cout << "Suspicious DLL detected: " << dllName << " in process ID: " << processID << std::endl;
            return true;
        }
    }
    return false;
}

bool DLLAnalyzer::IsKnownDLL(const std::string& dllName) {
    for (const auto& known : knownDLLs) {
        if (_stricmp(known.c_str(), dllName.c_str()) == 0) {
            return true;
        }
    }
    return false;
}
