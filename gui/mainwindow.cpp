#include "DLLAnalyzer.h"
#include "mainwindow.h"
#include "ProcessManager.h"
#include "Result.h"
#include "NetworkDLLAnalyzer.h"
#include "CodeSignatureAnalyzer.h"
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
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QFontDatabase>

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

    whitelistManager = new WhitelistManager();
    whitelistManager->loadWhitelist(":/whitelist.txt");
    hashComparator.loadHashList(":/known_hashes.txt");
    // 기본 설정
    setWindowTitle("Filter Dashboard");
    resize(1280, 800);

    QFontDatabase::addApplicationFont(":/fonts/DMSans-Bold.ttf");

    // 아이콘 및 스테이지 설정
    QStringList icons = {":/img/home.svg", ":/img/list.svg", ":/img/searching.svg", ":/img/pattern.svg"};
    QStringList stages = {"Home", "Process", "Detection", "Log"};

    // 메인 중앙 위젯 구성
    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #12131a;");
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---------------- 상단바 ----------------
    QWidget *topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background-color: #12131a;");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(0);

    QWidget *logoArea = new QWidget();
    logoArea->setFixedWidth(60);
    QHBoxLayout *logoLayout = new QHBoxLayout(logoArea);
    logoLayout->setContentsMargins(10, 10, 0, 10);
    logoLayout->setAlignment(Qt::AlignCenter);
    QLabel *logo = new QLabel();
    logo->setPixmap(QIcon(":/img/logo.svg").pixmap(24, 24));
    logo->setFixedSize(24, 24);
    logoLayout->addWidget(logo);
    logoArea->setStyleSheet("background-color: #12131a;");

    titleLabel = new QLabel("Content Area");
    titleLabel->setStyleSheet("font-family: 'DM-Sans'; color: white; font-size: 20px; font-weight: bold;");

    QFrame *logoSeparator = new QFrame();
    logoSeparator->setFrameShape(QFrame::VLine);
    logoSeparator->setFrameShadow(QFrame::Plain);
    logoSeparator->setStyleSheet("color: #2e2e3f;");
    logoSeparator->setFixedWidth(3);

    topBarLayout->addWidget(logoArea);
    topBarLayout->addSpacing(12);
    topBarLayout->addWidget(logoSeparator);
    topBarLayout->addSpacing(12);
    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();

    QFrame *topLine = new QFrame();
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Plain);
    topLine->setStyleSheet("color: #2e2e3f;");

    // ---------------- 콘텐츠 구성 ----------------
    QWidget *contentArea = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // ▶ 사이드바
    QWidget *sidePanel = new QWidget();
    sidePanel->setFixedWidth(72);
    sidePanel->setStyleSheet("background-color: #12131a;");
    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(20);

    int index = 0;
    for (const QString &icon : icons) {
        QToolButton *btn = new QToolButton();
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        btn->setFixedHeight(48);
        btn->setIcon(QIcon(icon));
        btn->setIconSize(QSize(24, 24));
        btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        btn->setStyleSheet(R"(
            QToolButton {
                border: none;
                background-color: transparent;
                color: white;
            }
            QToolButton:hover {
                background-color: #2e2e3f;
            }
            QToolButton:checked {
                background-color: #3e3e5e;
            })");
        btn->setToolTip(stages[index]);
        sideLayout->addWidget(btn);
        stageButtons.append(btn);
        connect(btn, &QToolButton::clicked, this, [=]() {
            handleStageClick(index);
        });
        ++index;
    }
    sideLayout->addStretch();

    QFrame *sideRightLine = new QFrame();
    sideRightLine->setFrameShape(QFrame::VLine);
    sideRightLine->setStyleSheet("color: #2e2e3f;");
    sideRightLine->setLineWidth(1);

    // ▶ 메인 콘텐츠
    QWidget *mainContent = new QWidget();
    mainContentLayout = new QVBoxLayout(mainContent);
    mainContentLayout->setContentsMargins(20, 20, 20, 20);
    mainContentLayout->setSpacing(20);

    // ▶ 프로세스 테이블 & DLL 영역
    resultTable = new QTableWidget(this);
    resultTable->setColumnCount(3);
    resultTable->setHorizontalHeaderLabels(QStringList() << "PID" << "프로세스 이름" << "DLL 개수");
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resultTable->hide();

    setupDLLArea();  // → dllScrollArea 생성됨
    dllScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *contentSplitLayout = new QVBoxLayout();
    contentSplitLayout->setSpacing(10);
    contentSplitLayout->addWidget(resultTable, 2);
    contentSplitLayout->addWidget(dllScrollArea, 3);
    qDebug() << "[디버그] resultTable:" << resultTable;
    qDebug() << "[디버그] dllScrollArea:" << dllScrollArea;
    mainContentLayout->addLayout(contentSplitLayout);


    qDebug() << "[디버그] mainContentLayout:" << mainContentLayout;
    // ▶ 탐지 버튼
    setupDetectButtonArea(mainContentLayout);  // → 내부에서 buttonWrapper를 addWidget

    // ▶ 탐지 방식 및 결과 UI
    qDebug() << "[디버그] detectionMethodWidget:" << detectionMethodWidget;
    setupDetectionMethodArea(mainContentLayout);
    qDebug() << "[디버그] detectionResultWidget:" << detectionResultWidget;
    setupDetectionResultArea(mainContentLayout);

    // ▶ 로그 뷰어 (탐지 로그 표시용)
    logViewer = new LogViewerWidget(this);
    logViewer->hide();
    mainContentLayout->addWidget(logViewer);

    // ---------------- 조립 ----------------
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
    dllScrollArea = new QScrollArea(this);
    dllScrollArea->setWidgetResizable(true);
    dllScrollArea->setStyleSheet("background-color: #12131A; color: white; border: none;");

    QWidget *dllContainer = new QWidget();
    QVBoxLayout *dllLayout = new QVBoxLayout(dllContainer);
    dllLayout->setContentsMargins(10, 10, 10, 10);
    dllLayout->setSpacing(8);
    dllContainer->setLayout(dllLayout);

    dllScrollArea->setWidget(dllContainer);

    // 이걸로 크기 유동적 할당 (필수!)
    dllScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


void MainWindow::setupDetectButtonArea(QVBoxLayout* layout) {
    detectButton = new QPushButton("탐지 시작", this);
    detectButton->setFixedSize(160, 40);
    detectButton->setVisible(false);

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


    connect(detectButton, &QPushButton::clicked, this, [=]() {
        handleStageClick(2);
    });

    QWidget* buttonWrapper = new QWidget();
    QVBoxLayout* wrapperLayout = new QVBoxLayout(buttonWrapper);
    wrapperLayout->setContentsMargins(0, 30, 0, 10);
    wrapperLayout->addWidget(detectButton, 0, Qt::AlignHCenter);
    layout->addWidget(buttonWrapper);
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
void MainWindow::startCodeSignatureDetection() {
    CodeSignatureAnalyzer analyzer;
    std::vector<std::pair<QString, QString>> suspicious;

    for (QString& dll : currentDllList) {
        if (analyzer.isSuspicious(dll)) {
            suspicious.emplace_back(QFileInfo(dll).fileName(), dll);
        }
    }

    if (suspicious.empty()) {
        showCleanResult();
    } else {
        showSuspiciousDLLs(suspicious);
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


        switch(currentStage){
        case AppStage::Home:
            //mainLabel->setText("홈");
            titleLabel->setText("Home");
            clearTable();
            clearDLLArea();
            break;
        case AppStage::ProcessSelected:
            //mainLabel->setText("프로세스 선택");
            titleLabel->setText("Process");
            break;
        case AppStage::DetectionStarted:
           // mainLabel->setText("DLL 탐지");
            titleLabel->setText("Detection");
            clearTable();
            clearDLLArea();
            if(detectionMethodWidget) detectionMethodWidget->show();
            break;
        case AppStage::LogSaved:
            //mainLabel->setText("로그 저장");
            titleLabel->setText("Log");
            break;
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
    lastSelectedRow = row;
    resultTable->selectRow(row);
    const Result &res = cachedResults[row];
    int pid = res.pid;

    DLLAnalyzer dllAnalyzer;
    std::vector<std::string> dllListRaw  = dllAnalyzer.GetLoadedModules(pid);
    std::vector<std::string> dllList = dllAnalyzer.GetLoadedModules(pid);

    QStringList dllListQt;
    for (const std::string &s : dllListRaw) {
        dllListQt.append(QString::fromStdString(s));
    }
    // ✅ 🔴 cachedResults[row].dllList를 갱신!
    cachedResults[row].dllList = dllListQt;
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
                    background-color: #12131A;
                    border: 1px solid #2e2e3f;
                    padding: 4px;
                    text-align: left;
                }
                QPushButton:hover {
                    background-color: #2e2e3f;
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

        // ✅ 수동으로 테스트 DLL 삽입
    //   currentDllList.clear();
  //     currentDllList.append("C:/Users/jeong/source/repos/UnsignedTestDLL/x64/Debug/UnsignedTestDLL.dll");

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

void MainWindow::setupDetectionMethodArea(QVBoxLayout* layout) {
    qDebug() << "[체크] setupDetectionMethodArea() 진입됨";

    detectionMethodWidget = new QWidget(this);
    QVBoxLayout* outerLayout = new QVBoxLayout(detectionMethodWidget);
    outerLayout->setContentsMargins(20, 20, 20, 20);
    outerLayout->setSpacing(16);

    QLabel* title = new QLabel("탐지 방식을 선택하세요:");
    title->setStyleSheet("color: white; font-weight: bold; font-size: 16px;");
    outerLayout->addWidget(title);

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

    pebButton = new QPushButton("해시 기반");
    hookButton = new QPushButton("훅 기반");
    entropyButton = new QPushButton("엔트로피 기반");
    networkButton = new QPushButton("코드 서명 검증");

    QList<QPushButton*> buttons = {pebButton, hookButton, entropyButton, networkButton};
    int row = 0, col = 0;
    for (QPushButton* btn : buttons) {
        btn->setCheckable(true);
        btn->setStyleSheet(baseStyle);
        btn->setMinimumWidth(180);
        btn->setMinimumHeight(60);
        grid->addWidget(btn, row, col);

        connect(btn, &QPushButton::clicked, this, [=]() {
            for (QPushButton* other : buttons)
                if (other != btn) other->setChecked(false);
            selectedDetectionButton = btn;
        });

        if (++col == 2) { row++; col = 0; }
    }

    outerLayout->addLayout(grid);

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
        startDetectionWithMethod(selectedDetectionButton->text());
    });

    outerLayout->addWidget(runBtn, 0, Qt::AlignRight);
    detectionMethodWidget->hide();

    layout->insertWidget(0, detectionMethodWidget);
}


// 현재는 예시로 탐지 방식을 작성해둔거에용
void MainWindow::startDetectionWithMethod(const QString& method) {
    qDebug() << "선택된 탐지 방식:" << method;

    // 🔴 탐지 결과 UI 초기화
    if (detectionResultWidget) {
        detectionResultWidget->show();
        dllResultTable->hide();
        resultStatusLabel->setStyleSheet("color: white; font-size: 14px;");
        resultStatusLabel->setText("🔍 탐지 중...");
    }

    // 🔴 1.5초 후 실제 탐지 수행
    QTimer::singleShot(1500, this, [=]() {
        if (method == "해시 기반") {
            const Result &res = cachedResults[lastSelectedRow];
            auto suspiciousDLLs = hashComparator.detectSuspiciousDLLs(res.dllList);
            if (suspiciousDLLs.empty()) {
                showCleanResult();
            } else {
                showSuspiciousDLLs(suspiciousDLLs);
            }
        } else if (method == "훅 기반") {
            qDebug() << "훅 기반 탐지 수행 (예제용)";
            showSuspiciousDLLs({{"bad_hook.dll", "C:/hook/bad_hook.dll"}});
        } else if (method == "엔트로피 기반") {
            qDebug() << "엔트로피 분석 수행 (예제용)";
            showSuspiciousDLLs({{"weird_entropy.dll", "C:/suspicious/weird_entropy.dll"}});

        } else if (method == "코드 서명 검증") {
            qDebug() << "코드 서명 검증 수행";
            startCodeSignatureDetection();
        } else {
            qDebug() << "⚠️ 알 수 없는 탐지 방식:" << method;
            resultStatusLabel->setText("⚠️ 알 수 없는 탐지 방식입니다.");
        }
    });  // QTimer::singleShot 닫힘
}  // startDetectionWithMethod 닫힘


void MainWindow::setupDetectionResultArea(QVBoxLayout* layout) {
    qDebug() << "[체크] setupDetectionResultArea() 진입됨";

    detectionResultWidget = new QWidget(this);
    QVBoxLayout* resultLayout = new QVBoxLayout(detectionResultWidget);
    resultLayout->setContentsMargins(10, 10, 10, 10);
    resultLayout->setSpacing(12);

    resultStatusLabel = new QLabel("탐지 결과가 여기에 표시됩니다.");
    resultStatusLabel->setStyleSheet("color: gray; font-size: 14px;");
    resultStatusLabel->setAlignment(Qt::AlignCenter);

    dllResultTable = new QTableWidget();
    dllResultTable->setColumnCount(2);
    dllResultTable->setHorizontalHeaderLabels(QStringList() << "DLL 이름" << "경로");
    dllResultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dllResultTable->hide();

    resultLayout->addWidget(resultStatusLabel);
    resultLayout->addWidget(dllResultTable);

    detectionResultWidget->hide();
    layout->addWidget(detectionResultWidget);
}


void MainWindow::showCleanResult() {
    dllResultTable->hide();
    resultStatusLabel->setText("✅ 의심되는 DLL이 없습니다!");
    resultStatusLabel->setStyleSheet(R"(
        QLabel {
            color: lightgreen;
            font-weight: bold;
            font-size: 16px;
        }
    )");

    // 페이드인 애니메이션
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(resultStatusLabel);
    resultStatusLabel->setGraphicsEffect(effect);

    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(700);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showSuspiciousDLLs(const std::vector<std::pair<QString, QString>>& dlls) {
    dllResultTable->setRowCount(static_cast<int>(dlls.size()));
    for (int i = 0; i < dlls.size(); ++i) {
        dllResultTable->setItem(i, 0, new QTableWidgetItem(dlls[i].first));
        dllResultTable->setItem(i, 1, new QTableWidgetItem(dlls[i].second));
    }
    resultStatusLabel->setText("❗ 의심되는 DLL이 발견되었습니다.");
    resultStatusLabel->setStyleSheet("color: orange; font-size: 14px;");
    dllResultTable->show();
}

//    const Result &res = cachedResults[row];
//    QString message = QString("PID: %1\n프로세스명: %2\n\nDLL 목록:\n").arg(res.pid).arg(res.processName);

//    for (const QString &dll : res.dllList) {
//        message += "- " + dll + "\n";
//    }

//    QMessageBox::information(this, "프로세스 DLL 목록", message);

