#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

void sendPredictRequest()
{
    // 1. 매니저 생성
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    // 2. 요청 객체 만들기
    QUrl url("http://127.0.0.1:5000/predict"); // Flask 서버 주소
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 3. JSON 데이터 준비
    QJsonObject json;
    json["feature1"] = 1.23;
    json["feature2"] = 4.56;
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    // 4. POST 요청 보내기
    QNetworkReply *reply = manager->post(request, jsonData);

    // 5. 결과 처리
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "서버 응답:" << response;
        } else {
            qDebug() << "에러 발생:" << reply->errorString();
        }
        reply->deleteLater();
        manager->deleteLater(); // 매니저도 같이 삭제
    });
}
