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

class ThemeSettingsDialog;

// ConfigWindow - 配置窗口（待实现）
class ConfigWindow : public QDialog {
    Q_OBJECT

public:
    explicit ConfigWindow(const WidgetConfig& config, QWidget* parent = nullptr);
    ~ConfigWindow() = default;

    // 获取更新后的配置
    WidgetConfig getUpdatedConfig() const { return m_config; }

private slots:
    void onBasicSettingsChanged();
    void onDisplaySettingsChanged();
    void onThemeSettingsClicked();
    void onColorButtonClicked();
    void onFontButtonClicked();
    void onApplyClicked();
    void onResetClicked();
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupBasicTab();
    void setupDisplayTab();
    void setupThemeTab();
    void setupButtons();
    
    void loadConfigToUI();
    void saveUIToConfig();
    void updateColorButton(QPushButton* button, const QColor& color);
    void updateFontButton(QPushButton* button, const QFont& font);
    
    QColor getColorFromButton(QPushButton* button) const;
    QFont getFontFromButton(QPushButton* button) const;

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
    QCheckBox* m_showDateCheck;
    QCheckBox* m_show24HourCheck;
    QCheckBox* m_showSecondsCheck;
    QPushButton* m_timeColorBtn;
    QPushButton* m_dateColorBtn;
    QPushButton* m_backgroundColorBtn;
    QPushButton* m_timeFontBtn;
    QPushButton* m_dateFontBtn;
    QSpinBox* m_timeFontSizeSpinBox;
    QSpinBox* m_dateFontSizeSpinBox;
    
    // 主题设置标签页
    QWidget* m_themeTab;
    QLabel* m_currentThemeLabel;
    QPushButton* m_themeSettingsBtn;
    QLabel* m_themePreviewLabel;
    
    // 按钮区域
    QPushButton* m_applyBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    // 当前选择的颜色和字体
    QColor m_timeColor;
    QColor m_dateColor;
    QColor m_backgroundColor;
    QFont m_timeFont;
    QFont m_dateFont;
}; 