#include "logger.h"
#include <QDateTime>

Logger* Logger::m_instance = nullptr;
QMutex Logger::m_mutex;

Logger* Logger::instance() {
    QMutexLocker locker(&m_mutex);

    if (!m_instance) {
        m_instance = new Logger();
    }

    return m_instance;
}

Logger::Logger(QObject *parent) : QObject(parent) {}

void Logger::log(const QString &message, int level) {
    QString levelStr;

    // TODO: Create enum
    switch(level) {
    case 1: levelStr = "[WARNING] "; break;
    case 2: levelStr = "[ERROR "; break;
    default: levelStr = "[INFO] "; break;
    }

    // TODO: Move to resource time template
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString fullMessage = QString("[%1] %2%3").arg(timestamp, levelStr, message);

    emit newMessage(fullMessage);
}