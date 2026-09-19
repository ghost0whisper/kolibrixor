#include "mainwindow.h"
// #include "ui_mainwindow.h"
#include "logger.h"

#include <QGroupBox>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

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
    // ui->setupUi(this);
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
    // delete ui;
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
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
    connect(m_resumeButton, &QPushButton::clicked, this, &MainWindow::onResumeClicked);
    connect(m_clearLogButton, &QPushButton::clicked, this, &MainWindow::onClearLog);

    connect(Logger::instance(), &Logger::newMessage, this, &MainWindow::onLog);

    connect(m_fileProcessorThread, &QThread::finished, m_fileProcessor, &QObject::deleteLater);
    connect(m_fileMonitorThread, &QThread::finished, m_fileMonitor, &QObject::deleteLater);
}

void MainWindow::onStartClicked() {}

void MainWindow::onStopClicked() {}

void MainWindow:: onPauseClicked() {}

void MainWindow::onResumeClicked() {}

void MainWindow::onBrowseInput() {}

void MainWindow::onBrowseOutput() {}

void MainWindow::onStartMonitoring() {}

void MainWindow::onStopMonitoring() {}

void MainWindow::onFileFound(const QString *filePath) {}

bool MainWindow::validateXorValue(const QString &hexString, QByteArray &result) {}

QString MainWindow::generateOutputFileName(const QString &inputPath) {}

void MainWindow::onProcessingStarted(const QString &filename) {}
void MainWindow::onProcessingProgress(qint64 processed, qint64 total) {}
void MainWindow::onProcessingFinished(bool success, const QString &message) {}
void MainWindow::onProcessingStatusChanged(const QString &status) {}

void MainWindow::onClearLog() {}
void MainWindow::onLog(const QString &message) {}

void MainWindow::closeEvent(QCloseEvent *event) {}