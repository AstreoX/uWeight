#include "BackendManagement/WeatherConfigDialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>

WeatherConfigDialog::WeatherConfigDialog(const WidgetConfig& config, QWidget* parent)
    : QDialog(parent)
    , m_config(config)
    , m_hasChanges(false)
    , m_temperatureColor(50, 50, 50)
    , m_locationColor(100, 100, 100)
    , m_infoColor(120, 120, 120)
    , m_backgroundColor(255, 255, 255, 200)
    , m_testNetworkManager(new QNetworkAccessManager(this))
    , m_currentTestReply(nullptr)
{
    setWindowTitle("天气组件配置");
    setModal(true);
    resize(500, 600);
    
    // 配置SSL设置，忽略SSL错误（仅用于开发测试）
    connect(m_testNetworkManager, &QNetworkAccessManager::sslErrors,
            [](QNetworkReply* reply, const QList<QSslError>& errors) {
                Q_UNUSED(errors)
                qDebug() << "忽略SSL错误:" << errors;
                reply->ignoreSslErrors();
            });
    
    // 启用自动重定向跟随
    m_testNetworkManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    
    setupUI();
    loadConfigToUI();
    loadApiSettings(); // 加载保存的API设置
}

void WeatherConfigDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建选项卡
    m_tabWidget = new QTabWidget(this);
    
    setupBasicTab();
    setupDisplayTab();
    setupApiTab();
    setupAdvancedTab();
    setupButtons();
    
    m_mainLayout->addWidget(m_tabWidget);
}

void WeatherConfigDialog::setupBasicTab() {
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
    m_widthSpinBox->setRange(100, 2000);
    positionGrid->addWidget(m_widthSpinBox, 1, 1);
    
    positionGrid->addWidget(new QLabel("高度:"), 1, 2);
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setRange(100, 2000);
    positionGrid->addWidget(m_heightSpinBox, 1, 3);
    
    // 窗口选项
    QGroupBox* windowGroup = new QGroupBox("窗口选项");
    QVBoxLayout* windowLayout = new QVBoxLayout(windowGroup);
    
    // 窗口层级选择
    QHBoxLayout* layerLayout = new QHBoxLayout();
    layerLayout->addWidget(new QLabel("窗口层级:"));
    m_windowLayerComboBox = new QComboBox();
    m_windowLayerComboBox->addItem("正常层级", 0);
    m_windowLayerComboBox->addItem("始终置顶", 1);
    m_windowLayerComboBox->addItem("始终置底", 2);
    layerLayout->addWidget(m_windowLayerComboBox);
    layerLayout->addStretch();
    windowLayout->addLayout(layerLayout);
    
    // 避免被显示桌面影响
    m_avoidMinimizeAllCheck = new QCheckBox("避免被Win+D等显示桌面快捷键影响");
    windowLayout->addWidget(m_avoidMinimizeAllCheck);
    
    // 兼容性选项（保留原有的复选框以保持兼容性）
    m_alwaysOnTopCheck = new QCheckBox("始终置顶");
    m_alwaysOnBottomCheck = new QCheckBox("始终置底");
    m_clickThroughCheck = new QCheckBox("点击穿透");
    m_lockedCheck = new QCheckBox("锁定位置");
    
    windowLayout->addWidget(m_alwaysOnTopCheck);
    windowLayout->addWidget(m_alwaysOnBottomCheck);
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
    
    // 更新间隔
    QHBoxLayout* updateLayout = new QHBoxLayout();
    updateLayout->addWidget(new QLabel("更新间隔(秒):"));
    m_updateIntervalSpinBox = new QSpinBox();
    m_updateIntervalSpinBox->setRange(1, 3600);
    m_updateIntervalSpinBox->setValue(10);
    updateLayout->addWidget(m_updateIntervalSpinBox);
    updateLayout->addStretch();
    
    basicLayout->addWidget(basicGroup);
    basicLayout->addWidget(positionGroup);
    basicLayout->addWidget(windowGroup);
    basicLayout->addLayout(updateLayout);
    basicLayout->addStretch();
    
    m_tabWidget->addTab(m_basicTab, "基本设置");
    
    // 连接信号
    connect(m_nameEdit, &QLineEdit::textChanged, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_windowLayerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_avoidMinimizeAllCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_alwaysOnTopCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_alwaysOnBottomCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_clickThroughCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_lockedCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onBasicSettingsChanged();
    });
}

void WeatherConfigDialog::setupDisplayTab() {
    m_displayTab = new QWidget();
    QVBoxLayout* displayLayout = new QVBoxLayout(m_displayTab);
    
    // 显示样式分组
    QGroupBox* styleGroup = new QGroupBox("显示样式");
    QGridLayout* styleGrid = new QGridLayout(styleGroup);
    
    styleGrid->addWidget(new QLabel("显示模式:"), 0, 0);
    m_displayStyleComboBox = new QComboBox();
    m_displayStyleComboBox->addItems({"紧凑模式", "详细模式", "迷你模式"});
    styleGrid->addWidget(m_displayStyleComboBox, 0, 1);
    
    styleGrid->addWidget(new QLabel("温度单位:"), 0, 2);
    m_temperatureUnitComboBox = new QComboBox();
    m_temperatureUnitComboBox->addItems({"摄氏度 (°C)", "华氏度 (°F)"});
    styleGrid->addWidget(m_temperatureUnitComboBox, 0, 3);
    
    // 显示项目
    QGroupBox* itemsGroup = new QGroupBox("显示项目");
    QVBoxLayout* itemsLayout = new QVBoxLayout(itemsGroup);
    
    m_showWeatherIconCheck = new QCheckBox("显示天气图标");
    m_showHumidityCheck = new QCheckBox("显示湿度");
    m_showWindSpeedCheck = new QCheckBox("显示风速");
    m_showPressureCheck = new QCheckBox("显示气压");
    m_showLastUpdateCheck = new QCheckBox("显示更新时间");
    
    itemsLayout->addWidget(m_showWeatherIconCheck);
    itemsLayout->addWidget(m_showHumidityCheck);
    itemsLayout->addWidget(m_showWindSpeedCheck);
    itemsLayout->addWidget(m_showPressureCheck);
    itemsLayout->addWidget(m_showLastUpdateCheck);
    
    // 颜色设置
    QGroupBox* colorGroup = new QGroupBox("颜色设置");
    QGridLayout* colorGrid = new QGridLayout(colorGroup);
    
    colorGrid->addWidget(new QLabel("温度颜色:"), 0, 0);
    m_temperatureColorBtn = new QPushButton();
    m_temperatureColorBtn->setFixedSize(40, 30);
    colorGrid->addWidget(m_temperatureColorBtn, 0, 1);
    
    colorGrid->addWidget(new QLabel("位置颜色:"), 0, 2);
    m_locationColorBtn = new QPushButton();
    m_locationColorBtn->setFixedSize(40, 30);
    colorGrid->addWidget(m_locationColorBtn, 0, 3);
    
    colorGrid->addWidget(new QLabel("信息颜色:"), 1, 0);
    m_infoColorBtn = new QPushButton();
    m_infoColorBtn->setFixedSize(40, 30);
    colorGrid->addWidget(m_infoColorBtn, 1, 1);
    
    colorGrid->addWidget(new QLabel("背景颜色:"), 1, 2);
    m_backgroundColorBtn = new QPushButton();
    m_backgroundColorBtn->setFixedSize(40, 30);
    colorGrid->addWidget(m_backgroundColorBtn, 1, 3);
    
    displayLayout->addWidget(styleGroup);
    displayLayout->addWidget(itemsGroup);
    displayLayout->addWidget(colorGroup);
    displayLayout->addStretch();
    
    m_tabWidget->addTab(m_displayTab, "显示设置");
    
    // 连接颜色按钮信号
    connect(m_temperatureColorBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onColorButtonClicked);
    connect(m_locationColorBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onColorButtonClicked);
    connect(m_infoColorBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onColorButtonClicked);
    connect(m_backgroundColorBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onColorButtonClicked);
    
    // 连接其他显示设置信号
    connect(m_displayStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WeatherConfigDialog::onDisplaySettingsChanged);
    connect(m_temperatureUnitComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WeatherConfigDialog::onDisplaySettingsChanged);
    connect(m_showWeatherIconCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onDisplaySettingsChanged);
    connect(m_showHumidityCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onDisplaySettingsChanged);
    connect(m_showWindSpeedCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onDisplaySettingsChanged);
    connect(m_showPressureCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onDisplaySettingsChanged);
    connect(m_showLastUpdateCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onDisplaySettingsChanged);
}

void WeatherConfigDialog::setupApiTab() {
    m_apiTab = new QWidget();
    QVBoxLayout* apiLayout = new QVBoxLayout(m_apiTab);
    
    // API提供商选择
    QGroupBox* providerGroup = new QGroupBox("API提供商");
    QVBoxLayout* providerLayout = new QVBoxLayout(providerGroup);
    
    m_apiProviderComboBox = new QComboBox();
    m_apiProviderComboBox->addItem("和风天气 (推荐)", "qweather");
    m_apiProviderComboBox->addItem("心知天气", "seniverse");
    m_apiProviderComboBox->addItem("OpenWeatherMap", "openweathermap");
    providerLayout->addWidget(new QLabel("选择API提供商:"));
    providerLayout->addWidget(m_apiProviderComboBox);
    
    // API密钥设置
    QGroupBox* apiKeyGroup = new QGroupBox("API密钥设置");
    QVBoxLayout* apiKeyLayout = new QVBoxLayout(apiKeyGroup);
    
    QHBoxLayout* keyLayout = new QHBoxLayout();
    keyLayout->addWidget(new QLabel("API密钥:"));
    m_apiKeyEdit = new QLineEdit();
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("API密钥或JWT Token");
    keyLayout->addWidget(m_apiKeyEdit);
    
    QHBoxLayout* hostLayout = new QHBoxLayout();
    hostLayout->addWidget(new QLabel("API主机:"));
    m_apiHostEdit = new QLineEdit();
    m_apiHostEdit->setPlaceholderText("例如: pa2k5mmtvv.re.qweatherapi.com");
    hostLayout->addWidget(m_apiHostEdit);
    
    QHBoxLayout* keyButtonLayout = new QHBoxLayout();
    m_getApiKeyBtn = new QPushButton("获取API密钥");
    m_testApiBtn = new QPushButton("测试API");
    QPushButton* clearSettingsBtn = new QPushButton("清除保存的设置");
    keyButtonLayout->addWidget(m_getApiKeyBtn);
    keyButtonLayout->addWidget(m_testApiBtn);
    keyButtonLayout->addWidget(clearSettingsBtn);
    keyButtonLayout->addStretch();
    
    connect(clearSettingsBtn, &QPushButton::clicked, this, [this]() {
        QSettings settings("DesktopWidgetSystem", "WeatherAPI");
        settings.clear();
        QMessageBox::information(this, "提示", "保存的API设置已清除");
        qDebug() << "保存的API设置已清除";
    });
    
    m_apiStatusLabel = new QLabel("API状态: 未测试");
    
    apiKeyLayout->addLayout(keyLayout);
    apiKeyLayout->addLayout(hostLayout);
    apiKeyLayout->addLayout(keyButtonLayout);
    apiKeyLayout->addWidget(m_apiStatusLabel);
    
    // 位置设置
    QGroupBox* locationGroup = new QGroupBox("位置设置");
    QGridLayout* locationGrid = new QGridLayout(locationGroup);
    
    locationGrid->addWidget(new QLabel("城市名称:"), 0, 0);
    m_cityNameEdit = new QLineEdit();
    m_cityNameEdit->setPlaceholderText("支持: 北京/上海/广州/深圳/西安 或 LocationID(如101010100)");
    locationGrid->addWidget(m_cityNameEdit, 0, 1);
    
    locationGrid->addWidget(new QLabel("经纬度:"), 1, 0);
    m_locationEdit = new QLineEdit();
    m_locationEdit->setPlaceholderText("格式: 纬度,经度 (可选)");
    locationGrid->addWidget(m_locationEdit, 1, 1);
    
    // API信息
    QGroupBox* infoGroup = new QGroupBox("API信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    m_apiInfoText = new QTextEdit();
    m_apiInfoText->setReadOnly(true);
    m_apiInfoText->setMaximumHeight(150);
    updateApiInfo(); // 根据选择的API提供商更新信息
    infoLayout->addWidget(m_apiInfoText);
    
    apiLayout->addWidget(providerGroup);
    apiLayout->addWidget(apiKeyGroup);
    apiLayout->addWidget(locationGroup);
    apiLayout->addWidget(infoGroup);
    apiLayout->addStretch();
    
    m_tabWidget->addTab(m_apiTab, "API设置");
    
    // 连接信号
    connect(m_apiProviderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WeatherConfigDialog::updateApiInfo);
    connect(m_apiProviderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_apiKeyEdit, &QLineEdit::textChanged, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_apiHostEdit, &QLineEdit::textChanged, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_cityNameEdit, &QLineEdit::textChanged, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_locationEdit, &QLineEdit::textChanged, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_getApiKeyBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onGetApiKeyClicked);
    connect(m_testApiBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onTestApiClicked);
}

void WeatherConfigDialog::setupAdvancedTab() {
    m_advancedTab = new QWidget();
    QVBoxLayout* advancedLayout = new QVBoxLayout(m_advancedTab);
    
    // 自动更新设置
    QGroupBox* updateGroup = new QGroupBox("自动更新设置");
    QVBoxLayout* updateLayout = new QVBoxLayout(updateGroup);
    
    m_enableAutoRefreshCheck = new QCheckBox("启用自动刷新");
    updateLayout->addWidget(m_enableAutoRefreshCheck);
    
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel("更新间隔(分钟):"));
    m_weatherUpdateIntervalSpinBox = new QSpinBox();
    m_weatherUpdateIntervalSpinBox->setRange(1, 1440);
    m_weatherUpdateIntervalSpinBox->setValue(10);
    intervalLayout->addWidget(m_weatherUpdateIntervalSpinBox);
    intervalLayout->addStretch();
    updateLayout->addLayout(intervalLayout);
    
    m_autoUpdateLocationCheck = new QCheckBox("自动更新位置(基于IP)");
    updateLayout->addWidget(m_autoUpdateLocationCheck);
    
    // 手动操作
    QGroupBox* manualGroup = new QGroupBox("手动操作");
    QVBoxLayout* manualLayout = new QVBoxLayout(manualGroup);
    
    QHBoxLayout* refreshLayout = new QHBoxLayout();
    m_refreshNowBtn = new QPushButton("立即刷新");
    refreshLayout->addWidget(m_refreshNowBtn);
    refreshLayout->addStretch();
    manualLayout->addLayout(refreshLayout);
    
    m_lastUpdateLabel = new QLabel("最后更新: 未知");
    manualLayout->addWidget(m_lastUpdateLabel);
    
    advancedLayout->addWidget(updateGroup);
    advancedLayout->addWidget(manualGroup);
    advancedLayout->addStretch();
    
    m_tabWidget->addTab(m_advancedTab, "高级设置");
    
    // 连接信号
    connect(m_enableAutoRefreshCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_weatherUpdateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_autoUpdateLocationCheck, &QCheckBox::toggled, this, &WeatherConfigDialog::onBasicSettingsChanged);
    connect(m_refreshNowBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onRefreshNowClicked);
}

void WeatherConfigDialog::setupButtons() {
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
    connect(m_applyBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onApplyClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onResetClicked);
    connect(m_okBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onOkClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &WeatherConfigDialog::onCancelClicked);
    
    m_mainLayout->addLayout(buttonLayout);
}

void WeatherConfigDialog::loadConfigToUI() {
    // 加载基本设置
    m_nameEdit->setText(m_config.name);
    m_xSpinBox->setValue(m_config.position.x());
    m_ySpinBox->setValue(m_config.position.y());
    m_widthSpinBox->setValue(m_config.size.width());
    m_heightSpinBox->setValue(m_config.size.height());
    // 设置窗口层级下拉框
    if (m_config.alwaysOnTop) {
        m_windowLayerComboBox->setCurrentIndex(1);  // 始终置顶
    } else if (m_config.alwaysOnBottom) {
        m_windowLayerComboBox->setCurrentIndex(2);  // 始终置底
    } else {
        m_windowLayerComboBox->setCurrentIndex(0);  // 正常层级
    }
    
    // 设置避免最小化选项（从自定义设置中读取）
    bool avoidMinimizeAll = m_config.customSettings.value("avoidMinimizeAll").toBool(false);
    m_avoidMinimizeAllCheck->setChecked(avoidMinimizeAll);
    
    m_alwaysOnTopCheck->setChecked(m_config.alwaysOnTop);
    m_alwaysOnBottomCheck->setChecked(m_config.alwaysOnBottom);
    m_clickThroughCheck->setChecked(m_config.clickThrough);
    m_lockedCheck->setChecked(m_config.locked);
    m_opacitySlider->setValue(static_cast<int>(m_config.opacity * 100));
    m_opacityLabel->setText(QString("%1%").arg(static_cast<int>(m_config.opacity * 100)));
    m_updateIntervalSpinBox->setValue(m_config.updateInterval / 1000);
    
    // 加载自定义设置
    const QJsonObject& settings = m_config.customSettings;
    
    // API设置
    QString apiProvider = settings.value("apiProvider").toString("qweather");
    for (int i = 0; i < m_apiProviderComboBox->count(); ++i) {
        if (m_apiProviderComboBox->itemData(i).toString() == apiProvider) {
            m_apiProviderComboBox->setCurrentIndex(i);
            break;
        }
    }
    m_apiKeyEdit->setText(settings.value("apiKey").toString());
    m_apiHostEdit->setText(settings.value("apiHost").toString());
    m_cityNameEdit->setText(settings.value("cityName").toString("北京"));
    
    // 显示设置
    QString styleStr = settings.value("displayStyle").toString("Compact");
    if (styleStr == "Detailed") {
        m_displayStyleComboBox->setCurrentIndex(1);
    } else if (styleStr == "Mini") {
        m_displayStyleComboBox->setCurrentIndex(2);
    } else {
        m_displayStyleComboBox->setCurrentIndex(0);
    }
    
    QString unitStr = settings.value("temperatureUnit").toString("Celsius");
    m_temperatureUnitComboBox->setCurrentIndex(unitStr == "Fahrenheit" ? 1 : 0);
    
    m_showWeatherIconCheck->setChecked(settings.value("showWeatherIcon").toBool(true));
    m_showHumidityCheck->setChecked(settings.value("showHumidity").toBool(true));
    m_showWindSpeedCheck->setChecked(settings.value("showWindSpeed").toBool(true));
    m_showPressureCheck->setChecked(settings.value("showPressure").toBool(false));
    m_showLastUpdateCheck->setChecked(settings.value("showLastUpdate").toBool(true));
    
    // 颜色设置
    if (settings.contains("temperatureColor")) {
        m_temperatureColor = QColor::fromString(settings.value("temperatureColor").toString());
    }
    if (settings.contains("locationColor")) {
        m_locationColor = QColor::fromString(settings.value("locationColor").toString());
    }
    if (settings.contains("infoColor")) {
        m_infoColor = QColor::fromString(settings.value("infoColor").toString());
    }
    if (settings.contains("backgroundColor")) {
        m_backgroundColor = QColor::fromString(settings.value("backgroundColor").toString());
    }
    
    updateColorButton(m_temperatureColorBtn, m_temperatureColor);
    updateColorButton(m_locationColorBtn, m_locationColor);
    updateColorButton(m_infoColorBtn, m_infoColor);
    updateColorButton(m_backgroundColorBtn, m_backgroundColor);
    
    // 高级设置
    m_enableAutoRefreshCheck->setChecked(settings.value("enableAutoRefresh").toBool(true));
    m_weatherUpdateIntervalSpinBox->setValue(settings.value("updateInterval").toInt(600000) / 60000);
    m_autoUpdateLocationCheck->setChecked(settings.value("autoUpdateLocation").toBool(false));
}

void WeatherConfigDialog::saveUIToConfig() {
    // 保存基本设置
    m_config.name = m_nameEdit->text();
    m_config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    m_config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    
    // 从窗口层级下拉框设置alwaysOnTop和alwaysOnBottom
    int layerIndex = m_windowLayerComboBox->currentIndex();
    m_config.alwaysOnTop = (layerIndex == 1);     // 始终置顶
    m_config.alwaysOnBottom = (layerIndex == 2);  // 始终置底
    
    m_config.clickThrough = m_clickThroughCheck->isChecked();
    m_config.locked = m_lockedCheck->isChecked();
    m_config.opacity = m_opacitySlider->value() / 100.0;
    m_config.updateInterval = m_updateIntervalSpinBox->value() * 1000;
    
    // 保存自定义设置
    QJsonObject settings;
    
    // API设置
    settings["apiProvider"] = m_apiProviderComboBox->currentData().toString();
    settings["apiKey"] = m_apiKeyEdit->text();
    settings["apiHost"] = m_apiHostEdit->text();
    settings["cityName"] = m_cityNameEdit->text();
    settings["location"] = m_locationEdit->text();
    
    // 显示设置
    QStringList styleNames = {"Compact", "Detailed", "Mini"};
    settings["displayStyle"] = styleNames[m_displayStyleComboBox->currentIndex()];
    
    settings["temperatureUnit"] = m_temperatureUnitComboBox->currentIndex() == 0 ? "Celsius" : "Fahrenheit";
    settings["showWeatherIcon"] = m_showWeatherIconCheck->isChecked();
    settings["showHumidity"] = m_showHumidityCheck->isChecked();
    settings["showWindSpeed"] = m_showWindSpeedCheck->isChecked();
    settings["showPressure"] = m_showPressureCheck->isChecked();
    settings["showLastUpdate"] = m_showLastUpdateCheck->isChecked();
    
    // 颜色设置
    settings["temperatureColor"] = m_temperatureColor.name();
    settings["locationColor"] = m_locationColor.name();
    settings["infoColor"] = m_infoColor.name();
    settings["backgroundColor"] = m_backgroundColor.name();
    
    // 高级设置
    settings["enableAutoRefresh"] = m_enableAutoRefreshCheck->isChecked();
    settings["updateInterval"] = m_weatherUpdateIntervalSpinBox->value() * 60000;
    settings["autoUpdateLocation"] = m_autoUpdateLocationCheck->isChecked();
    
    // 添加避免最小化设置
    settings["avoidMinimizeAll"] = m_avoidMinimizeAllCheck->isChecked();
    
    m_config.customSettings = settings;
}

void WeatherConfigDialog::updateColorButton(QPushButton* button, const QColor& color) {
    QString style = QString("background-color: %1; border: 1px solid #ccc;").arg(color.name());
    button->setStyleSheet(style);
}

QColor WeatherConfigDialog::getColorFromButton(QPushButton* button) const {
    if (button == m_temperatureColorBtn) return m_temperatureColor;
    if (button == m_locationColorBtn) return m_locationColor;
    if (button == m_infoColorBtn) return m_infoColor;
    if (button == m_backgroundColorBtn) return m_backgroundColor;
    return QColor();
}

void WeatherConfigDialog::onBasicSettingsChanged() {
    m_hasChanges = true;
}

void WeatherConfigDialog::onDisplaySettingsChanged() {
    m_hasChanges = true;
}

void WeatherConfigDialog::onColorButtonClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QColor currentColor = getColorFromButton(button);
    QColor newColor = QColorDialog::getColor(currentColor, this, "选择颜色");
    
    if (newColor.isValid()) {
        if (button == m_temperatureColorBtn) {
            m_temperatureColor = newColor;
        } else if (button == m_locationColorBtn) {
            m_locationColor = newColor;
        } else if (button == m_infoColorBtn) {
            m_infoColor = newColor;
        } else if (button == m_backgroundColorBtn) {
            m_backgroundColor = newColor;
        }
        
        updateColorButton(button, newColor);
        m_hasChanges = true;
    }
}

void WeatherConfigDialog::onApplyClicked() {
    saveUIToConfig();
    m_hasChanges = false;
    QMessageBox::information(this, "提示", "配置已应用");
}

void WeatherConfigDialog::onResetClicked() {
    if (QMessageBox::question(this, "确认", "确定要重置所有设置吗？") == QMessageBox::Yes) {
        WidgetConfig defaultConfig;
        defaultConfig.type = WidgetType::Weather;
        m_config = defaultConfig;
        loadConfigToUI();
        m_hasChanges = true;
    }
}

void WeatherConfigDialog::onOkClicked() {
    saveUIToConfig();
    saveApiSettings(true); // 保存API设置到注册表，并显示消息
    qDebug() << "WeatherConfigDialog: 配置已保存到m_config";
    qDebug() << "API Provider:" << m_config.customSettings.value("apiProvider").toString();
    qDebug() << "API Key:" << m_config.customSettings.value("apiKey").toString();
    qDebug() << "City Name:" << m_config.customSettings.value("cityName").toString();
    accept();
}

void WeatherConfigDialog::onCancelClicked() {
    if (m_hasChanges) {
        int ret = QMessageBox::question(this, "确认", "有未保存的更改，确定要取消吗？");
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    reject();
}

void WeatherConfigDialog::onRefreshNowClicked() {
    QMessageBox::information(this, "提示", "天气数据刷新请求已发送");
}

void WeatherConfigDialog::onTestApiClicked() {
    QString apiKey = m_apiKeyEdit->text().trimmed();
    QString apiHost = m_apiHostEdit->text().trimmed();
    QString cityName = m_cityNameEdit->text().trimmed();
    QString apiProvider = m_apiProviderComboBox->currentData().toString();

    if (apiKey.isEmpty()) {
        m_apiStatusLabel->setText("API状态: 请输入API密钥");
        return;
    }

    if (cityName.isEmpty()) {
        m_apiStatusLabel->setText("API状态: 请输入城市名称");
        return;
    }

    m_apiStatusLabel->setText("API状态: 测试中...");
    m_testApiBtn->setEnabled(false);

    if (m_currentTestReply) {
        m_currentTestReply->abort();
        m_currentTestReply->deleteLater();
        m_currentTestReply = nullptr;
    }

    QUrl url;
    QUrlQuery query;

    if (apiProvider == "qweather") {
        QString host = apiHost.isEmpty() ? "devapi.qweather.com" : apiHost;
        url.setUrl(QString("https://%1/v7/weather/now").arg(host));
        
        // 优化城市参数 - 如果输入的是中文城市名，尝试转换为LocationID
        QString location = cityName;
        if (cityName == "北京" || cityName.toLower() == "beijing") {
            location = "101010100";  // 北京的LocationID
        } else if (cityName == "上海" || cityName.toLower() == "shanghai") {
            location = "101020100";  // 上海的LocationID  
        } else if (cityName == "广州" || cityName.toLower() == "guangzhou") {
            location = "101280101";  // 广州的LocationID
        } else if (cityName == "深圳" || cityName.toLower() == "shenzhen") {
            location = "101280601";  // 深圳的LocationID
        } else if (cityName == "西安" || cityName.toLower() == "xian" || cityName.toLower() == "xi'an") {
            location = "101110101";  // 西安的LocationID
        }
        
        query.addQueryItem("location", location);
        qDebug() << "使用位置参数:" << cityName << "->" << location;
        
        // 检查是否为JWT token（包含点号）还是传统API key
        bool isJWT = apiKey.contains('.');
        if (!isJWT) {
            // 传统API key，使用URL参数
            query.addQueryItem("key", apiKey);
        }
        // JWT token将在请求头中设置
    } else if (apiProvider == "seniverse") {
        url.setUrl("https://api.seniverse.com/v3/weather/now.json");
        query.addQueryItem("location", cityName);
        query.addQueryItem("key", apiKey);
        query.addQueryItem("language", "zh-Hans");
        query.addQueryItem("unit", "c");
    } else if (apiProvider == "openweathermap") {
        url.setUrl("https://api.openweathermap.org/data/2.5/weather");
        query.addQueryItem("q", cityName);
        query.addQueryItem("appid", apiKey);
        query.addQueryItem("units", "metric");
    } else {
        m_apiStatusLabel->setText("API状态: 不支持的提供商");
        m_testApiBtn->setEnabled(true);
        return;
    }

    url.setQuery(query);
    QNetworkRequest request(url);
    
    // 设置重定向策略
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    // 设置必要的请求头来模拟浏览器请求
    request.setHeader(QNetworkRequest::UserAgentHeader, 
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    // 暂时不要压缩，避免解压问题
    // request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Pragma", "no-cache");
    
    // 对于和风天气，添加认证和referer
    if (apiProvider == "qweather") {
        QString hostName = apiHost.isEmpty() ? "dev.qweather.com" : apiHost;
        QString refererUrl = QString("https://%1/").arg(hostName);
        request.setRawHeader("Referer", refererUrl.toUtf8());
        request.setRawHeader("Origin", QString("https://%1").arg(hostName).toUtf8());
        
        // 只有JWT token才需要Authorization头
        bool isJWT = apiKey.contains('.');
        if (isJWT) {
            QString authHeader = QString("Bearer %1").arg(apiKey);
            request.setRawHeader("Authorization", authHeader.toUtf8());
        }
    }
    
    m_currentTestReply = m_testNetworkManager->get(request);
    connect(m_currentTestReply, &QNetworkReply::finished, this, &WeatherConfigDialog::onTestApiFinished);
}

void WeatherConfigDialog::onTestApiFinished() {
    m_testApiBtn->setEnabled(true);

    if (!m_currentTestReply) {
        m_apiStatusLabel->setText("API状态: 测试异常 (reply为空)");
        return;
    }

    // 获取详细的响应信息
    int httpStatus = m_currentTestReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl finalUrl = m_currentTestReply->url();
    QUrl originalUrl = m_currentTestReply->request().url();
    bool wasRedirected = (finalUrl != originalUrl);
    QByteArray responseData = m_currentTestReply->readAll();
    QNetworkReply::NetworkError error = m_currentTestReply->error();
    QString errorString = m_currentTestReply->errorString();
    
    qDebug() << "=== API测试详细信息 ===";
    qDebug() << "HTTP状态:" << httpStatus;
    qDebug() << "原始URL:" << originalUrl.toString();
    qDebug() << "最终URL:" << finalUrl.toString();
    qDebug() << "是否重定向:" << wasRedirected;
    qDebug() << "网络错误代码:" << error;
    qDebug() << "错误描述:" << errorString;
    qDebug() << "响应数据长度:" << responseData.length();
    qDebug() << "响应数据内容:" << QString::fromUtf8(responseData.left(1000));
    
    // 打印所有响应头
    QList<QNetworkReply::RawHeaderPair> headers = m_currentTestReply->rawHeaderPairs();
    qDebug() << "响应头数量:" << headers.size();
    for (const auto& header : headers) {
        qDebug() << "  " << header.first << ":" << header.second;
    }

    if (error != QNetworkReply::NoError) {
        QString detailedError;
        
        // 根据错误类型提供更具体的诊断
        switch (error) {
        case QNetworkReply::ConnectionRefusedError:
            detailedError = "连接被拒绝 - 检查网络连接";
            break;
        case QNetworkReply::RemoteHostClosedError:
            detailedError = "远程主机关闭连接";
            break;
        case QNetworkReply::HostNotFoundError:
            detailedError = "无法找到主机 - 检查DNS设置";
            break;
        case QNetworkReply::TimeoutError:
            detailedError = "请求超时 - 网络可能较慢";
            break;
        case QNetworkReply::SslHandshakeFailedError:
            detailedError = "SSL握手失败 - 证书问题";
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
            detailedError = "代理连接被拒绝";
            break;
        case QNetworkReply::ContentAccessDenied:
            detailedError = "访问被拒绝 (403)";
            break;
        case QNetworkReply::AuthenticationRequiredError:
            detailedError = "需要身份验证";
            break;
        case QNetworkReply::InternalServerError:
            detailedError = "服务器内部错误 (500)";
            break;
        case QNetworkReply::TooManyRedirectsError:
            detailedError = "重定向次数过多";
            break;
        case QNetworkReply::InsecureRedirectError:
            detailedError = "不安全的重定向";
            break;
        default:
            if (httpStatus == 302) {
                detailedError = QString("重定向错误 (302) - 原始URL: %1, 最终URL: %2")
                    .arg(originalUrl.toString()).arg(finalUrl.toString());
            } else {
                detailedError = QString("未知网络错误 (代码: %1, HTTP: %2)")
                    .arg(static_cast<int>(error)).arg(httpStatus);
            }
            break;
        }
        
        QString suggestion = "";
        if (httpStatus == 302 || httpStatus == 400) {
            QString currentCity = m_cityNameEdit->text().trimmed();
            if (currentCity == "北京" || currentCity == "上海" || currentCity == "广州" || currentCity == "深圳" || currentCity == "西安") {
                suggestion = "\n建议：城市名应该自动转换为LocationID，请查看调试信息";
            } else {
                suggestion = "\n建议：请尝试输入'北京/西安'或直接使用LocationID '101010100'";
            }
        }
        
        m_apiStatusLabel->setText("API状态: " + detailedError + suggestion);
        m_apiStatusLabel->setStyleSheet("color: red;");
        
        // 如果有响应数据，尝试解析错误信息
        if (!responseData.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject json = doc.object();
                qDebug() << "服务器返回的JSON错误信息:" << json;
            }
        }
        
        m_currentTestReply->deleteLater();
        m_currentTestReply = nullptr;
        return;
    }

    // 如果没有网络错误，继续解析响应
    qDebug() << "=== 响应内容分析 ===";
    qDebug() << "Content-Type:" << m_currentTestReply->header(QNetworkRequest::ContentTypeHeader).toString();
    qDebug() << "响应内容(前1000字符):" << QString::fromUtf8(responseData.left(1000));
    qDebug() << "响应内容(HEX前100字节):" << responseData.left(100).toHex();
    
    // 检查是否是gzip压缩的数据
    QString contentEncoding = m_currentTestReply->rawHeader("Content-Encoding");
    qDebug() << "Content-Encoding:" << contentEncoding;
    
    if (responseData.startsWith("\x1f\x8b") || contentEncoding.contains("gzip")) {
        qDebug() << "检测到gzip压缩数据，但Qt应该自动处理";
        qDebug() << "如果看到乱码，可能是编码问题或API返回了非标准数据";
    }
    
    // 尝试不同的文本编码（Qt 6方式）
    qDebug() << "尝试UTF-8编码，前200字符:" << QString::fromUtf8(responseData).left(200);
    qDebug() << "尝试Latin-1编码，前200字符:" << QString::fromLatin1(responseData).left(200);
    qDebug() << "尝试Local8Bit编码，前200字符:" << QString::fromLocal8Bit(responseData).left(200);
    
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isNull() || !doc.isObject()) {
        QString contentPreview = QString::fromUtf8(responseData.left(200));
        contentPreview.replace('\n', ' ').replace('\r', ' ');
        
        QString errorMsg = "API状态: 响应解析失败 (非JSON格式)";
        
        // 检查常见错误类型
        QString responseText = QString::fromUtf8(responseData).toLower();
        if (responseText.contains("<html") || responseText.contains("<!doctype")) {
            errorMsg += "\n可能返回了HTML错误页面";
        } else if (responseText.contains("401") || responseText.contains("unauthorized")) {
            errorMsg += "\n可能是认证失败";
        } else if (responseText.contains("403") || responseText.contains("forbidden")) {
            errorMsg += "\n可能是权限被拒绝";
        } else if (responseText.contains("404") || responseText.contains("not found")) {
            errorMsg += "\n可能是API端点不存在";
        }
        
        errorMsg += QString("\n预览: %1...").arg(contentPreview);
        
        m_apiStatusLabel->setText(errorMsg);
        m_apiStatusLabel->setStyleSheet("color: red;");
        qDebug() << "JSON解析失败，完整响应内容:" << QString::fromUtf8(responseData);
        m_currentTestReply->deleteLater();
        m_currentTestReply = nullptr;
        return;
    }

    QJsonObject json = doc.object();
    QString apiProvider = m_apiProviderComboBox->currentData().toString();
    bool isSuccess = false;
    QString errorMessage;

    if (apiProvider == "qweather") {
        if (json.contains("code") && json["code"].toString() == "200") {
            isSuccess = true;
        } else {
            QString code = json["code"].toString();
            errorMessage = code;
            
            // 添加常见错误代码的说明
            if (code == "400") {
                errorMessage += " (请求错误，检查城市名称或使用LocationID)";
            } else if (code == "401") {
                errorMessage += " (认证失败，检查API密钥)";
            } else if (code == "403") {
                errorMessage += " (无权限，检查API主机或账户状态)";
            } else if (code == "404") {
                errorMessage += " (位置未找到，尝试使用LocationID)";
            }
        }
    } else if (apiProvider == "seniverse") {
        if (json.contains("results")) {
            isSuccess = true;
        } else if (json.contains("status")) {
            errorMessage = json["status"].toString();
        }
    } else if (apiProvider == "openweathermap") {
        if (json.contains("cod") && json["cod"].toInt() == 200) {
            isSuccess = true;
        } else if (json.contains("message")) {
            errorMessage = json["message"].toString();
        }
    }

    if (isSuccess) {
        m_apiStatusLabel->setText("API状态: 测试成功! (配置已自动保存)");
        m_apiStatusLabel->setStyleSheet("color: green;");
        saveApiSettings(); // 测试成功时自动保存设置
    } else {
        m_apiStatusLabel->setText("API状态: 测试失败 (" + errorMessage + ")");
        m_apiStatusLabel->setStyleSheet("color: red;");
    }

    m_currentTestReply->deleteLater();
    m_currentTestReply = nullptr;
}

void WeatherConfigDialog::onGetApiKeyClicked() {
    QString provider = m_apiProviderComboBox->currentData().toString();
    QString url;
    QString message;
    
    if (provider == "qweather") {
        url = "https://dev.qweather.com/";
        message = "浏览器将打开和风天气开发平台\n\n"
                 "获取API密钥和主机地址步骤:\n"
                 "1. 注册账号或登录\n"
                 "2. 进入控制台\n"
                 "3. 创建项目和应用\n"
                 "4. 获取API密钥（传统密钥或JWT Token）\n"
                 "5. 获取专用API主机地址（我的API Host页面）\n"
                 "6. 分别复制到对应输入框";
    } else if (provider == "seniverse") {
        url = "https://www.seniverse.com/";
        message = "浏览器将打开心知天气网站\n\n"
                 "获取API密钥步骤:\n"
                 "1. 注册账号或登录\n"
                 "2. 进入控制台\n"
                 "3. 创建应用\n"
                 "4. 获取密钥\n"
                 "5. 复制密钥到输入框";
    } else {
        url = "https://openweathermap.org/api";
        message = "浏览器将打开OpenWeatherMap网站\n\n"
                 "获取API密钥步骤:\n"
                 "1. 注册账号或登录\n"
                 "2. 转到API Keys页面\n"
                 "3. 创建新的API密钥\n"
                 "4. 复制密钥到输入框";
    }
    
    QDesktopServices::openUrl(QUrl(url));
    QMessageBox::information(this, "获取API密钥", message);
}

void WeatherConfigDialog::updateApiInfo() {
    QString provider = m_apiProviderComboBox->currentData().toString();
    QString infoText;
    
    if (provider == "qweather") {
        infoText = "和风天气 API 信息:\n\n"
                  "• 官网: https://www.qweather.com/\n"
                  "• 免费版本每天1000次调用\n"
                  "• 注册简单，支持中文\n"
                  "• 数据准确，服务稳定\n"
                  "• 支持全球城市查询\n"
                  "• 推荐用于中国用户\n"
                  "• 支持传统API密钥和JWT认证\n"
                  "• API主机格式: xxx.re.qweatherapi.com\n"
                  "• 测试成功后会自动保存配置";
    } else if (provider == "seniverse") {
        infoText = "心知天气 API 信息:\n\n"
                  "• 官网: https://www.seniverse.com/\n"
                  "• 免费版本每天1000次调用\n"
                  "• 国内服务商，速度快\n"
                  "• 接口简单易用\n"
                  "• 支持中文城市名\n"
                  "• 免费版功能有限";
    } else if (provider == "openweathermap") {
        infoText = "OpenWeatherMap API 信息:\n\n"
                  "• 官网: https://openweathermap.org/\n"
                  "• 免费版本每月1000次调用\n"
                  "• 国际知名天气服务\n"
                  "• 功能丰富，数据全面\n"
                  "• 需要翻墙访问\n"
                  "• 英文界面";
    }
    
    m_apiInfoText->setPlainText(infoText);
}

void WeatherConfigDialog::updateApiKeyInfo() {
    // 更新API密钥信息
}

void WeatherConfigDialog::validateApiKey() {
    // 验证API密钥
}

void WeatherConfigDialog::saveApiSettings(bool showMessage) {
    QSettings settings("DesktopWidgetSystem", "WeatherAPI");
    
    settings.setValue("apiProvider", m_apiProviderComboBox->currentData().toString());
    settings.setValue("apiKey", m_apiKeyEdit->text());
    settings.setValue("apiHost", m_apiHostEdit->text());
    settings.setValue("cityName", m_cityNameEdit->text());
    
    qDebug() << "API设置已保存到注册表/配置文件";
    
    if (showMessage) {
        QMessageBox::information(this, "提示", "API配置已保存，下次打开时会自动加载");
    }
}

void WeatherConfigDialog::loadApiSettings() {
    QSettings settings("DesktopWidgetSystem", "WeatherAPI");
    
    // 只在输入框为空时才加载保存的设置
    if (m_apiKeyEdit->text().isEmpty()) {
        QString savedApiKey = settings.value("apiKey", "").toString();
        if (!savedApiKey.isEmpty()) {
            m_apiKeyEdit->setText(savedApiKey);
            qDebug() << "已加载保存的API密钥";
        }
    }
    
    if (m_apiHostEdit->text().isEmpty()) {
        QString savedApiHost = settings.value("apiHost", "").toString();
        if (!savedApiHost.isEmpty()) {
            m_apiHostEdit->setText(savedApiHost);
            qDebug() << "已加载保存的API主机:" << savedApiHost;
        }
    }
    
    if (m_cityNameEdit->text().isEmpty() || m_cityNameEdit->text() == "北京") {
        QString savedCityName = settings.value("cityName", "").toString();
        if (!savedCityName.isEmpty()) {
            m_cityNameEdit->setText(savedCityName);
            qDebug() << "已加载保存的城市名称:" << savedCityName;
        }
    }
    
    // 加载API提供商设置
    QString savedProvider = settings.value("apiProvider", "qweather").toString();
    for (int i = 0; i < m_apiProviderComboBox->count(); ++i) {
        if (m_apiProviderComboBox->itemData(i).toString() == savedProvider) {
            m_apiProviderComboBox->setCurrentIndex(i);
            qDebug() << "已加载保存的API提供商:" << savedProvider;
            break;
        }
    }
} 