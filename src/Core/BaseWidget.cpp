/**
 * @file BaseWidget.cpp
 * @brief 桌面小组件基类实现
 * @details 所有小组件的基础类，提供通用功能和接口
 * @author 陈佳燕 (Aveline)
 * @date 2025-5
 * @version 1.0.0
 * 
 * BaseWidget是所有桌面小组件的基础类，提供：
 * - 窗口管理和生命周期控制
 * - 拖拽、调整大小等基础交互
 * - 透明度、置顶、置底等窗口属性
 * - 右键菜单和配置界面支持
 * - 防Win+D最小化功能
 * - 跨平台窗口行为适配
 * - 主题和样式系统集成
 * 
 * 设计特点：
 * - 采用模板方法模式，子类只需实现具体绘制逻辑
 * - 完整的事件处理机制
 * - 智能的窗口层级管理
 * - 高效的绘制和更新机制
 * - 完善的错误处理和日志记录
 */

#include "Core/BaseWidget.h"
#include <QPainter>
#include <QApplication>
#include <QStyleOption>
#include <QSettings>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

/**
 * @brief BaseWidget构造函数
 * @param config 小组件配置信息
 * @param parent 父窗口指针
 * 
 * 初始化小组件基础功能：
 * - 设置窗口属性（透明背景、无边框等）
 * - 初始化更新定时器
 * - 创建右键菜单
 * - 配置拖拽系统
 * - 应用初始配置
 * 
 * 窗口属性设置：
 * - 透明背景：支持不规则形状和透明效果
 * - 无边框：创建现代化的小组件外观
 * - 工具窗口：避免在任务栏显示
 * 
 * 生命周期管理：
 * - 自动更新机制：根据配置定期刷新内容
 * - 状态监控：跟踪小组件的运行状态
 * - 资源管理：智能管理内存和系统资源
 */
BaseWidget::BaseWidget(const WidgetConfig& config, QWidget* parent)
    : QWidget(parent)
    , m_config(config)
    , m_status(WidgetStatus::Active)
    , m_updateTimer(new QTimer(this))
    , m_updateInterval(config.updateInterval)
    , m_dragging(false)
    , m_contextMenu(nullptr)
    , m_settingsAction(nullptr)
    , m_lockAction(nullptr)
    , m_alwaysOnTopAction(nullptr)
    , m_alwaysOnBottomAction(nullptr)
    , m_closeAction(nullptr)
{
    // 设置窗口基本标识
    setObjectName(config.id);
    setWindowTitle(config.name);
    
    // 设置基本窗口属性
    setAttribute(Qt::WA_TranslucentBackground, true); // 启用透明背景
    // 初始窗口标志将在applyConfig()中正确设置
    
    // 连接更新定时器到更新槽函数
    connect(m_updateTimer, &QTimer::timeout, this, &BaseWidget::onUpdateTimer);
    
    // 执行子类特定的初始化
    initialize();
    
    // 应用配置参数
    applyConfig();
}

void BaseWidget::initialize() {
    // 创建上下文菜单
    m_contextMenu = createContextMenu();
    
    // 设置默认大小策略
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void BaseWidget::start() {
    if (m_status != WidgetStatus::Active) {
        setStatus(WidgetStatus::Active);
    }
    
    if (m_updateInterval > 0) {
        m_updateTimer->start(m_updateInterval);
    }
    
    show();
    updateContent();
}

void BaseWidget::stop() {
    m_updateTimer->stop();
    setStatus(WidgetStatus::Hidden);
    hide();
}

void BaseWidget::cleanup() {
    stop();
    savePosition();
    
    #ifdef Q_OS_WIN
    // 清理维持置底的定时器
    if (m_maintainBottomTimer) {
        m_maintainBottomTimer->stop();
        m_maintainBottomTimer->deleteLater();
        m_maintainBottomTimer = nullptr;
    }
    #endif
}

void BaseWidget::setConfig(const WidgetConfig& config) {
    m_config = config;
    applyConfig();
    emit configChanged(m_config);
}

void BaseWidget::applyConfig() {
    setPosition(m_config.position);
    resize(m_config.size);
    setOpacity(m_config.opacity);
    
    // 从customSettings中读取avoidMinimizeAll设置并优先应用
    bool avoidMinimizeAll = m_config.customSettings.value("avoidMinimizeAll").toBool(false);
    m_config.avoidMinimizeAll = avoidMinimizeAll;
    
    // 应用避免最小化设置
    setAvoidMinimizeAll(avoidMinimizeAll);
    
    // 应用窗口层级设置（现在兼容防止最小化功能）
    setAlwaysOnTop(m_config.alwaysOnTop);
    setAlwaysOnBottom(m_config.alwaysOnBottom);
    
    setClickThrough(m_config.clickThrough);
    setLocked(m_config.locked);
    setUpdateInterval(m_config.updateInterval);
    
    setWindowTitle(m_config.name);
    setObjectName(m_config.id);
}

void BaseWidget::setStatus(WidgetStatus status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void BaseWidget::setUpdateInterval(int interval) {
    m_updateInterval = interval;
    m_config.updateInterval = interval;
    
    if (m_updateTimer->isActive()) {
        m_updateTimer->stop();
        if (interval > 0) {
            m_updateTimer->start(interval);
        }
    }
}

void BaseWidget::setPosition(const QPoint& position) {
    m_config.position = position;
    move(position);
}

void BaseWidget::setSize(const QSize& size) {
    m_config.size = size;
    resize(size);
}

void BaseWidget::setOpacity(double opacity) {
    m_config.opacity = qBound(Constants::MIN_OPACITY, opacity, Constants::MAX_OPACITY);
    setWindowOpacity(m_config.opacity);
}

void BaseWidget::setAlwaysOnTop(bool onTop) {
    m_config.alwaysOnTop = onTop;
    // 如果设置为置顶，则取消置底
    if (onTop) {
        m_config.alwaysOnBottom = false;
    }
    updateWindowFlags();
}

void BaseWidget::setAlwaysOnBottom(bool onBottom) {
    m_config.alwaysOnBottom = onBottom;
    // 如果设置为置底，则取消置顶
    if (onBottom) {
        m_config.alwaysOnTop = false;
    }
    updateWindowFlags();
}

void BaseWidget::setAvoidMinimizeAll(bool avoid) {
    bool oldValue = m_config.avoidMinimizeAll;
    m_config.avoidMinimizeAll = avoid;
    
    // 如果状态改变，更新窗口标志
    if (oldValue != avoid) {
        #ifdef Q_OS_WIN
        if (!avoid && oldValue) {
            // 如果之前是避免最小化状态，现在要取消，先清理Windows API设置
            removeWindowsAvoidMinimize();
        }
        #endif
        
        updateWindowFlags();
        
        // 如果开启了避免最小化，显示提示信息
        if (avoid) {
            qDebug() << "已为小组件" << m_config.name << "启用防止Win+D最小化功能";
        } else {
            qDebug() << "已为小组件" << m_config.name << "关闭防止Win+D最小化功能";
        }
    }
}

void BaseWidget::setClickThrough(bool clickThrough) {
    m_config.clickThrough = clickThrough;
    updateWindowFlags();
}

void BaseWidget::setLocked(bool locked) {
    m_config.locked = locked;
    // 锁定状态改变时更新上下文菜单
    if (m_contextMenu) {
        updateContextMenu();
    }
}

void BaseWidget::updateWindowFlags() {
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::Tool;
    
    // 设置避免Win+D的基础属性
    if (m_config.avoidMinimizeAll) {
        #ifdef Q_OS_WIN
        flags |= Qt::WindowDoesNotAcceptFocus;  // 避免获取焦点
        flags |= Qt::BypassWindowManagerHint;   // 绕过窗口管理器的某些行为
        #endif
    }
    
    // 智能窗口层级处理：解决防止最小化与始终置底的冲突
    if (m_config.avoidMinimizeAll && m_config.alwaysOnBottom) {
        // 特殊情况：同时开启防止最小化和始终置底
        // 优先保证防止最小化功能，但尽量实现置底效果
        qDebug() << "Widget" << m_config.name << "检测到防止最小化与始终置底冲突，将使用混合模式";
        
        #ifdef Q_OS_WIN
        // 在Windows上使用特殊的混合实现
        flags |= Qt::WindowStaysOnBottomHint;  // 先设置置底标志
        #else
        // 非Windows平台，防止最小化优先
        flags |= Qt::WindowStaysOnTopHint;
        #endif
    } else if (m_config.alwaysOnTop) {
        flags |= Qt::WindowStaysOnTopHint;
        qDebug() << "Widget" << m_config.name << "设置为始终置顶" << (m_config.avoidMinimizeAll ? " + 防止最小化" : "");
    } else if (m_config.alwaysOnBottom) {
        flags |= Qt::WindowStaysOnBottomHint;
        qDebug() << "Widget" << m_config.name << "设置为始终置底" << (m_config.avoidMinimizeAll ? " (混合模式)" : "");
    } else {
        // 如果只开启了防止最小化而没有其他层级设置，则使用默认置顶来防止被最小化
        if (m_config.avoidMinimizeAll) {
            flags |= Qt::WindowStaysOnTopHint;
            qDebug() << "Widget" << m_config.name << "设置为正常层级 + 防止最小化（默认置顶）";
        } else {
            qDebug() << "Widget" << m_config.name << "设置为正常层级";
        }
    }
    
    if (m_config.clickThrough) {
        flags |= Qt::WindowTransparentForInput;
    }
    
    // 保存当前的可见性状态
    bool wasVisible = isVisible();
    
    setWindowFlags(flags);
    
    // 确保窗口重新显示
    if (wasVisible) {
        show();
        
        // Windows平台特殊处理
        #ifdef Q_OS_WIN
        if (m_config.avoidMinimizeAll) {
            // 防止最小化模式下的特殊处理
            applyWindowsAvoidMinimize();
        } else if (m_config.alwaysOnBottom) {
            // 没有防止最小化时，仅应用始终置底
            lower();
        }
        #endif
    }
}

#ifdef Q_OS_WIN
void BaseWidget::applyWindowsAvoidMinimize() {
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd) return;
    
    // 方法1: 设置窗口为工具窗口
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    style |= WS_EX_TOOLWINDOW;  // 工具窗口，不在任务栏显示
    style |= WS_EX_NOACTIVATE;  // 不激活窗口
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
    
    // 方法2: 将窗口设置为桌面子窗口（更强的保护）
    HWND desktop = GetDesktopWindow();
    if (desktop) {
        SetParent(hwnd, desktop);
    }
    
    // 方法3: 智能层级设置
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    
    // 处理防止最小化与始终置底的特殊组合
    if (m_config.alwaysOnBottom && m_config.avoidMinimizeAll) {
        // 混合模式：使用定时器周期性将窗口移到底层，同时保持防最小化
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        
        // 启动定时器来维持置底状态
        if (!m_maintainBottomTimer) {
            m_maintainBottomTimer = new QTimer(this);
            connect(m_maintainBottomTimer, &QTimer::timeout, this, &BaseWidget::maintainBottomLayer);
        }
        m_maintainBottomTimer->start(1000); // 每秒检查一次
        
        qDebug() << "应用混合模式：防止最小化 + 始终置底";
    } else if (m_config.alwaysOnTop) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        qDebug() << "应用Windows API防止最小化保护 + 始终置顶";
    } else if (m_config.alwaysOnBottom) {
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        qDebug() << "应用Windows API防止最小化保护 + 始终置底";
    } else {
        // 没有特定层级要求时，使用置顶来确保防止最小化
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        qDebug() << "应用Windows API防止最小化保护（默认置顶）";
    }
}

void BaseWidget::removeWindowsAvoidMinimize() {
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd) return;
    
    // 停止维持置底的定时器
    if (m_maintainBottomTimer) {
        m_maintainBottomTimer->stop();
        m_maintainBottomTimer->deleteLater();
        m_maintainBottomTimer = nullptr;
    }
    
    // 恢复正常的窗口设置
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    style &= ~WS_EX_TOPMOST;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
    
    // 移除桌面父窗口关系
    SetParent(hwnd, nullptr);
    
    // 恢复正常的Z顺序
    if (m_config.alwaysOnTop) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else if (m_config.alwaysOnBottom) {
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    
    qDebug() << "已移除Windows API防止最小化保护";
}
#endif

void BaseWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !m_config.clickThrough && !m_config.locked) {
        m_dragging = true;
        m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void BaseWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton) && !m_config.clickThrough && !m_config.locked) {
        QPoint newPosition = event->globalPosition().toPoint() - m_dragStartPosition;
        setPosition(newPosition);
        event->accept();
        emit positionChanged(m_config.id, newPosition);
    }
    QWidget::mouseMoveEvent(event);
}

void BaseWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_dragging) {
            m_dragging = false;
            savePosition();
            event->accept();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void BaseWidget::contextMenuEvent(QContextMenuEvent* event) {
    if (m_contextMenu && !m_config.clickThrough) {
        m_contextMenu->exec(event->globalPos());
    }
}

void BaseWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), QColor(0, 0, 0, 50));
    
    // 调用子类的绘制方法
    drawContent(painter);
    
    QWidget::paintEvent(event);
}

QMenu* BaseWidget::createContextMenu() {
    QMenu* menu = new QMenu(this);
    
    m_settingsAction = menu->addAction("设置");
    connect(m_settingsAction, &QAction::triggered, this, &BaseWidget::onSettingsAction);
    
    menu->addSeparator();
    
    // 窗口层级菜单
    QMenu* layerMenu = menu->addMenu("窗口层级");
    
    m_alwaysOnTopAction = layerMenu->addAction("始终置顶");
    m_alwaysOnTopAction->setCheckable(true);
    m_alwaysOnTopAction->setChecked(m_config.alwaysOnTop);
    connect(m_alwaysOnTopAction, &QAction::triggered, [this](bool checked) {
        setAlwaysOnTop(checked);
        updateContextMenu();
        emit configChanged(m_config);
    });
    
    m_alwaysOnBottomAction = layerMenu->addAction("始终置底");
    m_alwaysOnBottomAction->setCheckable(true);
    m_alwaysOnBottomAction->setChecked(m_config.alwaysOnBottom);
    connect(m_alwaysOnBottomAction, &QAction::triggered, [this](bool checked) {
        setAlwaysOnBottom(checked);
        updateContextMenu();
        emit configChanged(m_config);
    });
    
    QAction* normalLayerAction = layerMenu->addAction("正常层级");
    connect(normalLayerAction, &QAction::triggered, [this]() {
        setAlwaysOnTop(false);
        setAlwaysOnBottom(false);
        updateContextMenu();
        emit configChanged(m_config);
    });
    
    menu->addSeparator();
    
    m_lockAction = menu->addAction(m_config.locked ? "解锁" : "锁定");
    m_lockAction->setCheckable(true);
    m_lockAction->setChecked(m_config.locked);
    connect(m_lockAction, &QAction::triggered, [this](bool checked) {
        setLocked(checked);
        emit configChanged(m_config);
    });
    
    menu->addSeparator();
    
    m_closeAction = menu->addAction("关闭");
    connect(m_closeAction, &QAction::triggered, this, &BaseWidget::onCloseAction);
    
    return menu;
}

void BaseWidget::updateContextMenu() {
    if (m_lockAction) {
        m_lockAction->setText(m_config.locked ? "解锁" : "锁定");
        m_lockAction->setChecked(m_config.locked);
    }
    
    if (m_alwaysOnTopAction) {
        m_alwaysOnTopAction->setChecked(m_config.alwaysOnTop);
    }
    
    if (m_alwaysOnBottomAction) {
        m_alwaysOnBottomAction->setChecked(m_config.alwaysOnBottom);
    }
}

void BaseWidget::savePosition() {
    QSettings settings;
    settings.beginGroup("Widgets/" + m_config.id);
    settings.setValue("position", pos());
    settings.endGroup();
}

void BaseWidget::onUpdateTimer() {
    if (m_status == WidgetStatus::Active) {
        updateContent();
    }
}

void BaseWidget::onSettingsAction() {
    emit settingsRequested(m_config.id);
}

void BaseWidget::onCloseAction() {
    emit closeRequested(m_config.id);
}

#ifdef Q_OS_WIN
void BaseWidget::maintainBottomLayer() {
    // 维持窗口在底层，同时保持防最小化特性
    if (!m_config.avoidMinimizeAll || !m_config.alwaysOnBottom) {
        // 如果配置改变了，停止定时器
        if (m_maintainBottomTimer) {
            m_maintainBottomTimer->stop();
        }
        return;
    }
    
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (hwnd && isVisible()) {
        // 定期将窗口移动到底层，但保持其防最小化特性
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}
#endif