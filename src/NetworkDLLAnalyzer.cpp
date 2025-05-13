#include "NetworkDLLAnalyzer.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

NetworkDLLAnalyzer::NetworkDLLAnalyzer(QObject *parent)
    : QObject(parent), networkManager(new QNetworkAccessManager(this))
{
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &NetworkDLLAnalyzer::onReplyFinished);
}

void NetworkDLLAnalyzer::analyzeDLL(const QString &dllPath)
{
    QUrl url("http://127.0.0.1:5000/predict");  // Flask 서버 주소
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["dll_path"] = dllPath;
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
