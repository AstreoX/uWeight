#pragma once
#include <QWidget>
#include <QTimer>
#include <QJsonObject>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include "Common/Types.h"

class BaseWidget : public QWidget {
    Q_OBJECT

public:
    explicit BaseWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    virtual ~BaseWidget() = default;

    // 配置相关
    const WidgetConfig& getConfig() const { return m_config; }
    void setConfig(const WidgetConfig& config);
    virtual void applyConfig();

    // Widget状态
    WidgetStatus getStatus() const { return m_status; }
    void setStatus(WidgetStatus status);

    // 生命周期
    virtual void initialize();
    virtual void start();
    virtual void stop();
    virtual void cleanup();

    // 更新相关
    virtual void updateContent() = 0;
    void setUpdateInterval(int interval);
    int getUpdateInterval() const { return m_updateInterval; }

    // 位置和大小
    void setPosition(const QPoint& position);
    void setSize(const QSize& size);
    void setOpacity(double opacity);

    // 窗口特性
    void setAlwaysOnTop(bool onTop);
    void setAlwaysOnBottom(bool onBottom);  // 新增：设置置于最底层
    void setAvoidMinimizeAll(bool avoid);  // 新增：设置避免被显示桌面影响
    void setClickThrough(bool clickThrough);
    void setLocked(bool locked);
    bool isLocked() const { return m_config.locked; }

#ifdef Q_OS_WIN
    // Windows平台特殊功能
    void applyWindowsAvoidMinimize();    // 应用Windows API防止最小化
    void removeWindowsAvoidMinimize();   // 移除Windows API防止最小化
#endif

signals:
    void configChanged(const WidgetConfig& config);
    void statusChanged(WidgetStatus status);
    void closeRequested(const QString& widgetId);
    void settingsRequested(const QString& widgetId);
    void positionChanged(const QString& widgetId, const QPoint& newPosition);

protected:
    // 事件处理
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    
    // 绘制相关
    void paintEvent(QPaintEvent* event) override;
    virtual void drawContent(QPainter& painter) = 0;

    // 辅助方法
    virtual QMenu* createContextMenu();
    void updateContextMenu();
    void updateWindowFlags();
    void savePosition();

protected slots:
    virtual void onUpdateTimer();
    virtual void onSettingsAction();
    virtual void onCloseAction();
    
    #ifdef Q_OS_WIN
    void maintainBottomLayer();  // 维持置底层级的定时器槽函数
    #endif

protected:
    WidgetConfig m_config;
    WidgetStatus m_status;
    QTimer* m_updateTimer;
    int m_updateInterval;
    
    // 拖拽相关
    bool m_dragging;
    QPoint m_dragStartPosition;
    
    // UI元素
    QMenu* m_contextMenu;
    QAction* m_settingsAction;
    QAction* m_lockAction;
    QAction* m_alwaysOnTopAction;     // 新增：置顶操作
    QAction* m_alwaysOnBottomAction;  // 新增：置底操作
    QAction* m_closeAction;
    
    // 混合模式相关
    #ifdef Q_OS_WIN
    QTimer* m_maintainBottomTimer{nullptr};  // 维持置底状态的定时器
    #endif
}; 