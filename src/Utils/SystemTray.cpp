#include "Utils/SystemTray.h"
#include "Framework/WidgetManager.h"
#include <QApplication>
#include <QPixmap>
#include <QIcon>

SystemTray::SystemTray(WidgetManager* widgetManager, QObject* parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_contextMenu(nullptr)
    , m_widgetManager(widgetManager)
{
    createTrayIcon();
    createContextMenu();
}

SystemTray::~SystemTray() = default;

void SystemTray::show() {
    if (m_trayIcon) {
        m_trayIcon->show();
    }
}

void SystemTray::hide() {
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool SystemTray::isVisible() const {
    return m_trayIcon && m_trayIcon->isVisible();
}

void SystemTray::showMessage(const QString& title, const QString& message, 
                            QSystemTrayIcon::MessageIcon icon) {
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, message, icon);
    }
}

void SystemTray::showStartupNotification() {
    if (m_trayIcon) {
        m_trayIcon->showMessage("uWidget桌面小组件系统", 
                               "🚀 应用程序已成功启动！\n"
                               "• 双击托盘图标打开管理界面\n"
                               "• 右键托盘图标查看更多选项\n"
                               "• Made by uTools Studio", 
                               QSystemTrayIcon::Information, 
                               5000);  // 显示5秒
    }
}

void SystemTray::showManagementWindowHiddenNotification() {
    if (m_trayIcon) {
        m_trayIcon->showMessage("管理窗口已隐藏", 
                               "📝 管理窗口已最小化到系统托盘\n"
                               "• 双击托盘图标可重新打开管理界面\n"
                               "• 您的小组件将继续在后台运行", 
                               QSystemTrayIcon::Information, 
                               3000);  // 显示3秒
    }
}

void SystemTray::createTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    
    // 尝试加载图标，如果失败则使用默认图标
    QIcon icon(":/icons/tray.png");
    if (icon.isNull()) {
        // 如果图标加载失败，创建一个简单的默认图标
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::blue);
        icon = QIcon(pixmap);
    }
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip("桌面小组件系统");
    
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &SystemTray::onTrayActivated);
}

void SystemTray::createContextMenu() {
    m_contextMenu = new QMenu();
    
    // 管理菜单
    m_showManagementAction = m_contextMenu->addAction("管理小组件");
    m_showManagementAction->setIcon(QIcon(":/icons/settings.png"));
    connect(m_showManagementAction, &QAction::triggered, this, &SystemTray::onShowManagement);
    
    m_contextMenu->addSeparator();
    
    // 防止Win+D影响选项
    QAction* avoidMinimizeAction = m_contextMenu->addAction("防止所有小组件被Win+D影响");
    avoidMinimizeAction->setCheckable(true);
    avoidMinimizeAction->setChecked(false);
    avoidMinimizeAction->setToolTip("开启后所有小组件都不会被Win+D等显示桌面快捷键影响");
    connect(avoidMinimizeAction, &QAction::triggered, this, &SystemTray::toggleAvoidMinimizeAll);
    
    m_contextMenu->addSeparator();
    
    // 创建小组件子菜单
    m_createWidgetMenu = m_contextMenu->addMenu("创建小组件");
    
    m_createClockAction = m_createWidgetMenu->addAction("时钟");
    connect(m_createClockAction, &QAction::triggered, this, &SystemTray::onCreateClockWidget);
    
    m_createAIRankingAction = m_createWidgetMenu->addAction("AI排行榜");
    connect(m_createAIRankingAction, &QAction::triggered, this, &SystemTray::onCreateAIRankingWidget);
    
    m_contextMenu->addSeparator();
    
    // 退出
    m_exitAction = m_contextMenu->addAction("退出");
    m_exitAction->setIcon(QIcon(":/icons/exit.png"));
    connect(m_exitAction, &QAction::triggered, this, &SystemTray::onExit);
    
    m_trayIcon->setContextMenu(m_contextMenu);
}

void SystemTray::updateMenu() {
    // TODO: 根据当前Widget状态更新菜单
}

void SystemTray::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        emit showManagementRequested();
    }
}

void SystemTray::onShowManagement() {
    emit showManagementRequested();
}

void SystemTray::onCreateClockWidget() {
    emit createWidgetRequested(WidgetType::Clock);
}

void SystemTray::onCreateAIRankingWidget() {
    emit createWidgetRequested(WidgetType::AIRanking);
}

void SystemTray::onExit() {
    emit exitRequested();
}

void SystemTray::toggleAvoidMinimizeAll() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;
    
    bool enabled = action->isChecked();
    
    if (m_widgetManager) {
        // 获取所有小组件的ID列表
        QStringList widgetIds = m_widgetManager->getWidgetIds();
        
        int successCount = 0;
        int totalCount = widgetIds.size();
        
        for (const QString& widgetId : widgetIds) {
            auto widget = m_widgetManager->getWidget(widgetId);
            if (widget) {
                WidgetConfig config = widget->getConfig();
                
                // 在自定义设置中设置avoidMinimizeAll
                config.customSettings["avoidMinimizeAll"] = enabled;
                
                if (m_widgetManager->updateWidgetConfig(widgetId, config)) {
                    successCount++;
                }
            }
        }
        
        // 显示通知
        if (totalCount > 0) {
            QString message;
            if (enabled) {
                message = QString("已为 %1/%2 个小组件启用防止Win+D影响功能")
                         .arg(successCount).arg(totalCount);
            } else {
                message = QString("已为 %1/%2 个小组件关闭防止Win+D影响功能")
                         .arg(successCount).arg(totalCount);
            }
            
            m_trayIcon->showMessage("小组件设置", message, 
                                   QSystemTrayIcon::Information, 3000);
        } else {
            m_trayIcon->showMessage("小组件设置", "当前没有活动的小组件", 
                                   QSystemTrayIcon::Warning, 2000);
        }
    }
} 