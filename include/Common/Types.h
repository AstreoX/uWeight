#pragma once
#include <QWidget>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QString>
#include <QJsonObject>
#include <QTimer>
#include <memory>
#include <vector>
#include <map>
#include <functional>

// 前向声明
class BaseWidget;
class WidgetManager;

// 智能指针类型别名
using WidgetPtr = std::shared_ptr<BaseWidget>;
using WidgetWeakPtr = std::weak_ptr<BaseWidget>;

// Widget相关类型
enum class WidgetType {
    Clock,
    Weather,    // 天气小组件
    SystemInfo,
    Calendar,
    SimpleNotes, // 极简便签小组件
    AIRanking,  // AI能力排行榜小组件
    SystemPerformance,  // 系统性能监测小组件
    Custom
};

enum class WidgetPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Center,
    Custom
};

enum class WidgetStatus {
    Active,
    Hidden,
    Minimized,
    Error
};

// Widget配置结构
struct WidgetConfig {
    QString id;
    WidgetType type;
    QString name;
    QPoint position;
    QSize size;
    bool alwaysOnTop;
    bool alwaysOnBottom;  // 新增：是否始终置于最底层
    bool avoidMinimizeAll;  // 新增：避免被Win+D等显示桌面影响
    bool clickThrough;
    double opacity;
    bool autoStart;
    int updateInterval;
    bool locked;  // 新增：是否锁定
    QJsonObject customSettings;
    
    WidgetConfig() : 
        type(WidgetType::Custom),
        position(100, 100),
        size(200, 150),
        alwaysOnTop(true),
        alwaysOnBottom(false),  // 默认不置于最底层
        avoidMinimizeAll(false),  // 默认不避免被显示桌面影响
        clickThrough(false),
        opacity(1.0),
        autoStart(false),
        updateInterval(1000),
        locked(false) {}  // 默认不锁定
};

// 回调函数类型
using WidgetCallback = std::function<void(const QString&)>;
using UpdateCallback = std::function<void()>;
using ConfigChangedCallback = std::function<void(const WidgetConfig&)>;

// 常量定义
namespace Constants {
    constexpr int DEFAULT_UPDATE_INTERVAL = 1000;
    constexpr double MIN_OPACITY = 0.1;
    constexpr double MAX_OPACITY = 1.0;
    constexpr int MIN_SIZE = 50;
    constexpr int MAX_SIZE = 2000;
} 