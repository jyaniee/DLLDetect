#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <vector>
#include <QString>
#include "Result.h"

class ProcessManager : public QObject
{
    Q_OBJECT

public:
    explicit ProcessManager(QObject* parent = nullptr);
    void runScan();

signals:
    void scanFinished(const std::vector<Result>& results);
};

#endif
