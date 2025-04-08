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
    setWindowTitle("Layout Structure");
    resize(1280, 800);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainVerticalLayout = new QVBoxLayout(central);
    mainVerticalLayout->setContentsMargins(0, 0, 0, 0);
    mainVerticalLayout->setSpacing(0);

    // ▶ 상단바
    QWidget *topBar = new QWidget();
    topBar->setFixedHeight(60);
    topBar->setStyleSheet("background-color: transparent;");

    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(10);

    // ▶ 루트 (좌측 패널 + 콘텐츠)
    QWidget *rootArea = new QWidget();
    QHBoxLayout *rootLayout = new QHBoxLayout(rootArea);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    central->setStyleSheet("background-color: #12131a;");

    // ▶ 좌측 패널
    QWidget *sidePanel = new QWidget();
    sidePanel->setFixedWidth(80);
    sidePanel->setStyleSheet("background-color: #1e1e2f;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(20);

    // 네비게이션 버튼들
    QStringList icons = {":/img/home.svg", ":/img/list.svg", ":/img/searching.svg", ":/img/pattern.svg"};
    for (const QString &iconPath : icons) {
        QToolButton *btn = new QToolButton();
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(24, 24));
        btn->setStyleSheet("QToolButton { border: none; } QToolButton:hover { background-color: #2e2e3f; }");
        sideLayout->addWidget(btn, 0, Qt::AlignHCenter);
    }
    sideLayout->addStretch();

    // ▶ 메인 콘텐츠 영역
    QWidget *mainContent = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainContent);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 콘텐츠 제목
    QLabel *pageTitle = new QLabel("Content Title");
    pageTitle->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    // 콘텐츠 Placeholder
    QLabel *placeholder = new QLabel("[ 콘텐츠 영역 ]");
    placeholder->setStyleSheet("color: gray; font-size: 14px;");
    placeholder->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(pageTitle);
    mainLayout->addStretch();
    mainLayout->addWidget(placeholder);
    mainLayout->addStretch();

    // 로고 (상단바 + 좌측 패널의 기준 정렬)
    QWidget *logoWrapper = new QWidget();
    logoWrapper->setFixedWidth(80);
    logoWrapper->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *logoLayout = new QHBoxLayout(logoWrapper);
    logoLayout->setContentsMargins(10, 10, 10, 10);
    logoLayout->setAlignment(Qt::AlignCenter);

    QLabel *logo = new QLabel();
    logo->setPixmap(QPixmap(":/img/logo.svg").scaled(24, 24));
    logo->setFixedSize(24, 24);
    logo->setScaledContents(true);

    logoLayout->addWidget(logo);

    topBarLayout->addWidget(logoWrapper);
    topBarLayout->addSpacing(4);
    topBarLayout->addWidget(pageTitle);
    topBarLayout->addStretch();

    // ▶ 조립
    rootLayout->addWidget(sidePanel);
    rootLayout->addWidget(mainContent);
    rootLayout->setStretch(0, 1);
    rootLayout->setStretch(1, 5);

    mainVerticalLayout->addWidget(topBar);
    mainVerticalLayout->addWidget(rootArea);
}


MainWindow::~MainWindow() {
    // 기본 소멸자, 비워도 문제 없음
}
