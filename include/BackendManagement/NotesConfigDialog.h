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
#include <QFileDialog>
#include <QTextEdit>
#include <QFontComboBox>
#include "Common/Types.h"

class NotesConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit NotesConfigDialog(const WidgetConfig& config, QWidget* parent = nullptr);
    ~NotesConfigDialog() = default;

    // 获取更新后的配置
    WidgetConfig getUpdatedConfig() const { return m_config; }

private slots:
    void onBasicSettingsChanged();
    void onNotesSettingsChanged();
    void onAppearanceSettingsChanged();
    void onColorButtonClicked();
    void onFontButtonClicked();
    void onBrowsePathClicked();
    void onApplyClicked();
    void onResetClicked();
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void setupBasicTab();
    void setupNotesTab();
    void setupAppearanceTab();
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
    
    // 便签设置标签页
    QWidget* m_notesTab;
    QCheckBox* m_autoSaveCheck;
    QSpinBox* m_autoSaveIntervalSpinBox;
    QLineEdit* m_notesFilePathEdit;
    QPushButton* m_browsePathBtn;
    QFontComboBox* m_defaultFontComboBox;
    QSpinBox* m_defaultFontSizeSpinBox;
    QPushButton* m_defaultTextColorBtn;
    QPushButton* m_defaultBackgroundColorBtn;
    QSpinBox* m_maxNotesSpinBox;
    
    // 外观设置标签页
    QWidget* m_appearanceTab;
    QPushButton* m_widgetBackgroundColorBtn;
    QPushButton* m_borderColorBtn;
    QSpinBox* m_borderWidthSpinBox;
    QSlider* m_leftPanelWidthSlider;
    QLabel* m_leftPanelWidthLabel;
    QCheckBox* m_showToolbarCheck;
    QCheckBox* m_showSearchBoxCheck;
    
    // 按钮区域
    QPushButton* m_applyBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    // 当前选择的颜色和字体
    QColor m_widgetBackgroundColor;
    QColor m_borderColor;
    QColor m_defaultTextColor;
    QColor m_defaultBackgroundColor;
    QFont m_defaultFont;
}; 