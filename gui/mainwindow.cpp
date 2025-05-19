#include "DLLAnalyzer.h"
#include "mainwindow.h"
#include "ProcessManager.h"
#include "Result.h"
#include "NetworkDLLAnalyzer.h"
#include <iostream>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QSpacerItem>
#include <QFrame>
#include <QPixmap>
#include <QStyleOptionSlider>
#include <QStringList>
#include <QIcon>
#include <QSize>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QSizePolicy>
#include <QFileInfo>
#include "LogManager.h"
#include <QDesktopServices>
#include <QUrl>
#include "LogViewerWidget.h"


// ------------------ MainWindow 생성자 ------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // ProcessManager 초기화 및 시그널 연결
    processManager = new ProcessManager(this);
    connect(processManager, &ProcessManager::scanFinished,
            this, &MainWindow::onScanResult);
    networkAnalyzer = new NetworkDLLAnalyzer(this);
    connect(networkAnalyzer, &NetworkDLLAnalyzer::analysisFinished,
            this, &MainWindow::onAnalysisFinished);
    // ✅ 화이트리스트 매니저 초기화
    whitelistManager = new WhitelistManager();
    if (!whitelistManager->loadWhitelist(":/whitelist.txt")) {
        qDebug() << "[Whitelist] 로드 실패";
    } else {
        qDebug() << "[Whitelist] 로드 성공";
    }

    // 기본 설정
    setWindowTitle("Filter Dashboard");
    resize(1280, 800);

    // 아이콘 및 스테이지 설정
    QStringList icons = {":/img/home.svg", ":/img/list.svg", ":/img/searching.svg", ":/img/pattern.svg"};
    QStringList stages = {"Home", "Process", "Detection", "Log"};

     // 메인 레이아웃
    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #12131a;");
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);


    // 상단바 구성 ----------------------------------
    QWidget *topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background-color: #12131a;");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(0);

    // 로고 영역 (사이드바와 동일한 크기)
    QWidget *logoArea = new QWidget();
    logoArea->setFixedWidth(60);  // 기존 80 → 60으로 조정
    logoArea->setStyleSheet("background-color: #12131a;");

    QHBoxLayout *logoLayout = new QHBoxLayout(logoArea);
    logoLayout->setContentsMargins(10, 10, 0, 10);
    logoLayout->setAlignment(Qt::AlignCenter);

    QLabel *logo = new QLabel();
    logo->setPixmap(QIcon(":/img/logo.svg").pixmap(24, 24));
    logo->setFixedSize(24, 24);
    logoLayout->addWidget(logo);

    // 상단바 타이틀
    titleLabel = new QLabel("Content Area");
    titleLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");

    // 로고와 텍스트 사이 구분선 추가
    QFrame *logoSeparator = new QFrame();
    logoSeparator->setFrameShape(QFrame::VLine);
    logoSeparator->setFrameShadow(QFrame::Plain);
    logoSeparator->setStyleSheet("color: #2e2e3f;");
    logoSeparator->setFixedWidth(3);

    // 상단바 레이아웃 조립
    topBarLayout->addWidget(logoArea);
    topBarLayout->addSpacing(12);
    topBarLayout->addWidget(logoSeparator);  // 구분선 추가
    topBarLayout->addSpacing(12);
    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();

    // 상단바 아래 구분선
    QFrame *topLine = new QFrame();
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Plain);
    topLine->setStyleSheet("color: #2e2e3f;");


     // 콘텐츠 영역 구성 ----------------------------------
    QWidget *contentArea = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 사이드 패널 ----------------------------------
    QWidget *sidePanel = new QWidget();
    sidePanel->setFixedWidth(72);  // 기존 80 → 60으로 조정
    sidePanel->setStyleSheet("background-color: #12131a;");
    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(20);

    int index = 0;
    for (const QString &icon : icons) {
        QToolButton *btn = new QToolButton();
        /*
        btn->setIcon(QIcon(icon));
        btn->setIconSize(QSize(24, 24));
        btn->setStyleSheet("QToolButton { border: none; } QToolButton:hover { background-color: #2e2e3f; }");
        */
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        btn->setFixedHeight(48);
        btn->setIcon(QIcon(icon));
        btn->setIconSize(QSize(24, 24));
        btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        btn->setStyleSheet(R"(
            QToolButton {
                border: none;
                padding-left: 0px;
                padding-right: 0px;
                background-color: transparent;
                color: white;
            }
            QToolButton:hover {
                background-color: #2e2e3f;
            }
            QToolButton:checked {
                background-color: #3e3e5e;
            }
        )");

        btn->setToolTip(stages[index]);
        // sideLayout->addWidget(btn, 0, Qt::AlignHCenter);
        sideLayout->addWidget(btn);
        stageButtons.append(btn);

        connect(btn, &QToolButton::clicked, this, [=](){
            handleStageClick(index);
        });

        ++index;
    }
    sideLayout->addStretch();

    // ▶ 사이드패널 오른쪽 구분선
    QFrame *sideRightLine = new QFrame();
    sideRightLine->setFrameShape(QFrame::VLine);
    sideRightLine->setStyleSheet("color: #2e2e3f;");
    sideRightLine->setLineWidth(1);

    // 메인 콘텐츠 ----------------------------------
    QWidget *mainContent = new QWidget();
    mainContentLayout = new QVBoxLayout(mainContent);
    mainContentLayout->setContentsMargins(20, 20, 20, 20);

    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(3);
    resultTable->setHorizontalHeaderLabels(QStringList() << "PID" << "프로세스 이름" << "DLL 개수");
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultTable->hide();

    logViewer = new LogViewerWidget(this);
    logViewer->hide();
    mainContentLayout->addWidget(logViewer);

    mainLabel = new QLabel("[ 콘텐츠 영역 ]");
    mainLabel->setStyleSheet("color: gray; font-size: 14px;");
    mainContentLayout->addWidget(resultTable);
    mainLabel->setAlignment(Qt::AlignCenter);

    mainContentLayout->addStretch();
    mainContentLayout->addWidget(mainLabel);
    mainContentLayout->addStretch();



    // DLL 영역 생성
    setupDLLArea();
    setupDetectButtonArea();

    // 탐지 방식 선택 영역
    setupDetectionMethodArea();

    // 최종 조립 ----------------------------------
    contentLayout->addWidget(sidePanel);
    contentLayout->addWidget(sideRightLine);
    contentLayout->addWidget(mainContent);
    contentLayout->setStretch(2, 1);

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(topLine);
    mainLayout->addWidget(contentArea);

    updateStage(AppStage::Home);
}

MainWindow::~MainWindow() {
    // 기본 소멸자, 비워도 문제 없음
}
void MainWindow::setupDLLArea() {
    // DLL 정보 스크롤 영역 설정
    dllScrollArea = new QScrollArea(this);

    // 위치 및 크기 설정
    int tableX = 100; // 테이블의 x 좌표와 동일하게 맞춤 (필요에 따라 조정)
    int tableWidth = 1160;
    int tableHeight = 300;
    int yPosition = 350;

    dllScrollArea->setGeometry(tableX, yPosition, tableWidth, tableHeight);
    dllScrollArea->setWidgetResizable(true);

    // 배경색 설정 (세 번째 이미지와 동일한 색상)
    dllScrollArea->setStyleSheet("background-color: #12131A; color: white; border: none;");

    // DLL 정보 컨테이너 위젯
    QWidget *dllContainer = new QWidget();
    QVBoxLayout *dllLayout = new QVBoxLayout(dllContainer);
    dllLayout->setContentsMargins(10, 10, 10, 10);
    dllLayout->setSpacing(8);
    dllContainer->setLayout(dllLayout);

    dllScrollArea->setWidget(dllContainer);
}

void MainWindow::setupDetectButtonArea() {
    detectButton = new QPushButton("탐지 시작", this);
    detectButton->setFixedSize(160, 40);
    detectButton->setVisible(false);  // 초기엔 숨김

    detectButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3e3e5e;
            color: white;
            font-weight: bold;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #5e5e7e;
        }
    )");

    // 클릭 시 스테이지 전환
    connect(detectButton, &QPushButton::clicked, this, [=]() {
        handleStageClick(2);
    });

    // 아래 여백 및 버튼 중앙 정렬
    QWidget* buttonWrapper = new QWidget();
    QVBoxLayout* wrapperLayout = new QVBoxLayout(buttonWrapper);
    wrapperLayout->setContentsMargins(0, 30, 0, 10);  // 위쪽 여백 30, 아래 여백 10
    wrapperLayout->addWidget(detectButton, 0, Qt::AlignHCenter);

    mainContentLayout->addWidget(buttonWrapper);
}



void MainWindow::handleStageClick(int index){
    switch (index){
    case 0: // HOME
        updateStage(AppStage::Home);
        resultTable->hide();
        dllScrollArea->hide();
        logViewer->hide();
        break;
    case 1: // 프로세스 목록
        updateStage(AppStage::ProcessSelected);
        logViewer->hide();
        dllScrollArea->show();
        loadProcesses();
        break;
    case 2:
        if(currentStage >= AppStage::ProcessSelected) {
            updateStage(AppStage::DetectionStarted);
            logViewer->hide();
            dllScrollArea->show();
        }else{
            warnUser("먼저 프로세스를 선택하세요.");
        }
        break;
    case 3:
        if (currentStage >= AppStage::DetectionStarted) {
            updateStage(AppStage::LogSaved);

            // ✅ 다른 콘텐츠 숨기기
            mainLabel->hide();
            resultTable->hide();
            dllScrollArea->hide();
            // (다른 콘텐츠 위젯이 있으면 같이 hide)

            // ✅ 로그 뷰어 보여주기
            logViewer->loadLogFile();
            logViewer->show();
        } else {
            warnUser("먼저 탐지를 시작하세요.");
        }
        break;

    }
}

void MainWindow::updateStage(AppStage newStage){
    currentStage = newStage;

    for(int i=0; i<stageButtons.size(); i++){
        QString style;
        //if(i == 0){
        //    style = "background-color: transparent;";
        //}
        if(i < static_cast<int>(currentStage)){
            // style = "background-color: #1e1e2e;";
        }else if(i == static_cast<int>(currentStage)){
            style = "background-color: #1e1e2e;";
        }else {
            style = "background-color: transparent;";
        }

        stageButtons[i]->setStyleSheet(
            QString("QToolButton { border: none; %1 } QToolButton:hover { background-color: #2e2e3f; }")
                .arg(style)
            );
    }

    // 탐지 버튼 숨기기 (다음 상태에서 필요한 경우만 다시 보여줌)
    if (detectButton) {
        detectButton->setVisible(false);
    }

    if (detectionMethodWidget) detectionMethodWidget->setVisible(false);


    if(mainLabel){
        switch(currentStage){
        case AppStage::Home:
            mainLabel->setText("홈");
            titleLabel->setText("Home");
            clearTable();
            clearDLLArea();
            break;
        case AppStage::ProcessSelected:
            mainLabel->setText("프로세스 선택");
            titleLabel->setText("Process");
            break;
        case AppStage::DetectionStarted:
            mainLabel->setText("DLL 탐지");
            titleLabel->setText("Detection");
            clearTable();
            clearDLLArea();
            if(detectionMethodWidget) detectionMethodWidget->show();
            break;
        case AppStage::LogSaved:
            mainLabel->setText("로그 저장");
            titleLabel->setText("Log");
            break;
        }
    }

}
void MainWindow::clearDLLArea() {
    if (!dllScrollArea) return;  // dllScrollArea가 nullptr인 경우 바로 리턴

    QWidget *dllContainer = dllScrollArea->widget();
    if (!dllContainer) return;

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(dllContainer->layout());
    if (!layout) return;

    // 레이아웃 내부의 모든 위젯 제거
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        QWidget *widget = item->widget();
        if (widget) {
            widget->deleteLater();  // 메모리 해제
        }
        delete item;
    }
}
void MainWindow::clearTable(){
    resultTable->clearContents();
    resultTable->setRowCount(0);
    resultTable->setColumnCount(0);
    resultTable->setStyleSheet("border: none;");
}

void MainWindow::loadProcesses() {
    clearTable();
    processManager->runScan();
}

void MainWindow::onScanResult(const std::vector<Result>& results){
    resultTable->clearContents();
    resultTable->setColumnCount(2);
    resultTable->setHorizontalHeaderLabels(QStringList() <<"PID" << "프로세스 이름");
    resultTable->setRowCount(static_cast<int>(results.size()));
    resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);



    for (int i = 0; i < static_cast<int>(results.size()); ++i) {
        const Result &res = results[i];
        resultTable->setItem(i, 0, new QTableWidgetItem(QString::number(res.pid)));
        resultTable->setItem(i, 1, new QTableWidgetItem(res.processName));
    }
    connect(resultTable, &QTableWidget::cellClicked, this, &MainWindow::handleRowClicked);
    cachedResults = results;

    resultTable->show();

    if (results.empty()) {
        QMessageBox::information(this, "결과 없음", "프로세스를 불러오지 못했습니다.");
    }
}

void MainWindow::warnUser(const QString &msg){
    QMessageBox::warning(this, "안내", msg);
}

void MainWindow::handleRowClicked(int row, int column) {
    if (row < 0 || row >= static_cast<int>(cachedResults.size())) return;

    const Result &res = cachedResults[row];
    int pid = res.pid;

    DLLAnalyzer dllAnalyzer;
    std::vector<std::string> dllList = dllAnalyzer.GetLoadedModules(pid);

    // DLL 정보 컨테이너 초기화
    QVBoxLayout *dllLayout = qobject_cast<QVBoxLayout*>(dllScrollArea->widget()->layout());
    QLayoutItem *child;
    while ((child = dllLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QLabel *title = new QLabel(QString("프로세스: %1\nPID: %2\nDLL 목록:").arg(res.processName).arg(pid));
    title->setStyleSheet("color: white; font-weight: bold;");
    dllLayout->addWidget(title);

    if (!dllList.empty()) {
        for (const std::string &dll : dllList) {
            QString dllPath = QString::fromStdString(dll);

            QPushButton *dllButton = new QPushButton(dllPath);
            dllButton->setStyleSheet(R"(
                QPushButton {
                    color: white;
                    background-color: #12131A;  /* 배경색과 동일하게 */
                    border: 1px solid #2e2e3f;   /* 테두리 색상 */
                    padding: 4px;
                    text-align: left;
                }
                QPushButton:hover {
                    background-color: #2e2e3f;  /* Hover 시 배경색 */
                }
            )");


            connect(dllButton, &QPushButton::clicked, this, [=]() {
                QString dllName = QFileInfo(dllPath).fileName();
                lastAnalyzedDllPath = dllPath;
                if (whitelistManager->isWhitelisted(dllName)) {
                    LogManager::writeLog(dllPath, 0, "whitelist", cachedResults);
                    emit networkAnalyzer->analysisFinished("정상 DLL입니다 (화이트리스트)");
                } else {
                    networkAnalyzer->analyzeDLL(dllPath);
                }
            });

            QLabel *dllLabel = new QLabel(dllPath);
            dllLabel->setStyleSheet(R"(
                color: white;
                background-color: #12131A;
                padding: 4px;
                text-align: left;
                border-bottom: 1px solid #2e2e3f;
            )");

            dllLayout->addWidget(dllLabel);
        }
    } else {
        QLabel *noDLLLabel = new QLabel("DLL 정보가 없습니다.");
        noDLLLabel->setStyleSheet(R"(
            color: gray;
            padding-top: 10px;
            text-align: center;
        )");

        noDLLLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        dllLayout->addWidget(noDLLLabel);
    }

    detectButton->setVisible(true);
}




void MainWindow::onAnalysisFinished(const QString &resultJson) {
    QMessageBox::information(this, "DLL 분석 결과", resultJson);

    QJsonDocument doc = QJsonDocument::fromJson(resultJson.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("prediction")) {
            int prediction = obj["prediction"].toInt();
            QString source = obj.value("source").toString();
            LogManager::writeLog(lastAnalyzedDllPath, prediction, source, cachedResults);
        }
    }
}

void MainWindow::setupDetectionMethodArea() {
    detectionMethodWidget = new QWidget(this);
    QVBoxLayout* outerLayout = new QVBoxLayout(detectionMethodWidget);
    outerLayout->setContentsMargins(20, 20, 20, 20);
    outerLayout->setSpacing(16);

    QLabel* title = new QLabel("탐지 방식을 선택하세요:");
    title->setStyleSheet("color: white; font-weight: bold; font-size: 16px;");
    outerLayout->addWidget(title);

    // 버튼 레이아웃
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(12);

    QString baseStyle = R"(
        QPushButton {
            background-color: #1e1e2e;
            color: white;
            padding: 15px;
            font-size: 14px;
            border: 1px solid #2e2e3f;
            border-radius: 10px;
        }
        QPushButton:checked {
            background-color: #3e3e5e;
            border: 2px solid #7aa2f7;
        }
    )";

    pebButton = new QPushButton("PEB 기반");
    hookButton = new QPushButton("훅 기반");
    entropyButton = new QPushButton("엔트로피 기반");
    networkButton = new QPushButton("네트워크 기반");

    QList<QPushButton*> buttons = {pebButton, hookButton, entropyButton, networkButton};
    int row = 0, col = 0;
    for (QPushButton* btn : buttons) {
        btn->setCheckable(true);
        btn->setStyleSheet(baseStyle);
        btn->setMinimumWidth(180);
        btn->setMinimumHeight(60);
        grid->addWidget(btn, row, col);

        connect(btn, &QPushButton::clicked, this, [=]() {
            // 다른 버튼들 전부 체크 해제
            for (QPushButton* other : buttons) {
                if (other != btn) other->setChecked(false);
            }
            selectedDetectionButton = btn;  // 현재 선택된 버튼 저장
        });

        col++;
        if (col == 2) { row++; col = 0; }
    }

    outerLayout->addLayout(grid);

    // 실행 버튼
    QPushButton* runBtn = new QPushButton("탐지 실행");
    runBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #7aa2f7;
            color: white;
            font-weight: bold;
            padding: 10px 20px;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #5e7ddc;
        }
    )");
    runBtn->setFixedSize(120, 40);

    connect(runBtn, &QPushButton::clicked, this, [=]() {
        if (!selectedDetectionButton) {
            QMessageBox::warning(this, "선택 필요", "탐지 방식을 선택해주세요.");
            return;
        }
        QString method = selectedDetectionButton->text();
        startDetectionWithMethod(method);
    });

    outerLayout->addWidget(runBtn, 0, Qt::AlignRight);

    detectionMethodWidget->hide();  // 초기엔 숨김
    mainContentLayout->insertWidget(0, detectionMethodWidget);  // 상단에 추가
}


// 현재는 예시로 탐지 방식을 작성해둔거에용
void MainWindow::startDetectionWithMethod(const QString& method) {
    qDebug() << "선택된 탐지 방식:" << method;
    if (method == "PEB 기반") {
        qDebug() << "PEB 탐지 수행";
        // PEB 기반 탐지 실행
    } else if (method == "훅 기반") {
        qDebug() << "훅 기반 탐지 수행";
        // 훅 기반 탐지 실행
    } else if (method == "엔트로피 기반") {
        qDebug() << "엔트로피 탐지 수행";
        // 엔트로피 분석 실행
    } else if (method == "네트워크 기반") {
        qDebug() << "네트워크 API 탐지 수행";
        // 네트워크 관련 DLL 분석
    } else {
        qDebug() << "알 수 없는 탐지 방식:" << method;
    }
}


//    const Result &res = cachedResults[row];
//    QString message = QString("PID: %1\n프로세스명: %2\n\nDLL 목록:\n").arg(res.pid).arg(res.processName);

//    for (const QString &dll : res.dllList) {
//        message += "- " + dll + "\n";
//    }

//    QMessageBox::information(this, "프로세스 DLL 목록", message);

