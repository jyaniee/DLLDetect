#ifndef RESULT_H
#define RESULT_H

#include <QString>
#include <QStringList>

class Result {
public:
    int pid;
    QString processName;
    QStringList dllList;
    bool suspicious;

    Result();

    Result(int pid, const QString& processName, const QStringList& dllList, bool suspicious);
};


#endif // RESULT_H
