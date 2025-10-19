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
#include <QCheckBox>
#include <QPushButton>
#include "ui_mainwindow.h"
#include "NetworkDLLAnalyzer.h"
#include "ProcessManager.h"
#include "WhitelistManager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include "ProcessManager.h"
#include "LogViewerWidget.h"
#include "HashComparator.h"
#include "DebugEventMonitor.h"

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

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    HashComparator hashComparator;
    LogViewerWidget* logViewer;
    ProcessManager* processManager;
    std::vector<Result> cachedResults;
    WhitelistManager* whitelistManager;
    int lastSelectedRow = -1;
    int currentSelectedPid = -1;

    QWidget* topBar = nullptr;

    DebugEventMonitor* monitor{nullptr};

    QWidget* windowFrame = nullptr;
    QVBoxLayout* frameLayout = nullptr;
    void updateChromeBorder();

    QStringList iconPaths;
    void applySidebarSelection(int index);
private slots:
    void onScanResult(const std::vector<Result>& results);
    void onAnalysisFinished(const QString &result);
    void onStartMonitorClicked();
    void onStopMonitorClicked();
    void onMonitorLog(const QString& s);
    void onMonitorAlert(const QString& action, int score, const QString& path);
private:
    void saveLog(const QString& dllPath, int prediction, const QString& source);


private:
    QPushButton* dynamicButton = nullptr;
    QCheckBox* chkAutoKill = nullptr;

    QString lastAnalyzedDllPath;

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
    QVBoxLayout* dllLayout;
    QPushButton* detectButton = nullptr;
    QVBoxLayout* mainContentLayout = nullptr;
    QWidget* detectionMethodWidget = nullptr;
    QPushButton* selectedDetectionButton = nullptr;
    QPushButton* pebButton = nullptr;
    QPushButton* entropyButton = nullptr;
    QPushButton* networkButton = nullptr;
    QLabel *titleLabel;
    QWidget* detectionResultWidget = nullptr;
    QLabel* resultStatusLabel = nullptr;
    QTableWidget* dllResultTable = nullptr;
    QStringList currentDllList;
    void setupDLLArea();
    void clearDLLArea();
    void handleStageClick(int index);
    void updateStage(AppStage newStage);
    void warnUser(const QString &msg);
    void loadProcesses();
    void clearTable();
    void handleRowClicked(int row, int column);
    void setupDetectButtonArea(QVBoxLayout* layout);
    void setupDetectionMethodArea(QVBoxLayout* layout);
    void startDetectionWithMethod(const QString& method);
    void setupDetectionResultArea(QVBoxLayout* layout);
    void showCleanResult();
    void showSuspiciousDLLs(const std::vector<std::pair<QString, QString>>& dlls);
    void startCodeSignatureDetection();

    QPushButton* signatureButton = nullptr;


};




#endif // MAINWINDOW_H
