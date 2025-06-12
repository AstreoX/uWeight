#include "Utils/SystemInfoCollector.h"
#include <QThread>
#include <QSysInfo>
#include <QStorageInfo>
#include <QHostInfo>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")

SystemInfoCollector& SystemInfoCollector::getInstance() {
    static SystemInfoCollector instance;
    return instance;
}

QString SystemInfoCollector::getCpuModel() {
    // 方法1：尝试从注册表获取CPU信息
    QString cpuName;
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR buffer[256];
        DWORD bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"ProcessorNameString", NULL, NULL, 
                           (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            cpuName = QString::fromWCharArray(buffer).trimmed();
        }
        RegCloseKey(hKey);
    }
    
    // 如果注册表方法成功，返回结果
    if (!cpuName.isEmpty()) {
        return cpuName;
    }
    
    // 方法2：备用方案 - 使用wmic命令
    QProcess process;
    process.start("wmic", QStringList() << "cpu" << "get" << "name" << "/format:list");
    process.waitForFinished(3000); // 3秒超时
    
    if (process.exitCode() == 0) {
        QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
        QRegularExpression regex("Name=(.+)");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch()) {
            cpuName = match.captured(1).trimmed();
            if (!cpuName.isEmpty()) {
                return cpuName;
            }
        }
    }
    
    // 如果都失败了，返回默认值
    return "Unknown CPU";
}

int SystemInfoCollector::getCpuCores() {
    return QThread::idealThreadCount();
}

double SystemInfoCollector::getCurrentCpuUsage() {
    static PDH_HQUERY cpuQuery = NULL;
    static PDH_HCOUNTER cpuTotal = NULL;
    static bool queryInitialized = false;
    static bool firstSample = true;
    static double lastCpuUsage = 0.0;
    
    // 初始化PDH查询
    if (!queryInitialized) {
        PDH_STATUS status = PdhOpenQuery(NULL, NULL, &cpuQuery);
        if (status != ERROR_SUCCESS) {
            qDebug() << "PdhOpenQuery failed with status:" << status;
            return 0.0;
        }
        
        // 添加CPU使用率计数器
        status = PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
        if (status != ERROR_SUCCESS) {
            qDebug() << "PdhAddEnglishCounter failed with status:" << status;
            PdhCloseQuery(cpuQuery);
            cpuQuery = NULL;
            return 0.0;
        }
        
        queryInitialized = true;
        qDebug() << "PDH CPU monitoring initialized successfully";
    }
    
    if (cpuQuery == NULL || cpuTotal == NULL) {
        return 0.0;
    }
    
    // 收集性能数据
    PDH_STATUS status = PdhCollectQueryData(cpuQuery);
    if (status != ERROR_SUCCESS) {
        qDebug() << "PdhCollectQueryData failed with status:" << status;
        return lastCpuUsage; // 返回上次的值
    }
    
    // 第一次采样需要跳过，因为需要两个采样点来计算差值
    if (firstSample) {
        firstSample = false;
        return 0.0;
    }
    
    // 获取格式化的计数器值
    PDH_FMT_COUNTERVALUE counterVal;
    status = PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    if (status != ERROR_SUCCESS) {
        qDebug() << "PdhGetFormattedCounterValue failed with status:" << status;
        return lastCpuUsage; // 返回上次的值
    }
    
    // 检查计数器状态
    if (counterVal.CStatus != ERROR_SUCCESS) {
        qDebug() << "Counter status error:" << counterVal.CStatus;
        return lastCpuUsage; // 返回上次的值
    }
    
    // 确保CPU使用率在合理范围内
    double cpuUsage = qBound(0.0, counterVal.doubleValue, 100.0);
    
    // 平滑处理：如果变化太大，使用加权平均
    if (lastCpuUsage > 0 && qAbs(cpuUsage - lastCpuUsage) > 30.0) {
        cpuUsage = (lastCpuUsage * 0.7) + (cpuUsage * 0.3);
    }
    
    lastCpuUsage = cpuUsage;
    return cpuUsage;
}

void SystemInfoCollector::updateMemoryInfo(qint64& total, qint64& available) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    total = memInfo.ullTotalPhys;
    available = memInfo.ullAvailPhys;
}

void SystemInfoCollector::getSystemInfo(QString& osName, QString& osVersion, QString& computerName, QString& userName) {
    osName = QSysInfo::prettyProductName();
    osVersion = QSysInfo::productVersion();
    computerName = QHostInfo::localHostName();
    userName = qgetenv("USERNAME");
}

QMap<QString, QPair<qint64, qint64>> SystemInfoCollector::getDiskSpace() {
    QMap<QString, QPair<qint64, qint64>> result;
    foreach(const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            QString rootPath = storage.rootPath();
            qint64 total = storage.bytesTotal();
            qint64 available = storage.bytesAvailable();
            result[rootPath] = qMakePair(total, available);
        }
    }
    return result;
}

SystemInfoCollector::SystemInfo SystemInfoCollector::collectSystemInfo() {
    SystemInfo info;
    
    // CPU信息
    info.cpuModel = getCpuModel();
    info.cpuCores = getCpuCores();
    info.cpuUsage = getCurrentCpuUsage();
    
    // 内存信息
    updateMemoryInfo(info.totalMemory, info.availableMemory);
    info.usedMemory = info.totalMemory - info.availableMemory;
    
    // 系统信息
    getSystemInfo(info.osName, info.osVersion, info.computerName, info.userName);
    
    // 磁盘信息
    info.diskSpace = getDiskSpace();
    
    return info;
} 