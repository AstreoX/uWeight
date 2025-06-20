#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "Common/Types.h"

class WidgetManager;

class SystemTray : public QObject {
    Q_OBJECT

public:
    explicit SystemTray(WidgetManager* widgetManager, QObject* parent = nullptr);
    ~SystemTray();

    void show();
    void hide();
    bool isVisible() const;

    void showMessage(const QString& title, const QString& message, 
                    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    
    // 新增：显示启动完成通知
    void showStartupNotification();
    
    // 新增：显示管理窗口隐藏通知
    void showManagementWindowHiddenNotification();

signals:
    void showManagementRequested();
    void createWidgetRequested(WidgetType type);
    void exitRequested();

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowManagement();
    void onCreateClockWidget();
    void onCreateAIRankingWidget();
    void onExit();
    void toggleAvoidMinimizeAll();

private:
    void createTrayIcon();
    void createContextMenu();
    void updateMenu();

private:
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_contextMenu;
    WidgetManager* m_widgetManager;
    
    // 菜单项
    QAction* m_showManagementAction;
    QMenu* m_createWidgetMenu;
    QAction* m_createClockAction;
    QAction* m_createAIRankingAction;
    QAction* m_exitAction;
}; 