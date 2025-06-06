#include "LogViewerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

LogViewerWidget::LogViewerWidget(QWidget *parent)
    : QWidget(parent), table(new QTableWidget(this)) {


    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    // 버튼 영역
    QWidget* buttonArea = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonArea);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    whitelistBtn = new QPushButton("화이트리스트 로그");
    mlBtn = new QPushButton("머신러닝 로그");
    hashBtn = new QPushButton("해시 로그");
    signatureBtn = new QPushButton("서명검증 로그");

    QList<QPushButton*> buttons = {whitelistBtn, mlBtn, hashBtn, signatureBtn};
    for (auto btn : buttons) {
        buttonLayout->addWidget(btn);
        btn->setMinimumHeight(48);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #1e1e2e;
                color: white;
                padding: 8px 16px;
                border-radius: 6px;
            }
            QPushButton:hover {
                background-color: #3e3e5e;
            }
        )");
    }

    // 테이블 초기 설정
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << "시간" << "PID" << "경로" << "결과");
    table->setColumnWidth(0, 288); // 시간
    table->setColumnWidth(1, 294);  // PID
    table->setColumnWidth(2, 294); // 경로
    table->setColumnWidth(3, 300); // 결과
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);
    separator->setStyleSheet("margin-bottom: 0px; padding-bottom: 0px; color: #2e2e3f;");
    layout->addWidget(buttonArea);
    layout->addWidget(separator);
    layout->addWidget(table);

    // 버튼 클릭 시 로그 파일 로드
    connect(whitelistBtn, &QPushButton::clicked, this, [=]() {
        loadLogFile("whitelist");
    });
    connect(mlBtn, &QPushButton::clicked, this, [=]() {
        loadLogFile("ml");
    });
    connect(hashBtn, &QPushButton::clicked, this, [=]() {
        loadLogFile("hash");
    });
    connect(signatureBtn, &QPushButton::clicked, this, [=]() {
        loadLogFile("signature");
    });
}

void LogViewerWidget::loadLogFile(const QString& method) {
    if (method.isEmpty()) {
        return;
    }
    table->clearContents();
    table->setRowCount(0);

    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                   + QString("/log_%1.csv").arg(method);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "오류", QString("파일 열기 실패:\n%1").arg(path));
        return;
    }

    QTextStream in(&file);
    bool firstLine = true;
    int row = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (firstLine) {
            firstLine = false;
            continue;
        }

        QStringList parts = line.split(",");
        if (parts.size() != 4) continue;

        table->insertRow(row);
        for (int col = 0; col < 4; ++col)
            table->setItem(row, col, new QTableWidgetItem(parts[col]));
        row++;
    }

    if (row == 0) {
        QMessageBox::information(this, "정보", QString("'%1' 로그에 기록된 데이터가 없습니다.").arg(method));
    }
}
