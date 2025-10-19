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
#include <QSvgRenderer>
#include <QPainter>
#include <QMouseEvent>
#include <QWindow>
#include <QLineEdit>
#include <QCheckBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <windows.h>
// ------------------ MainWindow 생성자 ------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Window);

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

    // 상태 애니메이션 타이머 초기화
    statusAnimTimer = new QTimer(this);
    statusAnimTimer->setInterval(500);
    connect(statusAnimTimer, &QTimer::timeout, this, [this]() {
        dotCount = (dotCount + 1) % 4;
        QString dots(dotCount, QChar('.'));
        resultStatusLabel->setText(baseStatusText + dots);
    });


    // 기본 설정
    setWindowTitle("No Syringe");
    resize(1280, 800);

    QFontDatabase::addApplicationFont(":/fonts/DMSans-Bold.ttf");

    // 아이콘 및 스테이지 설정
    QStringList icons = {":/img/home.svg", ":/img/list.svg", ":/img/searching.svg", ":/img/pattern.svg"};
    iconPaths = icons;
    QStringList stages = {"Home", "Process", "Detection", "Log"};

    // 메인 중앙 위젯 구성
    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #12131a;");
    // setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---------------- 상단바 ----------------
   // QWidget *topBar = new QWidget();
    topBar = new QWidget();
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


    auto makeWinBtn = [](const QString &glyph, const QString &objName = QString()) {
        auto *b = new QPushButton(glyph);
        b->setObjectName(objName);
        b->setFocusPolicy(Qt::NoFocus);
        b->setFlat(true);
        b->setMinimumSize(46, 32);   // 40x28로 줄여도 됨
        b->setMaximumSize(46, 32);
        QFont f(QStringLiteral("Segoe MDL2 Assets"));  // 윈도우 기본 아이콘 폰트
        f.setPixelSize(10);                            // 크기
        b->setFont(f);
        return b;
    };

    // Segoe MDL2 Assets 코드포인트
    const QChar GLYPH_MIN   = QChar(0xE921); // ChromeMinimize
    const QChar GLYPH_MAX   = QChar(0xE922); // ChromeMaximize
    const QChar GLYPH_REST  = QChar(0xE923); // ChromeRestore
    const QChar GLYPH_CLOSE = QChar(0xE8BB); // ChromeClose

    QPushButton* btnMin   = makeWinBtn(QString(GLYPH_MIN),   "min");
    QPushButton* btnMax   = makeWinBtn(QString(GLYPH_MAX),   "max");
    QPushButton* btnClose = makeWinBtn(QString(GLYPH_CLOSE), "close");


    topBar->setStyleSheet(R"(
      QPushButton#min, QPushButton#max, QPushButton#close {
        color: #cfd3dc;
        background: transparent;
        border: none;
      }
      QPushButton#min:hover, QPushButton#max:hover {
        background: #2e2e3f;
      }
      QPushButton#min:pressed, QPushButton#max:pressed {
        background: #1f1f2b;
      }
      QPushButton#close:hover {
        background: #d9534f;  /* 닫기 버튼 호버 시 빨간 배경 */
        color: white;
      }
      QPushButton#close:pressed {
        background: #b13f3a;
      }
    )");

    topBarLayout->addStretch();
    topBarLayout->addWidget(btnMin, 0, Qt::AlignTop);
    topBarLayout->addWidget(btnMax, 0, Qt::AlignTop);
    topBarLayout->addWidget(btnClose, 0, Qt::AlignTop);
    topBarLayout->setContentsMargins(0, 0, 0, 0);

    connect(btnMin, &QPushButton::clicked, this, [this]{ showMinimized(); });
    connect(btnMax, &QPushButton::clicked, this,
            [this, btnMax, GLYPH_REST, GLYPH_MAX]() {
                isMaximized() ? showNormal() : showMaximized();
                btnMax->setText(isMaximized()
                                    ? QString(GLYPH_REST)
                                    : QString(GLYPH_MAX));
            });
    connect(btnClose, &QPushButton::clicked, this, [this]{ close(); });
    btnMax->setText(isMaximized() ? QString(GLYPH_REST) : QString(GLYPH_MAX));

    // 상단바 잡고 이동할 수 있게 구현
    topBar->setMouseTracking(true);
    topBar->installEventFilter(this);


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

        btn->setCheckable(true);
        btn->setAutoExclusive(true);

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
                color: #05C7F2;
            })");
        btn->setToolTip(stages[index]);
        sideLayout->addWidget(btn);
        stageButtons.append(btn);

        connect(btn, &QToolButton::clicked, this, [=]() {
            handleStageClick(index); // updateStage 내부에서 applySidebarSelection이 호출됨
            /*
            handleStageClick(index);

            for(int i = 0; i < stageButtons.size(); ++i) {
                QToolButton *b  = stageButtons[i];
                if(i == index){
                    // 선택된 버튼 -> 아이콘 색 #05C7F2로 다시 설정
                    QPixmap colored = QPixmap(24, 24);
                    colored.fill(Qt::transparent);
                    QSvgRenderer renderer(icons[i]);
                    QPainter p(&colored);
                    renderer.render(&p);
                    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                    p.fillRect(colored.rect(), QColor("#05C7F2"));
                    p.end();

                    b->setIcon(QIcon(colored));
                } else {
                    // 선택 안 된 버튼 -> 기본 아이콘
                    b->setIcon(QIcon(icons[i]));
                }
            }*/
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

    dllScrollArea->hide();

    //QVBoxLayout *contentSplitLayout = new QVBoxLayout();
    contentSplitLayout = new QVBoxLayout();
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

    setupHomePage();

    // ---------------- 조립 ----------------
    contentLayout->addWidget(sidePanel);
    contentLayout->addWidget(sideRightLine);
    contentLayout->addWidget(mainContent);
    contentLayout->setStretch(2, 1);

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(topLine);
    mainLayout->addWidget(contentArea);

    updateStage(AppStage::Home);
    if(!stageButtons.isEmpty()){
        stageButtons[0]->setChecked(true);

        QPixmap colored(24, 24);
        colored.fill(Qt::transparent);

        const QString homeIconPath = icons.value(0);
        QSvgRenderer renderer(homeIconPath);
        QPainter p(&colored);
        renderer.render(&p);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(colored.rect(), QColor("#05C7F2"));
        p.end();

        stageButtons[0]->setIcon(QIcon(colored));

    }

    // 루트 프레임(테두리)
    auto *windowFrame = new QWidget;
    windowFrame->setObjectName("windowFrame");

    auto *frameLayout = new QVBoxLayout(windowFrame);
    frameLayout->setContentsMargins(1,1,1,1);
    frameLayout->setSpacing(0);
    frameLayout->addWidget(central);

    auto *root = new QWidget;
    auto *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0,0,0,0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(windowFrame);
    setCentralWidget(root);

    this->windowFrame = windowFrame;
    this->frameLayout = frameLayout;

    windowFrame->setStyleSheet(R"(
      /* 기본 배경 */
      #windowFrame { background-color: #0F1016; }

      /* 창모드(테두리 표시) - 활성/비활성 색상 */
      #windowFrame[noBorder="false"][active="true"]  { border: 1px solid #05C7F2; } /* #05C7F2 */
      #windowFrame[noBorder="false"][active="false"] { border: 1px solid #3A3A45; } /* 비활성 회색 */

      /* 최대화/전체화면(테두리 제거) */
      #windowFrame[noBorder="true"] { border: none; }
    )");

    updateChromeBorder();

    QTimer::singleShot(0, this, [this](){
        updateStage(AppStage::Home);  // 레이아웃/지오메트리 완성 후에 최종 정렬
    });
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
            background-color: #02abe1;
            color: white;
            font: 900 12px;
            border-radius: 20px;
            font-family: 'Noto Sans KR';
            font-weight: 700;
        }
        QPushButton:hover {
            background-color: #0298CC;
        }
    )");

    connect(detectButton, &QPushButton::clicked, this, [=]() {
        handleStageClick(2);
    });

    detectButtonWrapper = new QWidget();
    QVBoxLayout* wrapperLayout = new QVBoxLayout(detectButtonWrapper);
    wrapperLayout->setContentsMargins(0, 30, 0, 10);
    wrapperLayout->addWidget(detectButton, 0, Qt::AlignHCenter);

    detectButtonWrapper->setVisible(false);

    layout->addWidget(detectButtonWrapper);
}



void MainWindow::handleStageClick(int index){
    switch (index){
    case 0: // HOME
        updateStage(AppStage::Home);
        ensureProcFilterBar();
        if(procFilterBar) {
            procFilterBar->hide();
            if (procFilterEdit) procFilterEdit->clear(); // 스테이지 떠날 때 초기화
        }
        resultTable->hide();
        dllScrollArea->hide();
        logViewer->hide();
        break;
    case 1:{ // 프로세스 목록
        updateStage(AppStage::ProcessSelected);
        ensureProcFilterBar();

        int exists = contentSplitLayout->indexOf(procFilterBar);
        if (exists < 0) {
            int resultIndex = contentSplitLayout->indexOf(resultTable);
            if (resultIndex >= 0) contentSplitLayout->insertWidget(resultIndex, procFilterBar);
            else                  contentSplitLayout->addWidget(procFilterBar);
        }
        if (procFilterBar) procFilterBar->show();
        logViewer->hide();
        dllScrollArea->show();
        loadProcesses();
        break;
    }
    case 2:
        if(currentStage >= AppStage::ProcessSelected) {
            updateStage(AppStage::DetectionStarted);
            ensureProcFilterBar();
            if(procFilterBar) {
                procFilterBar->hide();
                if(procFilterEdit) procFilterEdit->clear();
            }
            resultTable->hide();
            logViewer->hide();
            dllScrollArea->show();
        }else{
            warnUser("먼저 프로세스를 선택하세요.");
        }
        break;
    case 3:
        if (currentStage >= AppStage::DetectionStarted) {
            updateStage(AppStage::LogSaved);

            ensureProcFilterBar();
            if(procFilterBar) {
                procFilterBar->hide();
                if(procFilterEdit) procFilterEdit->clear();
            }

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

    auto hideIf = [](QWidget* w){ if (w) w->hide(); };

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

    if (detectButtonWrapper) detectButtonWrapper->setVisible(false);

        switch(currentStage){
        case AppStage::Home:
            //mainLabel->setText("홈");
            titleLabel->setText("Home");
            // 홈 위젯만 표시
            if (homeWidget) homeWidget->show();

            // 숨김
            if (resultTable) resultTable->hide();
            if (dllScrollArea) dllScrollArea->hide();
            if (logViewer) logViewer->hide();
            if (procFilterBar) procFilterBar->hide();
            if (detectionMethodWidget) detectionMethodWidget->hide();
            if (detectionResultWidget) detectionResultWidget->hide();
            if (detectButtonWrapper) detectButtonWrapper->hide();
            clearTable();
            clearDLLArea();
            break;
        case AppStage::ProcessSelected:
            //mainLabel->setText("프로세스 선택");
            titleLabel->setText("Process");
            hideIf(homeWidget);
            if (detectionResultWidget) detectionResultWidget->hide();
            break;
        case AppStage::DetectionStarted:
           // mainLabel->setText("DLL 탐지");
            titleLabel->setText("Detection");
            clearTable();
            clearDLLArea();
            hideIf(homeWidget);
            if(detectionMethodWidget) detectionMethodWidget->show();
            break;
        case AppStage::LogSaved:
            //mainLabel->setText("로그 저장");
            titleLabel->setText("Log");
            hideIf(homeWidget);
            if (detectionResultWidget) detectionResultWidget->hide();
            break;
        }
        applySidebarSelection(static_cast<int>(currentStage));


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
        if (detectButtonWrapper) detectButtonWrapper->setVisible(true);
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
            border-radius: 10px;
            min-width: 150px;
        }
        QPushButton:checked {
            border: 2px solid #02abe1;
        }
        QPushButton:hover {
            background-color: #2a2a3c;
        }
        )";

        // ===== [섹션 1] 정적 / 스냅샷형 탐지 =====
        QLabel* staticTitle = new QLabel("정적 / 스냅샷형 탐지");
        staticTitle->setStyleSheet("color: #ffffff; font-weight: 700; font-size: 16px;");
        outerLayout->addWidget(staticTitle);

        QLabel* hint = new QLabel("※ 정적/스냅샷형: 실행 중 스캔 / 파일·서명·해시 기반 확인");
        hint->setStyleSheet("color:#a0a7b4; font-size:12px; ");
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
       // dynamicTitle->setStyleSheet("color: #ffffff; font-weight: 700; font-size: 16px;");
        dynamicTitle->setStyleSheet(R"(
            color: #ffffff;

            font-weight: 700;
            font-size: 16px;
        )");
        outerLayout->addWidget(dynamicTitle);

        QLabel* hint2 = new QLabel("※ 동적(실시간): OS 알림 기반 / CreateRemoteThread+LoadLibrary 주입 전용 감시");
        // hint2->setStyleSheet("color:#a0a7b4; font-size:12px;");
        hint2->setStyleSheet(R"(
            color: #a0a7b4;

            font-size: 12px;
        )");
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

                // 선택만 했을 땐는 항상 stop 비활성 (감시 시작 전이므로)
                if (auto sb = detectionMethodWidget->findChild<QPushButton*>("stopMonitorBtn"))
                    sb->setEnabled(false);
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
        stopBtn->setObjectName("stopMonitorBtn");
        stopBtn->setFixedSize(120, 40);
        stopBtn->setStyleSheet(R"(
            QPushButton#stopMonitorBtn {
                border-radius: 20px;
                font-family: 'Noto Sans KR';
                font-weight: 700;
                padding-left: 12px; padding-right: 12px;
            }
            QPushButton#stopMonitorBtn:enabled {
                background-color: #ffffff;   /* 해제(사용 가능) = 경고/정지 컬러 */
                color: black;
                border: none;
            }
            QPushButton#stopMonitorBtn:enabled:hover {
                background-color: #f2f2f2;
            }
            QPushButton#stopMonitorBtn:disabled {
                background-color: #2e2e3f;   /* 잠금(비활성) = 어두운 회색 */
                color: #9aa0a6;
                border: 1px solid #3a3f5c;
            }
        )");
        stopBtn->setEnabled(false);  // 초기 비활성화
        connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopMonitorClicked);

        QPushButton* runBtn = new QPushButton("탐지 시작");
        runBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #02abe1;
                color: white;
                font-weight: bold;
                padding: 10px 20px;
                border-radius: 20px;
                font-family: 'Noto Sans KR';
                font-weight: 700;
            }
            QPushButton:hover {
                background-color: #0298CC;
            }
        )");
        runBtn->setFixedSize(120, 40);

        connect(runBtn, &QPushButton::clicked, this, [=]() {
            if (!selectedDetectionButton) {
                QMessageBox::warning(this, "선택 필요", "탐지 방식을 선택해주세요.");
                return;
            }
            if (currentSelectedPid <= 0) {  // 안전망
                QMessageBox::warning(this, "선택 필요", "프로세스를 먼저 선택해주세요.");
                return;
            }
            startDetectionWithMethod(selectedDetectionButton->text());
            // startStatusAnimation(currentSelectedPid);
        });
        runRow->addStretch();
        runRow->addWidget(stopBtn);
        runRow->addWidget(runBtn);
        outerLayout->addLayout(runRow);

        // outerLayout->addWidget(runBtn, 0, Qt::AlignRight);
        detectionMethodWidget->hide();
        layout->insertWidget(0, detectionMethodWidget);
    }



void MainWindow::startDetectionWithMethod(const QString& method) {
    stopStatusAnimation(); // 어떤 방식이든 시작 시점에 애니메이션/문구를 안전하게 리셋

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

    // 기본값: stop 버튼 잠가두기
    if (auto sb = detectionMethodWidget->findChild<QPushButton*>("stopMonitorBtn"))
        sb->setEnabled(false);

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
        // 1) 모니터 시작
        bool autoKill = (chkAutoKill && chkAutoKill->isChecked());
        monitor->startMonitoring(DWORD(pid), autoKill);

        // 실제 감시 시작 후에만 stop 버튼 활성화
        if (auto sb = detectionMethodWidget->findChild<QPushButton*>("stopMonitorBtn"))
            sb->setEnabled(true);

        // 2) 상태 애니메이션 시작
        startStatusAnimation(pid);
        // resultStatusLabel->setText(QString("🟢 동적 감시 시작 (PID %1) — 새 스레드 시작 ↔ DLL 로드 알림을 상관 분석합니다.").arg(pid));

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
    stopStatusAnimation();
    /*
    if (resultStatusLabel) {
        resultStatusLabel->setText("🔴 감시 중지");
    }*/
    if (auto sb = detectionMethodWidget->findChild<QPushButton*>("stopMonitorBtn"))
        sb->setEnabled(false);   // 중지 후 즉시 잠금
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


bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == topBar) {
        if (ev->type() == QEvent::MouseButtonDblClick) {
            isMaximized() ? showNormal() : showMaximized();
            return true;
        }
        if (ev->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::LeftButton) {
                if (windowHandle())
                    windowHandle()->startSystemMove(); // Qt 5.15+/Qt 6
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

// 창 테두리 상태 토글
void MainWindow::updateChromeBorder() {
    const bool noBorder = isMaximized() || isFullScreen();
    const bool active   = isActiveWindow();

    windowFrame->setProperty("noBorder", noBorder);
    windowFrame->setProperty("active",   active);

    // 테두리 있을 때만 1px 마진
    frameLayout->setContentsMargins(noBorder ? 0 : 1, noBorder ? 0 : 1,
                                    noBorder ? 0 : 1, noBorder ? 0 : 1);

    // 스타일 재적용
    windowFrame->style()->unpolish(windowFrame);
    windowFrame->style()->polish(windowFrame);
    windowFrame->update();
}

void MainWindow::changeEvent(QEvent* e) {
    QMainWindow::changeEvent(e);
    if (e->type() == QEvent::WindowStateChange) {
        updateChromeBorder();
    }
}

bool MainWindow::event(QEvent* e) {
    if (e->type() == QEvent::ActivationChange) {
        updateChromeBorder();
    }
    return QMainWindow::event(e);
}

void MainWindow::applySidebarSelection(int index) {
    for (int i = 0; i < stageButtons.size(); ++i) {
        QToolButton* b = stageButtons[i];
        b->setChecked(i == index);

        if (i == index) {
            // 선택 아이콘을 #05C7F2 로 재채색
            QPixmap colored(24, 24);
            colored.fill(Qt::transparent);
            QSvgRenderer renderer(iconPaths.value(i));
            QPainter p(&colored);
            renderer.render(&p);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(colored.rect(), QColor("#05C7F2"));
            p.end();
            b->setIcon(QIcon(colored));
        } else {
            // 비선택 아이콘은 원본
            b->setIcon(QIcon(iconPaths.value(i)));
        }
    }
}

void MainWindow::startStatusAnimation(qint64 pid)
{
    dotCount = 0;
    baseStatusText =
        QString("🟢 동적 감시 시작 (PID %1) — 새 스레드 시작 ↔ DLL 로드 알림을 상관 분석합니다").arg(pid);
    resultStatusLabel->setText(baseStatusText);
    if (!statusAnimTimer->isActive()) statusAnimTimer->start();
}

void MainWindow::stopStatusAnimation()
{
    if (statusAnimTimer->isActive()) statusAnimTimer->stop();
    // 멈출 때 최종 상태 고정(원하면 색상/이모지 변경)
    resultStatusLabel->setText("🔵 동적 감시 대기 — 탐지를 중지했습니다");
}


void MainWindow::filterProcessTable(const QString& text)
{
    if (!resultTable) return;

    const QString key = text.trimmed();
    const bool noKey = key.isEmpty();

    for (int r = 0; r < resultTable->rowCount(); ++r) {
        const QString pidStr = resultTable->item(r, 0) ? resultTable->item(r, 0)->text() : QString();
        const QString name   = resultTable->item(r, 1) ? resultTable->item(r, 1)->text() : QString();

        const bool match = noKey
                           || pidStr.contains(key, Qt::CaseInsensitive)
                           || name.contains(key, Qt::CaseInsensitive);

        resultTable->setRowHidden(r, !match);
    }
}


void MainWindow::ensureProcFilterBar()
{
    if (procFilterBar) return;

    procFilterBar = new QWidget(this);
    auto* lay = new QHBoxLayout(procFilterBar);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(8);

    procFilterEdit = new QLineEdit(procFilterBar);
    procFilterEdit->setPlaceholderText("프로세스 이름 또는 PID 검색...");
    procFilterEdit->setClearButtonEnabled(true);
    procFilterEdit->setStyleSheet(
        "QLineEdit { background:#1e1e2e; color:white; border:1px solid #2e2e3f; padding:6px 10px; border-radius:8px; }"
        );

    lay->addWidget(procFilterEdit, 1);

    // 필터 동작 연결
    connect(procFilterEdit, &QLineEdit::textChanged,
            this, &MainWindow::filterProcessTable);

    // 처음엔 숨김
    procFilterBar->hide();
}

void MainWindow::setupHomePage() {
    if (homeWidget) return;

    homeWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(homeWidget);
    layout->setContentsMargins(20, 0, 20, 0);  // 상단 마진 제거
    layout->setSpacing(20);

    // 위쪽 여백 추가
    layout->addStretch(1);

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix(":/img/no_syringe.png");

    if (!logoPix.isNull()) {
        // 크기 변경 (예: 240x240)
        QPixmap scaled = logoPix.scaled(420, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // 색상 변환 (흰색으로)
        QPixmap whiteLogo(scaled.size());
        whiteLogo.fill(Qt::transparent);
        {
            QPainter p(&whiteLogo);
            p.drawPixmap(0, 0, scaled);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(whiteLogo.rect(), QColor("#FFFFFF")); // 흰색으로 덮기
            p.end();
        }

        logoLabel->setPixmap(whiteLogo);
        logoLabel->setAlignment(Qt::AlignCenter);
    } else {
        // 로고 로드 실패 시 텍스트 표시
        logoLabel->setText("No Syringe");
        logoLabel->setStyleSheet("color: white; font-size: 32px; font-weight: 900;");
        logoLabel->setAlignment(Qt::AlignCenter);
    }


    QLabel* subtitle = new QLabel("Detect, Analyze, and Visualize Injection.");
    subtitle->setStyleSheet("color: #a0a7b4; font-size: 14px;");
    subtitle->setAlignment(Qt::AlignCenter);

    QLabel* desc = new QLabel(
        "DLL 인젝션 행위를 분석하고 탐지하는 시스템입니다.\n"
        "아래 버튼을 눌러 프로세스를 선택하고 탐지를 시작하세요."
        );
    desc->setStyleSheet("color: #d0d0d0; font-size: 13px;");
    desc->setAlignment(Qt::AlignCenter);

    QPushButton* startBtn = new QPushButton("프로세스 선택으로 이동");
    startBtn->setFixedSize(220, 45);
    startBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #02abe1;
            color: white;
            font-family: 'Noto Sans KR';
            font-weight: 700;
            border-radius: 22px;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #0298cc; }
    )");
    connect(startBtn, &QPushButton::clicked, this, [=]() {
        handleStageClick(1);
    });

    layout->addWidget(logoLabel);
    layout->addSpacing(10);
    layout->addWidget(subtitle);
    layout->addSpacing(20);
    layout->addWidget(desc);
    layout->addSpacing(30);
    layout->addWidget(startBtn, 0, Qt::AlignHCenter);

    // 아래쪽 여백 추가
    layout->addStretch(1);

    mainContentLayout->addWidget(homeWidget);
    homeWidget->hide();
}
