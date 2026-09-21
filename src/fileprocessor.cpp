#include "../headers/fileprocessor.h"
#include "../headers/logger.h"
#include <QFile>
#include <QFileInfo>

Q_DECLARE_METATYPE(FileProcessor::ProcessingParams)

FileProcessor::FileProcessor(QObject *parent)
    : QObject(parent) {
    qRegisterMetaType<FileProcessor::ProcessingParams>("FileProcessor::ProcessingParams");
}

FileProcessor::~FileProcessor() {}

void FileProcessor::processFile(const ProcessingParams &params) {
    if (!validateXorValue(params.xorValue)) {
        finishProcess("", false, false, "XOR value validation failed");
        return;
    }

    m_isPaused = false;
    m_isStopped = false;

    doProcessFile(params);
}

void FileProcessor::pause() {
    QMutexLocker locker(&m_pauseMutex);

    m_isPaused = true;

    const QString &message = "Processing paused";
    Logger::instance()->log(message);
    emit statusChanged(message);
}

void FileProcessor::resume() {
    QMutexLocker locker(&m_pauseMutex);

    m_isPaused = false;
    m_pauseCondition.wakeAll();

    const QString &message = "Processing resumed";
    Logger::instance()->log(message);
    emit statusChanged(message);
}

void FileProcessor::stop() {
    QMutexLocker locker(&m_pauseMutex);

    m_isStopped = true;
    m_pauseCondition.wakeAll();

    Logger::instance()->log("Processing stopped");
}

bool FileProcessor::isPaused() const {
    return m_isPaused;
}

bool FileProcessor::isStopped() const {
    return m_isStopped;
}

void FileProcessor::doProcessFile(const ProcessingParams &params) {
    QFile inputFile(params.inputFilePath);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        finishProcess("", false, false, QString("Cannot open input file: %1").arg(params.inputFilePath));
        return;
    }

    QFile outputFile(params.outputFilePath);

    if (!outputFile.open(QIODevice::WriteOnly)) {
        inputFile.close();
        finishProcess("", false, false, QString("Cannot open output file: %1").arg(params.outputFilePath));
        return;
    }

    emit fileProcessingStarted(QFileInfo(params.outputFilePath).fileName());

    qint64 totalBytes = inputFile.size();
    qint64 bytesProcessed = 0;

    while (!inputFile.atEnd() && !m_isStopped) {
        {
            QMutexLocker locker(&m_pauseMutex);

            while (m_isPaused && !m_isStopped) {
                m_pauseCondition.wait(&m_pauseMutex);
            }
        }

        if (m_isStopped)
            break;

        QByteArray buffer = inputFile.read(params.bufferSize);

        if (buffer.isEmpty())
            break;

        for (int i = 0; i < buffer.size(); ++i) {
            buffer[i] ^= params.xorValue[i % params.xorValue.size()];
        }

        qint64 writtenChunk = outputFile.write(buffer);

        if (writtenChunk != buffer.size()) {
            inputFile.close();
            outputFile.close();
            finishProcess("", false, false, "Write error at file processing");
            return;
        }

        bytesProcessed += buffer.size();
        emit progressUpdated(bytesProcessed, totalBytes);
    }

    inputFile.close();
    outputFile.close();

    if (m_isStopped) {
        QFile::remove(params.outputFilePath);
        finishProcess("", false, true, "File processing stopped");
        return;
    }

    checkDeletingSources(params.inputFilePath, params.isDeleteSource);

    finishProcess(
        params.outputFilePath,
        true,
        false,
        QString("File processed successfully: %1").arg(params.outputFilePath)
    );
}

void FileProcessor::finishProcess(
    const QString &outputPath,
    bool isSuccess,
    bool isCanceled,
    const QString &message
) {
    Logger::instance()->log(message);

    emit statusChanged(message);
    emit fileProcessed(outputPath, isSuccess, isCanceled);
}

void FileProcessor::checkDeletingSources(const QString &inputFilePath, bool isDeleteSource) {
    if (isDeleteSource) {
        if (!QFile::remove(inputFilePath)) {
            const QString &message = QString("Warning: Could not delete source file: %1").arg(inputFilePath);
            Logger::instance()->log(message);
        }
    }
}

bool FileProcessor::validateXorValue(const QByteArray &xorValue) {
    return xorValue.size() == 8;
}