/**
 * @file WidgetManager.cpp
 * @brief 小组件管理系统核心实现
 * @details 负责小组件的生命周期管理、配置持久化和状态监控
 * @author 梁智搏 (YumeshioAmami)
 * @date 2025-5
 * @version 1.0.0
 * 
 * 小组件管理系统是整个桌面小组件应用的核心管理模块，提供：
 * - 小组件的创建、销毁和生命周期管理
 * - 配置文件的加载、保存和持久化
 * - 小组件状态的实时监控和更新
 * - 多种小组件类型的统一管理接口
 * - 配置的导入导出功能
 * - 自动保存和错误恢复机制
 * 
 * 该系统采用观察者模式和工厂模式，确保：
 * - 高度的可扩展性和可维护性
 * - 线程安全的操作接口
 * - 高效的内存和资源管理
 * - 完整的错误处理和日志记录
 */

#include "Framework/WidgetManager.h"
#include "Widgets/ClockWidget.h"
#include "Widgets/WeatherWidget.h"
#include "Widgets/AIRankingWidget.h"
#include "Widgets/SystemPerformanceWidget.h"
#include "Widgets/NotesWidget.h"
#include "Widgets/SimpleNotesWidget.h"
#include "Widgets/CalendarWidget.h"
#include "BackendManagement/ConfigWindow.h"
#include "BackendManagement/AIRankingConfigDialog.h"
#include "BackendManagement/WeatherConfigDialog.h"
#include "BackendManagement/NotesConfigDialog.h"
#include "Utils/Logger.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

/**
 * @brief WidgetManager构造函数
 * @param parent 父QObject指针
 * 
 * 初始化小组件管理器的核心组件：
 * - 设置自动保存定时器（5秒延迟）
 * - 初始化小组件容器
 * - 配置错误处理机制
 * - 建立信号槽连接
 * 
 * 自动保存机制设计：
 * - 使用单次触发定时器避免频繁保存
 * - 在配置更改后延迟5秒保存，避免性能影响
 * - 支持手动保存和强制保存模式
 */
WidgetManager::WidgetManager(QObject* parent)
    : QObject(parent)
    , m_saveTimer(new QTimer(this))
    , m_autoSave(true)
{
    // 配置自动保存定时器
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(5000); // 5秒延迟保存，平衡性能和数据安全
    connect(m_saveTimer, &QTimer::timeout, this, &WidgetManager::saveConfiguration);
}

/**
 * @brief WidgetManager析构函数
 * 
 * 清理所有管理的小组件资源：
 * - 停止所有正在运行的小组件
 * - 保存当前配置到磁盘
 * - 释放内存资源
 * - 断开所有信号槽连接
 */
WidgetManager::~WidgetManager() {
    cleanupAllWidgets();
}

/**
 * @brief 创建新的小组件实例
 * @param config 小组件配置信息
 * @return 创建成功返回true，失败返回false
 * 
 * 小组件创建流程：
 * 1. 验证配置有效性（ID唯一性、必需字段）
 * 2. 使用工厂模式创建对应类型的小组件
 * 3. 建立信号槽连接（状态监控、事件处理）
 * 4. 添加到管理容器
 * 5. 触发自动保存机制
 * 6. 发送创建完成信号
 * 
 * 错误处理：
 * - ID重复：记录警告并返回false
 * - 配置无效：记录错误并返回false
 * - 创建失败：记录错误并清理资源
 */
bool WidgetManager::createWidget(const WidgetConfig& config) {
    // 验证Widget ID的唯一性
    if (hasWidget(config.id)) {
        Logger::warning(QString("Widget已存在: %1").arg(config.id));
        return false;
    }
    
    // 验证配置的完整性和有效性
    if (!validateConfig(config)) {
        Logger::error(QString("Widget配置无效: %1").arg(config.id));
        return false;
    }
    
    // 使用工厂方法创建具体类型的Widget
    WidgetPtr widget = createWidgetByType(config.type, config);
    if (!widget) {
        Logger::error(QString("无法创建Widget: %1").arg(config.id));
        return false;
    }
    
    // 将Widget添加到管理容器
    m_widgets[config.id] = widget;
    
    // 连接Widget的信号到管理器的槽函数
    connectWidgetSignals(widget);
    
    // 通知外部组件Widget创建完成
    emit widgetCreated(config.id);
    
    // 触发自动保存机制
    if (m_autoSave) {
        m_saveTimer->start();
    }
    
    Logger::info(QString("Widget创建成功: %1").arg(config.id));
    return true;
}

bool WidgetManager::removeWidget(const QString& widgetId) {
    auto it = m_widgets.find(widgetId);
    if (it == m_widgets.end()) {
        return false;
    }
    
    WidgetPtr widget = it.value();
    disconnectWidgetSignals(widget);
    widget->cleanup();
    
    m_widgets.erase(it);
    emit widgetRemoved(widgetId);
    
    if (m_autoSave) {
        m_saveTimer->start();
    }
    
    Logger::info(QString("Widget移除成功: %1").arg(widgetId));
    return true;
}

bool WidgetManager::startWidget(const QString& widgetId) {
    WidgetPtr widget = getWidget(widgetId);
    if (!widget) {
        return false;
    }
    
    widget->start();
    return true;
}

bool WidgetManager::stopWidget(const QString& widgetId) {
    WidgetPtr widget = getWidget(widgetId);
    if (!widget) {
        return false;
    }
    
    widget->stop();
    return true;
}

void WidgetManager::startAllWidgets() {
    for (auto& widget : m_widgets) {
        widget->start();
    }
}

void WidgetManager::stopAllWidgets() {
    for (auto& widget : m_widgets) {
        widget->stop();
    }
}

void WidgetManager::cleanupAllWidgets() {
    for (auto& widget : m_widgets) {
        widget->cleanup();
    }
    m_widgets.clear();
}

WidgetPtr WidgetManager::getWidget(const QString& widgetId) const {
    auto it = m_widgets.find(widgetId);
    return (it != m_widgets.end()) ? it.value() : nullptr;
}

QList<WidgetPtr> WidgetManager::getAllWidgets() const {
    return m_widgets.values();
}

bool WidgetManager::hasWidget(const QString& widgetId) const {
    return m_widgets.contains(widgetId);
}

int WidgetManager::getWidgetCount() const {
    return m_widgets.size();
}

QStringList WidgetManager::getWidgetIds() const {
    return m_widgets.keys();
}

bool WidgetManager::updateWidgetConfig(const QString& widgetId, const WidgetConfig& config) {
    auto it = m_widgets.find(widgetId);
    if (it == m_widgets.end()) {
        Logger::warning(QString("尝试更新不存在的Widget: %1").arg(widgetId));
        return false;
    }
    
    WidgetPtr widget = it.value();
    if (!widget) {
        return false;
    }
    
    qDebug() << "WidgetManager::updateWidgetConfig: 开始更新配置";
    qDebug() << "Widget ID:" << widgetId;
    qDebug() << "New API Provider:" << config.customSettings.value("apiProvider").toString();
    qDebug() << "New API Key:" << config.customSettings.value("apiKey").toString();
    qDebug() << "New City Name:" << config.customSettings.value("cityName").toString();
    
    // 应用新配置
    widget->setConfig(config);
    
    // 验证配置是否正确设置
    WidgetConfig currentConfig = widget->getConfig();
    qDebug() << "验证 - Current API Provider:" << currentConfig.customSettings.value("apiProvider").toString();
    qDebug() << "验证 - Current API Key:" << currentConfig.customSettings.value("apiKey").toString();
    qDebug() << "验证 - Current City Name:" << currentConfig.customSettings.value("cityName").toString();
    
    if (m_autoSave) {
        qDebug() << "触发自动保存...";
        m_saveTimer->start();
    }
    
    Logger::info(QString("Widget配置已更新: %1").arg(widgetId));
    return true;
}

WidgetConfig WidgetManager::getWidgetConfig(const QString& widgetId) const {
    auto widget = getWidget(widgetId);
    if (widget) {
        return widget->getConfig();
    }
    return WidgetConfig();
}

QList<WidgetConfig> WidgetManager::getAllConfigs() const {
    QList<WidgetConfig> configs;
    for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
        configs.append(it.value()->getConfig());
    }
    return configs;
}

bool WidgetManager::saveConfiguration() const {
    QString configPath = getConfigFilePath();
    qDebug() << "WidgetManager::saveConfiguration: 开始保存配置到" << configPath;
    
    QJsonObject root;
    QJsonArray widgetsArray;
    
    for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
        const WidgetConfig& config = it.value()->getConfig();
        QJsonObject widgetObj;
        
        widgetObj["id"] = config.id;
        widgetObj["type"] = static_cast<int>(config.type);
        widgetObj["name"] = config.name;
        widgetObj["position"] = QJsonArray{config.position.x(), config.position.y()};
        widgetObj["size"] = QJsonArray{config.size.width(), config.size.height()};
        widgetObj["alwaysOnTop"] = config.alwaysOnTop;
        widgetObj["clickThrough"] = config.clickThrough;
        widgetObj["locked"] = config.locked;
        widgetObj["opacity"] = config.opacity;
        widgetObj["autoStart"] = config.autoStart;
        widgetObj["updateInterval"] = config.updateInterval;
        widgetObj["customSettings"] = config.customSettings;
        
        // 如果是天气组件，输出调试信息
        if (config.type == WidgetType::Weather) {
            qDebug() << "保存天气组件配置:";
            qDebug() << "  ID:" << config.id;
            qDebug() << "  API Provider:" << config.customSettings.value("apiProvider").toString();
            qDebug() << "  API Key:" << config.customSettings.value("apiKey").toString();
            qDebug() << "  City Name:" << config.customSettings.value("cityName").toString();
        }
        
        widgetsArray.append(widgetObj);
    }
    
    root["widgets"] = widgetsArray;
    root["version"] = "1.0.0";
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(root);
    
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::error(QString("无法写入配置文件: %1").arg(configPath));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "WidgetManager::saveConfiguration: 配置文件写入完成";
    Logger::info("配置文件保存成功");
    return true;
}

bool WidgetManager::loadConfiguration() {
    return loadConfigurationFromFile(getConfigFilePath());
}

bool WidgetManager::loadConfigurationFromFile(const QString& filePath) {
    QFile file(filePath);
    
    if (!file.exists()) {
        Logger::info("配置文件不存在，将创建新的配置");
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::error(QString("无法读取配置文件: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        Logger::error(QString("配置文件JSON解析错误: %1").arg(error.errorString()));
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray widgetsArray = root["widgets"].toArray();
    
    for (const QJsonValue& value : widgetsArray) {
        QJsonObject widgetObj = value.toObject();
        
        WidgetConfig config;
        config.id = widgetObj["id"].toString();
        config.type = static_cast<WidgetType>(widgetObj["type"].toInt());
        config.name = widgetObj["name"].toString();
        
        QJsonArray posArray = widgetObj["position"].toArray();
        config.position = QPoint(posArray[0].toInt(), posArray[1].toInt());
        
        QJsonArray sizeArray = widgetObj["size"].toArray();
        config.size = QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
        
        config.alwaysOnTop = widgetObj["alwaysOnTop"].toBool();
        config.clickThrough = widgetObj["clickThrough"].toBool();
        config.locked = widgetObj["locked"].toBool();
        config.opacity = widgetObj["opacity"].toDouble();
        config.autoStart = widgetObj["autoStart"].toBool();
        config.updateInterval = widgetObj["updateInterval"].toInt();
        config.customSettings = widgetObj["customSettings"].toObject();
        
        // 如果是天气组件，输出加载的配置调试信息
        if (config.type == WidgetType::Weather) {
            qDebug() << "加载天气组件配置:";
            qDebug() << "  ID:" << config.id;
            qDebug() << "  API Provider:" << config.customSettings.value("apiProvider").toString();
            qDebug() << "  API Key:" << config.customSettings.value("apiKey").toString();
            qDebug() << "  City Name:" << config.customSettings.value("cityName").toString();
        }
        
        if (createWidget(config) && config.autoStart) {
            startWidget(config.id);
        }
    }
    
    Logger::info(QString("配置加载成功，共加载%1个Widget").arg(widgetsArray.size()));
    return true;
}

bool WidgetManager::exportConfiguration(const QString& filePath) const {
    QJsonObject root;
    QJsonArray widgetsArray;
    
    for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
        const WidgetConfig& config = it.value()->getConfig();
        QJsonObject widgetObj;
        
        widgetObj["id"] = config.id;
        widgetObj["type"] = static_cast<int>(config.type);
        widgetObj["name"] = config.name;
        widgetObj["position"] = QJsonArray{config.position.x(), config.position.y()};
        widgetObj["size"] = QJsonArray{config.size.width(), config.size.height()};
        widgetObj["alwaysOnTop"] = config.alwaysOnTop;
        widgetObj["clickThrough"] = config.clickThrough;
        widgetObj["locked"] = config.locked;
        widgetObj["opacity"] = config.opacity;
        widgetObj["autoStart"] = config.autoStart;
        widgetObj["updateInterval"] = config.updateInterval;
        widgetObj["customSettings"] = config.customSettings;
        
        widgetsArray.append(widgetObj);
    }
    
    root["widgets"] = widgetsArray;
    root["version"] = "1.0.0";
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["exportedBy"] = "Desktop Widget System";
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::error(QString("无法写入导出文件: %1").arg(filePath));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    Logger::info(QString("配置导出成功至: %1").arg(filePath));
    return true;
}

bool WidgetManager::importConfiguration(const QString& filePath) {
    QFile file(filePath);
    
    if (!file.exists()) {
        Logger::error(QString("导入文件不存在: %1").arg(filePath));
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::error(QString("无法读取导入文件: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        Logger::error(QString("导入文件JSON解析错误: %1").arg(error.errorString()));
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray widgetsArray = root["widgets"].toArray();
    
    int successCount = 0;
    for (const QJsonValue& value : widgetsArray) {
        QJsonObject widgetObj = value.toObject();
        
        WidgetConfig config;
        config.id = generateUniqueId(static_cast<WidgetType>(widgetObj["type"].toInt()));
        config.type = static_cast<WidgetType>(widgetObj["type"].toInt());
        config.name = widgetObj["name"].toString();
        
        QJsonArray posArray = widgetObj["position"].toArray();
        config.position = QPoint(posArray[0].toInt(), posArray[1].toInt());
        
        QJsonArray sizeArray = widgetObj["size"].toArray();
        config.size = QSize(sizeArray[0].toInt(), sizeArray[1].toInt());
        
        config.alwaysOnTop = widgetObj["alwaysOnTop"].toBool();
        config.clickThrough = widgetObj["clickThrough"].toBool();
        config.locked = widgetObj["locked"].toBool();
        config.opacity = widgetObj["opacity"].toDouble();
        config.autoStart = widgetObj["autoStart"].toBool();
        config.updateInterval = widgetObj["updateInterval"].toInt();
        config.customSettings = widgetObj["customSettings"].toObject();
        
        if (createWidget(config)) {
            successCount++;
            if (config.autoStart) {
                startWidget(config.id);
            }
        }
    }
    
    if (m_autoSave) {
        saveConfiguration();
    }
    
    Logger::info(QString("配置导入完成，成功导入%1个Widget").arg(successCount));
    return successCount > 0;
}

WidgetPtr WidgetManager::createWidgetByType(WidgetType type, const WidgetConfig& config) {
    switch (type) {
        case WidgetType::Clock:
            return std::make_shared<ClockWidget>(config);
        case WidgetType::Weather:
            return std::make_shared<WeatherWidget>(config);
        case WidgetType::Calendar:
            return std::make_shared<CalendarWidget>(config);
        case WidgetType::AIRanking:
            return std::make_shared<AIRankingWidget>(config);
        case WidgetType::SystemPerformance:
            return std::make_shared<SystemPerformanceWidget>(config);
        case WidgetType::Notes:
            return std::make_shared<NotesWidget>(config);
        case WidgetType::SimpleNotes:
            return std::make_shared<SimpleNotesWidget>(config);
        default:
            Logger::warning(QString("未支持的Widget类型: %1").arg(static_cast<int>(type)));
            return nullptr;
    }
}

QString WidgetManager::getConfigFilePath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath + "/config.json";
}

QString WidgetManager::generateUniqueId(WidgetType type) const {
    QString typePrefix;
    switch (type) {
        case WidgetType::Clock: typePrefix = "clock"; break;
        case WidgetType::Weather: typePrefix = "weather"; break;
        case WidgetType::SystemInfo: typePrefix = "sysinfo"; break;
        case WidgetType::Calendar: typePrefix = "calendar"; break;
        case WidgetType::Notes: typePrefix = "notes"; break;
        case WidgetType::SimpleNotes: typePrefix = "simplenotes"; break;
        case WidgetType::AIRanking: typePrefix = "airanking"; break;
        case WidgetType::SystemPerformance: typePrefix = "sysperf"; break;
        default: typePrefix = "widget"; break;
    }
    
    return QString("%1_%2").arg(typePrefix).arg(QDateTime::currentMSecsSinceEpoch());
}

bool WidgetManager::validateConfig(const WidgetConfig& config) const {
    return !config.id.isEmpty() && !config.name.isEmpty();
}

void WidgetManager::connectWidgetSignals(WidgetPtr widget) {
    connect(widget.get(), &BaseWidget::closeRequested,
            this, &WidgetManager::onWidgetCloseRequested);
    connect(widget.get(), &BaseWidget::settingsRequested,
            this, &WidgetManager::onWidgetSettingsRequested);
    connect(widget.get(), &BaseWidget::configChanged,
            this, &WidgetManager::onWidgetConfigChanged);
    connect(widget.get(), &BaseWidget::statusChanged,
            this, &WidgetManager::onWidgetStatusChanged);
    connect(widget.get(), &BaseWidget::positionChanged,
            this, &WidgetManager::onWidgetPositionChanged);
}

void WidgetManager::disconnectWidgetSignals(WidgetPtr widget) {
    disconnect(widget.get(), nullptr, this, nullptr);
}

void WidgetManager::onWidgetCloseRequested(const QString& widgetId) {
    removeWidget(widgetId);
}

void WidgetManager::onWidgetSettingsRequested(const QString& widgetId) {
    WidgetPtr widget = getWidget(widgetId);
    if (!widget) {
        Logger::warning(QString("请求设置的Widget不存在: %1").arg(widgetId));
        return;
    }
    
    WidgetConfig config = widget->getConfig();
    
    // 根据小组件类型打开相应的配置对话框
    if (config.type == WidgetType::AIRanking) {
        AIRankingConfigDialog dialog(config, nullptr);
        if (dialog.exec() == QDialog::Accepted) {
            WidgetConfig updatedConfig = dialog.getUpdatedConfig();
            updateWidgetConfig(widgetId, updatedConfig);
            Logger::info(QString("AI排行榜小组件配置已更新: %1").arg(widgetId));
        }
    } else if (config.type == WidgetType::Weather) {
        WeatherConfigDialog dialog(config, nullptr);
        if (dialog.exec() == QDialog::Accepted) {
            WidgetConfig updatedConfig = dialog.getUpdatedConfig();
            qDebug() << "WidgetManager: 收到更新的配置";
            qDebug() << "API Provider:" << updatedConfig.customSettings.value("apiProvider").toString();
            qDebug() << "API Key:" << updatedConfig.customSettings.value("apiKey").toString();
            qDebug() << "City Name:" << updatedConfig.customSettings.value("cityName").toString();
            updateWidgetConfig(widgetId, updatedConfig);
            Logger::info(QString("天气小组件配置已更新: %1").arg(widgetId));
        }
    } else if (config.type == WidgetType::SystemPerformance) {
        // 对于系统性能监测小组件，使用通用配置窗口
        ConfigWindow dialog(config, nullptr);
        if (dialog.exec() == QDialog::Accepted) {
            WidgetConfig updatedConfig = dialog.getUpdatedConfig();
            updateWidgetConfig(widgetId, updatedConfig);
            Logger::info(QString("系统性能监测小组件配置已更新: %1").arg(widgetId));
        }
    } else if (config.type == WidgetType::Notes) {
        // 对于便签小组件，使用专门的配置对话框
        NotesConfigDialog dialog(config, nullptr);
        if (dialog.exec() == QDialog::Accepted) {
            WidgetConfig updatedConfig = dialog.getUpdatedConfig();
            updateWidgetConfig(widgetId, updatedConfig);
            Logger::info(QString("便签小组件配置已更新: %1").arg(widgetId));
        }
    } else {
        // 对于其他类型的小组件，使用通用配置窗口
        ConfigWindow dialog(config, nullptr);
        if (dialog.exec() == QDialog::Accepted) {
            WidgetConfig updatedConfig = dialog.getUpdatedConfig();
            updateWidgetConfig(widgetId, updatedConfig);
            Logger::info(QString("小组件配置已更新: %1").arg(widgetId));
        }
    }
}

void WidgetManager::onWidgetConfigChanged(const WidgetConfig& config) {
    if (m_autoSave) {
        m_saveTimer->start();
    }
    emit widgetConfigUpdated(config.id, config);
}

void WidgetManager::onWidgetStatusChanged(WidgetStatus status) {
    BaseWidget* widget = qobject_cast<BaseWidget*>(sender());
    if (widget) {
        emit widgetStatusChanged(widget->getConfig().id, status);
    }
}

void WidgetManager::onWidgetPositionChanged(const QString& widgetId, const QPoint& newPosition) {
    WidgetPtr widget = getWidget(widgetId);
    if (widget) {
        WidgetConfig config = widget->getConfig();
        if (config.position != newPosition) {
            config.position = newPosition;
            widget->setConfig(config);
            emit widgetPositionManuallyChanged(widgetId, newPosition);
        }
    }
} 