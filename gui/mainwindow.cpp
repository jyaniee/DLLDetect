#include "mainwindow.h"

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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
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

    // 로고 영역
    QWidget *logoArea = new QWidget();
    logoArea->setFixedWidth(80);
    logoArea->setStyleSheet("background-color: #12131a;");

    QHBoxLayout *logoLayout = new QHBoxLayout(logoArea);
    logoLayout->setContentsMargins(10, 10, 0, 10);
    logoLayout->setAlignment(Qt::AlignCenter);

    QLabel *logo = new QLabel();
    logo->setPixmap(QPixmap(":/img/logo.svg").scaled(24, 24));
    logo->setFixedSize(24, 24);
    logoLayout->addWidget(logo);

    // 상단바 텍스트
    QLabel *titleLabel = new QLabel("Content Area");
    titleLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");

    topBarLayout->addWidget(logoArea);
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

    // ▶ 좌측 패널
    QWidget *sidePanel = new QWidget();
    sidePanel->setFixedWidth(80);
    sidePanel->setStyleSheet("background-color: #12131a;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(20);

    QStringList icons = {":/img/home.svg", ":/img/list.svg", ":/img/searching.svg", ":/img/pattern.svg"};
    for (const QString &icon : icons) {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(icon));
        btn->setIconSize(QSize(24, 24));
        btn->setStyleSheet("QToolButton { border: none; } QToolButton:hover { background-color: #2e2e3f; }");
        sideLayout->addWidget(btn, 0, Qt::AlignHCenter);
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

    QLabel *placeholder = new QLabel("[ 콘텐츠 영역 ]");
    placeholder->setStyleSheet("color: gray; font-size: 14px;");
    placeholder->setAlignment(Qt::AlignCenter);

    mainContentLayout->addStretch();
    mainContentLayout->addWidget(placeholder);
    mainContentLayout->addStretch();

    // ▶ 조립
    contentLayout->addWidget(sidePanel);
    contentLayout->addWidget(sideRightLine);
    contentLayout->addWidget(mainContent);
    contentLayout->setStretch(2, 1);

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(topLine);
    mainLayout->addWidget(contentArea);
}

MainWindow::~MainWindow() {
    // 기본 소멸자, 비워도 문제 없음
}
