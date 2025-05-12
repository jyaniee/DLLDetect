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
    std::vector<Result> cachedResults;
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
