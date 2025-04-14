#include "gui/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QPixmap>
#include <QSvgRenderer>
#include <QDirIterator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    QPixmap pix(":/img/home.svg");
    if (pix.isNull()) {
        qDebug() << "[ERROR] QPixmap으로 로드 실패!";
    } else {
        qDebug() << "[OK] QPixmap으로 로드 성공!";
    }
    QFile::exists(":/img/home.svg");
    QFile file(":/img/home.svg");
    if (!file.exists()) {
        qDebug() << "[ERROR] QFile 존재하지 않음!";
    } else {
        qDebug() << "[OK] QFile로 존재 확인됨!";
    }
    QSvgRenderer renderer(QString(":/img/home.svg"));
    if (!renderer.isValid()) {
        qDebug() << "[ERROR] QSvgRenderer: home.svg is invalid!";
    } else {
        qDebug() << "[OK] QSvgRenderer: home.svg loaded successfully.";
    }
    QDirIterator it(":/", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << "[RESOURCE]" << it.next();
    }

    return a.exec();
}
