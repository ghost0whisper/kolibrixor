#include "fileprocessor.h"
#include "logger.h"

FileProcessor::FileProcessor(QObject *parent)
    : QObject(parent) {}

FileProcessor::~FileProcessor() {}

void FileProcessor::processFile(const ProcessingParams &params) {}
void FileProcessor::pause() {}
void FileProcessor::resume() {}
void FileProcessor::stop() {}
bool FileProcessor::isPaused() const {}

void FileProcessor::doProcessFile(const ProcessingParams &params) {}
bool FileProcessor::validateXorValue(const QByteArray &xorValue) {}