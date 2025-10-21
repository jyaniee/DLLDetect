#include "NetworkDLLAnalyzer.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>


NetworkDLLAnalyzer::NetworkDLLAnalyzer(QObject *parent)
    : QObject(parent), networkManager(new QNetworkAccessManager(this))
{
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &NetworkDLLAnalyzer::onReplyFinished);
}


void NetworkDLLAnalyzer::analyzeDLLs(const QStringList &dllList)
{

    QUrl url("http://107.21.184.244:5000/bulk_predict");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonArray filesArray;

    for (const QString &dllPath : dllList) {
        QFile file(dllPath);
        if (!file.open(QIODevice::ReadOnly)) {

            emit analysisFinished("Error: C++에서 파일을 읽을 수 없습니다: " + dllPath);
            continue;
        }


        QByteArray fileData = file.readAll();
        file.close();


        QByteArray base64Data = fileData.toBase64();


        QJsonObject fileObject;
        fileObject["file_name"] = QFileInfo(dllPath).fileName();
        fileObject["file_content"] = QString::fromUtf8(base64Data);


        filesArray.append(fileObject);
    }

    if (filesArray.isEmpty()) {

        emit analysisFinished("Error: 분석할 파일이 없습니다.");
        return;
    }


    QJsonObject json;
    json["files"] = filesArray;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    networkManager->post(request, data);
}



void NetworkDLLAnalyzer::analyzeDLL(const QString &dllPath)
{

    QUrl url("http://107.21.184.244:5000/predict");  // Flask 서버 주소
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QFile file(dllPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit analysisFinished("Error: C++에서 파일을 읽을 수 없습니다: " + dllPath);
        return;
    }


    QByteArray fileData = file.readAll();
    file.close();


    QByteArray base64Data = fileData.toBase64();


    QJsonObject json;
    json["file_name"] = QFileInfo(dllPath).fileName();
    json["file_content"] = QString::fromUtf8(base64Data);

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    networkManager->post(request, data);
}

void NetworkDLLAnalyzer::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString response = QString::fromUtf8(reply->readAll());
        emit analysisFinished(response);
    } else {
        emit analysisFinished("Error: " + reply->errorString());
    }
    reply->deleteLater();
}
