#include "BackendManagement/NotesConfigDialog.h"
#include <QGridLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>

NotesConfigDialog::NotesConfigDialog(const WidgetConfig& config, QWidget* parent)
    : QDialog(parent)
    , m_config(config)
    , m_hasChanges(false)
    , m_widgetBackgroundColor(240, 240, 240)
    , m_borderColor(200, 200, 200)
    , m_defaultTextColor(0, 0, 0)
    , m_defaultBackgroundColor(255, 255, 255)
    , m_defaultFont("Arial", 12)
{
    setWindowTitle("便签组件配置");
    setModal(true);
    resize(500, 600);
    
    setupUI();
    loadConfigToUI();
}

void NotesConfigDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡
    m_tabWidget = new QTabWidget(this);
    
    setupBasicTab();
    setupNotesTab();
    setupAppearanceTab();
    setupButtons();
    
    m_mainLayout->addWidget(m_tabWidget);
}

void NotesConfigDialog::setupBasicTab() {
    m_basicTab = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(m_basicTab);
    
    // 基本信息分组
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QGridLayout* basicGrid = new QGridLayout(basicGroup);
    
    // 组件名称
    basicGrid->addWidget(new QLabel("组件名称:"), 0, 0);
    m_nameEdit = new QLineEdit();
    basicGrid->addWidget(m_nameEdit, 0, 1);
    
    // 位置设置
    QGroupBox* positionGroup = new QGroupBox("位置设置");
    QGridLayout* positionGrid = new QGridLayout(positionGroup);
    
    positionGrid->addWidget(new QLabel("X坐标:"), 0, 0);
    m_xSpinBox = new QSpinBox();
    m_xSpinBox->setRange(0, 9999);
    positionGrid->addWidget(m_xSpinBox, 0, 1);
    
    positionGrid->addWidget(new QLabel("Y坐标:"), 0, 2);
    m_ySpinBox = new QSpinBox();
    m_ySpinBox->setRange(0, 9999);
    positionGrid->addWidget(m_ySpinBox, 0, 3);
    
    positionGrid->addWidget(new QLabel("宽度:"), 1, 0);
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setRange(300, 2000);
    positionGrid->addWidget(m_widthSpinBox, 1, 1);
    
    positionGrid->addWidget(new QLabel("高度:"), 1, 2);
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setRange(200, 2000);
    positionGrid->addWidget(m_heightSpinBox, 1, 3);
    
    // 窗口选项
    QGroupBox* windowGroup = new QGroupBox("窗口选项");
    QVBoxLayout* windowLayout = new QVBoxLayout(windowGroup);
    
    m_alwaysOnTopCheck = new QCheckBox("始终置顶");
    m_clickThroughCheck = new QCheckBox("点击穿透");
    m_lockedCheck = new QCheckBox("锁定位置");
    
    windowLayout->addWidget(m_alwaysOnTopCheck);
    windowLayout->addWidget(m_clickThroughCheck);
    windowLayout->addWidget(m_lockedCheck);
    
    // 透明度设置
    QHBoxLayout* opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(new QLabel("透明度:"));
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(10, 100);
    m_opacitySlider->setValue(100);
    opacityLayout->addWidget(m_opacitySlider);
    m_opacityLabel = new QLabel("100%");
    opacityLayout->addWidget(m_opacityLabel);
    
    windowLayout->addLayout(opacityLayout);
    
    basicLayout->addWidget(basicGroup);
    basicLayout->addWidget(positionGroup);
    basicLayout->addWidget(windowGroup);
    basicLayout->addStretch();
    
    m_tabWidget->addTab(m_basicTab, "基本设置");
    
    // 连接信号
    connect(m_nameEdit, &QLineEdit::textChanged, this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_alwaysOnTopCheck, &QCheckBox::toggled, this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_clickThroughCheck, &QCheckBox::toggled, this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_lockedCheck, &QCheckBox::toggled, this, &NotesConfigDialog::onBasicSettingsChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onBasicSettingsChanged();
    });
}

void NotesConfigDialog::setupNotesTab() {
    m_notesTab = new QWidget();
    QVBoxLayout* notesLayout = new QVBoxLayout(m_notesTab);
    
    // 自动保存设置
    QGroupBox* autoSaveGroup = new QGroupBox("自动保存设置");
    QVBoxLayout* autoSaveLayout = new QVBoxLayout(autoSaveGroup);
    
    m_autoSaveCheck = new QCheckBox("启用自动保存");
    autoSaveLayout->addWidget(m_autoSaveCheck);
    
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel("自动保存间隔(秒):"));
    m_autoSaveIntervalSpinBox = new QSpinBox();
    m_autoSaveIntervalSpinBox->setRange(5, 300);
    m_autoSaveIntervalSpinBox->setValue(30);
    intervalLayout->addWidget(m_autoSaveIntervalSpinBox);
    intervalLayout->addStretch();
    autoSaveLayout->addLayout(intervalLayout);
    
    // 存储设置
    QGroupBox* storageGroup = new QGroupBox("存储设置");
    QVBoxLayout* storageLayout = new QVBoxLayout(storageGroup);
    
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(new QLabel("便签文件路径:"));
    m_notesFilePathEdit = new QLineEdit();
    m_notesFilePathEdit->setReadOnly(true);
    pathLayout->addWidget(m_notesFilePathEdit);
    m_browsePathBtn = new QPushButton("浏览");
    pathLayout->addWidget(m_browsePathBtn);
    storageLayout->addLayout(pathLayout);
    
    QHBoxLayout* maxNotesLayout = new QHBoxLayout();
    maxNotesLayout->addWidget(new QLabel("最大便签数量:"));
    m_maxNotesSpinBox = new QSpinBox();
    m_maxNotesSpinBox->setRange(10, 1000);
    m_maxNotesSpinBox->setValue(100);
    maxNotesLayout->addWidget(m_maxNotesSpinBox);
    maxNotesLayout->addStretch();
    storageLayout->addLayout(maxNotesLayout);
    
    // 默认样式设置
    QGroupBox* defaultStyleGroup = new QGroupBox("默认样式设置");
    QGridLayout* styleGrid = new QGridLayout(defaultStyleGroup);
    
    styleGrid->addWidget(new QLabel("默认字体:"), 0, 0);
    m_defaultFontComboBox = new QFontComboBox();
    styleGrid->addWidget(m_defaultFontComboBox, 0, 1);
    
    styleGrid->addWidget(new QLabel("默认字体大小:"), 0, 2);
    m_defaultFontSizeSpinBox = new QSpinBox();
    m_defaultFontSizeSpinBox->setRange(8, 72);
    m_defaultFontSizeSpinBox->setValue(12);
    styleGrid->addWidget(m_defaultFontSizeSpinBox, 0, 3);
    
    styleGrid->addWidget(new QLabel("默认文本颜色:"), 1, 0);
    m_defaultTextColorBtn = new QPushButton();
    m_defaultTextColorBtn->setFixedSize(40, 30);
    styleGrid->addWidget(m_defaultTextColorBtn, 1, 1);
    
    styleGrid->addWidget(new QLabel("默认背景颜色:"), 1, 2);
    m_defaultBackgroundColorBtn = new QPushButton();
    m_defaultBackgroundColorBtn->setFixedSize(40, 30);
    styleGrid->addWidget(m_defaultBackgroundColorBtn, 1, 3);
    
    notesLayout->addWidget(autoSaveGroup);
    notesLayout->addWidget(storageGroup);
    notesLayout->addWidget(defaultStyleGroup);
    notesLayout->addStretch();
    
    m_tabWidget->addTab(m_notesTab, "便签设置");
    
    // 连接信号
    connect(m_autoSaveCheck, &QCheckBox::toggled, this, &NotesConfigDialog::onNotesSettingsChanged);
    connect(m_autoSaveIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onNotesSettingsChanged);
    connect(m_maxNotesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onNotesSettingsChanged);
    connect(m_defaultFontComboBox, &QFontComboBox::currentFontChanged, this, &NotesConfigDialog::onNotesSettingsChanged);
    connect(m_defaultFontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onNotesSettingsChanged);
    connect(m_browsePathBtn, &QPushButton::clicked, this, &NotesConfigDialog::onBrowsePathClicked);
    connect(m_defaultTextColorBtn, &QPushButton::clicked, this, &NotesConfigDialog::onColorButtonClicked);
    connect(m_defaultBackgroundColorBtn, &QPushButton::clicked, this, &NotesConfigDialog::onColorButtonClicked);
}

void NotesConfigDialog::setupAppearanceTab() {
    m_appearanceTab = new QWidget();
    QVBoxLayout* appearanceLayout = new QVBoxLayout(m_appearanceTab);
    
    // 小组件外观
    QGroupBox* widgetAppearanceGroup = new QGroupBox("小组件外观");
    QGridLayout* widgetGrid = new QGridLayout(widgetAppearanceGroup);
    
    widgetGrid->addWidget(new QLabel("背景颜色:"), 0, 0);
    m_widgetBackgroundColorBtn = new QPushButton();
    m_widgetBackgroundColorBtn->setFixedSize(40, 30);
    widgetGrid->addWidget(m_widgetBackgroundColorBtn, 0, 1);
    
    widgetGrid->addWidget(new QLabel("边框颜色:"), 0, 2);
    m_borderColorBtn = new QPushButton();
    m_borderColorBtn->setFixedSize(40, 30);
    widgetGrid->addWidget(m_borderColorBtn, 0, 3);
    
    widgetGrid->addWidget(new QLabel("边框宽度:"), 1, 0);
    m_borderWidthSpinBox = new QSpinBox();
    m_borderWidthSpinBox->setRange(0, 10);
    m_borderWidthSpinBox->setValue(1);
    widgetGrid->addWidget(m_borderWidthSpinBox, 1, 1);
    
    // 布局设置
    QGroupBox* layoutGroup = new QGroupBox("布局设置");
    QVBoxLayout* layoutLayout = new QVBoxLayout(layoutGroup);
    
    QHBoxLayout* leftPanelLayout = new QHBoxLayout();
    leftPanelLayout->addWidget(new QLabel("左侧面板宽度:"));
    m_leftPanelWidthSlider = new QSlider(Qt::Horizontal);
    m_leftPanelWidthSlider->setRange(100, 300);
    m_leftPanelWidthSlider->setValue(150);
    leftPanelLayout->addWidget(m_leftPanelWidthSlider);
    m_leftPanelWidthLabel = new QLabel("150px");
    leftPanelLayout->addWidget(m_leftPanelWidthLabel);
    layoutLayout->addLayout(leftPanelLayout);
    
    // 界面元素显示
    QGroupBox* uiElementsGroup = new QGroupBox("界面元素");
    QVBoxLayout* uiElementsLayout = new QVBoxLayout(uiElementsGroup);
    
    m_showToolbarCheck = new QCheckBox("显示工具栏");
    m_showSearchBoxCheck = new QCheckBox("显示搜索框");
    
    uiElementsLayout->addWidget(m_showToolbarCheck);
    uiElementsLayout->addWidget(m_showSearchBoxCheck);
    
    appearanceLayout->addWidget(widgetAppearanceGroup);
    appearanceLayout->addWidget(layoutGroup);
    appearanceLayout->addWidget(uiElementsGroup);
    appearanceLayout->addStretch();
    
    m_tabWidget->addTab(m_appearanceTab, "外观设置");
    
    // 连接信号
    connect(m_widgetBackgroundColorBtn, &QPushButton::clicked, this, &NotesConfigDialog::onColorButtonClicked);
    connect(m_borderColorBtn, &QPushButton::clicked, this, &NotesConfigDialog::onColorButtonClicked);
    connect(m_borderWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &NotesConfigDialog::onAppearanceSettingsChanged);
    connect(m_leftPanelWidthSlider, &QSlider::valueChanged, this, [this](int value) {
        m_leftPanelWidthLabel->setText(QString("%1px").arg(value));
        onAppearanceSettingsChanged();
    });
    connect(m_showToolbarCheck, &QCheckBox::toggled, this, &NotesConfigDialog::onAppearanceSettingsChanged);
    connect(m_showSearchBoxCheck, &QCheckBox::toggled, this, &NotesConfigDialog::onAppearanceSettingsChanged);
}

void NotesConfigDialog::setupButtons() {
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_applyBtn = new QPushButton("应用");
    m_resetBtn = new QPushButton("重置");
    m_okBtn = new QPushButton("确定");
    m_cancelBtn = new QPushButton("取消");
    
    buttonLayout->addWidget(m_applyBtn);
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    // 连接按钮信号
    connect(m_applyBtn, &QPushButton::clicked, this, &NotesConfigDialog::onApplyClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &NotesConfigDialog::onResetClicked);
    connect(m_okBtn, &QPushButton::clicked, this, &NotesConfigDialog::onOkClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &NotesConfigDialog::onCancelClicked);
    
    m_mainLayout->addLayout(buttonLayout);
}

void NotesConfigDialog::loadConfigToUI() {
    // 加载基本设置
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
    
    // 加载自定义设置
    const QJsonObject& settings = m_config.customSettings;
    
    // 便签设置
    m_autoSaveCheck->setChecked(settings.value("autoSave").toBool(true));
    m_autoSaveIntervalSpinBox->setValue(settings.value("autoSaveInterval").toInt(30000) / 1000);
    
    QString notesPath = settings.value("notesFilePath").toString();
    if (notesPath.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        notesPath = dataDir + "/notes.json";
    }
    m_notesFilePathEdit->setText(notesPath);
    
    m_maxNotesSpinBox->setValue(settings.value("maxNotes").toInt(100));
    
    // 默认样式设置
    QString defaultFontFamily = settings.value("defaultFontFamily").toString("Arial");
    m_defaultFontComboBox->setCurrentFont(QFont(defaultFontFamily));
    m_defaultFontSizeSpinBox->setValue(settings.value("defaultFontSize").toInt(12));
    
    if (settings.contains("defaultTextColor")) {
        m_defaultTextColor = QColor(settings.value("defaultTextColor").toString());
    }
    if (settings.contains("defaultBackgroundColor")) {
        m_defaultBackgroundColor = QColor(settings.value("defaultBackgroundColor").toString());
    }
    
    // 外观设置
    if (settings.contains("widgetBackgroundColor")) {
        m_widgetBackgroundColor = QColor(settings.value("widgetBackgroundColor").toString());
    }
    if (settings.contains("borderColor")) {
        m_borderColor = QColor(settings.value("borderColor").toString());
    }
    m_borderWidthSpinBox->setValue(settings.value("borderWidth").toInt(1));
    
    int leftPanelWidth = settings.value("leftPanelWidth").toInt(150);
    m_leftPanelWidthSlider->setValue(leftPanelWidth);
    m_leftPanelWidthLabel->setText(QString("%1px").arg(leftPanelWidth));
    
    m_showToolbarCheck->setChecked(settings.value("showToolbar").toBool(true));
    m_showSearchBoxCheck->setChecked(settings.value("showSearchBox").toBool(true));
    
    // 更新颜色按钮显示
    updateColorButton(m_defaultTextColorBtn, m_defaultTextColor);
    updateColorButton(m_defaultBackgroundColorBtn, m_defaultBackgroundColor);
    updateColorButton(m_widgetBackgroundColorBtn, m_widgetBackgroundColor);
    updateColorButton(m_borderColorBtn, m_borderColor);
}

void NotesConfigDialog::saveUIToConfig() {
    // 保存基本设置
    m_config.name = m_nameEdit->text();
    m_config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    m_config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    m_config.alwaysOnTop = m_alwaysOnTopCheck->isChecked();
    m_config.clickThrough = m_clickThroughCheck->isChecked();
    m_config.locked = m_lockedCheck->isChecked();
    m_config.opacity = m_opacitySlider->value() / 100.0;
    
    // 保存自定义设置
    QJsonObject settings;
    
    // 便签设置
    settings["autoSave"] = m_autoSaveCheck->isChecked();
    settings["autoSaveInterval"] = m_autoSaveIntervalSpinBox->value() * 1000;
    settings["notesFilePath"] = m_notesFilePathEdit->text();
    settings["maxNotes"] = m_maxNotesSpinBox->value();
    
    // 默认样式设置
    settings["defaultFontFamily"] = m_defaultFontComboBox->currentFont().family();
    settings["defaultFontSize"] = m_defaultFontSizeSpinBox->value();
    settings["defaultTextColor"] = m_defaultTextColor.name();
    settings["defaultBackgroundColor"] = m_defaultBackgroundColor.name();
    
    // 外观设置
    settings["widgetBackgroundColor"] = m_widgetBackgroundColor.name();
    settings["borderColor"] = m_borderColor.name();
    settings["borderWidth"] = m_borderWidthSpinBox->value();
    settings["leftPanelWidth"] = m_leftPanelWidthSlider->value();
    settings["showToolbar"] = m_showToolbarCheck->isChecked();
    settings["showSearchBox"] = m_showSearchBoxCheck->isChecked();
    
    m_config.customSettings = settings;
}

void NotesConfigDialog::updateColorButton(QPushButton* button, const QColor& color) {
    QString styleSheet = QString("QPushButton { background-color: %1; border: 1px solid #666; }").arg(color.name());
    button->setStyleSheet(styleSheet);
    button->setProperty("color", color);
}

void NotesConfigDialog::updateFontButton(QPushButton* button, const QFont& font) {
    QString text = QString("%1, %2pt").arg(font.family()).arg(font.pointSize());
    button->setText(text);
    button->setProperty("font", font);
}

QColor NotesConfigDialog::getColorFromButton(QPushButton* button) const {
    return button->property("color").value<QColor>();
}

QFont NotesConfigDialog::getFontFromButton(QPushButton* button) const {
    return button->property("font").value<QFont>();
}

void NotesConfigDialog::onBasicSettingsChanged() {
    m_hasChanges = true;
}

void NotesConfigDialog::onNotesSettingsChanged() {
    m_hasChanges = true;
}

void NotesConfigDialog::onAppearanceSettingsChanged() {
    m_hasChanges = true;
}

void NotesConfigDialog::onColorButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QColor currentColor = getColorFromButton(button);
    QColor color = QColorDialog::getColor(currentColor, this, "选择颜色");
    
    if (color.isValid()) {
        updateColorButton(button, color);
        
        // 更新对应的成员变量
        if (button == m_defaultTextColorBtn) {
            m_defaultTextColor = color;
            onNotesSettingsChanged();
        } else if (button == m_defaultBackgroundColorBtn) {
            m_defaultBackgroundColor = color;
            onNotesSettingsChanged();
        } else if (button == m_widgetBackgroundColorBtn) {
            m_widgetBackgroundColor = color;
            onAppearanceSettingsChanged();
        } else if (button == m_borderColorBtn) {
            m_borderColor = color;
            onAppearanceSettingsChanged();
        }
    }
}

void NotesConfigDialog::onFontButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QFont currentFont = getFontFromButton(button);
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currentFont, this, "选择字体");
    
    if (ok) {
        updateFontButton(button, font);
        m_defaultFont = font;
        onNotesSettingsChanged();
    }
}

void NotesConfigDialog::onBrowsePathClicked() {
    QString currentPath = m_notesFilePathEdit->text();
    if (currentPath.isEmpty()) {
        currentPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "选择便签文件位置", 
        currentPath, 
        "JSON文件 (*.json)");
    
    if (!fileName.isEmpty()) {
        m_notesFilePathEdit->setText(fileName);
        onNotesSettingsChanged();
    }
}

void NotesConfigDialog::onApplyClicked() {
    saveUIToConfig();
    m_hasChanges = false;
}

void NotesConfigDialog::onResetClicked() {
    if (m_hasChanges) {
        int ret = QMessageBox::question(this, "重置设置", 
            "确定要重置所有设置到默认值吗？这将丢失当前的修改。",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    // 重置到默认值
    WidgetConfig defaultConfig;
    defaultConfig.id = m_config.id;
    defaultConfig.type = m_config.type;
    defaultConfig.name = "便签";
    defaultConfig.size = QSize(400, 300);
    m_config = defaultConfig;
    
    loadConfigToUI();
    m_hasChanges = false;
}

void NotesConfigDialog::onOkClicked() {
    saveUIToConfig();
    accept();
}

void NotesConfigDialog::onCancelClicked() {
    if (m_hasChanges) {
        int ret = QMessageBox::question(this, "取消设置", 
            "设置已修改，确定要取消吗？",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    reject();
} 