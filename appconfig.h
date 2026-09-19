#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QByteArray>

struct AppConfig {
    QString inputPattern;
    QString inputDirectory;
    QString outputDirectoy;
    bool isDeleteSourceFiles;
    bool isOverwriteExistingFiles;
    bool isTimerMode;
    int pollIntervalMs;
    QByteArray xorKey;
};

#endif
