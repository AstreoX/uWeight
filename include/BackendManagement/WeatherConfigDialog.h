#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QColorDialog>
#include <QFontDialog>
#include <QTextEdit>
#include "Common/Types.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

class WeatherConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit WeatherConfigDialog(const WidgetConfig& config, QWidget* parent = nullptr);
    ~WeatherConfigDialog() = default;

    // 获取更新后的配置
    WidgetConfig getUpdatedConfig() const { return m_config; }

private slots:
    void onBasicSettingsChanged();
    void onDisplaySettingsChanged();
    void onColorButtonClicked();
    void onApplyClicked();
    void onResetClicked();
    void onOkClicked();
    void onCancelClicked();
    void onRefreshNowClicked();
    void onTestApiClicked();
    void onGetApiKeyClicked();
    void onTestApiFinished();

private:
    void setupUI();
    void setupBasicTab();
    void setupDisplayTab();
    void setupApiTab();
    void setupAdvancedTab();
    void setupButtons();
    
    void loadConfigToUI();
    void saveUIToConfig();
    void updateColorButton(QPushButton* button, const QColor& color);
    QColor getColorFromButton(QPushButton* button) const;
    void updateApiInfo();
    void updateApiKeyInfo();
    void validateApiKey();
    void saveApiSettings(bool showMessage = false);
    void loadApiSettings();

private:
    WidgetConfig m_config;
    bool m_hasChanges;
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // 基本设置标签页
    QWidget* m_basicTab;
    QLineEdit* m_nameEdit;
    QSpinBox* m_xSpinBox;
    QSpinBox* m_ySpinBox;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
    QCheckBox* m_alwaysOnTopCheck;
    QCheckBox* m_alwaysOnBottomCheck;   // 新增：始终置底选项
    QComboBox* m_windowLayerComboBox;   // 新增：窗口层级选择
    QCheckBox* m_avoidMinimizeAllCheck; // 新增：避免被显示桌面影响
    QCheckBox* m_clickThroughCheck;
    QCheckBox* m_lockedCheck;
    QSlider* m_opacitySlider;
    QLabel* m_opacityLabel;
    QSpinBox* m_updateIntervalSpinBox;
    
    // 显示设置标签页
    QWidget* m_displayTab;
    QComboBox* m_displayStyleComboBox;
    QComboBox* m_temperatureUnitComboBox;
    QCheckBox* m_showWeatherIconCheck;
    QCheckBox* m_showHumidityCheck;
    QCheckBox* m_showWindSpeedCheck;
    QCheckBox* m_showPressureCheck;
    QCheckBox* m_showLastUpdateCheck;
    QPushButton* m_temperatureColorBtn;
    QPushButton* m_locationColorBtn;
    QPushButton* m_infoColorBtn;
    QPushButton* m_backgroundColorBtn;
    QSpinBox* m_iconSizeSpinBox;
    QSpinBox* m_spacingSpinBox;
    QSpinBox* m_paddingSpinBox;
    
    // API设置标签页
    QWidget* m_apiTab;
    QComboBox* m_apiProviderComboBox;
    QLineEdit* m_apiKeyEdit;
    QLineEdit* m_apiHostEdit;
    QLineEdit* m_cityNameEdit;
    QLineEdit* m_locationEdit;
    QPushButton* m_testApiBtn;
    QPushButton* m_getApiKeyBtn;
    QLabel* m_apiStatusLabel;
    QTextEdit* m_apiInfoText;
    
    // 高级设置标签页
    QWidget* m_advancedTab;
    QCheckBox* m_enableAutoRefreshCheck;
    QSpinBox* m_weatherUpdateIntervalSpinBox;
    QCheckBox* m_autoUpdateLocationCheck;
    QPushButton* m_refreshNowBtn;
    QLabel* m_lastUpdateLabel;
    
    // 按钮区域
    QPushButton* m_applyBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    // 当前选择的颜色
    QColor m_temperatureColor;
    QColor m_locationColor;
    QColor m_infoColor;
    QColor m_backgroundColor;

    QNetworkAccessManager* m_testNetworkManager;
    QNetworkReply* m_currentTestReply;
}; 