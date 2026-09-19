#include "filemonitor.h"
#include "logger.h"
#include <QRegularExpression>

FileMonitor::FileMonitor(QObject *parent)
    :QObject(parent)
    , m_isMonitoring(false)
{
    m_watcher = new QFileSystemWatcher(this);
    m_timer = new QTimer(this);

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &FileMonitor::onDirectoryChanged);
    connect(m_timer, &QTimer::timeout, this, &FileMonitor::onTimerTimeout);
}

FileMonitor::~FileMonitor() {}

void FileMonitor::startMonitoring(const MonitorParams &params) {}
void FileMonitor::stopMonitoring() {}
bool FileMonitor::isMonitoring() const {}
void FileMonitor::setEnabled(bool isEnabled) {}

void FileMonitor::onDirectoryChanged(const QString &path) {}
void FileMonitor::onTimerTimeout() {}

void FileMonitor::scanDirectory() {}
bool FileMonitor::matchesPattern(const QString &fileName) const {}

QRegularExpression FileMonitor::patternToRegex(const QString &pattern) const {
    QString escaped = QRegularExpression::escape(pattern);
    escaped.replace(R"(\*)", ".*");
    escaped.replace(R"(\?)", ".");

    return QRegularExpression(QString("^%1$").arg(escaped));
}
