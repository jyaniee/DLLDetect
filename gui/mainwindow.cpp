#include "mainwindow.h"
#include "ProcessManager.h"
#include "Result.h"


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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    QStringList icons = {":/img/home.svg", ":/img/list.svg", ":/img/searching.svg", ":/img/pattern.svg"};
    QStringList stages = {"Home", "Process", "Detection", "Log"};
    int index = 0;

    setWindowTitle("Filter Dashboard");
    resize(1280, 800);

    // 전체 윈도우 배경
    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #12131a;");
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ▶ 상단바
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

    // 상단바 텍스트
    QLabel *titleLabel = new QLabel("Content Area");
    titleLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");

    // 로고와 텍스트 사이 구분선 추가
    QFrame *logoSeparator = new QFrame();
    logoSeparator->setFrameShape(QFrame::VLine);
    logoSeparator->setFrameShadow(QFrame::Plain);
    logoSeparator->setStyleSheet("color: #2e2e3f;");
    logoSeparator->setFixedWidth(3);

    // 상단바 레이아웃 구성
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

    // ▶ 콘텐츠 전체 구역 (좌측 패널 + 본문)
    QWidget *contentArea = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // ▶ 좌측 패널 (사이드바 크기 60px로 조정)
    QWidget *sidePanel = new QWidget();
    sidePanel->setFixedWidth(72);  // 기존 80 → 60으로 조정
    sidePanel->setStyleSheet("background-color: #12131a;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(20);

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

    // ▶ 본문 영역
    QWidget *mainContent = new QWidget();
    QVBoxLayout *mainContentLayout = new QVBoxLayout(mainContent);
    mainContentLayout->setContentsMargins(20, 20, 20, 20);

    mainLabel = new QLabel("[ 콘텐츠 영역 ]");
    mainLabel->setStyleSheet("color: gray; font-size: 14px;");
    mainContentLayout->addWidget(resultTable);
    mainLabel->setAlignment(Qt::AlignCenter);

    mainContentLayout->addStretch();
    mainContentLayout->addWidget(mainLabel);
    mainContentLayout->addStretch();

    // ▶ 조립
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

void MainWindow::handleStageClick(int index){
    switch (index){
    case 0: // HOME
        updateStage(AppStage::Home);
        break;
    case 1: // 프로세스 목록
        updateStage(AppStage::ProcessSelected);
        loadProcesses();
        break;
    case 2:
        if(currentStage >= AppStage::ProcessSelected)
            updateStage(AppStage::DetectionStarted);
        else
            warnUser("먼저 프로세스를 선택하세요.");
        break;
    case 3:
        if(currentStage >= AppStage::DetectionStarted)
            updateStage(AppStage::LogSaved);
        else
            warnUser("먼저 탐지를 시작하세요.");
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

    if(mainLabel){
        switch(currentStage){
        case AppStage::Home:
            mainLabel->setText("홈");
            break;
        case AppStage::ProcessSelected:
            mainLabel->setText("프로세스 선택");
            break;
        case AppStage::DetectionStarted:
            mainLabel->setText("DLL 탐지");
            break;
        case AppStage::LogSaved:
            mainLabel->setText("로그 저장");
            break;
        }
    }
}
void MainWindow::loadProcesses() {
    ProcessManager manager;
    std::vector<Result> results = manager.getProcessList();

    resultTable->clearContents();
    resultTable->setRowCount(static_cast<int>(results.size()));

    for (int i = 0; i < static_cast<int>(results.size()); ++i) {
        const Result &res = results[i];
        resultTable->setItem(i, 0, new QTableWidgetItem(QString::number(res.pid)));
        resultTable->setItem(i, 1, new QTableWidgetItem(res.processName));
        resultTable->setItem(i, 2, new QTableWidgetItem(QString::number(res.dllList.size())));
    }

    resultTable->show();

    if (results.empty()) {
        QMessageBox::information(this, "결과 없음", "프로세스를 불러오지 못했습니다.");
    }
}

void MainWindow::warnUser(const QString &msg){
    QMessageBox::warning(this, "안내", msg);
}

