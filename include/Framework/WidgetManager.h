#pragma once
#include <QObject>
#include <QMap>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include "Common/Types.h"
#include "Core/BaseWidget.h"

class WidgetManager : public QObject {
    Q_OBJECT

public:
    explicit WidgetManager(QObject* parent = nullptr);
    ~WidgetManager();

    // Widget生命周期管理
    bool createWidget(const WidgetConfig& config);
    bool removeWidget(const QString& widgetId);
    bool startWidget(const QString& widgetId);
    bool stopWidget(const QString& widgetId);
    void startAllWidgets();
    void stopAllWidgets();
    void cleanupAllWidgets();

    // Widget查询
    WidgetPtr getWidget(const QString& widgetId) const;
    QList<WidgetPtr> getAllWidgets() const;
    QList<WidgetPtr> getWidgetsByType(WidgetType type) const;
    bool hasWidget(const QString& widgetId) const;
    int getWidgetCount() const;

    // 配置管理
    bool updateWidgetConfig(const QString& widgetId, const WidgetConfig& config);
    WidgetConfig getWidgetConfig(const QString& widgetId) const;
    QList<WidgetConfig> getAllConfigs() const;

    // 持久化
    bool saveConfiguration() const;
    bool loadConfiguration();
    bool exportConfiguration(const QString& filePath) const;
    bool importConfiguration(const QString& filePath);
    
    // 内部方法
    bool loadConfigurationFromFile(const QString& filePath);

    // 模板和预设
    QList<WidgetConfig> getTemplates() const;
    bool saveAsTemplate(const QString& widgetId, const QString& templateName);
    bool createFromTemplate(const QString& templateName, const QString& newId);

    // 统计信息
    QMap<WidgetType, int> getWidgetStatistics() const;
    QStringList getActiveWidgetIds() const;
    QStringList getWidgetIds() const;  // 获取所有小组件ID列表

signals:
    void widgetCreated(const QString& widgetId);
    void widgetRemoved(const QString& widgetId);
    void widgetConfigUpdated(const QString& widgetId, const WidgetConfig& config);
    void widgetStatusChanged(const QString& widgetId, WidgetStatus status);
    void widgetPositionManuallyChanged(const QString& widgetId, const QPoint& newPosition);
    void configurationChanged();

private slots:
    void onWidgetCloseRequested(const QString& widgetId);
    void onWidgetSettingsRequested(const QString& widgetId);
    void onWidgetConfigChanged(const WidgetConfig& config);
    void onWidgetStatusChanged(WidgetStatus status);
    void onWidgetPositionChanged(const QString& widgetId, const QPoint& newPosition);

private:
    // Widget工厂方法
    WidgetPtr createWidgetByType(WidgetType type, const WidgetConfig& config);
    
    // 配置文件路径
    QString getConfigFilePath() const;
    QString getTemplatesFilePath() const;
    
    // 辅助方法
    QString generateUniqueId(WidgetType type) const;
    bool validateConfig(const WidgetConfig& config) const;
    void connectWidgetSignals(WidgetPtr widget);
    void disconnectWidgetSignals(WidgetPtr widget);

private:
    QMap<QString, WidgetPtr> m_widgets;
    QMap<QString, WidgetConfig> m_templates;
    mutable QTimer* m_saveTimer; // 延迟保存
    bool m_autoSave;
}; 