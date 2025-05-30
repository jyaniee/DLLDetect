#include <CodeSignatureAnalyzer.h>
#include <windows.h>
#include <wintrust.h>
#include <Softpub.h>
#include <QString>

#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "crypt32.lib")

bool CodeSignatureAnalyzer::isSuspicious(QString& dllPath) {
    std::wstring path = dllPath.toStdWString();

    WINTRUST_FILE_INFO fileInfo = {};
    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.pcwszFilePath = path.c_str();

    WINTRUST_DATA trustData = {};
    trustData.cbStruct = sizeof(WINTRUST_DATA);
    trustData.dwUIChoice = WTD_UI_NONE;
    trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    trustData.dwUnionChoice = WTD_CHOICE_FILE;
    trustData.pFile = &fileInfo;
    trustData.dwStateAction = 0;
    trustData.dwProvFlags = WTD_SAFER_FLAG;

    GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    LONG result = WinVerifyTrust(nullptr, &policyGUID, &trustData);

    if (result != ERROR_SUCCESS) {
        reason = "서명이 없거나 신뢰할 수 없음.";
        return true;
    }

    reason = "정상 서명됨.";
    return false;
}

QString CodeSignatureAnalyzer::lastReason(){
    return reason;
}
