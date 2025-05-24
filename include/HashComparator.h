#ifndef HASHCOMPARATOR_H
#define HASHCOMPARATOR_H

#include <QString>
#include <QSet>

class HashComparator {
public:
    HashComparator(); // 생성자
    void loadHashList(const QString& filePath);
    bool isKnown(const QString& dllPath);
    QString calculateHash(const QString& dllPath);

private:
    QSet<QString> knownHashes;
};

#endif
