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
#include <QJsonArray>
#include <QSet>
#include <windows.h>
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

    monitor = new DebugEventMonitor(this);
    connect(monitor, &DebugEventMonitor::logLine, this, &MainWindow::onMonitorLog);
    connect(monitor, &DebugEventMonitor::alert, this, &MainWindow::onMonitorAlert);

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
    connect(resultTable, &QTableWidget::cellClicked,
            this, &MainWindow::handleRowClicked);
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
    detectButton->setEnabled(false);

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
            //mainLabel->hide();
            resultTable->hide();
            dllScrollArea->hide();
            // (다른 콘텐츠 위젯이 있으면 같이 hide)

            // ✅ 로그 뷰어 보여주기
            logViewer->loadLogFile();
            logViewer->show();
            break;

        }
    }
}


void MainWindow::startCodeSignatureDetection() {
    CodeSignatureAnalyzer analyzer;
    std::vector<std::pair<QString, QString>> suspicious;

    // 🔧 DLL 리스트를 결과에 반영 (로그 저장용)
    currentDllList = cachedResults[lastSelectedRow].dllList;
    cachedResults[lastSelectedRow].dllList = currentDllList;

    for (QString& dll : currentDllList) {
        int prediction = 0;
        if (analyzer.isSuspicious(dll)) {
            prediction = 1;
            suspicious.emplace_back(QFileInfo(dll).fileName(), dll);
        }

        QString pidStr = QString::number(cachedResults[lastSelectedRow].pid);
        LogManager::writeLog(
            dll,
            prediction,
            "signature",
            cachedResults,
            "code_signature",
            pidStr
            );
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
            if (detectionResultWidget) detectionResultWidget->hide();
            clearTable();
            clearDLLArea();
            break;
        case AppStage::ProcessSelected:
            //mainLabel->setText("프로세스 선택");
            titleLabel->setText("Process");
            if (detectionResultWidget) detectionResultWidget->hide();
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
            if (detectionResultWidget) detectionResultWidget->hide();
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
//    connect(resultTable, &QTableWidget::cellClicked, this, &MainWindow::handleRowClicked);
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
                QString lowerPath = dllPath.toLower();

                bool isSystemDLL = lowerPath.startsWith("c:\\windows\\system32") ||
                                   lowerPath.startsWith("c:\\windows\\system64") ||
                                   lowerPath.startsWith("c:\\windows\\winsxs");


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
                        QString targetPid = QString::number(cachedResults[lastSelectedRow].pid);
                        LogManager::writeLog(dllPath, 0, "whitelist", cachedResults, "whitelist", targetPid);
                        emit networkAnalyzer->analysisFinished("정상 DLL입니다 (화이트리스트)");
                    } else {
                        networkAnalyzer->analyzeDLL(dllPath);
                    }
                });

                QLabel *dllLabel = new QLabel(dllPath);
                if (isSystemDLL) {
                    dllLabel->setStyleSheet(R"(
                    color: white;
                    background-color: #12131A;
                    padding: 4px;
                    border-bottom: 1px solid #2e2e3f;
                )");
                } else {
                    dllLabel->setStyleSheet(R"(
                    color: orange;
                    font-weight: bold;
                    background-color: #12131A;
                    padding: 4px;
                    border-bottom: 1px solid #2e2e3f;
                )");
                }

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
        detectButton->setEnabled(true);

        currentSelectedPid = pid;  // ← 화면 전환돼도 PID를 기억
        qDebug() << "[DEBUG] handleRowClicked: row=" << row
                 << " pid=" << pid
                 << " lastSelectedRow=" << lastSelectedRow
                 << " currentSelectedPid=" << currentSelectedPid;
}
        // ✅ 수동으로 테스트 DLL 삽입
    //   currentDllList.clear();
  //     currentDllList.append("C:/Users/jeong/source/repos/UnsignedTestDLL/x64/Debug/UnsignedTestDLL.dll");




    void MainWindow::onAnalysisFinished(const QString &resultJson) {
        // 1) JSON 파싱
        QJsonDocument doc = QJsonDocument::fromJson(resultJson.toUtf8());
        if (!doc.isObject()) return;
        QJsonObject  obj         = doc.object();
        QJsonArray   resultsArray = obj["results"].toArray();
        QString      pidStr      = QString::number(cachedResults[lastSelectedRow].pid);

        // 2) 의심 DLL을 담을 벡터 (원래 코드에 있던 이름 따라)
        std::vector<std::pair<QString, QString>> suspiciousDLLs;

        // 3) 배열 순회하며 prediction + whitelist 필터링
        for (const QJsonValue &val : resultsArray) {
            QJsonObject dllObj    = val.toObject();
            int         prediction = dllObj["prediction"].toInt();
            QString     path       = dllObj["dll_path"].toString();
            QString     dllName    = QFileInfo(path).fileName();

            // 로그 남기기 (원래 있던 코드)
            LogManager::writeLog(path, prediction, "ml", cachedResults, "ml", pidStr);

            // ★ 여기서 name 대신 dllName, 그리고 suspicious → suspiciousDLLs
            if (prediction == 1 && !whitelistManager->isWhitelisted(dllName)) {
                suspiciousDLLs.emplace_back(dllName, path);
            }
        }

        // 4) UI 갱신
        if (suspiciousDLLs.empty()) {
            showCleanResult();
        } else {
            showSuspiciousDLLs(suspiciousDLLs);
        }
    }

    void MainWindow::setupDetectionMethodArea(QVBoxLayout* layout) {
        qDebug() << "[체크] setupDetectionMethodArea() 진입됨";

        detectionMethodWidget = new QWidget(this);
        QVBoxLayout* outerLayout = new QVBoxLayout(detectionMethodWidget);
        outerLayout->setContentsMargins(20, 20, 20, 20);
        outerLayout->setSpacing(16);

        //QLabel* title = new QLabel("탐지 방식을 선택하세요:");
        //title->setStyleSheet("color: white; font-weight: bold; font-size: 16px;");
        //outerLayout->addWidget(title);

        QString baseStyle = R"(
        QPushButton {
            background-color: #1e1e2e;
            color: white;
            padding: 15px;
            font-size: 14px;
            border: 1px solid #2e2e3f;
            border-radius: 10px;
            min-width: 150px;
        }
        QPushButton:checked {
            background-color: #3e3e5e;
            border: 2px solid #7aa2f7;
        }
        )";

        // ===== [섹션 1] 정적 / 스냅샷형 탐지 =====
        QLabel* staticTitle = new QLabel("정적 / 스냅샷형 탐지");
        staticTitle->setStyleSheet("color: #ffffff; font-weight: 700; font-size: 16px;");
        outerLayout->addWidget(staticTitle);

        QLabel* hint = new QLabel("※ 정적/스냅샷형: 실행 중 스캔 / 파일·서명·해시 기반 확인");
        hint->setStyleSheet("color:#a0a7b4; font-size:12px;");
        outerLayout->addWidget(hint);

        // 정적 버튼 3개 (훅 버튼 제거)
        pebButton = new QPushButton("해시 기반");
        entropyButton = new QPushButton("WhitelistMLFilter");
        networkButton = new QPushButton("코드 서명 검증");

        // 행 레이아웃
        QHBoxLayout* staticRow = new QHBoxLayout();
        staticRow->setSpacing(20);
        for (QPushButton* btn : { entropyButton, pebButton, networkButton }) {
            btn->setCheckable(true);
            btn->setStyleSheet(baseStyle);
            staticRow->addWidget(btn);
        }
        outerLayout->addLayout(staticRow);

        // 섹션 구분선
        QFrame* divider = new QFrame();
        divider->setFrameShape(QFrame::HLine);
        divider->setFrameShadow(QFrame::Sunken);
        divider->setStyleSheet("color: #2e2e3f;");
        outerLayout->addWidget(divider);


        // ===== [섹션 2] 동적 탐지 (실시간) =====
        QLabel* dynamicTitle = new QLabel("동적 탐지 (실시간)");
        dynamicTitle->setStyleSheet("color: #ffffff; font-weight: 700; font-size: 16px;");
        outerLayout->addWidget(dynamicTitle);

        QLabel* hint2 = new QLabel("※ 동적(실시간): OS 알림 기반 / CreateRemoteThread+LoadLibrary 주입 전용 감시");
        hint2->setStyleSheet("color:#a0a7b4; font-size:12px;");
        outerLayout->addWidget(hint2);

        // 동적 탐지 버튼
        dynamicButton = new QPushButton("동적 감시(LoadLibrary)");
        dynamicButton->setCheckable(true);
        dynamicButton->setStyleSheet(baseStyle);

        QHBoxLayout* dynamicRow = new QHBoxLayout();
        dynamicRow->setSpacing(20);
        dynamicRow->addWidget(dynamicButton);

        chkAutoKill = new QCheckBox("감지 시 프로세스 종료");
        chkAutoKill->setStyleSheet("color: #c0d1d9;");
        chkAutoKill->setChecked(false);

        dynamicRow->addWidget(chkAutoKill);
        dynamicRow->addStretch();
        outerLayout->addLayout(dynamicRow);


        // ===== 버튼 상호배타 선택 연결 (두 섹션 모두 포함) =====
        QList<QPushButton*> allButtons = { entropyButton, pebButton, networkButton, dynamicButton };
        for (QPushButton* btn : allButtons) {
            connect(btn, &QPushButton::clicked, this, [=]() {
                for (QPushButton* other : allButtons)
                    if (other != btn) other->setChecked(false);
                selectedDetectionButton = btn;
            });
        }

        /* QHBoxLayout* buttonRow = new QHBoxLayout();
        buttonRow->setSpacing(20);
        for (QPushButton* btn : buttons) {
            btn->setCheckable(true);
            btn->setStyleSheet(baseStyle);
            buttonRow->addWidget(btn);

            connect(btn, &QPushButton::clicked, this, [=]() {
                for (QPushButton* other : buttons)
                    if (other != btn) other->setChecked(false);
                selectedDetectionButton = btn;
            });
        }
        */

        // outerLayout->addLayout(buttonRow);


        // ===== 실행/중지 버튼 행 =====
        QHBoxLayout* runRow = new QHBoxLayout();
        runRow->setSpacing(12);

        QPushButton* stopBtn = new QPushButton("감시 중지");
        stopBtn->setFixedSize(120, 40);
        stopBtn->setStyleSheet(R"(
        QPushButton { background-color: #444c56; color: white; border-radius: 6px; }
        QPushButton:hover { background-color: #586069; }
        )");
        connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopMonitorClicked);

        QPushButton* runBtn = new QPushButton("탐지 시작");
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
        runRow->addStretch();
        runRow->addWidget(stopBtn);
        runRow->addWidget(runBtn);
        outerLayout->addLayout(runRow);

        // outerLayout->addWidget(runBtn, 0, Qt::AlignRight);
        detectionMethodWidget->hide();
        layout->insertWidget(0, detectionMethodWidget);
    }



// 현재는 예시로 탐지 방식을 작성해둔거에용
void MainWindow::startDetectionWithMethod(const QString& method) {
    qDebug() << "[DEBUG] startDetectionWithMethod called, method=" << method
             << " lastSelectedRow=" << lastSelectedRow
             << " cachedResults.size()=" << cachedResults.size();

    qDebug() << "선택된 탐지 방식:" << method;

    // 🔴 탐지 결과 UI 초기화
    if (detectionResultWidget) {
        detectionResultWidget->show();
        dllResultTable->hide();
        resultStatusLabel->setStyleSheet("color: white; font-size: 14px;");
        resultStatusLabel->setText("🔍 탐지 중...");
    }
    if (method == "동적 감시(LoadLibrary)") {
        // currentRow()는 탭 이동 시 리셋되므로, 마지막 클릭된 행만 신뢰
        int row = lastSelectedRow;

        // cachedResults 기준으로 범위 확인
        if (row < 0 || row >= static_cast<int>(cachedResults.size())) {
            QMessageBox::warning(this, "선택 필요", "프로세스를 먼저 선택하세요.");
            return;
        }


/*        bool ok=false;
        int pid = resultTable->item(row, 0)->text().toInt(&ok);

        if(!ok || pid <= 0){
            QMessageBox::warning(this,"오류","PID해석 실패");
            return;
        }*/
        const Result& r = cachedResults[row];
        int pid = r.pid;
        if (pid <= 0) {
            QMessageBox::warning(this, "오류", "PID 해석 실패");
            return;
        }
        DWORD selfPid = GetCurrentProcessId();
        if(DWORD(pid) == selfPid){
            QMessageBox::warning(this, "대상 오류", "자기 프로세스(PID)에는 감시를 시작할 수 없습니다.\n다른 프로세스를 선택하세요.");
            return;
        }
        // ---------- [ADD] 프리플라이트 가드 ----------
        auto isCriticalName = [](const QString& nameLower) {
            static const QSet<QString> ban = {
                "system", "smss.exe", "csrss.exe", "wininit.exe",
                "services.exe", "lsass.exe", "winlogon.exe"
            };
            return ban.contains(nameLower);
        };
        if (pid == 4 || isCriticalName(r.processName.toLower())) {
            QMessageBox::warning(this, "차단됨",
                                 "시스템/보호 프로세스에는 동적 감시를 붙일 수 없습니다.\n"
                                 "메모장(notepad.exe) 같은 일반 사용자 프로세스로 테스트하세요.");
            return;
        }
        DWORD sess = 0;
        if (ProcessIdToSessionId(DWORD(pid), &sess) && sess == 0) {
            QMessageBox::warning(this, "차단됨",
                                 "세션 0(서비스) 프로세스는 동적 감시 대상에서 제외됩니다.");
            return;
        }
        HANDLE hp = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, DWORD(pid));
        if (!hp) {
            QMessageBox::warning(this, "실패", "프로세스 핸들을 열 수 없습니다. (종료/권한 문제)");
            return;
        }
        BOOL dbg = FALSE;
        CheckRemoteDebuggerPresent(hp, &dbg);
        CloseHandle(hp);
        if (dbg) {
            QMessageBox::warning(this, "차단됨", "이미 다른 디버거가 붙어있는 프로세스입니다.");
            return;
        }
        // ---------------------------------------------
        bool autoKill = (chkAutoKill && chkAutoKill->isChecked());
        monitor->startMonitoring(DWORD(pid), autoKill);

        resultStatusLabel->setText(QString("🟢 동적 감시 시작 (PID %1) — 새 스레드 시작 ↔ DLL 로드 알림을 상관 분석합니다.").arg(pid));

        return;
    }
    // 🔴 1.5초 후 실제 탐지 수행
    QTimer::singleShot(1500, this, [=]() {
        if (method == "해시 기반") {
            const Result &res = cachedResults[lastSelectedRow];
            auto suspiciousDLLs = hashComparator.detectSuspiciousDLLs(res.dllList);
            QSet<QString> suspiciousSet;
            for (const auto& pair : suspiciousDLLs)
                suspiciousSet.insert(pair.second);

            // ✅ 로그 저장
            QString targetPid = QString::number(cachedResults[lastSelectedRow].pid);
            LogManager::writeBulkLog(res.dllList, suspiciousSet, cachedResults, "hash", "hash", targetPid);
            if (suspiciousDLLs.empty()) {
                showCleanResult();
            } else {
                showSuspiciousDLLs(suspiciousDLLs);
            }

        } else if (method == "WhitelistMLFilter") {
            qDebug() << "머신러닝 탐지 수행";
            const Result &res = cachedResults[lastSelectedRow];
            networkAnalyzer->analyzeDLLs(res.dllList);

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

    QTimer::singleShot(3000, this, [=]() {
        resultStatusLabel->clear(); // 또는 resultStatusLabel->hide();
    });
}

void MainWindow::showSuspiciousDLLs(const std::vector<std::pair<QString, QString>>& dlls) {
    if (currentStage != AppStage::DetectionStarted) {
        if (detectionResultWidget) detectionResultWidget->hide();
        return;
    }

    // 🔓 탐지 탭일 때만 표시
    detectionResultWidget->show();
    dllResultTable->setRowCount(static_cast<int>(dlls.size()));
    for (int i = 0; i < dlls.size(); ++i) {
        dllResultTable->setItem(i, 0, new QTableWidgetItem(dlls[i].first));
        dllResultTable->setItem(i, 1, new QTableWidgetItem(dlls[i].second));
    }

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

void MainWindow::onStartMonitorClicked(){
    int row = resultTable->currentRow();
    if(row<0) { warnUser("프로세스를 먼저 선택하세요"); return; }
    bool ok=false;
    int pid = resultTable->item(row, 0)->text().toInt(&ok);
    if(!ok) { warnUser("PID 해석 실패"); return; }
    monitor->startMonitoring(DWORD(pid), /*autoKill=*/false);
    warnUser(QString("PID %1 감시 시작").arg(pid));
}

void MainWindow::onStopMonitorClicked() {
    monitor->stopMonitoring();
    if (resultStatusLabel) {
        resultStatusLabel->setText("⏹ 감시 중지");
    }
}

void MainWindow::onMonitorAlert(const QString& action, int score, const QString& path){
    if(action=="warn"){
        warnUser(QString("[경고] 의심 점수 %1, DLL=%2").arg(score).arg(path));
    } else if(action=="terminate"){
        warnUser(QString("[차단] 프로세스 종료 (점수 %1)").arg(score));
    }
}

void MainWindow::onMonitorLog(const QString& s){
    qDebug() << s;
    // LogManager::writeLog() 연동 -> 승찬이
}
