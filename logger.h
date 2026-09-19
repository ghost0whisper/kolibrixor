#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QMutex>

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger* instance();
    void log(const QString &message, int level = 0);

signals:
    void newMessage(const QString &text);

private:
    explicit Logger(QObject *parent = nullptr);
    static Logger *m_instance;
    static QMutex m_mutex;
};

#endif
