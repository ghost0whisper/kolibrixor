#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <atomic>
#include <QMutex>
#include <QWaitCondition>

class FileProcessor : public QObject {
    Q_OBJECT

public:
    struct ProcessingParams {
        QString inputFilePath;
        QString outputFilePath;
        QByteArray xorValue;
        qint64 bufferSize;
        bool isDeleteSource;
    };

    explicit FileProcessor(QObject *parent = nullptr);
    ~FileProcessor();

    void pause();
    void resume();
    void stop();
    bool isPaused() const;
    bool isStopped() const;

public slots:
    void processFile(const FileProcessor::ProcessingParams &params);

signals:
    void fileProcessingStarted(const QString &path);
    void progressUpdated(qint64 bytesProcessed, qint64 totalBytes);
    void statusChanged(const QString &status);
    void fileProcessed(const QString &outputPath, bool isSuccess, bool isCanceled);

private:
    void doProcessFile(const ProcessingParams &params);
    bool validateXorValue(const QByteArray &xorValue);
    void finishProcess(const QString &outputPath, bool isSuccess, bool isCanceled, const QString &message);
    void checkDeletingSources(const QString &inputFilePath, bool isDeleteSource);

    mutable QMutex m_pauseMutex;
    QWaitCondition m_pauseCondition;
    std::atomic<bool> m_isPaused{false};
    std::atomic<bool> m_isStopped{false};
};

#endif
