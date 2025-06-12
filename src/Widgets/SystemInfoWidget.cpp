#include "Widgets/SystemInfoWidget.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QStyle>
#include <QPainter>

SystemInfoWidget::SystemInfoWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , systemInfo(SystemInfoCollector::getInstance())
{
    setObjectName("SystemInfoWidget");
    initUI();
    initConnections();
    updateSystemInfo();
}

SystemInfoWidget::~SystemInfoWidget() {
    if (updateTimer) {
        updateTimer->stop();
        delete updateTimer;
    }
}

void SystemInfoWidget::updateContent() {
    // 在这里更新小组件的内容
    updateSystemInfo();
    updateCPUUsage();
    updateMemoryUsage();
    updateDiskUsage();
}

void SystemInfoWidget::drawContent(QPainter& painter) {
    // 由于我们使用Qt Widgets来显示内容，这里不需要自定义绘制
    Q_UNUSED(painter);
}

void SystemInfoWidget::initUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // CPU信息组
    QGroupBox* cpuGroup = new QGroupBox("CPU信息", this);
    QVBoxLayout* cpuLayout = new QVBoxLayout(cpuGroup);
    
    cpuModelLabel = new QLabel(this);
    cpuCoresLabel = new QLabel(this);
    cpuUsageBar = new QProgressBar(this);
    cpuUsageBar->setRange(0, 100);
    cpuUsageBar->setTextVisible(true);
    
    cpuLayout->addWidget(cpuModelLabel);
    cpuLayout->addWidget(cpuCoresLabel);
    cpuLayout->addWidget(cpuUsageBar);
    
    // 内存信息组
    QGroupBox* memoryGroup = new QGroupBox("内存信息", this);
    QVBoxLayout* memoryLayout = new QVBoxLayout(memoryGroup);
    
    memoryTotalLabel = new QLabel(this);
    memoryUsageBar = new QProgressBar(this);
    memoryUsageBar->setRange(0, 100);
    memoryUsageBar->setTextVisible(true);
    
    memoryLayout->addWidget(memoryTotalLabel);
    memoryLayout->addWidget(memoryUsageBar);
    
    // 系统信息组
    QGroupBox* sysGroup = new QGroupBox("系统信息", this);
    QVBoxLayout* sysLayout = new QVBoxLayout(sysGroup);
    
    osInfoLabel = new QLabel(this);
    computerInfoLabel = new QLabel(this);
    
    sysLayout->addWidget(osInfoLabel);
    sysLayout->addWidget(computerInfoLabel);
    
    // 磁盘信息组
    QGroupBox* diskGroup = new QGroupBox("磁盘信息", this);
    diskLayout = new QGridLayout(diskGroup);
    
    // 添加所有组到主布局
    mainLayout->addWidget(cpuGroup);
    mainLayout->addWidget(memoryGroup);
    mainLayout->addWidget(sysGroup);
    mainLayout->addWidget(diskGroup);
    
    // 创建更新定时器
    updateTimer = new QTimer(this);
    updateTimer->setInterval(2000); // 每2秒更新一次
    updateTimer->start();
}

void SystemInfoWidget::initConnections() {
    connect(updateTimer, &QTimer::timeout, this, &SystemInfoWidget::updateCPUUsage);
    connect(updateTimer, &QTimer::timeout, this, &SystemInfoWidget::updateMemoryUsage);
    connect(updateTimer, &QTimer::timeout, this, &SystemInfoWidget::updateDiskUsage);
}

void SystemInfoWidget::updateTheme() {
    // 应用主题样式
    QString style = QString(
        "QGroupBox { "
        "    border: 1px solid %1; "
        "    border-radius: 5px; "
        "    margin-top: 1ex; "
        "    font-weight: bold; "
        "} "
        "QGroupBox::title { "
        "    subcontrol-origin: margin; "
        "    subcontrol-position: top center; "
        "    padding: 0 3px; "
        "    color: %2; "
        "} "
        "QProgressBar { "
        "    border: 1px solid %1; "
        "    border-radius: 3px; "
        "    text-align: center; "
        "    background-color: %3; "
        "} "
        "QProgressBar::chunk { "
        "    background-color: %4; "
        "    border-radius: 2px; "
        "} "
    ).arg(palette().mid().color().name(),
          palette().windowText().color().name(),
          palette().base().color().name(),
          palette().highlight().color().name());
    
    setStyleSheet(style);
}

QString SystemInfoWidget::formatSize(qint64 bytes) {
    const qint64 kb = 1024;
    const qint64 mb = kb * 1024;
    const qint64 gb = mb * 1024;
    
    if (bytes >= gb) {
        return QString::number(double(bytes) / gb, 'f', 2) + " GB";
    } else if (bytes >= mb) {
        return QString::number(double(bytes) / mb, 'f', 2) + " MB";
    } else if (bytes >= kb) {
        return QString::number(double(bytes) / kb, 'f', 2) + " KB";
    }
    return QString::number(bytes) + " B";
}

void SystemInfoWidget::updateSystemInfo() {
    auto info = systemInfo.collectSystemInfo();
    
    // 更新CPU信息
    cpuModelLabel->setText("型号: " + info.cpuModel);
    cpuCoresLabel->setText(QString("核心数: %1").arg(info.cpuCores));
    
    // 更新系统信息
    osInfoLabel->setText(QString("操作系统: %1 %2").arg(info.osName, info.osVersion));
    computerInfoLabel->setText(QString("计算机名: %1\n用户名: %2").arg(info.computerName, info.userName));
    
    // 更新内存信息
    memoryTotalLabel->setText("总内存: " + formatSize(info.totalMemory));
    
    // 更新磁盘信息
    // 清理旧的磁盘信息控件
    QLayoutItem* item;
    while ((item = diskLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    // 添加新的磁盘信息
    int row = 0;
    for (auto it = info.diskSpace.constBegin(); it != info.diskSpace.constEnd(); ++it) {
        QString driveName = it.key();
        qint64 total = it.value().first;
        qint64 available = it.value().second;
        
        QLabel* driveLabel = new QLabel(QString("%1 总容量: %2").arg(driveName, formatSize(total)), this);
        QProgressBar* usageBar = new QProgressBar(this);
        usageBar->setRange(0, 100);
        int usagePercent = int((double(total - available) / total) * 100);
        usageBar->setValue(usagePercent);
        usageBar->setFormat(QString("%1 可用").arg(formatSize(available)));
        
        diskLayout->addWidget(driveLabel, row, 0);
        diskLayout->addWidget(usageBar, row, 1);
        
        diskUsageBars[driveName] = usageBar;
        diskLabels[driveName] = driveLabel;
        
        row++;
    }
}

void SystemInfoWidget::updateCPUUsage() {
    double usage = systemInfo.getCurrentCpuUsage();
    cpuUsageBar->setValue(int(usage));
    cpuUsageBar->setFormat(QString("使用率: %1%").arg(int(usage)));
}

void SystemInfoWidget::updateMemoryUsage() {
    qint64 total, available;
    systemInfo.updateMemoryInfo(total, available);
    qint64 used = total - available;
    int usagePercent = int((double(used) / total) * 100);
    
    memoryUsageBar->setValue(usagePercent);
    memoryUsageBar->setFormat(QString("已用: %1 (可用: %2)").arg(formatSize(used), formatSize(available)));
}

void SystemInfoWidget::updateDiskUsage() {
    auto diskSpace = systemInfo.getDiskSpace();
    
    for (auto it = diskSpace.constBegin(); it != diskSpace.constEnd(); ++it) {
        QString driveName = it.key();
        qint64 total = it.value().first;
        qint64 available = it.value().second;
        
        if (diskUsageBars.contains(driveName)) {
            int usagePercent = int((double(total - available) / total) * 100);
            diskUsageBars[driveName]->setValue(usagePercent);
            diskUsageBars[driveName]->setFormat(QString("%1 可用").arg(formatSize(available)));
        }
    }
} 