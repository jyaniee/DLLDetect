#ifndef NETWORK_DLL_ANALYZER_H
#define NETWORK_DLL_ANALYZER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkDLLAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit NetworkDLLAnalyzer(QObject *parent = nullptr);
    void analyzeDLL(const QString &dllPath);

signals:
    void analysisFinished(const QString &result);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
};

#endif // NETWORK_DLL_ANALYZER_H
