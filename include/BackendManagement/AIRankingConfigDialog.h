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
#include "Common/Types.h"

class AIRankingConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit AIRankingConfigDialog(const WidgetConfig& config, QWidget* parent = nullptr);
    ~AIRankingConfigDialog() = default;

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
    void onDataSourceChanged();
    void onCapabilityChanged();
    void onPreviewDataClicked();

private:
    void setupUI();
    void setupBasicTab();
    void setupDisplayTab();
    void setupDataTab();
    void setupAdvancedTab();
    void setupButtons();
    
    void loadConfigToUI();
    void saveUIToConfig();
    void updateColorButton(QPushButton* button, const QColor& color);
    QColor getColorFromButton(QPushButton* button) const;
    void updateDataSourceDescription();
    void updateCapabilityDescription();

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
    QCheckBox* m_clickThroughCheck;
    QCheckBox* m_lockedCheck;
    QSlider* m_opacitySlider;
    QLabel* m_opacityLabel;
    QSpinBox* m_updateIntervalSpinBox;
    
    // 显示设置标签页
    QWidget* m_displayTab;
    QSpinBox* m_maxDisplayCountSpinBox;
    QCheckBox* m_showProviderCheck;
    QCheckBox* m_showScoreCheck;
    QCheckBox* m_showLastUpdateCheck;
    QPushButton* m_headerColorBtn;
    QPushButton* m_textColorBtn;
    QPushButton* m_backgroundColorBtn;
    QSpinBox* m_headerFontSizeSpinBox;
    QSpinBox* m_modelFontSizeSpinBox;
    QSpinBox* m_itemHeightSpinBox;
    
    // 数据源和能力设置标签页
    QWidget* m_dataTab;
    QComboBox* m_dataSourceComboBox;
    QComboBox* m_capabilityComboBox;
    QLabel* m_dataSourceDescLabel;
    QLabel* m_capabilityDescLabel;
    QPushButton* m_previewDataBtn;
    
    // 高级设置标签页
    QWidget* m_advancedTab;
    QCheckBox* m_autoRefreshCheck;
    QSpinBox* m_refreshIntervalSpinBox;
    QPushButton* m_refreshNowBtn;
    QLabel* m_lastUpdateLabel;
    
    // 按钮区域
    QPushButton* m_applyBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    // 当前选择的颜色
    QColor m_headerColor;
    QColor m_textColor;
    QColor m_backgroundColor;
}; 