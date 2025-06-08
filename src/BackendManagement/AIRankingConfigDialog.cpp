#include "BackendManagement/AIRankingConfigDialog.h"
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QDateTime>
#include <QDebug>

AIRankingConfigDialog::AIRankingConfigDialog(const WidgetConfig& config, QWidget* parent)
    : QDialog(parent)
    , m_config(config)
    , m_hasChanges(false)
{
    setWindowTitle("AI排行榜配置 - " + config.name);
    setMinimumSize(600, 500);
    setModal(true);
    
    // 初始化颜色
    m_headerColor = QColor(config.customSettings.contains("headerColor") ? 
                          config.customSettings.value("headerColor").toString() : "#FFFFFF");
    m_textColor = QColor(config.customSettings.contains("textColor") ? 
                        config.customSettings.value("textColor").toString() : "#FFFFFF");
    m_backgroundColor = QColor(config.customSettings.contains("backgroundColor") ? 
                              config.customSettings.value("backgroundColor").toString() : "rgba(30, 30, 30, 200)");
    
    setupUI();
    loadConfigToUI();
}

void AIRankingConfigDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget;
    m_mainLayout->addWidget(m_tabWidget);
    
    setupBasicTab();
    setupDisplayTab();
    setupDataTab();
    setupAdvancedTab();
    setupButtons();
    
    // 连接基本设置信号
    connect(m_nameEdit, &QLineEdit::textChanged, this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_alwaysOnTopCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_clickThroughCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_lockedCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &AIRankingConfigDialog::onBasicSettingsChanged);
    connect(m_updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onBasicSettingsChanged);
    
    // 连接显示设置信号
    connect(m_maxDisplayCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_showProviderCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_showScoreCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_showLastUpdateCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_headerFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_modelFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_itemHeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    
    // 连接数据源和能力设置信号
    connect(m_dataSourceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIRankingConfigDialog::onDataSourceChanged);
    connect(m_capabilityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIRankingConfigDialog::onCapabilityChanged);
    connect(m_previewDataBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onPreviewDataClicked);
    
    // 连接高级设置信号
    connect(m_autoRefreshCheck, &QCheckBox::toggled, this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    connect(m_refreshIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AIRankingConfigDialog::onDisplaySettingsChanged);
    
    // 连接颜色按钮信号
    connect(m_headerColorBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onColorButtonClicked);
    connect(m_textColorBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onColorButtonClicked);
    connect(m_backgroundColorBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onColorButtonClicked);
    
    // 连接功能按钮信号
    connect(m_refreshNowBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onRefreshNowClicked);
    connect(m_applyBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onApplyClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onResetClicked);
    connect(m_okBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onOkClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &AIRankingConfigDialog::onCancelClicked);
}

void AIRankingConfigDialog::setupBasicTab() {
    m_basicTab = new QWidget;
    QGridLayout* layout = new QGridLayout(m_basicTab);
    
    // 基本信息组
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QGridLayout* basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel("名称:"), 0, 0);
    m_nameEdit = new QLineEdit;
    basicLayout->addWidget(m_nameEdit, 0, 1);
    
    layout->addWidget(basicGroup, 0, 0, 1, 2);
    
    // 位置和大小组
    QGroupBox* positionGroup = new QGroupBox("位置和大小");
    QGridLayout* positionLayout = new QGridLayout(positionGroup);
    
    positionLayout->addWidget(new QLabel("X坐标:"), 0, 0);
    m_xSpinBox = new QSpinBox;
    m_xSpinBox->setRange(-9999, 9999);
    positionLayout->addWidget(m_xSpinBox, 0, 1);
    
    positionLayout->addWidget(new QLabel("Y坐标:"), 0, 2);
    m_ySpinBox = new QSpinBox;
    m_ySpinBox->setRange(-9999, 9999);
    positionLayout->addWidget(m_ySpinBox, 0, 3);
    
    positionLayout->addWidget(new QLabel("宽度:"), 1, 0);
    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setRange(250, 2000);
    positionLayout->addWidget(m_widthSpinBox, 1, 1);
    
    positionLayout->addWidget(new QLabel("高度:"), 1, 2);
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setRange(200, 2000);
    positionLayout->addWidget(m_heightSpinBox, 1, 3);
    
    layout->addWidget(positionGroup, 1, 0, 1, 2);
    
    // 窗口属性组
    QGroupBox* windowGroup = new QGroupBox("窗口属性");
    QGridLayout* windowLayout = new QGridLayout(windowGroup);
    
    m_alwaysOnTopCheck = new QCheckBox("总是置顶");
    windowLayout->addWidget(m_alwaysOnTopCheck, 0, 0);
    
    m_clickThroughCheck = new QCheckBox("点击穿透");
    windowLayout->addWidget(m_clickThroughCheck, 0, 1);
    
    m_lockedCheck = new QCheckBox("锁定位置");
    windowLayout->addWidget(m_lockedCheck, 1, 0);
    
    windowLayout->addWidget(new QLabel("透明度:"), 2, 0);
    QHBoxLayout* opacityLayout = new QHBoxLayout;
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(10, 100);
    m_opacitySlider->setValue(100);
    opacityLayout->addWidget(m_opacitySlider);
    
    m_opacityLabel = new QLabel("100%");
    m_opacityLabel->setMinimumWidth(40);
    opacityLayout->addWidget(m_opacityLabel);
    
    windowLayout->addLayout(opacityLayout, 2, 1);
    
    windowLayout->addWidget(new QLabel("更新间隔(ms):"), 3, 0);
    m_updateIntervalSpinBox = new QSpinBox;
    m_updateIntervalSpinBox->setRange(500, 10000);
    m_updateIntervalSpinBox->setValue(1000);
    windowLayout->addWidget(m_updateIntervalSpinBox, 3, 1);
    
    layout->addWidget(windowGroup, 2, 0, 1, 2);
    
    layout->setRowStretch(3, 1);
    
    m_tabWidget->addTab(m_basicTab, "基本设置");
}

void AIRankingConfigDialog::setupDisplayTab() {
    m_displayTab = new QWidget;
    QGridLayout* layout = new QGridLayout(m_displayTab);
    
    // 显示选项组
    QGroupBox* displayGroup = new QGroupBox("显示选项");
    QGridLayout* displayLayout = new QGridLayout(displayGroup);
    
    m_showProviderCheck = new QCheckBox("显示提供商");
    displayLayout->addWidget(m_showProviderCheck, 0, 0);
    
    m_showScoreCheck = new QCheckBox("显示评分");
    displayLayout->addWidget(m_showScoreCheck, 0, 1);
    
    m_showLastUpdateCheck = new QCheckBox("显示更新时间");
    displayLayout->addWidget(m_showLastUpdateCheck, 1, 0);
    
    layout->addWidget(displayGroup, 0, 0, 1, 2);
    
    // 颜色设置组
    QGroupBox* colorGroup = new QGroupBox("颜色设置");
    QGridLayout* colorLayout = new QGridLayout(colorGroup);
    
    colorLayout->addWidget(new QLabel("标题颜色:"), 0, 0);
    m_headerColorBtn = new QPushButton;
    m_headerColorBtn->setMinimumSize(80, 30);
    colorLayout->addWidget(m_headerColorBtn, 0, 1);
    
    colorLayout->addWidget(new QLabel("文本颜色:"), 1, 0);
    m_textColorBtn = new QPushButton;
    m_textColorBtn->setMinimumSize(80, 30);
    colorLayout->addWidget(m_textColorBtn, 1, 1);
    
    colorLayout->addWidget(new QLabel("背景颜色:"), 2, 0);
    m_backgroundColorBtn = new QPushButton;
    m_backgroundColorBtn->setMinimumSize(80, 30);
    colorLayout->addWidget(m_backgroundColorBtn, 2, 1);
    
    layout->addWidget(colorGroup, 1, 0);
    
    // 字体和布局设置组
    QGroupBox* layoutGroup = new QGroupBox("字体和布局");
    QGridLayout* layoutLayout = new QGridLayout(layoutGroup);
    
    layoutLayout->addWidget(new QLabel("标题字号:"), 0, 0);
    m_headerFontSizeSpinBox = new QSpinBox;
    m_headerFontSizeSpinBox->setRange(8, 24);
    m_headerFontSizeSpinBox->setValue(12);
    layoutLayout->addWidget(m_headerFontSizeSpinBox, 0, 1);
    
    layoutLayout->addWidget(new QLabel("内容字号:"), 1, 0);
    m_modelFontSizeSpinBox = new QSpinBox;
    m_modelFontSizeSpinBox->setRange(6, 20);
    m_modelFontSizeSpinBox->setValue(10);
    layoutLayout->addWidget(m_modelFontSizeSpinBox, 1, 1);
    
    layoutLayout->addWidget(new QLabel("项目高度:"), 2, 0);
    m_itemHeightSpinBox = new QSpinBox;
    m_itemHeightSpinBox->setRange(20, 80);
    m_itemHeightSpinBox->setValue(45);
    m_itemHeightSpinBox->setSuffix(" px");
    layoutLayout->addWidget(m_itemHeightSpinBox, 2, 1);
    
    layout->addWidget(layoutGroup, 1, 1);
    
    layout->setRowStretch(2, 1);
    
    m_tabWidget->addTab(m_displayTab, "显示设置");
}

void AIRankingConfigDialog::setupDataTab() {
    m_dataTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_dataTab);
    
    // 数据源设置组
    QGroupBox* dataSourceGroup = new QGroupBox("数据源设置");
    QGridLayout* dataSourceLayout = new QGridLayout(dataSourceGroup);
    
    dataSourceLayout->addWidget(new QLabel("数据源:"), 0, 0);
    m_dataSourceComboBox = new QComboBox;
    m_dataSourceComboBox->addItems(QStringList() 
        << "ChatBotArena" << "OpenAI Evals" << "HuggingFace" 
        << "PaperswithCode" << "自定义数据源");
    dataSourceLayout->addWidget(m_dataSourceComboBox, 0, 1);
    
    m_dataSourceDescLabel = new QLabel("ChatBotArena: 基于真实用户投票的AI模型排行榜");
    m_dataSourceDescLabel->setWordWrap(true);
    m_dataSourceDescLabel->setStyleSheet("color: #666; font-size: 11px; margin: 5px;");
    dataSourceLayout->addWidget(m_dataSourceDescLabel, 1, 0, 1, 2);
    
    layout->addWidget(dataSourceGroup);
    
    // 能力指标设置组
    QGroupBox* capabilityGroup = new QGroupBox("能力指标设置");
    QGridLayout* capabilityLayout = new QGridLayout(capabilityGroup);
    
    capabilityLayout->addWidget(new QLabel("能力类型:"), 0, 0);
    m_capabilityComboBox = new QComboBox;
    m_capabilityComboBox->addItems(QStringList() 
        << "综合能力" << "推理能力" << "编程能力" 
        << "多模态能力" << "数学能力" << "语言理解" << "创意写作");
    capabilityLayout->addWidget(m_capabilityComboBox, 0, 1);
    
    m_capabilityDescLabel = new QLabel("综合能力: 基于多项任务的整体评估结果");
    m_capabilityDescLabel->setWordWrap(true);
    m_capabilityDescLabel->setStyleSheet("color: #666; font-size: 11px; margin: 5px;");
    capabilityLayout->addWidget(m_capabilityDescLabel, 1, 0, 1, 2);
    
    layout->addWidget(capabilityGroup);
    
    // 显示数量设置组
    QGroupBox* displayGroup = new QGroupBox("显示设置");
    QGridLayout* displayLayout = new QGridLayout(displayGroup);
    
    displayLayout->addWidget(new QLabel("显示前几名:"), 0, 0);
    m_maxDisplayCountSpinBox = new QSpinBox;
    m_maxDisplayCountSpinBox->setRange(1, 20);
    m_maxDisplayCountSpinBox->setValue(5);
    m_maxDisplayCountSpinBox->setSuffix(" 名");
    displayLayout->addWidget(m_maxDisplayCountSpinBox, 0, 1);
    
    QLabel* displayHelpLabel = new QLabel("可以自定义显示前n名AI模型，默认显示前5名。显示更多模型需要更大的窗口高度。");
    displayHelpLabel->setWordWrap(true);
    displayHelpLabel->setStyleSheet("color: #666; font-size: 11px; margin: 5px;");
    displayLayout->addWidget(displayHelpLabel, 1, 0, 1, 2);
    
    layout->addWidget(displayGroup);
    
    // 预览和操作组
    QGroupBox* actionGroup = new QGroupBox("数据预览");
    QVBoxLayout* actionLayout = new QVBoxLayout(actionGroup);
    
    m_previewDataBtn = new QPushButton("预览当前设置的排行榜数据");
    m_previewDataBtn->setMinimumHeight(35);
    actionLayout->addWidget(m_previewDataBtn);
    
    QLabel* previewHelpLabel = new QLabel(
        "点击预览按钮可以查看当前数据源和能力指标设置下的排行榜数据。\n"
        "不同的能力指标会显示该领域最强的AI模型排名。");
    previewHelpLabel->setWordWrap(true);
    previewHelpLabel->setStyleSheet("color: #999; font-size: 10px;");
    actionLayout->addWidget(previewHelpLabel);
    
    layout->addWidget(actionGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_dataTab, "数据源");
}

void AIRankingConfigDialog::setupAdvancedTab() {
    m_advancedTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_advancedTab);
    
    // 刷新设置组
    QGroupBox* refreshGroup = new QGroupBox("数据刷新设置");
    QGridLayout* refreshLayout = new QGridLayout(refreshGroup);
    
    m_autoRefreshCheck = new QCheckBox("自动刷新");
    refreshLayout->addWidget(m_autoRefreshCheck, 0, 0, 1, 2);
    
    refreshLayout->addWidget(new QLabel("刷新间隔:"), 1, 0);
    QHBoxLayout* intervalLayout = new QHBoxLayout;
    m_refreshIntervalSpinBox = new QSpinBox;
    m_refreshIntervalSpinBox->setRange(5, 1440);
    m_refreshIntervalSpinBox->setValue(60);
    m_refreshIntervalSpinBox->setSuffix(" 分钟");
    intervalLayout->addWidget(m_refreshIntervalSpinBox);
    intervalLayout->addStretch();
    refreshLayout->addLayout(intervalLayout, 1, 1);
    
    m_refreshNowBtn = new QPushButton("立即刷新数据");
    m_refreshNowBtn->setMinimumHeight(35);
    refreshLayout->addWidget(m_refreshNowBtn, 2, 0, 1, 2);
    
    layout->addWidget(refreshGroup);
    
    // 状态信息组
    QGroupBox* statusGroup = new QGroupBox("状态信息");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    
    m_lastUpdateLabel = new QLabel("最后更新: 未知");
    m_lastUpdateLabel->setStyleSheet("font-size: 12px; color: #666;");
    statusLayout->addWidget(m_lastUpdateLabel);
    
    QLabel* helpLabel = new QLabel(
        "注意事项：\n"
        "• 当前版本使用模拟数据展示功能\n"
        "• 实际部署时可接入真实的AI排行榜API\n"
        "• 建议刷新间隔不少于5分钟以避免频繁请求\n"
        "• 显示数量越多，所需窗口高度越大"
    );
    helpLabel->setStyleSheet("color: #999; font-size: 11px; margin-top: 10px;");
    helpLabel->setWordWrap(true);
    statusLayout->addWidget(helpLabel);
    
    layout->addWidget(statusGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_advancedTab, "高级设置");
}

void AIRankingConfigDialog::setupButtons() {
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_applyBtn = new QPushButton("应用");
    m_resetBtn = new QPushButton("重置");
    m_okBtn = new QPushButton("确定");
    m_cancelBtn = new QPushButton("取消");
    
    buttonLayout->addWidget(m_applyBtn);
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addLayout(buttonLayout);
}

void AIRankingConfigDialog::loadConfigToUI() {
    // 基本设置
    m_nameEdit->setText(m_config.name);
    m_xSpinBox->setValue(m_config.position.x());
    m_ySpinBox->setValue(m_config.position.y());
    m_widthSpinBox->setValue(m_config.size.width());
    m_heightSpinBox->setValue(m_config.size.height());
    m_alwaysOnTopCheck->setChecked(m_config.alwaysOnTop);
    m_clickThroughCheck->setChecked(m_config.clickThrough);
    m_lockedCheck->setChecked(m_config.locked);
    m_opacitySlider->setValue(static_cast<int>(m_config.opacity * 100));
    m_opacityLabel->setText(QString("%1%").arg(static_cast<int>(m_config.opacity * 100)));
    m_updateIntervalSpinBox->setValue(m_config.updateInterval);
    
    // 显示设置
    const QJsonObject& settings = m_config.customSettings;
    m_maxDisplayCountSpinBox->setValue(settings.contains("maxDisplayCount") ? 
                                      settings.value("maxDisplayCount").toInt() : 5);
    m_showProviderCheck->setChecked(settings.contains("showProvider") ? 
                                   settings.value("showProvider").toBool() : true);
    m_showScoreCheck->setChecked(settings.contains("showScore") ? 
                                settings.value("showScore").toBool() : true);
    m_showLastUpdateCheck->setChecked(settings.contains("showLastUpdate") ? 
                                     settings.value("showLastUpdate").toBool() : true);
    m_headerFontSizeSpinBox->setValue(settings.contains("headerFontSize") ? 
                                     settings.value("headerFontSize").toInt() : 12);
    m_modelFontSizeSpinBox->setValue(settings.contains("modelFontSize") ? 
                                    settings.value("modelFontSize").toInt() : 10);
    m_itemHeightSpinBox->setValue(settings.contains("itemHeight") ? 
                                 settings.value("itemHeight").toInt() : 45);
    
    // 数据源和能力设置
    QString dataSource = settings.contains("dataSource") ? 
                        settings.value("dataSource").toString() : "ChatBotArena";
    int dataSourceIndex = m_dataSourceComboBox->findText(dataSource);
    if (dataSourceIndex >= 0) {
        m_dataSourceComboBox->setCurrentIndex(dataSourceIndex);
    }
    
    QString capability = settings.contains("capability") ? 
                        settings.value("capability").toString() : "综合能力";
    int capabilityIndex = m_capabilityComboBox->findText(capability);
    if (capabilityIndex >= 0) {
        m_capabilityComboBox->setCurrentIndex(capabilityIndex);
    }
    
    // 更新描述
    updateDataSourceDescription();
    updateCapabilityDescription();
    
    // 高级设置
    m_autoRefreshCheck->setChecked(settings.contains("autoRefresh") ? 
                                  settings.value("autoRefresh").toBool() : true);
    m_refreshIntervalSpinBox->setValue(settings.contains("refreshInterval") ? 
                                      settings.value("refreshInterval").toInt() : 60);
    
    // 更新颜色按钮
    updateColorButton(m_headerColorBtn, m_headerColor);
    updateColorButton(m_textColorBtn, m_textColor);
    updateColorButton(m_backgroundColorBtn, m_backgroundColor);
    
    // 更新状态信息
    QString lastUpdate = "未知";
    if (settings.contains("lastUpdateTime")) {
        QDateTime updateTime = QDateTime::fromString(settings.value("lastUpdateTime").toString(), Qt::ISODate);
        if (updateTime.isValid()) {
            lastUpdate = updateTime.toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    m_lastUpdateLabel->setText("最后更新: " + lastUpdate);
    
    // 连接透明度滑块信号
    connect(m_opacitySlider, &QSlider::valueChanged, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onBasicSettingsChanged();
    });
}

void AIRankingConfigDialog::saveUIToConfig() {
    // 基本设置
    m_config.name = m_nameEdit->text();
    m_config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    m_config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    m_config.alwaysOnTop = m_alwaysOnTopCheck->isChecked();
    m_config.clickThrough = m_clickThroughCheck->isChecked();
    m_config.locked = m_lockedCheck->isChecked();
    m_config.opacity = m_opacitySlider->value() / 100.0;
    m_config.updateInterval = m_updateIntervalSpinBox->value();
    
    // 显示设置
    m_config.customSettings["maxDisplayCount"] = m_maxDisplayCountSpinBox->value();
    m_config.customSettings["showProvider"] = m_showProviderCheck->isChecked();
    m_config.customSettings["showScore"] = m_showScoreCheck->isChecked();
    m_config.customSettings["showLastUpdate"] = m_showLastUpdateCheck->isChecked();
    m_config.customSettings["headerColor"] = m_headerColor.name();
    m_config.customSettings["textColor"] = m_textColor.name();
    m_config.customSettings["backgroundColor"] = m_backgroundColor.name(QColor::HexArgb);
    m_config.customSettings["headerFontSize"] = m_headerFontSizeSpinBox->value();
    m_config.customSettings["modelFontSize"] = m_modelFontSizeSpinBox->value();
    m_config.customSettings["itemHeight"] = m_itemHeightSpinBox->value();
    
    // 数据源和能力设置
    m_config.customSettings["dataSource"] = m_dataSourceComboBox->currentText();
    m_config.customSettings["capability"] = m_capabilityComboBox->currentText();
    
    // 高级设置
    m_config.customSettings["autoRefresh"] = m_autoRefreshCheck->isChecked();
    m_config.customSettings["refreshInterval"] = m_refreshIntervalSpinBox->value();
}

void AIRankingConfigDialog::onBasicSettingsChanged() {
    m_hasChanges = true;
}

void AIRankingConfigDialog::onDisplaySettingsChanged() {
    m_hasChanges = true;
}

void AIRankingConfigDialog::onColorButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QColor currentColor = getColorFromButton(button);
    QColor newColor = QColorDialog::getColor(currentColor, this, "选择颜色");
    
    if (newColor.isValid()) {
        updateColorButton(button, newColor);
        
        if (button == m_headerColorBtn) {
            m_headerColor = newColor;
        } else if (button == m_textColorBtn) {
            m_textColor = newColor;
        } else if (button == m_backgroundColorBtn) {
            m_backgroundColor = newColor;
        }
        
        onDisplaySettingsChanged();
    }
}

void AIRankingConfigDialog::onRefreshNowClicked() {
    // 更新最后更新时间
    QDateTime now = QDateTime::currentDateTime();
    m_lastUpdateLabel->setText("最后更新: " + now.toString("yyyy-MM-dd hh:mm:ss"));
    
    QMessageBox::information(this, "刷新", "数据刷新请求已发送！\n\n注意：当前版本使用模拟数据。");
}

void AIRankingConfigDialog::onApplyClicked() {
    saveUIToConfig();
    m_hasChanges = false;
    QMessageBox::information(this, "成功", "AI排行榜配置已应用");
}

void AIRankingConfigDialog::onResetClicked() {
    int ret = QMessageBox::question(
        this,
        "确认重置",
        "确定要重置所有设置到默认值吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        // 重置为默认配置
        WidgetConfig defaultConfig;
        defaultConfig.id = m_config.id;
        defaultConfig.type = m_config.type;
        defaultConfig.name = "AI排行榜";
        defaultConfig.size = QSize(400, 300);
        
        // 设置默认自定义配置
        defaultConfig.customSettings["maxDisplayCount"] = 5;
        defaultConfig.customSettings["showProvider"] = true;
        defaultConfig.customSettings["showScore"] = true;
        defaultConfig.customSettings["showLastUpdate"] = true;
        defaultConfig.customSettings["autoRefresh"] = true;
        defaultConfig.customSettings["refreshInterval"] = 60;
        defaultConfig.customSettings["headerColor"] = "#FFFFFF";
        defaultConfig.customSettings["textColor"] = "#FFFFFF";
        defaultConfig.customSettings["backgroundColor"] = "rgba(30, 30, 30, 200)";
        defaultConfig.customSettings["headerFontSize"] = 12;
        defaultConfig.customSettings["modelFontSize"] = 10;
        defaultConfig.customSettings["itemHeight"] = 45;
        
        m_config = defaultConfig;
        
        // 重新初始化颜色
        m_headerColor = QColor("#FFFFFF");
        m_textColor = QColor("#FFFFFF");
        m_backgroundColor = QColor("rgba(30, 30, 30, 200)");
        
        loadConfigToUI();
        m_hasChanges = true;
    }
}

void AIRankingConfigDialog::onOkClicked() {
    saveUIToConfig();
    accept();
}

void AIRankingConfigDialog::onCancelClicked() {
    if (m_hasChanges) {
        int ret = QMessageBox::question(
            this,
            "确认取消",
            "有未保存的更改，确定要取消吗？",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (ret == QMessageBox::Yes) {
            reject();
        }
    } else {
        reject();
    }
}

void AIRankingConfigDialog::updateColorButton(QPushButton* button, const QColor& color) {
    QString styleSheet = QString(
        "QPushButton { background-color: %1; border: 1px solid #ccc; color: %2; }"
        "QPushButton:hover { border: 2px solid #999; }"
    ).arg(color.name()).arg(color.lightnessF() > 0.5 ? "#000" : "#FFF");
    
    button->setStyleSheet(styleSheet);
    button->setText(color.name());
}

QColor AIRankingConfigDialog::getColorFromButton(QPushButton* button) const {
    if (button == m_headerColorBtn) return m_headerColor;
    if (button == m_textColorBtn) return m_textColor;
    if (button == m_backgroundColorBtn) return m_backgroundColor;
    return QColor();
}

void AIRankingConfigDialog::onDataSourceChanged() {
    updateDataSourceDescription();
    onDisplaySettingsChanged();
}

void AIRankingConfigDialog::onCapabilityChanged() {
    updateCapabilityDescription();
    onDisplaySettingsChanged();
}

void AIRankingConfigDialog::onPreviewDataClicked() {
    QString dataSource = m_dataSourceComboBox->currentText();
    QString capability = m_capabilityComboBox->currentText();
    
    QString previewText = QString(
        "数据源: %1\n"
        "能力指标: %2\n\n"
        "预览前5名AI模型:\n"
    ).arg(dataSource, capability);
    
    // 根据能力类型提供预览数据
    if (capability == "推理能力") {
        previewText += "1. GPT-4 (OpenAI) - 96.2分\n"
                      "2. Claude-3 Opus (Anthropic) - 95.8分\n"
                      "3. Gemini Ultra (Google) - 94.5分\n"
                      "4. Claude-3.5 Sonnet (Anthropic) - 93.9分\n"
                      "5. GPT-4 Turbo (OpenAI) - 93.2分";
    } else if (capability == "编程能力") {
        previewText += "1. GPT-4 (OpenAI) - 97.5分\n"
                      "2. Claude-3.5 Sonnet (Anthropic) - 96.8分\n"
                      "3. Codex (OpenAI) - 95.2分\n"
                      "4. Claude-3 Opus (Anthropic) - 94.7分\n"
                      "5. Gemini Pro (Google) - 93.3分";
    } else if (capability == "多模态能力") {
        previewText += "1. GPT-4V (OpenAI) - 98.1分\n"
                      "2. Gemini Ultra (Google) - 96.5分\n"
                      "3. Claude-3 Opus (Anthropic) - 95.3分\n"
                      "4. Gemini Pro Vision (Google) - 93.8分\n"
                      "5. LLaVA-1.5 (LMSys) - 91.2分";
    } else {
        previewText += "1. GPT-4 Turbo (OpenAI) - 96.3分\n"
                      "2. Claude-3 Opus (Anthropic) - 95.8分\n"
                      "3. Gemini Ultra (Google) - 94.2分\n"
                      "4. Claude-3.5 Sonnet (Anthropic) - 93.5分\n"
                      "5. GPT-4 (OpenAI) - 92.8分";
    }
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("排行榜数据预览");
    msgBox.setText(previewText);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void AIRankingConfigDialog::updateDataSourceDescription() {
    QString dataSource = m_dataSourceComboBox->currentText();
    QString description;
    
    if (dataSource == "ChatBotArena") {
        description = "ChatBotArena: 基于真实用户投票的AI模型排行榜，数据来源于用户对话体验";
    } else if (dataSource == "OpenAI Evals") {
        description = "OpenAI Evals: OpenAI官方评估框架，提供标准化的模型能力测试";
    } else if (dataSource == "HuggingFace") {
        description = "HuggingFace: 开源AI社区排行榜，包含大量开源和商业模型评测";
    } else if (dataSource == "PaperswithCode") {
        description = "PaperswithCode: 学术论文驱动的模型评测，基于最新研究成果";
    } else {
        description = "自定义数据源: 可配置的第三方数据源，支持自定义API接入";
    }
    
    m_dataSourceDescLabel->setText(description);
}

void AIRankingConfigDialog::updateCapabilityDescription() {
    QString capability = m_capabilityComboBox->currentText();
    QString description;
    
    if (capability == "综合能力") {
        description = "综合能力: 基于多项任务的整体评估结果，包含语言理解、推理、创作等";
    } else if (capability == "推理能力") {
        description = "推理能力: 逻辑推理、因果关系理解、复杂问题解决能力评估";
    } else if (capability == "编程能力") {
        description = "编程能力: 代码生成、调试、算法实现、软件工程相关任务评估";
    } else if (capability == "多模态能力") {
        description = "多模态能力: 图像理解、视觉问答、图文结合等跨模态任务评估";
    } else if (capability == "数学能力") {
        description = "数学能力: 数学问题求解、定理证明、计算推理等数学相关任务";
    } else if (capability == "语言理解") {
        description = "语言理解: 自然语言理解、语法分析、语义理解等语言任务";
    } else if (capability == "创意写作") {
        description = "创意写作: 创意内容生成、文学创作、风格模仿等创作任务";
    }
    
    m_capabilityDescLabel->setText(description);
} 