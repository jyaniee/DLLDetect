#ifndef DLL_ANALYZER_H
#define DLL_ANALYZER_H

#include <vector>
#include <string>
#include <windows.h>

class DLLAnalyzer {
public:
    DLLAnalyzer();
    ~DLLAnalyzer();

    // 현재 시스템에서 실행 중인 프로세스 목록을 반환
    std::vector<DWORD> GetRunningProcesses();

    // 특정 프로세스의 로드된 모듈 목록을 반환
    std::vector<std::string> GetLoadedModules(DWORD processID);

    // 의심스러운 DLL을 탐지
    bool DetectSuspiciousDLLs(DWORD processID);

    void SetKnownDLLs(const std::vector<std::string>& dllList);

private:
    // 내부 헬퍼 함수들
    bool IsKnownDLL(const std::string& dllName);
    std::vector<std::string> knownDLLs;
};

#endif // DLL_ANALYZER_H
