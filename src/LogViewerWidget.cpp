#include "LogViewerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>

LogViewerWidget::LogViewerWidget(QWidget *parent)
    : QWidget(parent), table(new QTableWidget(this)), logComboBox(new QComboBox(this)) {

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // 콤보박스 상단 설명
    QLabel* label = new QLabel("🔽 로그 파일을 선택하세요:", this);
    label->setStyleSheet("color: white; font-weight: bold;");
    layout->addWidget(label);
    layout->addWidget(logComboBox);

    //csv 테이블
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels(QStringList() << "시간" << "PID" << "경로" << "결과");
    // 개별 열 너비 조절
    table->setColumnWidth(0, 180); // 시간
    table->setColumnWidth(1, 130);  // PID (좁게)
    table->setColumnWidth(2, 600); // 경로 (넓게)
    table->setColumnWidth(3, 150); // 결과
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table);
    populateLogFileList();

    // 콤보박스에서 항목 선택 시 loadLogFile 실행
    connect(logComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (index >= 0) {
            QString filename = logComboBox->itemText(index).trimmed();
            if (!filename.isEmpty()) {
                loadLogFile(filename);
            }
        }
    });

}

void LogViewerWidget::loadLogFile(const QString& fileName) {
    if (fileName.trimmed().isEmpty()) return;

    table->clearContents();
    table->setRowCount(0);

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString filePath = desktopPath + "/" + fileName;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[LogViewer] 파일 열기 실패:" << filePath;
        return; // 🔇 그냥 return만 하고 알림창 X
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

    file.close();
}
void LogViewerWidget::populateLogFileList() {
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QDir dir(desktopPath);
    QStringList files = dir.entryList(QStringList() << "*.csv", QDir::Files);

    logComboBox->blockSignals(true);  // 시그널 일시 차단
    logComboBox->clear();
    logComboBox->addItems(files);
    logComboBox->blockSignals(false); // 시그널 다시 연결

    if (!files.isEmpty()) {
        logComboBox->setCurrentIndex(0); // 첫 항목 선택
        loadLogFile(files.first());      // 직접 호출
    } else {
        //QMessageBox::information(this, "안내", "유효한 로그 파일이 없습니다.");
    }
}


void LogViewerWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    populateLogFileList();  // 창이 보여질 때마다 로그 파일 목록 갱신
}

