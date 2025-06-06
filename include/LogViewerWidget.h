#ifndef LOGVIEWERWIDGET_H
#define LOGVIEWERWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

class LogViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit LogViewerWidget(QWidget *parent = nullptr);

    void loadLogFile(const QString& method = "");
private:
    QTableWidget* table;
    QPushButton* whitelistBtn;
    QPushButton* mlBtn;
    QPushButton* hashBtn;
    QPushButton* signatureBtn;
};

#endif // LOGVIEWERWIDGET_H
