#include "Utils/Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <iostream>

Logger::LogLevel Logger::s_logLevel = Logger::Info;
QString Logger::s_logFilePath;
QMutex Logger::s_mutex;

void Logger::initialize() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    s_logFilePath = appDataPath + "/widget_system.log";
}

void Logger::setLogLevel(LogLevel level) {
    s_logLevel = level;
}

void Logger::setLogFile(const QString& filePath) {
    s_logFilePath = filePath;
}

void Logger::debug(const QString& message) {
    log(Debug, message);
}

void Logger::info(const QString& message) {
    log(Info, message);
}

void Logger::warning(const QString& message) {
    log(Warning, message);
}

void Logger::error(const QString& message) {
    log(Error, message);
}

void Logger::log(LogLevel level, const QString& message) {
    if (level < s_logLevel) {
        return;
    }
    
    writeLog(level, message);
}

void Logger::writeLog(LogLevel level, const QString& message) {
    QMutexLocker locker(&s_mutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[%1] [%2] %3")
                         .arg(timestamp)
                         .arg(levelToString(level))
                         .arg(message);
    
    // 输出到控制台
    std::cout << logMessage.toStdString() << std::endl;
    
    // 输出到文件
    if (!s_logFilePath.isEmpty()) {
        QFile file(s_logFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            QTextStream stream(&file);
            stream << logMessage << Qt::endl;
            file.close();
        }
    }
}

QString Logger::levelToString(LogLevel level) {
    switch (level) {
        case Debug: return "DEBUG";
        case Info: return "INFO";
        case Warning: return "WARNING";
        case Error: return "ERROR";
        default: return "UNKNOWN";
    }
} 