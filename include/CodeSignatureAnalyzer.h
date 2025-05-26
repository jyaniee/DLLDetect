#ifndef CODESIGNATUREANALYZER_H
#define CODESIGNATUREANALYZER_H
#pragma once
#include <QString>
#include <string>

class CodeSignatureAnalyzer {
public:
    bool isSuspicious(QString& dllPath);
    QString lastReason();

private:
    QString reason;
};
#endif
