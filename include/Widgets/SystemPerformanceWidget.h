#pragma once
#include "Core/BaseWidget.h"
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QProcess>
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#endif

// 性能数据结构
struct PerformanceData {
    double cpuUsage = 0.0;          // CPU使用率 (0-100)
    double memoryUsage = 0.0;       // 内存使用率 (0-100)
    double diskUsage = 0.0;         // 磁盘使用率 (0-100)
    double networkUpload = 0.0;     // 网络上传速度 (KB/s)
    double networkDownload = 0.0;   // 网络下载速度 (KB/s)
    qint64 totalMemory = 0;         // 总内存 (MB)
    qint64 usedMemory = 0;          // 已用内存 (MB)
    qint64 totalDisk = 0;           // 总磁盘空间 (GB)
    qint64 usedDisk = 0;            // 已用磁盘空间 (GB)
    QDateTime timestamp;            // 数据时间戳
};

// 性能监测线程
class PerformanceMonitor : public QThread {
    Q_OBJECT

public:
    explicit PerformanceMonitor(QObject* parent = nullptr);
    ~PerformanceMonitor();
    
    void stopMonitoring();
    PerformanceData getCurrentData() const;

protected:
    void run() override;

signals:
    void dataUpdated(const PerformanceData& data);

private:
    void collectPerformanceData();
    double getCpuUsage();
    void getMemoryInfo(qint64& total, qint64& used);
    void getDiskInfo(qint64& total, qint64& used);
    void getNetworkInfo(double& upload, double& download);

    // PDH相关函数
    void initializePdh();
    void uninitializePdh();
    double getCpuUsagePdh();
    double getDiskUsagePdh();
    void getNetworkInfoPdh(double& upload, double& download);

private:
    bool m_running;
    PerformanceData m_currentData;
    mutable QMutex m_dataMutex;
    
    // 网络监测相关
    qint64 m_lastBytesReceived;
    qint64 m_lastBytesSent;
    QDateTime m_lastNetworkCheckTime;

    // PDH相关成员变量
#ifdef Q_OS_WIN
    PDH_HQUERY m_hQuery;
    PDH_HCOUNTER m_hCpuTotal;
    PDH_HCOUNTER m_hDiskTime;
    PDH_HCOUNTER m_hNetworkTotal;
#endif
};

class SystemPerformanceWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit SystemPerformanceWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~SystemPerformanceWidget();

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void applyConfig() override;

private slots:
    void onPerformanceDataUpdated(const PerformanceData& data);

private:
    void setupDefaultConfig();
    void parseCustomSettings();
    void drawPerformanceGraph(QPainter& painter, const QRect& rect, 
                             const QString& label, double value, 
                             const QColor& color, const QString& unit = "%");
    void drawProgressBar(QPainter& painter, const QRect& rect, 
                        double value, const QColor& color);
    void drawMemoryInfo(QPainter& painter, const QRect& rect);
    void drawDiskInfo(QPainter& painter, const QRect& rect);
    void drawNetworkInfo(QPainter& painter, const QRect& rect);

private:
    PerformanceMonitor* m_monitor;
    PerformanceData m_currentData;
    QMutex m_dataMutex;
    
    // 显示配置
    QFont m_labelFont;
    QFont m_valueFont;
    QColor m_cpuColor;
    QColor m_memoryColor;
    QColor m_diskColor;
    QColor m_networkColor;
    QColor m_textColor;
    QColor m_backgroundColor;
    QColor m_borderColor;
    
    bool m_showCpu;
    bool m_showMemory;
    bool m_showDisk;
    bool m_showNetwork;
    bool m_showDetailed;
    bool m_showProgressBars;
    
    int m_itemSpacing;
    int m_borderRadius;
    double m_backgroundOpacity;
}; 