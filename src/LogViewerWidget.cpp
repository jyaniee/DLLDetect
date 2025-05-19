#include "LogViewerWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

LogViewerWidget::LogViewerWidget(QWidget *parent)
    : QWidget(parent), table(new QTableWidget(this)) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(table);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << "timestamp" << "PID" << "dll_path" << "result");
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void LogViewerWidget::loadLogFile() {
    table->clearContents();
    table->setRowCount(0);

    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/log.csv";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "오류", "log.csv 파일을 열 수 없습니다.");
        return;
    }

    QTextStream in(&file);
    bool firstLine = true;
    int row = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        qDebug() << "[line]:" << line;

        QStringList parts = line.split(",");
        qDebug() << "[parts]:" << parts;

        if (firstLine) {
            firstLine = false;
            continue;
        }

        if (parts.size() != 4) continue;

        table->insertRow(row);
        for (int col = 0; col < 4; ++col)
            table->setItem(row, col, new QTableWidgetItem(parts[col]));
        row++;
    }

    if (row == 0) {
        QMessageBox::information(this, "정보", "log.csv에 기록된 데이터가 없습니다.");
    }
}
