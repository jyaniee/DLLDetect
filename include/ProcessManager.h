#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <vector>
#include <QString>
#include "Result.h"

class ProcessManager
{
public:
    ProcessManager();


    std::vector<Result> getProcessList();
};

#endif
