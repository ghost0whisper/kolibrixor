#include "mainwindow.h"
#include "logger.h"

#include <QGroupBox>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    // , ui(new Ui::MainWindow)
    , m_fileProcessor(nullptr)
    , m_fileProcessorThread(nullptr)
    , m_fileMonitor(nullptr)
    , m_fileMonitorThread(nullptr)
    , m_isProcessing(false)
    , m_isPaused(false)
{
    setWindowTitle("Kolibri XOR");
    setGeometry(100, 100, 1000, 600);

    m_fileProcessorThread = new QThread(this);
    m_fileProcessor = new FileProcessor();
    m_fileProcessor->moveToThread(m_fileProcessorThread);

    m_fileMonitorThread = new QThread(this);
    m_fileMonitor = new FileMonitor();
    m_fileMonitor->moveToThread(m_fileMonitorThread);

    setupUI();
    createConnections();

    m_fileProcessorThread->start();
    m_fileMonitorThread->start();

    Logger::instance()->log("App is started");
}

MainWindow::~MainWindow()
{
    m_fileProcessorThread->quit();
    m_fileProcessorThread->wait();

    m_fileMonitorThread->quit();
    m_fileMonitorThread->wait();

    Logger::instance()->log("App is finished");
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);

    setCentralWidget(centralWidget);

    QBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    auto pathsBox = pathsBoxSetup();
    auto paramsBox = paramsBoxSetup();
    auto monitorBox = monitoringBoxSetup();
    auto controlBox = controlBoxSetup();
    auto progressBox = progressBoxSetup();
    auto logBox = logBoxSetup();

    mainLayout->addWidget(pathsBox);
    mainLayout->addWidget(paramsBox);
    mainLayout->addWidget(monitorBox);
    mainLayout->addWidget(controlBox);
    mainLayout->addWidget(progressBox);
    mainLayout->addWidget(logBox);
    mainLayout->addStretch();
    centralWidget->setLayout(mainLayout);
}

QHBoxLayout* MainWindow::inputLayoutSetup() {
    m_inputPathEdit = new QLineEdit();
    m_inputPathEdit->setPlaceholderText("C:/input");

    m_selectInputPathButton = new QPushButton("Overview...");
    m_selectInputPathButton->setMaximumWidth(100);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Input folder:"));
    inputLayout->addWidget(m_inputPathEdit);
    inputLayout->addWidget(m_selectInputPathButton);

    return inputLayout;
}

QHBoxLayout* MainWindow::outputLayoutSetup() {
    m_outputPathEdit = new QLineEdit();
    m_outputPathEdit->setPlaceholderText("C:/output");

    m_selectOutputPathButton = new QPushButton("Overview...");
    m_selectOutputPathButton->setMaximumWidth(100);

    QHBoxLayout *outputLayout = new QHBoxLayout();
    outputLayout->addWidget(new QLabel("Output folder:"));
    outputLayout->addWidget(m_outputPathEdit);
    outputLayout->addWidget(m_selectOutputPathButton);

    return outputLayout;
}

QHBoxLayout* MainWindow::filePatternLayoutSetup() {
    QHBoxLayout *patternLayout = new QHBoxLayout();
    patternLayout->addWidget(new QLabel("Files pattern:"));

    m_filePatternEdit = new QLineEdit();
    m_filePatternEdit->setText("*.txt");
    m_filePatternEdit->setMaximumWidth(200);
    m_filePatternEdit->setPlaceholderText("Example: *.txt or test.bin");

    patternLayout->addWidget(m_filePatternEdit);
    patternLayout->addStretch();

    return patternLayout;
}

QGroupBox* MainWindow::pathsBoxSetup() {
    auto inputLayout = inputLayoutSetup();
    auto outputLayout = outputLayoutSetup();
    auto patternLayout = filePatternLayoutSetup();

    QGroupBox *pathsGroup = new QGroupBox("Path processing", this);
    QVBoxLayout *pathsLayout = new QVBoxLayout(pathsGroup);

    pathsLayout->addLayout(inputLayout);
    pathsLayout->addLayout(outputLayout);
    pathsLayout->addLayout(patternLayout);

    return pathsGroup;
}

QHBoxLayout* MainWindow::xorPatternLayoutSetup() {
    QHBoxLayout *xorLayout = new QHBoxLayout();
    xorLayout->addWidget(new QLabel("XOR value (HEX, 8 byte):"));

    // TODO: Move pattern to resource
    const QString pattern = QString("1234567890ABCDEF");

    m_xorPatternEdit = new QLineEdit();
    m_xorPatternEdit->setText(pattern);
    m_xorPatternEdit->setMaximumWidth(200);
    m_xorPatternEdit->setPlaceholderText(pattern);

    xorLayout->addWidget(m_xorPatternEdit);
    xorLayout->addStretch();

    return xorLayout;
}

QHBoxLayout* MainWindow::overwriteLayoutSetup() {
    QHBoxLayout *overwriteLayout = new QHBoxLayout();
    overwriteLayout->addWidget(new QLabel("If file names match"));

    m_overwriteComboBox = new QComboBox();
    m_overwriteComboBox->addItem("Overwrite");
    m_overwriteComboBox->addItem("Add counter");
    m_overwriteComboBox->setMaximumWidth(200);

    overwriteLayout->addWidget(m_overwriteComboBox);
    overwriteLayout->addStretch();

    return overwriteLayout;
}

QGroupBox* MainWindow::paramsBoxSetup() {
    QGroupBox *paramsGroup = new QGroupBox("Processing parameters", this);
    QVBoxLayout *paramsLayout = new QVBoxLayout(paramsGroup);

    m_deleteSourceCheckBox = new QCheckBox("Delete source files after processing");

    auto xorLayout = xorPatternLayoutSetup();
    auto overwriteLayout = overwriteLayoutSetup();

    paramsLayout->addLayout(xorLayout);
    paramsLayout->addLayout(overwriteLayout);
    paramsLayout->addWidget(m_deleteSourceCheckBox);

    return paramsGroup;
}

QHBoxLayout* MainWindow::intervalLayoutSeteup() {
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel("Check interval (ms):"));

    m_monitorIntervalSpinBox = new QSpinBox();
    m_monitorIntervalSpinBox->setMinimum(100);
    m_monitorIntervalSpinBox->setMaximum(60000);
    m_monitorIntervalSpinBox->setValue(100);
    m_monitorIntervalSpinBox->setMaximumWidth(150);

    intervalLayout->addWidget(m_monitorIntervalSpinBox);
    intervalLayout->addStretch();

    return intervalLayout;
}

QGroupBox* MainWindow::monitoringBoxSetup() {
    QGroupBox *monitorGroup = new QGroupBox("Folder monitoring", this);
    QVBoxLayout *monitorLayout = new QVBoxLayout(monitorGroup);

    auto intervalLayout = intervalLayoutSeteup();

    monitorLayout->addLayout(intervalLayout);

    m_autoStartCheckBox = new QCheckBox("Auto start when a file is found");

    monitorLayout->addWidget(m_autoStartCheckBox);

    return monitorGroup;
}

QGroupBox* MainWindow::controlBoxSetup() {
    QGroupBox *controlGroup = new QGroupBox("Management");
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    m_startButton = new QPushButton("Start");
    m_startButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    m_pauseButton = new QPushButton("Pause");
    m_pauseButton->setEnabled(false);

    m_resumeButton = new QPushButton("Resume");
    m_resumeButton->setEnabled(false);

    m_stopButton = new QPushButton("Stop");
    m_stopButton->setEnabled(false);
    m_stopButton->setStyleSheet("background-color: #f44336; color: white;");

    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_pauseButton);
    controlLayout->addWidget(m_resumeButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addStretch();

    return controlGroup;
}

QGroupBox* MainWindow::progressBoxSetup() {
    QGroupBox *progressGroup = new QGroupBox("Execution progress", this);
    QVBoxLayout *progressLayout = new QVBoxLayout(progressGroup);

    m_fileInfoLabel = new QLabel("Waiting...");

    m_progressBar = new QProgressBar();
    m_progressBar->setValue(0);

    m_statusLabel = new QLabel("Status: Ready");

    progressLayout->addWidget(m_fileInfoLabel);
    progressLayout->addWidget(m_progressBar);
    progressLayout->addWidget(m_statusLabel);

    return progressGroup;
}

QGroupBox* MainWindow::logBoxSetup() {
    QGroupBox *logGroup = new QGroupBox("Log", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);

    m_logEdit = new QPlainTextEdit();
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(150);

    m_clearLogButton = new QPushButton("Clear log");
    m_clearLogButton->setMaximumWidth(150);

    logLayout->addWidget(m_logEdit);
    logLayout->addWidget(m_clearLogButton);

    return logGroup;
}

void MainWindow::createConnections() {
    connect(m_selectInputPathButton, &QPushButton::clicked, this, &MainWindow::onBrowseInput);
    connect(m_selectOutputPathButton, &QPushButton::clicked, this, &MainWindow::onBrowseOutput);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartMonitoring);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopMonitoring);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseMonitoring);
    connect(m_resumeButton, &QPushButton::clicked, this, &MainWindow::onResumeMonitoring);
    connect(m_clearLogButton, &QPushButton::clicked, this, &MainWindow::onClearLog);

    connect(Logger::instance(), &Logger::newMessage, this, &MainWindow::onLog);

    connect(m_fileProcessorThread, &QThread::finished, m_fileProcessor, &QObject::deleteLater);
    connect(m_fileMonitorThread, &QThread::finished, m_fileMonitor, &QObject::deleteLater);
}

void MainWindow::onPauseMonitoring() {
    QMetaObject::invokeMethod(m_fileProcessor, "pause", Qt::QueuedConnection);

    m_isPaused = true;

    updateButtonStates();

    Logger::instance()->log("Processing is stopped");
}

void MainWindow::onResumeMonitoring() {
    QMetaObject::invokeMethod(m_fileProcessor, "resume", Qt::QueuedConnection);

    m_isPaused = false;

    updateButtonStates();

    Logger::instance()->log("Processing is continue");
}

void MainWindow::onCancelProcessing() {
    QMetaObject::invokeMethod(m_fileProcessor, "cancel", Qt::QueuedConnection);

    m_fileQueue.clear();

    Logger::instance()->log("Processing is canceled");

    updateButtonStates();
}

void MainWindow::onBrowseInput() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choose directory of input files", m_inputPathEdit->text());

    if (!dir.isEmpty()) {
        m_inputPathEdit->setText(dir);
    }
}

void MainWindow::onBrowseOutput() {
    QString dir = QFileDialog::getExistingDirectory(this, "Choose directory of output files", m_outputPathEdit->text());

    if (!dir.isEmpty()) {
        m_outputPathEdit->setText(dir);
    }
}

void MainWindow::onStartMonitoring() {
    updateConfigFromUI();

    if (m_config.inputDirectory.isEmpty()) {
        const QString &message = QString("Error: set directory of input files");
        Logger::instance()->log(message, 1);
        QMessageBox::critical(this, "Error", message);
        return;
    }

    if (m_config.outputDirectoy.isEmpty()) {
        const QString &message = QString("Error: set directory of output files");
        Logger::instance()->log(message, 1);
        QMessageBox::critical(this, "Error", message);
        return;
    }

    QByteArray xorKey;
    auto xorHexValue = m_config.xorKey.data();

    if (!validateXorValue(xorHexValue, xorKey)) {
        const QString &message = QString("Error: invalid XOR value");
        Logger::instance()->log(message, 1);
        QMessageBox::critical(this, "Error", message);
        return;
    }

    m_config.xorKey = xorKey;

    auto monitorParams = createMonitorParams();

    QMetaObject::invokeMethod(
        m_fileMonitor, "startMonitoring", Qt::QueuedConnection, Q_ARG(FileMonitor::MonitorParams, monitorParams)
    );

    m_isProcessing = true;

    updateButtonStates();

    Logger::instance()->log(
        QString("Monitoring is started. Folder: %1, Pattern: %2").arg(m_config.inputDirectory, m_config.inputPattern)
    );
}

void MainWindow::onStopMonitoring() {
    QMetaObject::invokeMethod(m_fileMonitor, "stopMonitoring", Qt::QueuedConnection);

    m_isProcessing = false;
    m_fileQueue.clear();

    updateButtonStates();

    Logger::instance()->log("Monitoring is stopped");
}

void MainWindow::onFileFound(const QString &filePath) {
    m_fileQueue.enqueue(filePath);

    Logger::instance()->log(QString("File is detected: %1").arg(QFileInfo(filePath).fileName()));

    if (!m_isProcessing && m_autoStartCheckBox->isChecked()) {
        processNextFile();
    } else if (m_isProcessing && !m_isPaused) {
        processNextFile();
    }
}

bool MainWindow::validateXorValue(const QString &hexString, QByteArray &result) {
    QString hex = hexString.trimmed().toUpper();

    if (hex.length() != 16)
        return false;

    for (QChar c : hex) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
            return false;
    }

    result = QByteArray::fromHex(hex.toLatin1());

    return result.size() == 8;
}

QString MainWindow::generateOutputFileName(const QString &inputPath) {
    QFileInfo fileInfo(inputPath);
    QString baseName = fileInfo.baseName();
    QString suffix = fileInfo.suffix();
    QString outputDir = m_config.outputDirectoy;
    QString outputPartPath = outputDir + "/" + baseName + "_xor";
    QString outputPath = outputPartPath + "." + suffix;

    auto finalOutputPath = [outputPartPath, suffix](int counter) {
        return QString(outputPartPath + "_" + QString::number(counter) + "." + suffix);
    };

    if (QFileInfo::exists(outputPath)) {
        if (m_config.isOverwriteExistingFiles) {
            return outputPath;
        } else {
            int counter = 1;

            while (QFileInfo::exists(finalOutputPath(counter))) {
                counter++;
            }

            outputPath = finalOutputPath(counter);
        }
    }

    return outputPath;
}

void MainWindow::processNextFile() {
    if (m_fileQueue.isEmpty()) {
        return;
    }

    QString inputFile = m_fileQueue.dequeue();
    QString outputFile = generateOutputFileName(inputFile);

    FileProcessor::ProcessingParams params;
    params.inputFilePath = inputFile;
    params.outputFilePath = outputFile;
    params.xorValue = m_config.xorKey;
    params.bufferSize = 1024 * 1024; // 1 MB
    params.isDeleteSource = m_config.isDeleteSourceFiles;

    m_currentFile = QFileInfo(inputFile).fileName();

    QMetaObject::invokeMethod(
        m_fileProcessor, "processFile", Qt::QueuedConnection, Q_ARG(FileProcessor::ProcessingParams, params)
    );
}

void MainWindow::onProcessingStarted(const QString &filename) {
    updateStatus(QString("Processing %1...").arg(filename));

    m_progressBar->setValue(0);

    Logger::instance()->log(QString("Start processing %1").arg(filename));
}

void MainWindow::onProcessingProgress(qint64 processed, qint64 total) {
    int percent = (total > 0) ? (100 & processed / total) : 0;

    int mb = 1024 * 1024;

    m_progressBar->setValue(percent);

    m_fileInfoLabel->setText(
        QString("Executed: %1 MB / %2 MB")
        .arg(processed / mb)
        .arg(total / mb)
    );
}

void MainWindow::onProcessingFinished(bool success, const QString &message) {
    if (success) {
        Logger::instance()->log(QString("Success processed: %1").arg(m_currentFile));
    } else {
        Logger::instance()->log(QString("Error at processing: %1").arg(message), 1);
    }

    m_progressBar->setValue(100);

    updateStatus(QString("Ready"));

    if (!m_fileQueue.isEmpty()) {
        processNextFile();
    } else if (m_autoStartCheckBox->isChecked()) {
        updateStatus("Waiting a new files...");
    }
}

void MainWindow::onProcessingStatusChanged(const QString &status) {
    Logger::instance()->log(QString("i %1").arg(status));
}

void MainWindow::updateButtonStates() {
    m_startButton->setEnabled(!m_isProcessing);
    m_stopButton->setEnabled(m_isProcessing);
    m_pauseButton->setEnabled(m_isProcessing && !m_isPaused);
    m_resumeButton->setEnabled(m_isProcessing && m_isPaused);

    bool isEditEnabled = !m_isProcessing;

    m_inputPathEdit->setEnabled(isEditEnabled);
    m_outputPathEdit->setEnabled(isEditEnabled);
    m_filePatternEdit->setEnabled(isEditEnabled);
    m_xorPatternEdit->setEnabled(isEditEnabled);
}

void MainWindow::updateConfigFromUI() {
    m_config.inputDirectory = m_inputPathEdit->text();
    m_config.outputDirectoy = m_outputPathEdit->text();
    m_config.inputPattern = m_filePatternEdit->text();
    m_config.isDeleteSourceFiles = m_deleteSourceCheckBox->isChecked();
    m_config.isOverwriteExistingFiles = (m_overwriteComboBox->currentIndex() == 0);
    m_config.isTimerMode = m_autoStartCheckBox->isChecked();
    m_config.pollIntervalMs = m_monitorIntervalSpinBox->value();
}

void MainWindow::updateStatus(const QString &status) {
    m_statusLabel->setText(QString("Status: %1").arg(status));
}

FileMonitor::MonitorParams MainWindow::createMonitorParams() {
    FileMonitor::MonitorParams params;
    params.path = m_config.inputDirectory;
    params.filePattern = m_config.inputPattern;
    params.checkIntervalMs = m_config.pollIntervalMs;
    params.isEnabled = true;

    return params;
}

void MainWindow::onClearLog() {
    m_logEdit->clear();
}

void MainWindow::onLog(const QString &message) {
    m_logEdit->appendPlainText(message);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_isProcessing) {
        int isClose = QMessageBox::warning(
            this, "Close application?", "Files is processing", QMessageBox::Yes, QMessageBox::No
        );

        if (isClose == QMessageBox::No) {
            event->ignore();
            return;
        }

        onStopMonitoring();
        onCancelProcessing();
    }

    int threadWait = 5000;

    m_fileProcessorThread->quit();
    m_fileProcessorThread->wait(threadWait);

    m_fileMonitorThread->quit();
    m_fileMonitorThread->wait(threadWait);
}