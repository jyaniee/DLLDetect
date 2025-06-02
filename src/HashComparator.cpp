#include "HashComparator.h"
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDebug>
#include <QFileInfo>

HashComparator::HashComparator() {
    // 생성자에서 초기화 필요시 작성
}

void HashComparator::loadHashList(const QString& filePath) {
    knownHashes.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                knownHashes.insert(line.toLower());
            }
        }
        file.close();
    } else {
        qWarning() << "해시 목록 파일을 열 수 없습니다:" << filePath;
    }
}

QString HashComparator::calculateHash(const QString& dllPath) {
    QFile file(dllPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "파일 열기 실패:" << dllPath;
        return "";
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        qWarning() << "해시 계산 실패:" << dllPath;
        return "";
    }

    return hash.result().toHex();
}

bool HashComparator::isKnown(const QString& dllPath) {
    QString hash = calculateHash(dllPath);
    return knownHashes.contains(hash);
}

// 🔴 여기에 추가!
std::vector<std::pair<QString, QString>> HashComparator::detectSuspiciousDLLs(const QStringList& dllList) {
    std::vector<std::pair<QString, QString>> suspiciousDLLs;

    for (const QString &dllPath : dllList) {
        QString hash = calculateHash(dllPath).trimmed().toLower();
        bool isKnownResult = isKnown(dllPath);

        qDebug() << "DLL 경로:" << dllPath << "해시:" << hash;
        qDebug() << "isKnown 결과:" << isKnownResult;

        if (isKnownResult) {
            QString dllName = QFileInfo(dllPath).fileName();
            suspiciousDLLs.emplace_back(dllName, dllPath);
        }
    }

    return suspiciousDLLs;
}
