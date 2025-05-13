#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QScrollArea>
#include <QVBoxLayout>
#include "Result.h"
#include <QMainWindow>
#include <QToolButton>
#include <QLabel>
#include <QVector>
#include <QString>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include "ui_mainwindow.h"
#include "NetworkDLLAnalyzer.h"
#include "ProcessManager.h"

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
    ProcessManager* processManager;
        std::vector<Result> cachedResults;
private slots:
    void onScanResult(const std::vector<Result>& results);
    void onAnalysisFinished(const QString &result);  // <-- 추가

private:

    Ui::MainWindow *ui;
    NetworkDLLAnalyzer *networkAnalyzer;
    QPushButton *loadButton;

    AppStage currentStage = AppStage::Home;
    QVector<QToolButton*> stageButtons;
    QLabel *mainLabel;
    QTableWidget *resultTable;
    QTableWidget *processTable;
    QTableWidget *dllTable;
    QScrollArea *dllScrollArea;
    void setupDLLArea();

    void handleStageClick(int index);
    void updateStage(AppStage newStage);
    void warnUser(const QString &msg);
    void loadProcesses();
    void clearTable();
    void handleRowClicked(int row, int column);

};




#endif // MAINWINDOW_H
