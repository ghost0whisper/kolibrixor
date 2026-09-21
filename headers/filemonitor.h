#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>

class FileMonitor : public QObject {
    Q_OBJECT

public:
    struct MonitorParams {
        QString path;
        QString filePattern;
        qint64 checkIntervalMs;
        bool isEnabled;
    };

    explicit FileMonitor(QObject *parent = nullptr);
    ~FileMonitor();
    bool isMonitoring() const;
    void setEnabled(bool isEnabled);

public slots:
    void startMonitoring(const FileMonitor::MonitorParams &params);
    void stopMonitoring();

signals:
    void fileFound(const QString &filePath);
    void statusChanged(const QString &status);

private slots:
    void onDirectoryChanged(const QString &path);
    void onTimerTimeout();

private:
    void scanDirectory();
    bool matchesPattern(const QString &fileName) const;
    QRegularExpression patternToRegex(const QString &pattern) const;
    void updateStatus(const QString &message);

    MonitorParams m_params;
    QStringList m_lastFoundFiles;
    QFileSystemWatcher *m_watcher;
    QTimer *m_timer;
    bool m_isMonitoring;
};

#endif // FILEMONITOR_H
