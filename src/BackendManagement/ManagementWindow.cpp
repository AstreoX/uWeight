#include "BackendManagement/ManagementWindow.h"
#include "BackendManagement/CreateWidgetDialog.h"
#include "BackendManagement/ConfigWindow.h"
#include "BackendManagement/AIRankingConfigDialog.h"
#include "BackendManagement/WeatherConfigDialog.h"
#include "BackendManagement/NotesConfigDialog.h"
#include "Framework/WidgetManager.h"
#include <QApplication>
#include <QCloseEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QBrush>
#include <QListWidgetItem>
#include <QFormLayout>
#include <QScreen>
#include <QJsonDocument>
#include <QJsonParseError>
#include <iostream>

ManagementWindow::ManagementWindow(WidgetManager* widgetManager, QWidget* parent)
    : QMainWindow(parent)
    , m_widgetManager(widgetManager)
{
    setWindowTitle("桌面小组件管理");
    setWindowIcon(QIcon(":/icons/window.png"));
    setMinimumSize(800, 600);
    setupUI();
    clearSettingsPanel(); // 初始化时清空设置面板
    
    // 显示屏幕分辨率信息
    QScreen* primaryScreen = QApplication::primaryScreen();
    if (primaryScreen && m_statusLabel) {
        QRect screenGeometry = primaryScreen->geometry();
        int width = screenGeometry.width();
        int height = screenGeometry.height();
        
        if (width > 0 && height > 0) {
            m_statusLabel->setText(QString("就绪 - 屏幕分辨率: %1x%2").arg(width).arg(height));
        } else {
            m_statusLabel->setText("就绪 - 使用默认分辨率");
        }
    } else if (m_statusLabel) {
        m_statusLabel->setText("就绪");
    }

    // 连接WidgetManager的信号
    if (m_widgetManager) {
        connect(m_widgetManager, &WidgetManager::widgetPositionManuallyChanged,
                this, &ManagementWindow::onWidgetManuallyMoved);
        // 可以考虑在这里也连接widgetConfigUpdated信号，如果通过其他方式更新配置也需要刷新UI
        connect(m_widgetManager, &WidgetManager::widgetConfigUpdated,
            [this](const QString& widgetId, const WidgetConfig& /*config*/) {
                if (widgetId == getCurrentSelectedWidgetId()) {
                    updateSettingsPanel(); // 如果当前选中的组件配置被更新，则刷新设置面板
                }
            });
    }
}

ManagementWindow::~ManagementWindow() = default;

void ManagementWindow::showAndRaise() {
    show();
    raise();
    activateWindow();
}

void ManagementWindow::refreshWidgetList() {
    if (!m_widgetListWidget) return;
    
    m_widgetListWidget->clear();
    
    auto widgets = m_widgetManager->getAllWidgets();
    for (const auto& widget : widgets) {
        const auto& config = widget->getConfig();
        WidgetStatus status = widget->getStatus();
        
        QString statusText;
        QString statusColor;
        
        switch (status) {
            case WidgetStatus::Active:
                statusText = "运行中";
                statusColor = "green";
                break;
            case WidgetStatus::Hidden:
                statusText = "已隐藏";
                statusColor = "orange";
                break;
            case WidgetStatus::Minimized:
                statusText = "最小化";
                statusColor = "blue";
                break;
            case WidgetStatus::Error:
                statusText = "错误";
                statusColor = "red";
                break;
        }
        
        QString typeText;
        switch (config.type) {
            case WidgetType::Clock: typeText = "时钟"; break;
            case WidgetType::Weather: typeText = "天气"; break;
            case WidgetType::SystemInfo: typeText = "系统信息"; break;
            case WidgetType::Calendar: typeText = "日历"; break;
            case WidgetType::Notes: typeText = "便签"; break;
            case WidgetType::SimpleNotes: typeText = "极简便签"; break;
            case WidgetType::AIRanking: typeText = "AI排行榜"; break;
            case WidgetType::SystemPerformance: typeText = "系统性能监测"; break;
            default: typeText = "自定义"; break;
        }
        
        QString lockText = config.locked ? " 🔒" : "";
        QString itemText = QString("%1 [%2] - %3%4 (%5)")
                          .arg(config.name)
                          .arg(typeText)
                          .arg(statusText)
                          .arg(lockText)
                          .arg(config.id);
        
        QListWidgetItem* item = new QListWidgetItem(itemText);
        
        // 根据状态设置颜色
        if (status == WidgetStatus::Active) {
            item->setForeground(QBrush(QColor("darkgreen")));
        } else if (status == WidgetStatus::Error) {
            item->setForeground(QBrush(QColor("red")));
        } else if (status == WidgetStatus::Hidden) {
            item->setForeground(QBrush(QColor("orange")));
        } else {
            item->setForeground(QBrush(QColor("blue")));
        }
        
        m_widgetListWidget->addItem(item);
    }
    
    updateWidgetInfo();
}

void ManagementWindow::setupUI() {
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    
    setupMenuBar();
    setupWidgetList();
    setupControlButtons();
    setupSettingsPanel();
    setupStatusBar();
    
    updateSliderRanges();
}

void ManagementWindow::setupWidgetList() {
    m_listGroupBox = new QGroupBox("Widget列表");
    m_listLayout = new QVBoxLayout(m_listGroupBox);
    
    // 添加搜索框
    QHBoxLayout* searchLayout = new QHBoxLayout;
    QLabel* searchLabel = new QLabel("搜索:");
    QLineEdit* searchLineEdit = new QLineEdit;
    searchLineEdit->setPlaceholderText("输入组件名称或类型进行搜索...");
    
    QComboBox* statusFilterCombo = new QComboBox;
    statusFilterCombo->addItem("全部状态", -1);
    statusFilterCombo->addItem("运行中", static_cast<int>(WidgetStatus::Active));
    statusFilterCombo->addItem("已隐藏", static_cast<int>(WidgetStatus::Hidden));
    statusFilterCombo->addItem("最小化", static_cast<int>(WidgetStatus::Minimized));
    statusFilterCombo->addItem("错误", static_cast<int>(WidgetStatus::Error));
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchLineEdit, 1);
    searchLayout->addWidget(statusFilterCombo);
    
    m_widgetListWidget = new QListWidget;
    
    m_listLayout->addLayout(searchLayout);
    m_listLayout->addWidget(m_widgetListWidget);
    
    m_mainLayout->addWidget(m_listGroupBox);
    
    connect(m_widgetListWidget, &QListWidget::currentRowChanged,
            this, &ManagementWindow::onWidgetListSelectionChanged);
    
    // 连接搜索和过滤信号
    connect(searchLineEdit, &QLineEdit::textChanged, [this, searchLineEdit, statusFilterCombo]() {
        filterWidgetList(searchLineEdit->text(), statusFilterCombo->currentData().toInt());
    });
    
    connect(statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this, searchLineEdit, statusFilterCombo]() {
        filterWidgetList(searchLineEdit->text(), statusFilterCombo->currentData().toInt());
    });
}

void ManagementWindow::setupControlButtons() {
    m_buttonLayout = new QVBoxLayout;
    
    m_createButton = new QPushButton("创建");
    m_removeButton = new QPushButton("移除");
    m_startButton = new QPushButton("启动");
    m_stopButton = new QPushButton("停止");
    m_configureButton = new QPushButton("配置");
    
    m_buttonLayout->addWidget(m_createButton);
    m_buttonLayout->addWidget(m_removeButton);
    m_buttonLayout->addSpacing(10);
    m_buttonLayout->addWidget(m_startButton);
    m_buttonLayout->addWidget(m_stopButton);
    m_buttonLayout->addWidget(m_configureButton);
    m_buttonLayout->addSpacing(10);
    
    // 批量操作按钮
    QPushButton* startAllButton = new QPushButton("启动全部");
    QPushButton* stopAllButton = new QPushButton("停止全部");
    QPushButton* refreshButton = new QPushButton("刷新");
    
    startAllButton->setToolTip("启动所有组件");
    stopAllButton->setToolTip("停止所有组件");
    refreshButton->setToolTip("刷新组件列表");
    
    m_buttonLayout->addWidget(startAllButton);
    m_buttonLayout->addWidget(stopAllButton);
    m_buttonLayout->addWidget(refreshButton);
    m_buttonLayout->addStretch();
    
    // 连接批量操作信号
    connect(startAllButton, &QPushButton::clicked, [this]() {
        m_widgetManager->startAllWidgets();
        refreshWidgetList();
        m_statusLabel->setText("已启动所有组件");
    });
    
    connect(stopAllButton, &QPushButton::clicked, [this]() {
        m_widgetManager->stopAllWidgets();
        refreshWidgetList();
        m_statusLabel->setText("已停止所有组件");
    });
    
    connect(refreshButton, &QPushButton::clicked, [this]() {
        refreshWidgetList();
        m_statusLabel->setText("组件列表已刷新");
    });
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    connect(m_createButton, &QPushButton::clicked, this, &ManagementWindow::onCreateWidget);
    connect(m_removeButton, &QPushButton::clicked, this, &ManagementWindow::onRemoveWidget);
    connect(m_startButton, &QPushButton::clicked, this, &ManagementWindow::onStartWidget);
    connect(m_stopButton, &QPushButton::clicked, this, &ManagementWindow::onStopWidget);
    connect(m_configureButton, &QPushButton::clicked, this, &ManagementWindow::onConfigureWidget);
}

void ManagementWindow::setupSettingsPanel() {
    m_settingsGroupBox = new QGroupBox("组件设置");
    m_settingsLayout = new QVBoxLayout(m_settingsGroupBox);
    
    // 基本信息区域
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    
    m_nameLineEdit = new QLineEdit;
    m_nameLineEdit->setReadOnly(true);
    
    m_typeComboBox = new QComboBox;
    m_typeComboBox->addItem("时钟", static_cast<int>(WidgetType::Clock));
    m_typeComboBox->addItem("天气", static_cast<int>(WidgetType::Weather));
    m_typeComboBox->addItem("系统信息", static_cast<int>(WidgetType::SystemInfo));
    m_typeComboBox->addItem("日历", static_cast<int>(WidgetType::Calendar));
    m_typeComboBox->addItem("便签", static_cast<int>(WidgetType::Notes));
    m_typeComboBox->addItem("极简便签", static_cast<int>(WidgetType::SimpleNotes));
    m_typeComboBox->addItem("AI排行榜", static_cast<int>(WidgetType::AIRanking));
    m_typeComboBox->addItem("系统性能监测", static_cast<int>(WidgetType::SystemPerformance));
    m_typeComboBox->setEnabled(false);
    
    basicLayout->addRow("名称:", m_nameLineEdit);
    basicLayout->addRow("类型:", m_typeComboBox);
    
    // 位置和大小区域
    QGroupBox* geometryGroup = new QGroupBox("位置和大小");
    QFormLayout* geometryLayout = new QFormLayout(geometryGroup);
    
    // X坐标控件组合
    QHBoxLayout* xLayout = new QHBoxLayout;
    m_xSpinBox = new QSpinBox;
    m_xSpinBox->setRange(0, 9999);
    m_xSpinBox->setToolTip("输入X坐标值，按回车应用");
    m_xSlider = new QSlider(Qt::Horizontal);
    m_xSlider->setTickPosition(QSlider::TicksBelow);
    m_xSlider->setToolTip("拖动滑条调整X坐标，释放时应用\n范围将根据屏幕分辨率自动调整");
    xLayout->addWidget(m_xSpinBox);
    xLayout->addWidget(m_xSlider, 1);
    
    // Y坐标控件组合
    QHBoxLayout* yLayout = new QHBoxLayout;
    m_ySpinBox = new QSpinBox;
    m_ySpinBox->setRange(0, 9999);
    m_ySpinBox->setToolTip("输入Y坐标值，按回车应用");
    m_ySlider = new QSlider(Qt::Horizontal);
    m_ySlider->setTickPosition(QSlider::TicksBelow);
    m_ySlider->setToolTip("拖动滑条调整Y坐标，释放时应用\n范围将根据屏幕分辨率自动调整");
    yLayout->addWidget(m_ySpinBox);
    yLayout->addWidget(m_ySlider, 1);
    
    // 宽度控件组合
    QHBoxLayout* widthLayout = new QHBoxLayout;
    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setRange(Constants::MIN_SIZE, Constants::MAX_SIZE);
    m_widthSpinBox->setToolTip("输入宽度值，按回车应用");
    m_widthSlider = new QSlider(Qt::Horizontal);
    m_widthSlider->setTickPosition(QSlider::TicksBelow);
    m_widthSlider->setToolTip("拖动滑条调整宽度，释放时应用\n范围：最小尺寸到屏幕宽度的80%");
    widthLayout->addWidget(m_widthSpinBox);
    widthLayout->addWidget(m_widthSlider, 1);
    
    // 高度控件组合
    QHBoxLayout* heightLayout = new QHBoxLayout;
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setRange(Constants::MIN_SIZE, Constants::MAX_SIZE);
    m_heightSpinBox->setToolTip("输入高度值，按回车应用");
    m_heightSlider = new QSlider(Qt::Horizontal);
    m_heightSlider->setTickPosition(QSlider::TicksBelow);
    m_heightSlider->setToolTip("拖动滑条调整高度，释放时应用\n范围：最小尺寸到屏幕高度的80%");
    heightLayout->addWidget(m_heightSpinBox);
    heightLayout->addWidget(m_heightSlider, 1);
    
    geometryLayout->addRow("X坐标:", xLayout);
    geometryLayout->addRow("Y坐标:", yLayout);
    geometryLayout->addRow("宽度:", widthLayout);
    geometryLayout->addRow("高度:", heightLayout);
    
    // 显示属性区域
    QGroupBox* displayGroup = new QGroupBox("显示属性");
    QFormLayout* displayLayout = new QFormLayout(displayGroup);
    
    // 透明度控件组合
    QHBoxLayout* opacityLayout = new QHBoxLayout;
    m_opacitySpinBox = new QDoubleSpinBox;
    m_opacitySpinBox->setRange(Constants::MIN_OPACITY, Constants::MAX_OPACITY);
    m_opacitySpinBox->setSingleStep(0.01);
    m_opacitySpinBox->setDecimals(2);
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(static_cast<int>(Constants::MIN_OPACITY * 100), 
                             static_cast<int>(Constants::MAX_OPACITY * 100));
    m_opacitySlider->setTickPosition(QSlider::TicksBelow);
    m_opacitySlider->setTickInterval(10);
    opacityLayout->addWidget(m_opacitySpinBox);
    opacityLayout->addWidget(m_opacitySlider, 1);
    
    m_updateIntervalSpinBox = new QSpinBox;
    m_updateIntervalSpinBox->setRange(0, 60000);
    m_updateIntervalSpinBox->setSuffix(" ms");
    
    m_alwaysOnTopCheckBox = new QCheckBox;
    m_alwaysOnBottomCheckBox = new QCheckBox;
    
    // 窗口层级选择
    m_windowLayerComboBox = new QComboBox;
    m_windowLayerComboBox->addItem("正常层级", 0);
    m_windowLayerComboBox->addItem("始终置顶", 1);
    m_windowLayerComboBox->addItem("始终置底", 2);
    
    // 避免被显示桌面影响
    m_avoidMinimizeAllCheckBox = new QCheckBox;
    m_avoidMinimizeAllCheckBox->setToolTip("避免被Win+D等显示桌面快捷键影响");
    
    m_clickThroughCheckBox = new QCheckBox;
    m_lockedCheckBox = new QCheckBox;
    m_autoStartCheckBox = new QCheckBox;
    
    displayLayout->addRow("透明度:", opacityLayout);
    displayLayout->addRow("更新间隔:", m_updateIntervalSpinBox);
    displayLayout->addRow("窗口层级:", m_windowLayerComboBox);
    displayLayout->addRow("避免最小化:", m_avoidMinimizeAllCheckBox);
    displayLayout->addRow("始终置顶:", m_alwaysOnTopCheckBox);
    displayLayout->addRow("始终置底:", m_alwaysOnBottomCheckBox);
    displayLayout->addRow("鼠标穿透:", m_clickThroughCheckBox);
    displayLayout->addRow("锁定位置:", m_lockedCheckBox);
    displayLayout->addRow("自动启动:", m_autoStartCheckBox);
    
    // 自定义设置区域
    QGroupBox* customGroup = new QGroupBox("自定义设置");
    QVBoxLayout* customLayout = new QVBoxLayout(customGroup);
    
    m_customSettingsTextEdit = new QTextEdit;
    m_customSettingsTextEdit->setMaximumHeight(100);
    m_customSettingsTextEdit->setPlaceholderText("JSON格式的自定义设置...");
    
    customLayout->addWidget(m_customSettingsTextEdit);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_applyButton = new QPushButton("应用");
    m_applyButton->setEnabled(false);
    
    m_resetButton = new QPushButton("重置");
    m_resetButton->setEnabled(false);
    
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    
    // 组装设置面板
    m_settingsLayout->addWidget(basicGroup);
    m_settingsLayout->addWidget(geometryGroup);
    m_settingsLayout->addWidget(displayGroup);
    m_settingsLayout->addWidget(customGroup);
    m_settingsLayout->addLayout(buttonLayout);
    m_settingsLayout->addStretch();
    
    m_mainLayout->addWidget(m_settingsGroupBox);
    
    // 连接信号槽
    connect(m_applyButton, &QPushButton::clicked, this, &ManagementWindow::onApplySettings);
    connect(m_resetButton, &QPushButton::clicked, this, &ManagementWindow::onResetSettings);
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ManagementWindow::onWidgetTypeChanged);
    
    // 连接滑条与输入框的双向绑定
    setupSliderConnections();
    
    // 连接变化信号来启用应用按钮（仅标记为已修改）
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_windowLayerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManagementWindow::onSettingsChanged);
    connect(m_avoidMinimizeAllCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onSettingsChanged);
    connect(m_avoidMinimizeAllCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onAvoidMinimizeChanged);
    connect(m_alwaysOnTopCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onSettingsChanged);
    connect(m_alwaysOnBottomCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onSettingsChanged);
    connect(m_alwaysOnBottomCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onAvoidMinimizeChanged);
    connect(m_clickThroughCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onSettingsChanged);
    connect(m_lockedCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onSettingsChanged);
    connect(m_autoStartCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onSettingsChanged);
    connect(m_customSettingsTextEdit, &QTextEdit::textChanged, this, &ManagementWindow::onSettingsChanged);
    
    // 连接回车和失去焦点事件来立即应用设置
    connect(m_xSpinBox, &QSpinBox::editingFinished, this, &ManagementWindow::onInstantApplySettings);
    connect(m_ySpinBox, &QSpinBox::editingFinished, this, &ManagementWindow::onInstantApplySettings);
    connect(m_widthSpinBox, &QSpinBox::editingFinished, this, &ManagementWindow::onInstantApplySettings);
    connect(m_heightSpinBox, &QSpinBox::editingFinished, this, &ManagementWindow::onInstantApplySettings);
    connect(m_opacitySpinBox, &QDoubleSpinBox::editingFinished, this, &ManagementWindow::onInstantApplySettings);
    
    // 连接复选框和下拉框的即时应用
    connect(m_windowLayerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManagementWindow::onInstantApplySettings);
    connect(m_avoidMinimizeAllCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onInstantApplySettings);
    connect(m_clickThroughCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onInstantApplySettings);
    connect(m_lockedCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onInstantApplySettings);
    connect(m_autoStartCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onInstantApplySettings);
    
    // 连接复选框与下拉框的同步逻辑
    connect(m_alwaysOnTopCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onAlwaysOnTopCheckChanged);
    connect(m_alwaysOnBottomCheckBox, &QCheckBox::toggled, this, &ManagementWindow::onAlwaysOnBottomCheckChanged);
    connect(m_windowLayerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManagementWindow::onWindowLayerComboChanged);
}

void ManagementWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // 文件菜单
    QMenu* fileMenu = menuBar->addMenu("文件(&F)");
    
    QAction* newAction = fileMenu->addAction("新建组件(&N)");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &ManagementWindow::onCreateWidget);
    
    fileMenu->addSeparator();
    
    QAction* importAction = fileMenu->addAction("导入配置(&I)...");
    importAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(importAction, &QAction::triggered, this, &ManagementWindow::onImportConfig);
    
    QAction* exportAction = fileMenu->addAction("导出配置(&E)...");
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(exportAction, &QAction::triggered, this, &ManagementWindow::onExportConfig);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence("Alt+F4"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // 组件菜单
    QMenu* widgetMenu = menuBar->addMenu("组件(&W)");
    
    QAction* startAction = widgetMenu->addAction("启动(&S)");
    startAction->setShortcut(QKeySequence("F5"));
    connect(startAction, &QAction::triggered, this, &ManagementWindow::onStartWidget);
    
    QAction* stopAction = widgetMenu->addAction("停止(&T)");
    stopAction->setShortcut(QKeySequence("F6"));
    connect(stopAction, &QAction::triggered, this, &ManagementWindow::onStopWidget);
    
    QAction* configAction = widgetMenu->addAction("配置(&C)");
    configAction->setShortcut(QKeySequence("F2"));
    connect(configAction, &QAction::triggered, this, &ManagementWindow::onConfigureWidget);
    
    widgetMenu->addSeparator();
    
    QAction* removeAction = widgetMenu->addAction("删除(&D)");
    removeAction->setShortcut(QKeySequence::Delete);
    connect(removeAction, &QAction::triggered, this, &ManagementWindow::onRemoveWidget);
    
    // 帮助菜单
    QMenu* helpMenu = menuBar->addMenu("帮助(&H)");
    
    QAction* aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于",
            "桌面小组件系统 v1.0.0\n\n"
            "一个高性能的桌面小组件管理系统\n"
            "支持多种类型的小组件，具有良好的扩展性。\n\n"
            "开发工具: Qt 6 + C++17");
    });
}

void ManagementWindow::setupStatusBar() {
    m_statusLabel = new QLabel("就绪");
    m_widgetCountLabel = new QLabel("Widget数量: 0");
    
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_widgetCountLabel);
}

void ManagementWindow::updateSliderRanges() {
    // 获取主屏幕分辨率，添加更安全的检查
    QScreen* primaryScreen = QApplication::primaryScreen();
    if (!primaryScreen) {
        // 如果无法获取屏幕信息，使用默认值
        int defaultWidth = 1920;
        int defaultHeight = 1080;
        
        m_xSlider->setRange(0, defaultWidth);
        m_xSlider->setTickInterval(defaultWidth / 10);
        
        m_ySlider->setRange(0, defaultHeight);
        m_ySlider->setTickInterval(defaultHeight / 10);
        
        m_widthSlider->setRange(Constants::MIN_SIZE, static_cast<int>(defaultWidth * 0.8));
        m_widthSlider->setTickInterval(static_cast<int>(defaultWidth * 0.8) / 10);
        
        m_heightSlider->setRange(Constants::MIN_SIZE, static_cast<int>(defaultHeight * 0.8));
        m_heightSlider->setTickInterval(static_cast<int>(defaultHeight * 0.8) / 10);
        
        if (m_statusLabel) {
            m_statusLabel->setText(QString("就绪 - 使用默认分辨率: %1x%2").arg(defaultWidth).arg(defaultHeight));
        }
        return;
    }
    
    QRect screenGeometry = primaryScreen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    
    // 检查屏幕尺寸的有效性
    if (screenWidth <= 0 || screenHeight <= 0) {
        // 如果屏幕尺寸无效，使用默认值
        screenWidth = 1920;
        screenHeight = 1080;
    }
    
    // 设置X坐标滑条范围（0到屏幕宽度）
    m_xSlider->setRange(0, screenWidth);
    m_xSlider->setTickInterval(qMax(1, screenWidth / 10)); // 确保刻度间隔至少为1
    
    // 设置Y坐标滑条范围（0到屏幕高度）
    m_ySlider->setRange(0, screenHeight);
    m_ySlider->setTickInterval(qMax(1, screenHeight / 10)); // 确保刻度间隔至少为1
    
    // 设置宽度滑条范围（最小尺寸到屏幕宽度的80%）
    int maxWidth = qMax(Constants::MIN_SIZE + 100, static_cast<int>(screenWidth * 0.8));
    m_widthSlider->setRange(Constants::MIN_SIZE, maxWidth);
    m_widthSlider->setTickInterval(qMax(1, (maxWidth - Constants::MIN_SIZE) / 10));
    
    // 设置高度滑条范围（最小尺寸到屏幕高度的80%）
    int maxHeight = qMax(Constants::MIN_SIZE + 100, static_cast<int>(screenHeight * 0.8));
    m_heightSlider->setRange(Constants::MIN_SIZE, maxHeight);
    m_heightSlider->setTickInterval(qMax(1, (maxHeight - Constants::MIN_SIZE) / 10));
    
    // 透明度滑条保持不变
    m_opacitySlider->setRange(static_cast<int>(Constants::MIN_OPACITY * 100), 
                             static_cast<int>(Constants::MAX_OPACITY * 100));
    m_opacitySlider->setTickInterval(10);
    
    // 更新状态栏显示屏幕信息
    if (m_statusLabel) {
        m_statusLabel->setText(QString("屏幕分辨率: %1x%2").arg(screenWidth).arg(screenHeight));
    }
}

void ManagementWindow::setupSliderConnections() {
    // 确保滑条已正确初始化
    if (!m_xSlider || !m_ySlider || !m_widthSlider || !m_heightSlider || !m_opacitySlider) {
        return;
    }
    
    // X坐标滑条与输入框双向绑定（需要处理范围差异）
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [this](int value) {
                if (m_xSlider) {
                    m_xSlider->blockSignals(true);
                    m_xSlider->setValue(qBound(m_xSlider->minimum(), value, m_xSlider->maximum()));
                    m_xSlider->blockSignals(false);
                }
            });
    connect(m_xSlider, &QSlider::valueChanged, 
            [this](int value) {
                if (m_xSpinBox) {
                    m_xSpinBox->blockSignals(true);
                    m_xSpinBox->setValue(value);
                    m_xSpinBox->blockSignals(false);
                }
            });
    connect(m_xSlider, &QSlider::sliderReleased, 
            this, &ManagementWindow::onInstantApplySettings);
    
    // Y坐标滑条与输入框双向绑定
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [this](int value) {
                if (m_ySlider) {
                    m_ySlider->blockSignals(true);
                    m_ySlider->setValue(qBound(m_ySlider->minimum(), value, m_ySlider->maximum()));
                    m_ySlider->blockSignals(false);
                }
            });
    connect(m_ySlider, &QSlider::valueChanged, 
            [this](int value) {
                if (m_ySpinBox) {
                    m_ySpinBox->blockSignals(true);
                    m_ySpinBox->setValue(value);
                    m_ySpinBox->blockSignals(false);
                }
            });
    connect(m_ySlider, &QSlider::sliderReleased, 
            this, &ManagementWindow::onInstantApplySettings);
    
    // 宽度滑条与输入框双向绑定
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [this](int value) {
                if (m_widthSlider) {
                    m_widthSlider->blockSignals(true);
                    m_widthSlider->setValue(qBound(m_widthSlider->minimum(), value, m_widthSlider->maximum()));
                    m_widthSlider->blockSignals(false);
                }
            });
    connect(m_widthSlider, &QSlider::valueChanged, 
            [this](int value) {
                if (m_widthSpinBox) {
                    m_widthSpinBox->blockSignals(true);
                    m_widthSpinBox->setValue(value);
                    m_widthSpinBox->blockSignals(false);
                }
            });
    connect(m_widthSlider, &QSlider::sliderReleased, 
            this, &ManagementWindow::onInstantApplySettings);
    
    // 高度滑条与输入框双向绑定
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [this](int value) {
                if (m_heightSlider) {
                    m_heightSlider->blockSignals(true);
                    m_heightSlider->setValue(qBound(m_heightSlider->minimum(), value, m_heightSlider->maximum()));
                    m_heightSlider->blockSignals(false);
                }
            });
    connect(m_heightSlider, &QSlider::valueChanged, 
            [this](int value) {
                if (m_heightSpinBox) {
                    m_heightSpinBox->blockSignals(true);
                    m_heightSpinBox->setValue(value);
                    m_heightSpinBox->blockSignals(false);
                }
            });
    connect(m_heightSlider, &QSlider::sliderReleased, 
            this, &ManagementWindow::onInstantApplySettings);
    
    // 透明度滑条与输入框双向绑定（需要转换）
    connect(m_opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            [this](double value) {
                if (m_opacitySlider) {
                    m_opacitySlider->blockSignals(true);
                    m_opacitySlider->setValue(static_cast<int>(value * 100));
                    m_opacitySlider->blockSignals(false);
                }
            });
    connect(m_opacitySlider, &QSlider::valueChanged, 
            [this](int value) {
                if (m_opacitySpinBox) {
                    m_opacitySpinBox->blockSignals(true);
                    m_opacitySpinBox->setValue(value / 100.0);
                    m_opacitySpinBox->blockSignals(false);
                }
            });
    connect(m_opacitySlider, &QSlider::sliderReleased, 
            this, &ManagementWindow::onInstantApplySettings);
}

void ManagementWindow::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore(); // 不真正关闭，只是隐藏
}

void ManagementWindow::onCreateWidget() {
    CreateWidgetDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        WidgetConfig config = dialog.getWidgetConfig();
        if (m_widgetManager->createWidget(config)) {
            refreshWidgetList();
            updateWidgetInfo();
            m_statusLabel->setText(QString("成功创建组件: %1").arg(config.name));
            
            // 询问是否立即启动
            int ret = QMessageBox::question(this, "启动组件", 
                QString("组件 '%1' 创建成功！\n是否立即启动？").arg(config.name),
                QMessageBox::Yes | QMessageBox::No);
            
            if (ret == QMessageBox::Yes) {
                m_widgetManager->startWidget(config.id);
                m_statusLabel->setText(QString("组件 '%1' 已启动").arg(config.name));
            }
        } else {
            QMessageBox::warning(this, "创建失败", "组件创建失败，请检查配置！");
            m_statusLabel->setText("组件创建失败");
        }
    }
}

void ManagementWindow::onRemoveWidget() {
    QString widgetId = getCurrentSelectedWidgetId();
    if (widgetId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要删除的组件！");
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        QMessageBox::warning(this, "错误", "选中的组件不存在！");
        return;
    }
    
    QString widgetName = widget->getConfig().name;
    int ret = QMessageBox::question(this, "确认删除", 
        QString("确定要删除组件 '%1' 吗？").arg(widgetName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (m_widgetManager->removeWidget(widgetId)) {
            refreshWidgetList();
            updateWidgetInfo();
            m_statusLabel->setText(QString("已删除组件: %1").arg(widgetName));
        } else {
            QMessageBox::warning(this, "删除失败", "组件删除失败！");
        }
    }
}

void ManagementWindow::onStartWidget() {
    QString widgetId = getCurrentSelectedWidgetId();
    if (widgetId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要启动的组件！");
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        QMessageBox::warning(this, "错误", "选中的组件不存在！");
        return;
    }
    
    if (m_widgetManager->startWidget(widgetId)) {
        m_statusLabel->setText(QString("已启动组件: %1").arg(widget->getConfig().name));
    } else {
        QMessageBox::warning(this, "启动失败", "组件启动失败！");
    }
}

void ManagementWindow::onStopWidget() {
    QString widgetId = getCurrentSelectedWidgetId();
    if (widgetId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要停止的组件！");
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        QMessageBox::warning(this, "错误", "选中的组件不存在！");
        return;
    }
    
    if (m_widgetManager->stopWidget(widgetId)) {
        m_statusLabel->setText(QString("已停止组件: %1").arg(widget->getConfig().name));
    } else {
        QMessageBox::warning(this, "停止失败", "组件停止失败！");
    }
}

void ManagementWindow::onConfigureWidget() {
    QString widgetId = getCurrentSelectedWidgetId();
    if (widgetId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要配置的组件！");
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        QMessageBox::warning(this, "错误", "选中的组件不存在！");
        return;
    }
    
    WidgetConfig config = widget->getConfig();
    QDialog* configDialog = nullptr;
    
    // 根据小组件类型创建相应的配置对话框
    switch (config.type) {
        case WidgetType::Weather: {
            configDialog = new WeatherConfigDialog(config, this);
            break;
        }
        case WidgetType::Notes:
        case WidgetType::SimpleNotes: {
            configDialog = new NotesConfigDialog(config, this);
            break;
        }
        case WidgetType::AIRanking: {
            configDialog = new AIRankingConfigDialog(config, this);
            break;
        }
        case WidgetType::Clock:
        case WidgetType::SystemInfo:
        case WidgetType::Calendar:
        case WidgetType::SystemPerformance:
        case WidgetType::Custom:
        default: {
            configDialog = new ConfigWindow(config, this);
            break;
        }
    }
    
    if (configDialog && configDialog->exec() == QDialog::Accepted) {
        // 使用统一的方式获取更新后的配置
        WidgetConfig updatedConfig;
        
        if (auto weatherDialog = qobject_cast<WeatherConfigDialog*>(configDialog)) {
            updatedConfig = weatherDialog->getUpdatedConfig();
        } else if (auto notesDialog = qobject_cast<NotesConfigDialog*>(configDialog)) {
            updatedConfig = notesDialog->getUpdatedConfig();
        } else if (auto aiDialog = qobject_cast<AIRankingConfigDialog*>(configDialog)) {
            updatedConfig = aiDialog->getUpdatedConfig();
        } else if (auto generalDialog = qobject_cast<ConfigWindow*>(configDialog)) {
            updatedConfig = generalDialog->getUpdatedConfig();
        }
        
        if (m_widgetManager->updateWidgetConfig(widgetId, updatedConfig)) {
            m_statusLabel->setText(QString("已配置组件: %1").arg(updatedConfig.name));
            refreshWidgetList();
            updateSettingsPanel();
        } else {
            QMessageBox::warning(this, "错误", "配置应用失败！");
        }
    }
    
    // 清理对话框内存
    if (configDialog) {
        configDialog->deleteLater();
    }
}

void ManagementWindow::onWidgetListSelectionChanged() {
    updateSettingsPanel();
}

void ManagementWindow::onWidgetTypeChanged(int index) {
    Q_UNUSED(index)
    // TODO: 处理Widget类型变化
}

void ManagementWindow::onApplySettings() {
    QString widgetId = getCurrentSelectedWidgetId();
    if (widgetId.isEmpty()) {
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        return;
    }
    
    try {
        WidgetConfig newConfig = getConfigFromSettings();
        newConfig.id = widgetId; // 保持原有ID
        
        // 验证自定义设置JSON格式
        if (!m_customSettingsTextEdit->toPlainText().isEmpty()) {
            QJsonParseError error;
            QJsonDocument::fromJson(m_customSettingsTextEdit->toPlainText().toUtf8(), &error);
            if (error.error != QJsonParseError::NoError) {
                QMessageBox::warning(this, "JSON格式错误", 
                    QString("自定义设置JSON格式错误：%1").arg(error.errorString()));
                return;
            }
        }
        
        if (m_widgetManager->updateWidgetConfig(widgetId, newConfig)) {
            m_applyButton->setEnabled(false);
            m_statusLabel->setText(QString("已应用设置: %1").arg(newConfig.name));
            refreshWidgetList();
        } else {
            QMessageBox::warning(this, "应用失败", "设置应用失败！");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("应用设置时发生错误: %1").arg(e.what()));
    }
}

void ManagementWindow::onResetSettings() {
    QString widgetId = getCurrentSelectedWidgetId();
    if (widgetId.isEmpty()) {
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        return;
    }
    
    int ret = QMessageBox::question(this, "重置设置", 
        "确定要重置当前组件的设置吗？",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        populateSettingsFromConfig(widget->getConfig());
        m_applyButton->setEnabled(false);
        m_statusLabel->setText("设置已重置");
    }
}

void ManagementWindow::onImportConfig() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "导入配置文件",
        QString(),
        "JSON文件 (*.json);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        if (m_widgetManager->importConfiguration(fileName)) {
            refreshWidgetList();
            updateWidgetInfo();
            m_statusLabel->setText("配置导入成功");
            QMessageBox::information(this, "成功", "配置文件导入成功！");
        } else {
            QMessageBox::warning(this, "导入失败", "配置文件导入失败，请检查文件格式！");
            m_statusLabel->setText("配置导入失败");
        }
    }
}

void ManagementWindow::onExportConfig() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出配置文件",
        QString("widget_config_%1.json").arg(QDate::currentDate().toString("yyyy-MM-dd")),
        "JSON文件 (*.json);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        if (m_widgetManager->exportConfiguration(fileName)) {
            m_statusLabel->setText("配置导出成功");
            QMessageBox::information(this, "成功", 
                QString("配置文件已导出到: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "导出失败", "配置文件导出失败！");
            m_statusLabel->setText("配置导出失败");
        }
    }
}

void ManagementWindow::updateWidgetInfo() {
    int count = m_widgetManager->getWidgetCount();
    m_widgetCountLabel->setText(QString("Widget数量: %1").arg(count));
}

void ManagementWindow::updateSettingsPanel() {
    QString widgetId = getCurrentSelectedWidgetId();
    
    if (widgetId.isEmpty()) {
        clearSettingsPanel();
        return;
    }
    
    auto widget = m_widgetManager->getWidget(widgetId);
    if (!widget) {
        clearSettingsPanel();
        return;
    }
    
    populateSettingsFromConfig(widget->getConfig());
    
    // 启用设置控件
    m_xSpinBox->setEnabled(true);
    m_ySpinBox->setEnabled(true);
    m_widthSpinBox->setEnabled(true);
    m_heightSpinBox->setEnabled(true);
    m_opacitySpinBox->setEnabled(true);
    m_updateIntervalSpinBox->setEnabled(true);
    m_windowLayerComboBox->setEnabled(true);
    m_avoidMinimizeAllCheckBox->setEnabled(true);
    m_alwaysOnTopCheckBox->setEnabled(true);
    m_alwaysOnBottomCheckBox->setEnabled(true);
    m_clickThroughCheckBox->setEnabled(true);
    m_lockedCheckBox->setEnabled(true);
    m_autoStartCheckBox->setEnabled(true);
    m_customSettingsTextEdit->setEnabled(true);
    
    // 启用滑条
    m_xSlider->setEnabled(true);
    m_ySlider->setEnabled(true);
    m_widthSlider->setEnabled(true);
    m_heightSlider->setEnabled(true);
    m_opacitySlider->setEnabled(true);
    
    m_resetButton->setEnabled(true);
    
    m_applyButton->setEnabled(false); // 重置为未修改状态
}

void ManagementWindow::clearSettingsPanel() {
    // 清空所有设置控件
    m_nameLineEdit->clear();
    m_typeComboBox->setCurrentIndex(0);
    m_xSpinBox->setValue(0);
    if(m_xSlider) m_xSlider->setValue(m_xSlider->minimum()); // 重置滑块到最小值
    m_ySpinBox->setValue(0);
    if(m_ySlider) m_ySlider->setValue(m_ySlider->minimum()); // 重置滑块到最小值
    m_widthSpinBox->setValue(Constants::MIN_SIZE); // 改为使用Constants::MIN_SIZE
    if(m_widthSlider) m_widthSlider->setValue(m_widthSlider->minimum()); // 重置滑块到最小值
    m_heightSpinBox->setValue(Constants::MIN_SIZE); // 改为使用Constants::MIN_SIZE
    if(m_heightSlider) m_heightSlider->setValue(m_heightSlider->minimum()); // 重置滑块到最小值
    m_opacitySpinBox->setValue(Constants::MAX_OPACITY); // 改为使用Constants::MAX_OPACITY
    if(m_opacitySlider) m_opacitySlider->setValue(m_opacitySlider->maximum()); // 透明度滑块通常表示不透明，所以是最大值
    m_updateIntervalSpinBox->setValue(1000);
    m_windowLayerComboBox->setCurrentIndex(0);  // 正常层级
    m_avoidMinimizeAllCheckBox->setChecked(false);
    m_alwaysOnTopCheckBox->setChecked(false);
    m_alwaysOnBottomCheckBox->setChecked(false);
    m_clickThroughCheckBox->setChecked(false);
    m_lockedCheckBox->setChecked(false);
    m_autoStartCheckBox->setChecked(false);
    m_customSettingsTextEdit->clear();
    
    // 禁用所有设置控件
    m_xSpinBox->setEnabled(false);
    m_ySpinBox->setEnabled(false);
    m_widthSpinBox->setEnabled(false);
    m_heightSpinBox->setEnabled(false);
    m_opacitySpinBox->setEnabled(false);
    m_updateIntervalSpinBox->setEnabled(false);
    m_windowLayerComboBox->setEnabled(false);
    m_avoidMinimizeAllCheckBox->setEnabled(false);
    m_alwaysOnTopCheckBox->setEnabled(false);
    m_alwaysOnBottomCheckBox->setEnabled(false);
    m_clickThroughCheckBox->setEnabled(false);
    m_lockedCheckBox->setEnabled(false);
    m_autoStartCheckBox->setEnabled(false);
    m_customSettingsTextEdit->setEnabled(false);
    
    // 禁用滑条
    if (m_xSlider) m_xSlider->setEnabled(false);
    if (m_ySlider) m_ySlider->setEnabled(false);
    if (m_widthSlider) m_widthSlider->setEnabled(false);
    if (m_heightSlider) m_heightSlider->setEnabled(false);
    if (m_opacitySlider) m_opacitySlider->setEnabled(false);
    
    m_applyButton->setEnabled(false);
    m_resetButton->setEnabled(false);
}

QString ManagementWindow::getCurrentSelectedWidgetId() const {
    if (!m_widgetListWidget || m_widgetListWidget->currentRow() < 0) {
        return QString();
    }
    
    QListWidgetItem* currentItem = m_widgetListWidget->currentItem();
    if (!currentItem) {
        return QString();
    }
    
    // 从文本中提取ID (格式: "名称 (ID)")
    QString text = currentItem->text();
    int startPos = text.lastIndexOf('(');
    int endPos = text.lastIndexOf(')');
    
    if (startPos >= 0 && endPos > startPos) {
        return text.mid(startPos + 1, endPos - startPos - 1);
    }
    
    return QString();
}

void ManagementWindow::restoreWidgetSelection(const QString& widgetId) {
    if (!m_widgetListWidget || widgetId.isEmpty()) {
        return;
    }
    
    // 查找包含指定Widget ID的列表项
    for (int i = 0; i < m_widgetListWidget->count(); ++i) {
        QListWidgetItem* item = m_widgetListWidget->item(i);
        if (!item) continue;
        
        QString itemText = item->text();
        if (itemText.contains(QString("(%1)").arg(widgetId))) {
            // 找到对应的项，设置为当前选中项
            m_widgetListWidget->blockSignals(true);
            m_widgetListWidget->setCurrentItem(item);
            m_widgetListWidget->blockSignals(false);
            
            // 手动触发设置面板更新，但不清空
            auto widget = m_widgetManager->getWidget(widgetId);
            if (widget) {
                populateSettingsFromConfig(widget->getConfig());
                
                // 确保控件处于启用状态
                m_xSpinBox->setEnabled(true);
                m_ySpinBox->setEnabled(true);
                m_widthSpinBox->setEnabled(true);
                m_heightSpinBox->setEnabled(true);
                m_opacitySpinBox->setEnabled(true);
                m_updateIntervalSpinBox->setEnabled(true);
                m_windowLayerComboBox->setEnabled(true);
                m_avoidMinimizeAllCheckBox->setEnabled(true);
                m_alwaysOnTopCheckBox->setEnabled(true);
                m_alwaysOnBottomCheckBox->setEnabled(true);
                m_clickThroughCheckBox->setEnabled(true);
                m_lockedCheckBox->setEnabled(true);
                m_autoStartCheckBox->setEnabled(true);
                m_customSettingsTextEdit->setEnabled(true);
                
                // 启用滑条
                m_xSlider->setEnabled(true);
                m_ySlider->setEnabled(true);
                m_widthSlider->setEnabled(true);
                m_heightSlider->setEnabled(true);
                m_opacitySlider->setEnabled(true);
                
                m_resetButton->setEnabled(true);
                m_applyButton->setEnabled(false); // 实时更新后，应用按钮应为不可用状态
            }
            break;
        }
    }
}

void ManagementWindow::populateSettingsFromConfig(const WidgetConfig& config) {
    // 临时断开信号连接，避免触发设置变更
    m_xSpinBox->blockSignals(true);
    m_ySpinBox->blockSignals(true);
    m_widthSpinBox->blockSignals(true);
    m_heightSpinBox->blockSignals(true);
    m_opacitySpinBox->blockSignals(true);
    m_updateIntervalSpinBox->blockSignals(true);
    m_windowLayerComboBox->blockSignals(true);
    m_avoidMinimizeAllCheckBox->blockSignals(true);
    m_alwaysOnTopCheckBox->blockSignals(true);
    m_alwaysOnBottomCheckBox->blockSignals(true);
    m_clickThroughCheckBox->blockSignals(true);
    m_lockedCheckBox->blockSignals(true);
    m_autoStartCheckBox->blockSignals(true);
    m_customSettingsTextEdit->blockSignals(true);
    
    // 同时断开滑条信号
    if (m_xSlider) m_xSlider->blockSignals(true);
    if (m_ySlider) m_ySlider->blockSignals(true);
    if (m_widthSlider) m_widthSlider->blockSignals(true);
    if (m_heightSlider) m_heightSlider->blockSignals(true);
    if (m_opacitySlider) m_opacitySlider->blockSignals(true);
    
    // 填充设置
    m_nameLineEdit->setText(config.name);
    
    // 设置类型下拉框
    for (int i = 0; i < m_typeComboBox->count(); ++i) {
        if (static_cast<WidgetType>(m_typeComboBox->itemData(i).toInt()) == config.type) {
            m_typeComboBox->setCurrentIndex(i);
            break;
        }
    }
    
    m_xSpinBox->setValue(config.position.x());
    if (m_xSlider) m_xSlider->setValue(qBound(m_xSlider->minimum(), config.position.x(), m_xSlider->maximum()));
    
    m_ySpinBox->setValue(config.position.y());
    if (m_ySlider) m_ySlider->setValue(qBound(m_ySlider->minimum(), config.position.y(), m_ySlider->maximum()));
    
    m_widthSpinBox->setValue(config.size.width());
    if (m_widthSlider) m_widthSlider->setValue(qBound(m_widthSlider->minimum(), config.size.width(), m_widthSlider->maximum()));
    
    m_heightSpinBox->setValue(config.size.height());
    if (m_heightSlider) m_heightSlider->setValue(qBound(m_heightSlider->minimum(), config.size.height(), m_heightSlider->maximum()));
    
    m_opacitySpinBox->setValue(config.opacity);
    if (m_opacitySlider) m_opacitySlider->setValue(qBound(m_opacitySlider->minimum(), static_cast<int>(config.opacity * 100), m_opacitySlider->maximum()));
    
    m_updateIntervalSpinBox->setValue(config.updateInterval);
    
    // 设置窗口层级下拉框
    if (config.alwaysOnTop) {
        m_windowLayerComboBox->setCurrentIndex(1);  // 始终置顶
    } else if (config.alwaysOnBottom) {
        m_windowLayerComboBox->setCurrentIndex(2);  // 始终置底
    } else {
        m_windowLayerComboBox->setCurrentIndex(0);  // 正常层级
    }
    
    // 设置避免最小化选项（从自定义设置中读取）
    bool avoidMinimizeAll = config.customSettings.value("avoidMinimizeAll").toBool(false);
    m_avoidMinimizeAllCheckBox->setChecked(avoidMinimizeAll);
    
    m_alwaysOnTopCheckBox->setChecked(config.alwaysOnTop);
    m_alwaysOnBottomCheckBox->setChecked(config.alwaysOnBottom);
    m_clickThroughCheckBox->setChecked(config.clickThrough);
    m_lockedCheckBox->setChecked(config.locked);
    m_autoStartCheckBox->setChecked(config.autoStart);
    
    // 设置自定义设置（JSON格式）
    if (!config.customSettings.isEmpty()) {
        QJsonDocument doc(config.customSettings);
        m_customSettingsTextEdit->setPlainText(doc.toJson(QJsonDocument::Indented));
    } else {
        m_customSettingsTextEdit->clear();
    }
    
    // 恢复信号连接
    m_xSpinBox->blockSignals(false);
    m_ySpinBox->blockSignals(false);
    m_widthSpinBox->blockSignals(false);
    m_heightSpinBox->blockSignals(false);
    m_opacitySpinBox->blockSignals(false);
    m_updateIntervalSpinBox->blockSignals(false);
    m_windowLayerComboBox->blockSignals(false);
    m_avoidMinimizeAllCheckBox->blockSignals(false);
    m_alwaysOnTopCheckBox->blockSignals(false);
    m_alwaysOnBottomCheckBox->blockSignals(false);
    m_clickThroughCheckBox->blockSignals(false);
    m_lockedCheckBox->blockSignals(false);
    m_autoStartCheckBox->blockSignals(false);
    m_customSettingsTextEdit->blockSignals(false);
    
    // 恢复滑条信号连接
    if (m_xSlider) m_xSlider->blockSignals(false);
    if (m_ySlider) m_ySlider->blockSignals(false);
    if (m_widthSlider) m_widthSlider->blockSignals(false);
    if (m_heightSlider) m_heightSlider->blockSignals(false);
    if (m_opacitySlider) m_opacitySlider->blockSignals(false);
    
    // 检查防止最小化与始终置底的冲突
    onAvoidMinimizeChanged();
}

WidgetConfig ManagementWindow::getConfigFromSettings() const {
    WidgetConfig config;
    
    config.name = m_nameLineEdit->text();
    config.type = static_cast<WidgetType>(m_typeComboBox->currentData().toInt());
    config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    config.opacity = m_opacitySpinBox->value();
    config.updateInterval = m_updateIntervalSpinBox->value();
    
    // 从窗口层级下拉框设置alwaysOnTop和alwaysOnBottom
    int layerIndex = m_windowLayerComboBox->currentIndex();
    config.alwaysOnTop = (layerIndex == 1);     // 始终置顶
    config.alwaysOnBottom = (layerIndex == 2);  // 始终置底
    
    config.clickThrough = m_clickThroughCheckBox->isChecked();
    config.locked = m_lockedCheckBox->isChecked();
    config.autoStart = m_autoStartCheckBox->isChecked();
    
    // 解析自定义设置JSON
    QString customSettingsText = m_customSettingsTextEdit->toPlainText().trimmed();
    if (!customSettingsText.isEmpty()) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(customSettingsText.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            config.customSettings = doc.object();
        }
    }
    
    // 添加避免最小化设置到自定义设置中
    config.customSettings["avoidMinimizeAll"] = m_avoidMinimizeAllCheckBox->isChecked();
    
    return config;
}

void ManagementWindow::onSettingsChanged() {
    m_applyButton->setEnabled(true);
}

void ManagementWindow::onInstantApplySettings() {
    // 即时应用配置更改（用于回车、失去焦点等场景）
    QString widgetId = getCurrentSelectedWidgetId();
    if (!widgetId.isEmpty()) {
        auto widget = m_widgetManager->getWidget(widgetId);
        if (widget) {
            try {
                WidgetConfig newConfig = getConfigFromSettings();
                newConfig.id = widgetId; // 保持原有ID
                
                // 验证自定义设置JSON格式（如果不为空）
                QString customSettingsText = m_customSettingsTextEdit->toPlainText().trimmed();
                if (!customSettingsText.isEmpty()) {
                    QJsonParseError error;
                    QJsonDocument::fromJson(customSettingsText.toUtf8(), &error);
                    if (error.error != QJsonParseError::NoError) {
                        // JSON格式错误时不应用更改
                        return;
                    }
                }
                
                // 立即应用配置更改
                if (m_widgetManager->updateWidgetConfig(widgetId, newConfig)) {
                    // 记住当前选中的Widget ID
                    QString currentSelectedId = widgetId;
                    
                    // 刷新Widget列表以显示状态变化
                    refreshWidgetList();
                    
                    // 恢复选中状态，保持在管理界面
                    restoreWidgetSelection(currentSelectedId);
                    
                    m_statusLabel->setText(QString("实时更新: %1").arg(newConfig.name));
                    // 不重置应用按钮状态，保持管理状态
                }
            } catch (const std::exception&) {
                // 发生错误时忽略实时更新
            }
        }
    }
}

void ManagementWindow::filterWidgetList(const QString& searchText, int statusFilter) {
    if (!m_widgetListWidget) return;
    
    for (int i = 0; i < m_widgetListWidget->count(); ++i) {
        QListWidgetItem* item = m_widgetListWidget->item(i);
        if (!item) continue;
        
        QString itemText = item->text();
        bool visible = true;
        
        // 文本搜索过滤
        if (!searchText.isEmpty()) {
            visible = itemText.contains(searchText, Qt::CaseInsensitive);
        }
        
        // 状态过滤
        if (visible && statusFilter >= 0) {
            QString statusText;
            switch (static_cast<WidgetStatus>(statusFilter)) {
                case WidgetStatus::Active: statusText = "运行中"; break;
                case WidgetStatus::Hidden: statusText = "已隐藏"; break;
                case WidgetStatus::Minimized: statusText = "最小化"; break;
                case WidgetStatus::Error: statusText = "错误"; break;
            }
            visible = itemText.contains(statusText);
        }
        
        item->setHidden(!visible);
    }
}

void ManagementWindow::onWidgetManuallyMoved(const QString& widgetId, const QPoint& newPosition) {
    // 检查当前选中的widget是否是正在移动的widget
    if (widgetId == getCurrentSelectedWidgetId()) {
        // 临时阻塞信号，避免循环更新
        m_xSpinBox->blockSignals(true);
        m_ySpinBox->blockSignals(true);
        if (m_xSlider) m_xSlider->blockSignals(true);
        if (m_ySlider) m_ySlider->blockSignals(true);

        m_xSpinBox->setValue(newPosition.x());
        m_ySpinBox->setValue(newPosition.y());

        if (m_xSlider) m_xSlider->setValue(qBound(m_xSlider->minimum(), newPosition.x(), m_xSlider->maximum()));
        if (m_ySlider) m_ySlider->setValue(qBound(m_ySlider->minimum(), newPosition.y(), m_ySlider->maximum()));

        // 恢复信号
        m_xSpinBox->blockSignals(false);
        m_ySpinBox->blockSignals(false);
        if (m_xSlider) m_xSlider->blockSignals(false);
        if (m_ySlider) m_ySlider->blockSignals(false);
        
        // 应用按钮可以根据需要启用，表示有未保存的更改
        // m_applyButton->setEnabled(true);
    }
}

void ManagementWindow::onAlwaysOnTopCheckChanged(bool checked) {
    if (checked) {
        // 阻塞信号避免循环触发
        m_windowLayerComboBox->blockSignals(true);
        m_alwaysOnBottomCheckBox->blockSignals(true);
        
        // 设置下拉框为置顶，清除置底复选框
        m_windowLayerComboBox->setCurrentIndex(1);  // 始终置顶
        m_alwaysOnBottomCheckBox->setChecked(false);
        
        // 恢复信号
        m_windowLayerComboBox->blockSignals(false);
        m_alwaysOnBottomCheckBox->blockSignals(false);
    } else {
        // 如果取消置顶，设置为正常层级
        m_windowLayerComboBox->blockSignals(true);
        m_windowLayerComboBox->setCurrentIndex(0);  // 正常层级
        m_windowLayerComboBox->blockSignals(false);
    }
    
    // 触发设置更改和即时应用
    onSettingsChanged();
    onInstantApplySettings();
}

void ManagementWindow::onAlwaysOnBottomCheckChanged(bool checked) {
    if (checked) {
        // 阻塞信号避免循环触发
        m_windowLayerComboBox->blockSignals(true);
        m_alwaysOnTopCheckBox->blockSignals(true);
        
        // 设置下拉框为置底，清除置顶复选框
        m_windowLayerComboBox->setCurrentIndex(2);  // 始终置底
        m_alwaysOnTopCheckBox->setChecked(false);
        
        // 恢复信号
        m_windowLayerComboBox->blockSignals(false);
        m_alwaysOnTopCheckBox->blockSignals(false);
    } else {
        // 如果取消置底，设置为正常层级
        m_windowLayerComboBox->blockSignals(true);
        m_windowLayerComboBox->setCurrentIndex(0);  // 正常层级
        m_windowLayerComboBox->blockSignals(false);
    }
    
    // 触发设置更改和即时应用
    onSettingsChanged();
    onInstantApplySettings();
}

void ManagementWindow::onWindowLayerComboChanged(int index) {
    // 阻塞复选框信号避免循环触发
    m_alwaysOnTopCheckBox->blockSignals(true);
    m_alwaysOnBottomCheckBox->blockSignals(true);
    
    // 根据下拉框选择更新复选框
    switch (index) {
        case 0:  // 正常层级
            m_alwaysOnTopCheckBox->setChecked(false);
            m_alwaysOnBottomCheckBox->setChecked(false);
            break;
        case 1:  // 始终置顶
            m_alwaysOnTopCheckBox->setChecked(true);
            m_alwaysOnBottomCheckBox->setChecked(false);
            break;
        case 2:  // 始终置底
            m_alwaysOnTopCheckBox->setChecked(false);
            m_alwaysOnBottomCheckBox->setChecked(true);
            break;
    }
    
    // 恢复复选框信号
    m_alwaysOnTopCheckBox->blockSignals(false);
    m_alwaysOnBottomCheckBox->blockSignals(false);
    
    // 检查冲突
    onAvoidMinimizeChanged();
    
    // 触发设置更改和即时应用
    onSettingsChanged();
    onInstantApplySettings();
}

void ManagementWindow::onAvoidMinimizeChanged() {
    // 检测防止最小化与始终置底的冲突
    bool avoidMinimize = m_avoidMinimizeAllCheckBox->isChecked();
    bool alwaysOnBottom = m_alwaysOnBottomCheckBox->isChecked();
    
    if (avoidMinimize && alwaysOnBottom) {
        // 显示冲突提示
        QString tooltip = "⚠️ 注意：防止最小化与始终置底同时开启时，将使用混合模式\n"
                         "• Windows系统会尽量保持窗口在底层\n"
                         "• 同时确保不会被Win+D等快捷键影响\n"
                         "• 可能会有轻微的性能开销";
        m_avoidMinimizeAllCheckBox->setToolTip(tooltip);
        m_alwaysOnBottomCheckBox->setToolTip(tooltip);
        
        // 更新状态栏提示
        if (m_statusLabel) {
            m_statusLabel->setText("混合模式：防止最小化 + 始终置底");
            m_statusLabel->setStyleSheet("color: orange;");
        }
    } else {
        // 恢复正常提示
        m_avoidMinimizeAllCheckBox->setToolTip("避免被Win+D等显示桌面快捷键影响");
        m_alwaysOnBottomCheckBox->setToolTip("");
        
        if (m_statusLabel) {
            m_statusLabel->setStyleSheet("");
        }
    }
} 