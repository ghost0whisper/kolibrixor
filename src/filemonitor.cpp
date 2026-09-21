#include "../headers/filemonitor.h"
#include "../headers/logger.h"
#include <QRegularExpression>
#include <QDir>
#include <QFileInfo>

Q_DECLARE_METATYPE(FileMonitor::MonitorParams)

FileMonitor::FileMonitor(QObject *parent)
    :QObject(parent)
    , m_isMonitoring(false)
{
    m_watcher = new QFileSystemWatcher(this);
    m_timer = new QTimer(this);

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &FileMonitor::onDirectoryChanged);
    connect(m_timer, &QTimer::timeout, this, &FileMonitor::onTimerTimeout);

    qRegisterMetaType<FileMonitor::MonitorParams>("FileMonitor::MonitorParams");
}

FileMonitor::~FileMonitor() {}

void FileMonitor::startMonitoring(const MonitorParams &params) {
    m_params = params;
    m_isMonitoring = true;
    m_lastFoundFiles.clear();

    if (!QDir(params.path).exists()) {
        m_isMonitoring = false;
        updateStatus(QString("The path directory is not exist: %1").arg(params.path));
        return;
    }

    if (!m_watcher->addPath(params.path))
        Logger::instance()->log("[WARNING]: Could not add path to file system watcher");

    m_timer->start(params.checkIntervalMs);

    updateStatus(
        QString("Monitoring started: %1 (pattern: %2, interval: %3ms)")
        .arg(params.path)
        .arg(params.filePattern)
        .arg(params.checkIntervalMs)
        );

    scanDirectory();
}

void FileMonitor::stopMonitoring() {
    m_timer->stop();
    m_watcher->removePaths(m_watcher->directories());
    m_isMonitoring = false;
    m_lastFoundFiles.clear();

    updateStatus("Monitoring stopped");
}

bool FileMonitor::isMonitoring() const {
    return m_isMonitoring;
}

void FileMonitor::setEnabled(bool isEnabled) {
    if (isEnabled && !m_isMonitoring) {
        startMonitoring(m_params);
    } else if (!isEnabled && m_isMonitoring) {
        stopMonitoring();
    }
}

void FileMonitor::updateStatus(const QString &message) {
    Logger::instance()->log(message);
    emit statusChanged(message);
}

void FileMonitor::onDirectoryChanged(const QString &path) {
    if (path == m_params.path && m_params.isEnabled)
        scanDirectory();
}

void FileMonitor::onTimerTimeout() {
    if (m_params.isEnabled)
        scanDirectory();
}

void FileMonitor::scanDirectory() {
    QDir dir(m_params.path);
    QStringList currentFiles;

    for (const QFileInfo &fileInfo : dir.entryInfoList(QDir::Files)) {
        if (matchesPattern(fileInfo.fileName()))
            currentFiles.append(fileInfo.absoluteFilePath());
    }

    for (const QString &filePath : currentFiles) {
        if (!m_lastFoundFiles.contains(filePath)) {
            Logger::instance()->log(QString("File found: %1").arg(filePath));
            emit fileFound(filePath);
        }
    }

    m_lastFoundFiles = currentFiles;
}

bool FileMonitor::matchesPattern(const QString &fileName) const {
    QRegularExpression regex = patternToRegex(m_params.filePattern);

    return regex.match(fileName).hasMatch();
}

QRegularExpression FileMonitor::patternToRegex(const QString &pattern) const {
    QString escaped = QRegularExpression::escape(pattern);
    escaped.replace(R"(\*)", ".*");
    escaped.replace(R"(\?)", ".");

    return QRegularExpression(QString("^%1$").arg(escaped));
}
