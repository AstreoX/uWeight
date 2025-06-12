#pragma once

#include <QString>
#include <QMap>

class SystemInfoCollector {
public:
    static SystemInfoCollector& getInstance();

    struct SystemInfo {
        // CPU信息
        QString cpuModel;
        int cpuCores;
        double cpuUsage;
        
        // 内存信息
        qint64 totalMemory;
        qint64 usedMemory;
        qint64 availableMemory;
        
        // 系统信息
        QString osName;
        QString osVersion;
        QString computerName;
        QString userName;
        
        // 磁盘信息
        QMap<QString, QPair<qint64, qint64>> diskSpace; // 盘符 -> {总空间, 可用空间}
    };

    SystemInfo collectSystemInfo();
    double getCurrentCpuUsage();
    void updateMemoryInfo(qint64& total, qint64& available);
    QMap<QString, QPair<qint64, qint64>> getDiskSpace();

private:
    SystemInfoCollector() = default;
    ~SystemInfoCollector() = default;
    SystemInfoCollector(const SystemInfoCollector&) = delete;
    SystemInfoCollector& operator=(const SystemInfoCollector&) = delete;

    QString getCpuModel();
    int getCpuCores();
    void getSystemInfo(QString& osName, QString& osVersion, QString& computerName, QString& userName);
}; 