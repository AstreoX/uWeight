#pragma once

#include "Core/BaseWidget.h"
#include "Utils/SystemInfoCollector.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTimer>
#include <QMap>

class QGridLayout;

class SystemInfoWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit SystemInfoWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~SystemInfoWidget() override;

protected:
    // BaseWidget纯虚函数实现
    void updateContent() override;
    void drawContent(QPainter& painter) override;

private:
    void initUI();
    void initConnections();
    void updateTheme();

private slots:
    void updateSystemInfo();
    void updateCPUUsage();
    void updateMemoryUsage();
    void updateDiskUsage();

private:
    QString formatSize(qint64 bytes);
    
    QVBoxLayout* mainLayout;
    QGridLayout* diskLayout;
    
    // CPU信息显示
    QLabel* cpuModelLabel;
    QLabel* cpuCoresLabel;
    QProgressBar* cpuUsageBar;
    
    // 内存信息显示
    QLabel* memoryTotalLabel;
    QProgressBar* memoryUsageBar;
    
    // 系统信息显示
    QLabel* osInfoLabel;
    QLabel* computerInfoLabel;
    
    // 磁盘信息显示
    QMap<QString, QProgressBar*> diskUsageBars;
    QMap<QString, QLabel*> diskLabels;
    
    QTimer* updateTimer;
    SystemInfoCollector& systemInfo;
}; 