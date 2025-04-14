#include "Result.h"

Result::Result()
    : pid(0), processName(""), suspicious(false){}

Result::Result(int pid, const QString& processName, const QStringList& dllList, bool suspicious)
    : pid(pid), processName(processName), dllList(dllList), suspicious(suspicious){}
