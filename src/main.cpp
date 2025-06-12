/**
 * @file main.cpp
 * @brief 桌面小组件系统主程序入口
 * @details 初始化应用程序环境，创建核心组件并建立连接
 * @author 项目团队
 * @date 2025-5
 * @version 1.0.0
 * 
 * 主程序职责：
 * - Qt应用程序环境初始化
 * - 系统托盘功能检查和配置
 * - 核心组件创建和生命周期管理
 * - 组件间信号槽连接
 * - 配置文件加载和错误处理
 * - 应用程序事件循环启动
 * 
 * 核心组件：
 * - WidgetManager: 小组件管理系统 (梁智搏 YumeshioAmami)
 * - SystemTray: 系统托盘集成
 * - ManagementWindow: 后端管理界面
 * - Logger: 日志记录系统
 * 
 * 设计特点：
 * - 无主窗口设计，以系统托盘为主要交互方式
 * - 组件化架构，松耦合设计
 * - 完整的错误处理和日志记录
 * - 优雅的资源管理和清理机制
 */

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QSharedMemory>
#include "Framework/WidgetManager.h"
#include "Utils/SystemTray.h"
#include "BackendManagement/ManagementWindow.h"
#include "Utils/Logger.h"

/**
 * @brief 主程序入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出码，0表示正常退出
 * 
 * 程序启动流程：
 * 1. 创建QApplication实例
 * 2. 设置应用程序元信息
 * 3. 检查系统托盘可用性
 * 4. 初始化日志系统
 * 5. 创建核心管理组件
 * 6. 建立组件间信号槽连接
 * 7. 加载配置文件
 * 8. 显示系统托盘并启动事件循环
 */
int main(int argc, char *argv[])
{
    // 单实例检测
    QSharedMemory sharedMemory("uWidget_UniqueKey");
    if (!sharedMemory.create(1)) {
        QMessageBox::warning(nullptr, "uWidget 已在运行", "检测到已有 uWidget 实例在运行，不能重复启动。");
        return 0;
    }

    QApplication app(argc, argv);
    
    // 设置应用程序基本信息
    app.setApplicationName("uWidget");
    app.setApplicationVersion("1.1.0");
    app.setOrganizationName("uWidget");
    app.setOrganizationDomain("uwidget.com");
    app.setWindowIcon(QIcon(":/icons/window.png"));
    
    // 检查系统托盘支持（必需功能）
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "系统托盘",
                             "无法在此系统上检测到系统托盘。");
        return -1;
    }
    
    // 设置应用程序行为：关闭最后一个窗口时不退出程序
    // 这样应用程序可以在后台通过系统托盘继续运行
    QApplication::setQuitOnLastWindowClosed(false);
    
    // 初始化日志系统 - 必须在其他组件之前初始化
    Logger::initialize();
    Logger::info("应用程序启动");
    
    // 创建应用程序数据目录
    // 确保配置文件、日志文件等有存储位置
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    
    // 创建核心组件
    // 注意：组件创建顺序很重要，WidgetManager必须先创建
    WidgetManager widgetManager;           // 小组件管理系统 (梁智搏 YumeshioAmami)
    SystemTray systemTray(&widgetManager);        // 系统托盘管理
    ManagementWindow managementWindow(&widgetManager);  // 后端管理窗口
    
    // 建立组件间信号槽连接
    // 这些连接确保各组件能够正确通信和协作
    
    // 1. 系统托盘 -> 管理窗口：显示管理界面
    QObject::connect(&systemTray, &SystemTray::showManagementRequested,
                     &managementWindow, &ManagementWindow::showAndRaise);
    
    // 2. 系统托盘 -> Widget管理器：快速创建Widget
    // 使用Lambda表达式处理不同类型Widget的默认配置
    QObject::connect(&systemTray, &SystemTray::createWidgetRequested,
                     [&widgetManager](WidgetType type) {
                         // 创建基础配置
                         WidgetConfig config;
                         config.type = type;
                         config.id = QString("widget_%1").arg(QDateTime::currentMSecsSinceEpoch());
                         
                         // 根据Widget类型设置默认参数
                         // 每种Widget都有适合的默认尺寸和名称
                         switch (type) {
                             case WidgetType::Clock:
                                 config.name = "时钟";
                                 config.size = QSize(200, 100);  // 时钟适中尺寸
                                 break;
                             case WidgetType::Weather:
                                 config.name = "天气";
                                 config.size = QSize(250, 150);  // 天气信息较多
                                 break;
                             case WidgetType::SystemInfo:
                                 config.name = "系统信息";
                                 config.size = QSize(300, 200);  // 系统信息需要更大空间
                                 break;
                             case WidgetType::Calendar:
                                 config.name = "日历";
                                 config.size = QSize(250, 200);  // 日历标准尺寸
                                 break;
                             case WidgetType::SimpleNotes:
                                 config.name = "极简便签";
                                 config.size = QSize(250, 200);  // 极简版较小
                                 break;
                             case WidgetType::AIRanking:
                                 config.name = "AI排行榜";
                                 config.size = QSize(400, 300);  // 排行榜需要列表空间
                                 break;
                             default:
                                 config.name = "自定义";
                                 break;
                         }
                         
                         // 创建并启动Widget
                         if (widgetManager.createWidget(config)) {
                             widgetManager.startWidget(config.id);
                             Logger::info(QString("创建并启动Widget: %1").arg(config.name));
                         }
                     });
    
    // 3. 系统托盘 -> 应用程序：退出处理
    QObject::connect(&systemTray, &SystemTray::exitRequested, [&]() {
        Logger::info("应用程序退出");
        // 清理所有Widget资源
        widgetManager.cleanupAllWidgets();
        // 优雅退出Qt事件循环
        QApplication::quit();
    });
    
    // 4. Widget管理器 -> 管理窗口：Widget列表同步
    // 当Widget被创建时，更新管理界面的列表
    QObject::connect(&widgetManager, &WidgetManager::widgetCreated,
                     &managementWindow, &ManagementWindow::refreshWidgetList);
    
    // 当Widget被删除时，更新管理界面的列表
    QObject::connect(&widgetManager, &WidgetManager::widgetRemoved,
                     &managementWindow, &ManagementWindow::refreshWidgetList);
    
    // 5. 管理窗口 -> 系统托盘：窗口隐藏通知
    // 当管理窗口被隐藏到托盘时，显示通知
    QObject::connect(&managementWindow, &ManagementWindow::windowHiddenToTray,
                     &systemTray, &SystemTray::showManagementWindowHiddenNotification);
    
    // 加载用户配置文件
    // 恢复上次关闭时的Widget状态
    if (!widgetManager.loadConfiguration()) {
        Logger::warning("无法加载配置文件，将使用默认设置");
        // 即使配置加载失败，程序仍可正常运行
    }
    
    // 显示系统托盘图标并发送启动通知
    systemTray.show();
    systemTray.showStartupNotification();
    
    Logger::info("应用程序初始化完成");
    
    // 启动Qt事件循环，程序正式运行
    // 程序将在此处持续运行，直到用户退出
    return app.exec();
} 