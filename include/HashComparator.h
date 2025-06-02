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
     std::vector<std::pair<QString, QString>> detectSuspiciousDLLs(const QStringList& dllList);
    size_t getKnownHashCount() const { return knownHashes.size(); }

private:
    QSet<QString> knownHashes;

};

#endif
