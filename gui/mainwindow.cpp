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

    QHBoxLayout *rootLayout = new QHBoxLayout(central);
    central->setStyleSheet("background-color: #12131a;");

    // ▶ 좌측 패널
    QWidget *sidePanel = new QWidget();
    sidePanel->setFixedWidth(80);
    sidePanel->setStyleSheet("background-color: #1e1e2f;");

    QVBoxLayout *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(20);

    // 로고
    QLabel *logo = new QLabel();
    logo->setPixmap(QPixmap(":/img/logo.svg").scaled(24, 24));
    logo->setFixedSize(24, 24);
    logo->setScaledContents(true);
    sideLayout->addWidget(logo, 0, Qt::AlignHCenter);

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

    // 페이지 이름 (ex: Training)
    QLabel *pageTitle = new QLabel("Content Area");
    pageTitle->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(pageTitle);

    // 콘텐츠 Placeholder
    QLabel *placeholder = new QLabel("[ 콘텐츠 영역 ]");
    placeholder->setStyleSheet("color: gray; font-size: 14px;");
    placeholder->setAlignment(Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(placeholder);
    mainLayout->addStretch();

    // ▶ 레이아웃 구성
    rootLayout->addWidget(sidePanel);
    rootLayout->addWidget(mainContent);
    rootLayout->setStretch(0, 1); // 사이드바
    rootLayout->setStretch(1, 5); // 콘텐츠
}

MainWindow::~MainWindow() {
    // 기본 소멸자, 비워도 문제 없음
}
