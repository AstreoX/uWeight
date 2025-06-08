#include "BackendManagement/ConfigWindow.h"
#include "BackendManagement/ThemeSettingsDialog.h"
#include "Utils/ThemeResourceManager.h"
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

ConfigWindow::ConfigWindow(const WidgetConfig& config, QWidget* parent)
    : QDialog(parent)
    , m_config(config)
    , m_hasChanges(false)
{
    setWindowTitle("小组件配置 - " + config.name);
    setMinimumSize(600, 500);
    setModal(true);
    
    // 初始化颜色和字体
    m_timeColor = QColor(config.customSettings.contains("timeColor") ? 
                        config.customSettings.value("timeColor").toString() : "#FFFFFF");
    m_dateColor = QColor(config.customSettings.contains("dateColor") ? 
                        config.customSettings.value("dateColor").toString() : "#CCCCCC");
    m_backgroundColor = QColor(config.customSettings.contains("backgroundColor") ? 
                              config.customSettings.value("backgroundColor").toString() : "#000000AA");
    
    int timeFontSize = config.customSettings.contains("timeFontSize") ? 
                      config.customSettings.value("timeFontSize").toInt() : 14;
    int dateFontSize = config.customSettings.contains("dateFontSize") ? 
                      config.customSettings.value("dateFontSize").toInt() : 10;
    m_timeFont = QFont("Arial", timeFontSize, QFont::Bold);
    m_dateFont = QFont("Arial", dateFontSize);
    
    setupUI();
    loadConfigToUI();
}

void ConfigWindow::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget;
    m_mainLayout->addWidget(m_tabWidget);
    
    setupBasicTab();
    setupDisplayTab();
    setupThemeTab();
    setupButtons();
    
    // 连接信号
    connect(m_nameEdit, &QLineEdit::textChanged, this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_alwaysOnTopCheck, &QCheckBox::toggled, this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_clickThroughCheck, &QCheckBox::toggled, this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_lockedCheck, &QCheckBox::toggled, this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &ConfigWindow::onBasicSettingsChanged);
    connect(m_updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onBasicSettingsChanged);
    
    connect(m_showDateCheck, &QCheckBox::toggled, this, &ConfigWindow::onDisplaySettingsChanged);
    connect(m_show24HourCheck, &QCheckBox::toggled, this, &ConfigWindow::onDisplaySettingsChanged);
    connect(m_showSecondsCheck, &QCheckBox::toggled, this, &ConfigWindow::onDisplaySettingsChanged);
    connect(m_timeFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onDisplaySettingsChanged);
    connect(m_dateFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ConfigWindow::onDisplaySettingsChanged);
    
    connect(m_timeColorBtn, &QPushButton::clicked, this, &ConfigWindow::onColorButtonClicked);
    connect(m_dateColorBtn, &QPushButton::clicked, this, &ConfigWindow::onColorButtonClicked);
    connect(m_backgroundColorBtn, &QPushButton::clicked, this, &ConfigWindow::onColorButtonClicked);
    connect(m_timeFontBtn, &QPushButton::clicked, this, &ConfigWindow::onFontButtonClicked);
    connect(m_dateFontBtn, &QPushButton::clicked, this, &ConfigWindow::onFontButtonClicked);
    
    connect(m_themeSettingsBtn, &QPushButton::clicked, this, &ConfigWindow::onThemeSettingsClicked);
    
    connect(m_applyBtn, &QPushButton::clicked, this, &ConfigWindow::onApplyClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &ConfigWindow::onResetClicked);
    connect(m_okBtn, &QPushButton::clicked, this, &ConfigWindow::onOkClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ConfigWindow::onCancelClicked);
}

void ConfigWindow::setupBasicTab() {
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
    m_widthSpinBox->setRange(50, 2000);
    positionLayout->addWidget(m_widthSpinBox, 1, 1);
    
    positionLayout->addWidget(new QLabel("高度:"), 1, 2);
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setRange(50, 2000);
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
    m_updateIntervalSpinBox->setRange(100, 10000);
    m_updateIntervalSpinBox->setValue(1000);
    windowLayout->addWidget(m_updateIntervalSpinBox, 3, 1);
    
    layout->addWidget(windowGroup, 2, 0, 1, 2);
    
    layout->setRowStretch(3, 1);
    
    m_tabWidget->addTab(m_basicTab, "基本设置");
}

void ConfigWindow::setupDisplayTab() {
    m_displayTab = new QWidget;
    QGridLayout* layout = new QGridLayout(m_displayTab);
    
    // 显示选项组
    QGroupBox* displayGroup = new QGroupBox("显示选项");
    QGridLayout* displayLayout = new QGridLayout(displayGroup);
    
    m_showDateCheck = new QCheckBox("显示日期");
    displayLayout->addWidget(m_showDateCheck, 0, 0);
    
    m_show24HourCheck = new QCheckBox("24小时制");
    displayLayout->addWidget(m_show24HourCheck, 0, 1);
    
    m_showSecondsCheck = new QCheckBox("显示秒数");
    displayLayout->addWidget(m_showSecondsCheck, 1, 0);
    
    layout->addWidget(displayGroup, 0, 0, 1, 2);
    
    // 颜色设置组
    QGroupBox* colorGroup = new QGroupBox("颜色设置");
    QGridLayout* colorLayout = new QGridLayout(colorGroup);
    
    colorLayout->addWidget(new QLabel("时间颜色:"), 0, 0);
    m_timeColorBtn = new QPushButton;
    m_timeColorBtn->setMinimumSize(80, 30);
    colorLayout->addWidget(m_timeColorBtn, 0, 1);
    
    colorLayout->addWidget(new QLabel("日期颜色:"), 1, 0);
    m_dateColorBtn = new QPushButton;
    m_dateColorBtn->setMinimumSize(80, 30);
    colorLayout->addWidget(m_dateColorBtn, 1, 1);
    
    colorLayout->addWidget(new QLabel("背景颜色:"), 2, 0);
    m_backgroundColorBtn = new QPushButton;
    m_backgroundColorBtn->setMinimumSize(80, 30);
    colorLayout->addWidget(m_backgroundColorBtn, 2, 1);
    
    layout->addWidget(colorGroup, 1, 0);
    
    // 字体设置组
    QGroupBox* fontGroup = new QGroupBox("字体设置");
    QGridLayout* fontLayout = new QGridLayout(fontGroup);
    
    fontLayout->addWidget(new QLabel("时间字体:"), 0, 0);
    m_timeFontBtn = new QPushButton;
    m_timeFontBtn->setMinimumSize(120, 30);
    fontLayout->addWidget(m_timeFontBtn, 0, 1);
    
    fontLayout->addWidget(new QLabel("时间字号:"), 0, 2);
    m_timeFontSizeSpinBox = new QSpinBox;
    m_timeFontSizeSpinBox->setRange(8, 72);
    fontLayout->addWidget(m_timeFontSizeSpinBox, 0, 3);
    
    fontLayout->addWidget(new QLabel("日期字体:"), 1, 0);
    m_dateFontBtn = new QPushButton;
    m_dateFontBtn->setMinimumSize(120, 30);
    fontLayout->addWidget(m_dateFontBtn, 1, 1);
    
    fontLayout->addWidget(new QLabel("日期字号:"), 1, 2);
    m_dateFontSizeSpinBox = new QSpinBox;
    m_dateFontSizeSpinBox->setRange(8, 72);
    fontLayout->addWidget(m_dateFontSizeSpinBox, 1, 3);
    
    layout->addWidget(fontGroup, 1, 1);
    
    layout->setRowStretch(2, 1);
    
    m_tabWidget->addTab(m_displayTab, "显示设置");
}

void ConfigWindow::setupThemeTab() {
    m_themeTab = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_themeTab);
    
    // 当前主题信息组
    QGroupBox* currentThemeGroup = new QGroupBox("当前主题");
    QVBoxLayout* currentThemeLayout = new QVBoxLayout(currentThemeGroup);
    
    m_currentThemeLabel = new QLabel("当前主题: 未设置");
    m_currentThemeLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    currentThemeLayout->addWidget(m_currentThemeLabel);
    
    m_themePreviewLabel = new QLabel;
    m_themePreviewLabel->setMinimumSize(300, 150);
    m_themePreviewLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");
    m_themePreviewLabel->setAlignment(Qt::AlignCenter);
    m_themePreviewLabel->setText("主题预览");
    currentThemeLayout->addWidget(m_themePreviewLabel);
    
    layout->addWidget(currentThemeGroup);
    
    // 主题管理组
    QGroupBox* themeManagementGroup = new QGroupBox("主题管理");
    QVBoxLayout* themeManagementLayout = new QVBoxLayout(themeManagementGroup);
    
    m_themeSettingsBtn = new QPushButton("打开主题设置");
    m_themeSettingsBtn->setMinimumHeight(40);
    m_themeSettingsBtn->setStyleSheet("font-size: 14px; font-weight: bold;");
    themeManagementLayout->addWidget(m_themeSettingsBtn);
    
    QLabel* themeHelpLabel = new QLabel(
        "在主题设置中，您可以：\n"
        "• 选择预设主题\n"
        "• 导入自定义背景图片\n"
        "• 调整图片缩放模式和透明度\n"
        "• 实时预览主题效果"
    );
    themeHelpLabel->setStyleSheet("color: #666; font-size: 11px;");
    themeManagementLayout->addWidget(themeHelpLabel);
    
    layout->addWidget(themeManagementGroup);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_themeTab, "主题设置");
}

void ConfigWindow::setupButtons() {
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

void ConfigWindow::loadConfigToUI() {
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
    m_showDateCheck->setChecked(settings.contains("showDate") ? 
                               settings.value("showDate").toBool() : true);
    m_show24HourCheck->setChecked(settings.contains("show24Hour") ? 
                                 settings.value("show24Hour").toBool() : true);
    m_showSecondsCheck->setChecked(settings.contains("showSeconds") ? 
                                  settings.value("showSeconds").toBool() : true);
    m_timeFontSizeSpinBox->setValue(settings.contains("timeFontSize") ? 
                                   settings.value("timeFontSize").toInt() : 14);
    m_dateFontSizeSpinBox->setValue(settings.contains("dateFontSize") ? 
                                   settings.value("dateFontSize").toInt() : 10);
    
    // 更新颜色按钮
    updateColorButton(m_timeColorBtn, m_timeColor);
    updateColorButton(m_dateColorBtn, m_dateColor);
    updateColorButton(m_backgroundColorBtn, m_backgroundColor);
    
    // 更新字体按钮
    updateFontButton(m_timeFontBtn, m_timeFont);
    updateFontButton(m_dateFontBtn, m_dateFont);
    
    // 主题信息
    QString currentTheme = settings.contains("currentTheme") ? 
                          settings.value("currentTheme").toString() : "未设置";
    m_currentThemeLabel->setText("当前主题: " + currentTheme);
    
    // 连接透明度滑块信号
    connect(m_opacitySlider, &QSlider::valueChanged, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onBasicSettingsChanged();
    });
}

void ConfigWindow::saveUIToConfig() {
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
    m_config.customSettings["showDate"] = m_showDateCheck->isChecked();
    m_config.customSettings["show24Hour"] = m_show24HourCheck->isChecked();
    m_config.customSettings["showSeconds"] = m_showSecondsCheck->isChecked();
    m_config.customSettings["timeColor"] = m_timeColor.name();
    m_config.customSettings["dateColor"] = m_dateColor.name();
    m_config.customSettings["backgroundColor"] = m_backgroundColor.name(QColor::HexArgb);
    m_config.customSettings["timeFontSize"] = m_timeFontSizeSpinBox->value();
    m_config.customSettings["dateFontSize"] = m_dateFontSizeSpinBox->value();
}

void ConfigWindow::onBasicSettingsChanged() {
    m_hasChanges = true;
}

void ConfigWindow::onDisplaySettingsChanged() {
    m_hasChanges = true;
}

void ConfigWindow::onThemeSettingsClicked() {
    ThemeSettingsDialog dialog(m_config, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_config = dialog.getUpdatedConfig();
        
        // 更新主题信息显示
        QString currentTheme = m_config.customSettings.contains("currentTheme") ? 
                              m_config.customSettings.value("currentTheme").toString() : "未设置";
        m_currentThemeLabel->setText("当前主题: " + currentTheme);
        
        m_hasChanges = true;
        QMessageBox::information(this, "成功", "主题设置已更新");
    }
}

void ConfigWindow::onColorButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QColor currentColor = getColorFromButton(button);
    QColor newColor = QColorDialog::getColor(currentColor, this, "选择颜色");
    
    if (newColor.isValid()) {
        updateColorButton(button, newColor);
        
        if (button == m_timeColorBtn) {
            m_timeColor = newColor;
        } else if (button == m_dateColorBtn) {
            m_dateColor = newColor;
        } else if (button == m_backgroundColorBtn) {
            m_backgroundColor = newColor;
        }
        
        onDisplaySettingsChanged();
    }
}

void ConfigWindow::onFontButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QFont currentFont = getFontFromButton(button);
    bool ok;
    QFont newFont = QFontDialog::getFont(&ok, currentFont, this, "选择字体");
    
    if (ok) {
        updateFontButton(button, newFont);
        
        if (button == m_timeFontBtn) {
            m_timeFont = newFont;
            m_timeFontSizeSpinBox->setValue(newFont.pointSize());
        } else if (button == m_dateFontBtn) {
            m_dateFont = newFont;
            m_dateFontSizeSpinBox->setValue(newFont.pointSize());
        }
        
        onDisplaySettingsChanged();
    }
}

void ConfigWindow::onApplyClicked() {
    saveUIToConfig();
    m_hasChanges = false;
    QMessageBox::information(this, "成功", "配置已应用");
}

void ConfigWindow::onResetClicked() {
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
        defaultConfig.name = m_config.name;
        
        m_config = defaultConfig;
        loadConfigToUI();
        m_hasChanges = true;
    }
}

void ConfigWindow::onOkClicked() {
    saveUIToConfig();
    accept();
}

void ConfigWindow::onCancelClicked() {
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

void ConfigWindow::updateColorButton(QPushButton* button, const QColor& color) {
    QString styleSheet = QString(
        "QPushButton { background-color: %1; border: 1px solid #ccc; }"
        "QPushButton:hover { border: 2px solid #999; }"
    ).arg(color.name());
    
    button->setStyleSheet(styleSheet);
    button->setText(color.name());
}

void ConfigWindow::updateFontButton(QPushButton* button, const QFont& font) {
    button->setText(QString("%1, %2pt").arg(font.family()).arg(font.pointSize()));
    button->setFont(font);
}

QColor ConfigWindow::getColorFromButton(QPushButton* button) const {
    if (button == m_timeColorBtn) return m_timeColor;
    if (button == m_dateColorBtn) return m_dateColor;
    if (button == m_backgroundColorBtn) return m_backgroundColor;
    return QColor();
}

QFont ConfigWindow::getFontFromButton(QPushButton* button) const {
    if (button == m_timeFontBtn) return m_timeFont;
    if (button == m_dateFontBtn) return m_dateFont;
    return QFont();
} 