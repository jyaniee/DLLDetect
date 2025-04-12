#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>
#include <QLabel>
#include <QVector>
#include <QString>
#include <QTableWidget>
#include <QPushButton>

enum class AppStage {
    Home,
    ProcessSelected,
    DetectionStarted,
    LogSaved
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QPushButton *loadButton;

    AppStage currentStage = AppStage::Home;
    QVector<QToolButton*> stageButtons;
    QLabel *mainLabel;
    QTableWidget *resultTable;

    void handleStageClick(int index);
    void updateStage(AppStage newStage);
    void warnUser(const QString &msg);
    void loadProcesses();

};




#endif // MAINWINDOW_H
