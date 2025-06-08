#pragma once
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger {
public:
    enum LogLevel {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3
    };

    static void initialize();
    static void setLogLevel(LogLevel level);
    static void setLogFile(const QString& filePath);
    
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
    
    static void log(LogLevel level, const QString& message);

private:
    static void writeLog(LogLevel level, const QString& message);
    static QString levelToString(LogLevel level);
    
    static LogLevel s_logLevel;
    static QString s_logFilePath;
    static QMutex s_mutex;
}; 