#ifndef LOGVIEWERWIDGET_H
#define LOGVIEWERWIDGET_H

#include <QWidget>
#include <QTableWidget>

class LogViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit LogViewerWidget(QWidget *parent = nullptr);

    void loadLogFile();

private:
    QTableWidget* table;
};

#endif // LOGVIEWERWIDGET_H
