#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QSpinBox>
#include <QLabel>
#include <QThread>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QQueue>
#include "appconfig.h"
#include "fileprocessor.h"
#include "filemonitor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // File execution
    void onFileFound(const QString &filePath);
    void onProcessingStarted(const QString &filename);
    void onProcessingProgress(qint64 processed, qint64 total);
    void onProcessingFinished(const QString &path, bool isSuccess, bool isCanceled);

    // User handling
    void onStartMonitoring();
    void onStopMonitoring();
    void onPauseProcessing();
    void onResumeProcessing();
    void onStopProcessing();
    void onBrowseInput();
    void onBrowseOutput();

    void updateStatus(const QString &status);
    void onClearLog();
    void onLog(const QString &message);

private:
    void setupUI();
    void createConnections();
    void updateButtonStates();
    void updateConfigFromUI();
    void setXorValueFromUI();
    void processNextFile();
    bool validateXorValue(const QString &hexString, QByteArray &result);
    QString generateOutputFileName(const QString &inputPath);
    FileMonitor::MonitorParams createMonitorParams();
    QHBoxLayout* inputLayoutSetup();
    QHBoxLayout* outputLayoutSetup();
    QHBoxLayout* filePatternLayoutSetup();
    QHBoxLayout* xorPatternLayoutSetup();
    QHBoxLayout* overwriteLayoutSetup();
    QHBoxLayout* intervalLayoutSeteup();
    QGroupBox* pathsBoxSetup();
    QGroupBox* paramsBoxSetup();
    QGroupBox* monitoringBoxSetup();
    QGroupBox* controlBoxSetup();
    QGroupBox* progressBoxSetup();
    QGroupBox* logBoxSetup();

    // Ui::MainWindow *ui;
    AppConfig m_config;
    QQueue<QString> m_fileQueue;
    bool m_isMonitoring;
    bool m_isProcessing;
    bool m_isPaused;

    FileProcessor *m_fileProcessor;
    QThread *m_fileProcessorThread;

    FileMonitor *m_fileMonitor;
    QThread *m_fileMonitorThread;

    // Input, output UI elements
    QLineEdit *m_inputPathEdit;
    QLineEdit *m_outputPathEdit;
    QPushButton *m_selectInputPathButton;
    QPushButton *m_selectOutputPathButton;

    // User settings UI elements
    QLineEdit *m_filePatternEdit;
    QLineEdit *m_xorPatternEdit;
    QCheckBox *m_deleteSourceCheckBox;
    QComboBox *m_overwriteComboBox;
    QSpinBox *m_monitorIntervalSpinBox;
    QCheckBox *m_autoStartCheckBox;

    // Control UI elements
    QPushButton *m_startButton;
    QPushButton *m_stopMonitoringButton;
    QPushButton *m_pauseButton;
    QPushButton *m_resumeButton;
    QPushButton *m_stopProcessingButton;

    // Progress UI elements
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_fileInfoLabel;

    // Log UI elements
    QPlainTextEdit *m_logEdit;
    QPushButton *m_clearLogButton;
};

#endif // MAINWINDOW_H
