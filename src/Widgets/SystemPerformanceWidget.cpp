#include "Widgets/SystemPerformanceWidget.h"
#include <QPainter>
#include <QJsonObject>
#include <QRect>
#include <QDebug>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#include <iphlpapi.h>
#endif

// PerformanceMonitor
PerformanceMonitor::PerformanceMonitor(QObject* parent)
    : QThread(parent)
    , m_running(true)
    , m_lastBytesReceived(0)
    , m_lastBytesSent(0)
#ifdef Q_OS_WIN
    , m_hQuery(nullptr)
    , m_hCpuTotal(nullptr)
    , m_hDiskTime(nullptr)
    , m_hNetworkTotal(nullptr)
#endif
{
    m_lastNetworkCheckTime = QDateTime::currentDateTime();
#ifdef Q_OS_WIN
    initializePdh();
#endif
}

PerformanceMonitor::~PerformanceMonitor() {
    m_running = false;
    wait();
#ifdef Q_OS_WIN
    uninitializePdh();
#endif
}

void PerformanceMonitor::stopMonitoring() {
    m_running = false;
}

PerformanceData PerformanceMonitor::getCurrentData() const {
    QMutexLocker locker(&m_dataMutex);
    return m_currentData;
}

void PerformanceMonitor::run() {
    while (m_running) {
#ifdef Q_OS_WIN
        if (m_hQuery) {
            collectPerformanceData();
        } else {
            qDebug() << "PDH query not initialized, falling back to basic monitoring";
            // 使用基本监控方法
            collectPerformanceData();
        }
#else
        collectPerformanceData();
#endif
        QThread::msleep(1000); // 每秒更新一次
    }
}

void PerformanceMonitor::collectPerformanceData() {
    PerformanceData data;
    data.timestamp = QDateTime::currentDateTime();

#ifdef Q_OS_WIN
    if (m_hQuery) {
        // 使用PDH API获取性能数据
        data.cpuUsage = getCpuUsagePdh();
        data.diskUsage = getDiskUsagePdh();
        getNetworkInfoPdh(data.networkUpload, data.networkDownload);
    } else {
        // 使用基本方法获取性能数据
        data.cpuUsage = getCpuUsage();
        getNetworkInfo(data.networkUpload, data.networkDownload);
    }
#else
    // 非Windows平台使用基本方法
    data.cpuUsage = getCpuUsage();
    getNetworkInfo(data.networkUpload, data.networkDownload);
#endif

    // 获取内存和磁盘信息（这些使用Windows API而不是PDH）
    getMemoryInfo(data.totalMemory, data.usedMemory);
    getDiskInfo(data.totalDisk, data.usedDisk);
    
    data.memoryUsage = data.totalMemory > 0 ? (double)data.usedMemory / data.totalMemory * 100.0 : 0.0;

    {
        QMutexLocker locker(&m_dataMutex);
        m_currentData = data;
    }

    emit dataUpdated(data);
}

#ifdef Q_OS_WIN
void PerformanceMonitor::initializePdh() {
    PDH_STATUS status = PdhOpenQuery(nullptr, 0, &m_hQuery);
    if (status != ERROR_SUCCESS) {
        qDebug() << "Failed to open PDH query";
        m_hQuery = nullptr;
        return;
    }

    // 添加CPU计数器
    status = PdhAddCounter(m_hQuery, L"\\Processor(_Total)\\% Processor Time", 0, &m_hCpuTotal);
    if (status != ERROR_SUCCESS) {
        qDebug() << "Failed to add CPU counter";
    }

    // 添加磁盘计数器
    status = PdhAddCounter(m_hQuery, L"\\PhysicalDisk(_Total)\\% Disk Time", 0, &m_hDiskTime);
    if (status != ERROR_SUCCESS) {
        qDebug() << "Failed to add disk counter";
    }

    // 添加网络计数器
    status = PdhAddCounter(m_hQuery, L"\\Network Interface(*)\\Bytes Total/sec", 0, &m_hNetworkTotal);
    if (status != ERROR_SUCCESS) {
        qDebug() << "Failed to add network counter";
    }

    // 收集第一个样本
    status = PdhCollectQueryData(m_hQuery);
    if (status != ERROR_SUCCESS) {
        qDebug() << "Failed to collect initial PDH data";
    }
}

void PerformanceMonitor::uninitializePdh() {
    if (m_hQuery) {
        PdhCloseQuery(m_hQuery);
        m_hQuery = nullptr;
    }
}

double PerformanceMonitor::getCpuUsagePdh() {
    if (!m_hQuery || !m_hCpuTotal) {
        return getCpuUsage(); // 回退到基本方法
    }

    PDH_STATUS status = PdhCollectQueryData(m_hQuery);
    if (status != ERROR_SUCCESS) {
        return getCpuUsage();
    }

    PDH_FMT_COUNTERVALUE value;
    status = PdhGetFormattedCounterValue(m_hCpuTotal, PDH_FMT_DOUBLE, nullptr, &value);
    if (status != ERROR_SUCCESS) {
        return getCpuUsage();
    }

    return value.doubleValue;
}

double PerformanceMonitor::getDiskUsagePdh() {
    if (!m_hQuery || !m_hDiskTime) {
        return 0.0; // 没有基本的回退方法
    }

    PDH_STATUS status = PdhCollectQueryData(m_hQuery);
    if (status != ERROR_SUCCESS) {
        return 0.0;
    }

    PDH_FMT_COUNTERVALUE value;
    status = PdhGetFormattedCounterValue(m_hDiskTime, PDH_FMT_DOUBLE, nullptr, &value);
    if (status != ERROR_SUCCESS) {
        return 0.0;
    }

    return value.doubleValue;
}

void PerformanceMonitor::getNetworkInfoPdh(double& upload, double& download) {
    if (!m_hQuery || !m_hNetworkTotal) {
        getNetworkInfo(upload, download); // 回退到基本方法
        return;
    }

    PDH_STATUS status = PdhCollectQueryData(m_hQuery);
    if (status != ERROR_SUCCESS) {
        getNetworkInfo(upload, download);
        return;
    }

    PDH_FMT_COUNTERVALUE value;
    status = PdhGetFormattedCounterValue(m_hNetworkTotal, PDH_FMT_DOUBLE, nullptr, &value);
    if (status != ERROR_SUCCESS) {
        getNetworkInfo(upload, download);
        return;
    }

    // PDH返回的是总字节/秒，我们将其分成上传和下载
    // 这是一个简化的处理方式，实际上可能需要分别监控上传和下载计数器
    double totalBytes = value.doubleValue;
    upload = totalBytes / 2.0 / 1024.0;    // 转换为KB/s
    download = totalBytes / 2.0 / 1024.0;   // 转换为KB/s
}

void PerformanceMonitor::getMemoryInfo(qint64& total, qint64& used) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        total = memInfo.ullTotalPhys / (1024 * 1024); // MB
        used = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024); // MB
    } else {
        total = 0; 
        used = 0;
    }
}

void PerformanceMonitor::getDiskInfo(qint64& total, qint64& used) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        total = totalBytes.QuadPart / (1024 * 1024 * 1024); // GB
        used = (totalBytes.QuadPart - totalFreeBytes.QuadPart) / (1024 * 1024 * 1024); // GB
    } else {
        total = 0;
        used = 0;
    }
}

double PerformanceMonitor::getCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;
    static FILETIME lastIdleTime = {0, 0}, lastKernelTime = {0, 0}, lastUserTime = {0, 0};
    static double lastCpuUsage = 0.0;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return lastCpuUsage;
    }

    ULARGE_INTEGER idle, kernel, user, lastIdle, lastKernel, lastUser;
    idle.LowPart = idleTime.dwLowDateTime;
    idle.HighPart = idleTime.dwHighDateTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;

    lastIdle.LowPart = lastIdleTime.dwLowDateTime;
    lastIdle.HighPart = lastIdleTime.dwHighDateTime;
    lastKernel.LowPart = lastKernelTime.dwLowDateTime;
    lastKernel.HighPart = lastKernelTime.dwHighDateTime;
    lastUser.LowPart = lastUserTime.dwLowDateTime;
    lastUser.HighPart = lastUserTime.dwHighDateTime;

    ULONGLONG idleDiff = idle.QuadPart - lastIdle.QuadPart;
    ULONGLONG kernelDiff = kernel.QuadPart - lastKernel.QuadPart;
    ULONGLONG userDiff = user.QuadPart - lastUser.QuadPart;
    ULONGLONG totalDiff = kernelDiff + userDiff;

    if (totalDiff > 0) {
        lastCpuUsage = ((totalDiff - idleDiff) * 100.0) / totalDiff;
    }

    lastIdleTime = idleTime;
    lastKernelTime = kernelTime;
    lastUserTime = userTime;

    return lastCpuUsage;
}

void PerformanceMonitor::getNetworkInfo(double& upload, double& download) {
    MIB_IFTABLE* pIfTable = nullptr;
    DWORD dwSize = 0;
    static DWORD lastTime = 0;
    static ULONG64 lastBytesIn = 0;
    static ULONG64 lastBytesOut = 0;
    DWORD currentTime = GetTickCount();
    
    upload = download = 0.0;  // 初始化为0
    
    // 获取所需缓冲区大小
    if (GetIfTable(nullptr, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER) {
        pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable) {
            if (GetIfTable(pIfTable, &dwSize, TRUE) == NO_ERROR) {
                ULONG64 totalBytesIn = 0;
                ULONG64 totalBytesOut = 0;
                
                // 累加所有活动的网络接口的流量
                for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
                    // 检查接口是否为以太网或无线网络接口
                    if (pIfTable->table[i].dwType == IF_TYPE_ETHERNET_CSMACD ||
                        pIfTable->table[i].dwType == IF_TYPE_IEEE80211) {
                        
                        // 检查接口是否处于活动状态且有流量
                        if ((pIfTable->table[i].dwOperStatus == IF_OPER_STATUS_OPERATIONAL) &&
                            (pIfTable->table[i].dwInOctets > 0 || pIfTable->table[i].dwOutOctets > 0)) {
                            
                            totalBytesIn += pIfTable->table[i].dwInOctets;
                            totalBytesOut += pIfTable->table[i].dwOutOctets;
                            
                            qDebug() << "Interface" << i 
                                    << "Type:" << pIfTable->table[i].dwType
                                    << "Status:" << pIfTable->table[i].dwOperStatus
                                    << "In:" << pIfTable->table[i].dwInOctets
                                    << "Out:" << pIfTable->table[i].dwOutOctets;
                        }
                    }
                }
                
                // 计算速率（KB/s）
                if (lastTime > 0) {
                    DWORD timeDiff = currentTime - lastTime;
                    if (timeDiff > 0) {
                        // 处理计数器溢出的情况
                        if (totalBytesIn >= lastBytesIn) {
                            download = ((totalBytesIn - lastBytesIn) * 1000.0) / (timeDiff * 1024.0);
                        }
                        if (totalBytesOut >= lastBytesOut) {
                            upload = ((totalBytesOut - lastBytesOut) * 1000.0) / (timeDiff * 1024.0);
                        }
                        
                        qDebug() << "Network Speed -"
                                << "Upload:" << upload << "KB/s,"
                                << "Download:" << download << "KB/s,"
                                << "Time diff:" << timeDiff << "ms,"
                                << "Total In:" << totalBytesIn
                                << "Last In:" << lastBytesIn
                                << "Total Out:" << totalBytesOut
                                << "Last Out:" << lastBytesOut;
                    }
                }
                
                lastBytesIn = totalBytesIn;
                lastBytesOut = totalBytesOut;
            } else {
                qDebug() << "Failed to get network interface table";
            }
            free(pIfTable);
        } else {
            qDebug() << "Failed to allocate memory for network interface table";
        }
    } else {
        qDebug() << "Failed to get network interface table size";
    }
    
    lastTime = currentTime;
    
    // 确保返回的值是有效的
    if (upload < 0) upload = 0;
    if (download < 0) download = 0;
}
#endif

// SystemPerformanceWidget
SystemPerformanceWidget::SystemPerformanceWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , m_monitor(nullptr)
{
    setupDefaultConfig();
    parseCustomSettings();
    setMinimumSize(250, 200);
    
    // 创建性能监测器
    m_monitor = new PerformanceMonitor(this);
    connect(m_monitor, &PerformanceMonitor::dataUpdated, 
            this, &SystemPerformanceWidget::onPerformanceDataUpdated);
    
    // 启动监测
    m_monitor->start();
}

SystemPerformanceWidget::~SystemPerformanceWidget() {
    if (m_monitor) {
        m_monitor->stopMonitoring();
        delete m_monitor;
    }
}

void SystemPerformanceWidget::setupDefaultConfig() {
    m_labelFont = QFont("微软雅黑", 9);
    m_valueFont = QFont("微软雅黑", 11, QFont::Bold);
    
    m_cpuColor = QColor(255, 100, 100);      // 红色
    m_memoryColor = QColor(100, 255, 100);   // 绿色
    m_diskColor = QColor(100, 150, 255);     // 蓝色
    m_networkColor = QColor(255, 200, 100);  // 橙色
    m_textColor = Qt::white;
    m_backgroundColor = QColor(0, 0, 0, 150);
    m_borderColor = QColor(255, 255, 255, 100);
    
    m_showCpu = true;
    m_showMemory = true;
    m_showDisk = true;
    m_showNetwork = true;
    m_showDetailed = true;
    m_showProgressBars = true;
    
    m_itemSpacing = 8;
    m_borderRadius = 8;
    m_backgroundOpacity = 0.8;
}

void SystemPerformanceWidget::parseCustomSettings() {
    const QJsonObject& settings = m_config.customSettings;
    
    if (settings.contains("showCpu")) {
        m_showCpu = settings["showCpu"].toBool();
    }
    
    if (settings.contains("showMemory")) {
        m_showMemory = settings["showMemory"].toBool();
    }
    
    if (settings.contains("showDisk")) {
        m_showDisk = settings["showDisk"].toBool();
    }
    
    if (settings.contains("showNetwork")) {
        m_showNetwork = settings["showNetwork"].toBool();
    }
    
    if (settings.contains("showDetailed")) {
        m_showDetailed = settings["showDetailed"].toBool();
    }
    
    if (settings.contains("showProgressBars")) {
        m_showProgressBars = settings["showProgressBars"].toBool();
    }
    
    if (settings.contains("textColor")) {
        m_textColor = QColor(settings["textColor"].toString());
    }
    
    if (settings.contains("backgroundColor")) {
        m_backgroundColor = QColor(settings["backgroundColor"].toString());
    }
    
    if (settings.contains("cpuColor")) {
        m_cpuColor = QColor(settings["cpuColor"].toString());
    }
    
    if (settings.contains("memoryColor")) {
        m_memoryColor = QColor(settings["memoryColor"].toString());
    }
    
    if (settings.contains("diskColor")) {
        m_diskColor = QColor(settings["diskColor"].toString());
    }
    
    if (settings.contains("networkColor")) {
        m_networkColor = QColor(settings["networkColor"].toString());
    }
    
    if (settings.contains("labelFontSize")) {
        m_labelFont.setPointSize(settings["labelFontSize"].toInt());
    }
    
    if (settings.contains("valueFontSize")) {
        m_valueFont.setPointSize(settings["valueFontSize"].toInt());
    }
    
    if (settings.contains("itemSpacing")) {
        m_itemSpacing = settings["itemSpacing"].toInt();
    }
    
    if (settings.contains("borderRadius")) {
        m_borderRadius = settings["borderRadius"].toInt();
    }
    
    if (settings.contains("backgroundOpacity")) {
        m_backgroundOpacity = settings["backgroundOpacity"].toDouble();
        if (m_backgroundOpacity < 0.0) m_backgroundOpacity = 0.0;
        if (m_backgroundOpacity > 1.0) m_backgroundOpacity = 1.0;
    }
}

void SystemPerformanceWidget::onPerformanceDataUpdated(const PerformanceData& data) {
    QMutexLocker locker(&m_dataMutex);
    m_currentData = data;
    update(); // 触发重绘
}

void SystemPerformanceWidget::updateContent() {
    // 性能数据由后台线程更新，这里只需要触发重绘
    update();
}

void SystemPerformanceWidget::drawContent(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 应用与系统信息小组件完全相同的样式
    QString style = QString(
        "QWidget { "
        "    border: 1px solid %1; "
        "    border-radius: 5px; "
        "    font-weight: bold; "
        "} "
    ).arg(palette().mid().color().name());
    
    setStyleSheet(style);
    
    // 不绘制自定义背景，使用BaseWidget的默认背景（QColor(0, 0, 0, 50)）
    
    // 绘制边框
    painter.setPen(QPen(m_borderColor, 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), m_borderRadius, m_borderRadius);
    
    // 获取当前性能数据
    PerformanceData data;
    {
        QMutexLocker locker(&m_dataMutex);
        data = m_currentData;
    }
    
    // 计算布局
    int margin = 10;
    int availableHeight = rect().height() - 2 * margin;
    int itemCount = 0;
    
    if (m_showCpu) itemCount++;
    if (m_showMemory) itemCount++;
    if (m_showDisk) itemCount++;
    if (m_showNetwork) itemCount++;
    
    if (itemCount == 0) return;
    
    int itemHeight = (availableHeight - (itemCount - 1) * m_itemSpacing) / itemCount;
    int currentY = margin;
    
    // 绘制CPU信息
    if (m_showCpu) {
        QRect cpuRect(margin, currentY, rect().width() - 2 * margin, itemHeight);
        drawPerformanceGraph(painter, cpuRect, "CPU", data.cpuUsage, m_cpuColor);
        currentY += itemHeight + m_itemSpacing;
    }
    
    // 绘制内存信息
    if (m_showMemory) {
        QRect memRect(margin, currentY, rect().width() - 2 * margin, itemHeight);
        if (m_showDetailed) {
            drawMemoryInfo(painter, memRect);
        } else {
            drawPerformanceGraph(painter, memRect, "内存", data.memoryUsage, m_memoryColor);
        }
        currentY += itemHeight + m_itemSpacing;
    }
    
    // 绘制磁盘信息
    if (m_showDisk) {
        QRect diskRect(margin, currentY, rect().width() - 2 * margin, itemHeight);
        if (m_showDetailed) {
            drawDiskInfo(painter, diskRect);
        } else {
            drawPerformanceGraph(painter, diskRect, "磁盘", data.diskUsage, m_diskColor);
        }
        currentY += itemHeight + m_itemSpacing;
    }
    
    // 绘制网络信息
    if (m_showNetwork) {
        QRect netRect(margin, currentY, rect().width() - 2 * margin, itemHeight);
        drawNetworkInfo(painter, netRect);
    }
}

void SystemPerformanceWidget::drawPerformanceGraph(QPainter& painter, const QRect& rect, 
                                                  const QString& label, double value, 
                                                  const QColor& color, const QString& unit) {
    // 绘制标签
    painter.setFont(m_labelFont);
    painter.setPen(m_textColor);
    
    QRect labelRect = rect;
    labelRect.setHeight(rect.height() / 2);
    painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, label);
    
    // 绘制数值
    painter.setFont(m_valueFont);
    QString valueText = QString("%1%2").arg(QString::number(value, 'f', 1)).arg(unit);
    painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, valueText);
    
    // 绘制进度条
    if (m_showProgressBars) {
        QRect progressRect = rect;
        progressRect.setTop(rect.top() + rect.height() / 2 + 2);
        progressRect.setHeight(rect.height() / 2 - 4);
        drawProgressBar(painter, progressRect, value, color);
    }
}

void SystemPerformanceWidget::drawProgressBar(QPainter& painter, const QRect& rect, 
                                            double value, const QColor& color) {
    // 绘制背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(color.red(), color.green(), color.blue(), 50));
    painter.drawRoundedRect(rect, 3, 3);
    
    // 绘制进度
    if (value > 0) {
        QRect progressRect = rect;
        progressRect.setWidth(rect.width() * qBound(0.0, value / 100.0, 1.0));
        
        QLinearGradient gradient(progressRect.topLeft(), progressRect.topRight());
        gradient.setColorAt(0, color);
        gradient.setColorAt(1, color.darker(120));
        
        painter.setBrush(gradient);
        painter.drawRoundedRect(progressRect, 3, 3);
    }
}

void SystemPerformanceWidget::drawMemoryInfo(QPainter& painter, const QRect& rect) {
    PerformanceData data;
    {
        QMutexLocker locker(&m_dataMutex);
        data = m_currentData;
    }
    
    painter.setFont(m_labelFont);
    painter.setPen(m_textColor);
    
    // 第一行：标签和百分比
    QRect firstLineRect = rect;
    firstLineRect.setHeight(rect.height() / 3);
    painter.drawText(firstLineRect, Qt::AlignLeft | Qt::AlignVCenter, "内存");
    
    QString percentText = QString("%1%").arg(QString::number(data.memoryUsage, 'f', 1));
    painter.drawText(firstLineRect, Qt::AlignRight | Qt::AlignVCenter, percentText);
    
    // 第二行：详细信息
    if (data.totalMemory > 0) {
        QRect secondLineRect = rect;
        secondLineRect.setTop(rect.top() + rect.height() / 3);
        secondLineRect.setHeight(rect.height() / 3);
        
        painter.setFont(QFont(m_labelFont.family(), m_labelFont.pointSize() - 1));
        QString detailText = QString("%1MB / %2MB")
            .arg(data.usedMemory)
            .arg(data.totalMemory);
        painter.drawText(secondLineRect, Qt::AlignCenter, detailText);
    }
    
    // 第三行：进度条
    if (m_showProgressBars) {
        QRect progressRect = rect;
        progressRect.setTop(rect.top() + 2 * rect.height() / 3 + 2);
        progressRect.setHeight(rect.height() / 3 - 4);
        drawProgressBar(painter, progressRect, data.memoryUsage, m_memoryColor);
    }
}

void SystemPerformanceWidget::drawDiskInfo(QPainter& painter, const QRect& rect) {
    PerformanceData data;
    {
        QMutexLocker locker(&m_dataMutex);
        data = m_currentData;
    }
    
    painter.setFont(m_labelFont);
    painter.setPen(m_textColor);
    
    // 第一行：标签和百分比
    QRect firstLineRect = rect;
    firstLineRect.setHeight(rect.height() / 3);
    painter.drawText(firstLineRect, Qt::AlignLeft | Qt::AlignVCenter, "磁盘");
    
    QString percentText = QString("%1%").arg(QString::number(data.diskUsage, 'f', 1));
    painter.drawText(firstLineRect, Qt::AlignRight | Qt::AlignVCenter, percentText);
    
    // 第二行：详细信息
    if (data.totalDisk > 0) {
        QRect secondLineRect = rect;
        secondLineRect.setTop(rect.top() + rect.height() / 3);
        secondLineRect.setHeight(rect.height() / 3);
        
        painter.setFont(QFont(m_labelFont.family(), m_labelFont.pointSize() - 1));
        QString detailText = QString("%1GB / %2GB")
            .arg(data.usedDisk)
            .arg(data.totalDisk);
        painter.drawText(secondLineRect, Qt::AlignCenter, detailText);
    }
    
    // 第三行：进度条
    if (m_showProgressBars) {
        QRect progressRect = rect;
        progressRect.setTop(rect.top() + 2 * rect.height() / 3 + 2);
        progressRect.setHeight(rect.height() / 3 - 4);
        drawProgressBar(painter, progressRect, data.diskUsage, m_diskColor);
    }
}

void SystemPerformanceWidget::drawNetworkInfo(QPainter& painter, const QRect& rect) {
    PerformanceData data;
    {
        QMutexLocker locker(&m_dataMutex);
        data = m_currentData;
    }
    
    painter.setFont(m_labelFont);
    painter.setPen(m_textColor);
    
    // 第一行：标签
    QRect firstLineRect = rect;
    firstLineRect.setHeight(rect.height() / 2);
    painter.drawText(firstLineRect, Qt::AlignLeft | Qt::AlignVCenter, "网络");
    
    // 第二行：上传/下载速度
    QRect secondLineRect = rect;
    secondLineRect.setTop(rect.top() + rect.height() / 2);
    secondLineRect.setHeight(rect.height() / 2);
    
    painter.setFont(QFont(m_labelFont.family(), m_labelFont.pointSize() - 1));
    
    QString uploadText = QString("↑ %1 KB/s").arg(QString::number(data.networkUpload, 'f', 1));
    QString downloadText = QString("↓ %1 KB/s").arg(QString::number(data.networkDownload, 'f', 1));
    
    // 分两列显示上传和下载
    QRect uploadRect = secondLineRect;
    uploadRect.setWidth(secondLineRect.width() / 2);
    painter.drawText(uploadRect, Qt::AlignLeft | Qt::AlignVCenter, uploadText);
    
    QRect downloadRect = secondLineRect;
    downloadRect.setLeft(secondLineRect.left() + secondLineRect.width() / 2);
    painter.drawText(downloadRect, Qt::AlignLeft | Qt::AlignVCenter, downloadText);
}

void SystemPerformanceWidget::applyConfig() {
    BaseWidget::applyConfig();
    parseCustomSettings();
    updateContent();
}


