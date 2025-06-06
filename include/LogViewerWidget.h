#ifndef LOGVIEWERWIDGET_H
#define LOGVIEWERWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <Qcombobox>
class LogViewerWidget : public QWidget {
    Q_OBJECT

protected:
    void showEvent(QShowEvent* event) override;
public:
    explicit LogViewerWidget(QWidget *parent = nullptr);

    void loadLogFile(const QString& method = "");
private:
    void populateLogFileList();
    QTableWidget* table;
    QPushButton* whitelistBtn;
    QPushButton* mlBtn;
    QPushButton* hashBtn;
    QPushButton* signatureBtn;
    QComboBox* logComboBox;
};


#endif // LOGVIEWERWIDGET_H
