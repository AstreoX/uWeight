#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QStatusBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QSlider>
#include <QScreen>
#include <QApplication>
#include "Common/Types.h"

class WidgetManager;

class ManagementWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ManagementWindow(WidgetManager* widgetManager, QWidget* parent = nullptr);
    ~ManagementWindow();

signals:
    void windowHiddenToTray();  // 新增：当窗口隐藏到托盘时发出的信号

public slots:
    void showAndRaise();
    void refreshWidgetList();

private slots:
    void onCreateWidget();
    void onRemoveWidget();
    void onStartWidget();
    void onStopWidget();
    void onConfigureWidget();
    void onWidgetListSelectionChanged();
    void onWidgetTypeChanged(int index);
    void onApplySettings();
    void onResetSettings();
    void onImportConfig();
    void onExportConfig();
    void onSettingsChanged();
    void onInstantApplySettings();
    void onWidgetManuallyMoved(const QString& widgetId, const QPoint& newPosition);
    void onAlwaysOnTopCheckChanged(bool checked);      // 新增：处理置顶复选框变化
    void onAlwaysOnBottomCheckChanged(bool checked);   // 新增：处理置底复选框变化
    void onWindowLayerComboChanged(int index);         // 新增：处理窗口层级下拉框变化
    void onAvoidMinimizeChanged();                     // 新增：处理防止最小化与置底冲突

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void setupWidgetList();
    void setupControlButtons();
    void setupSettingsPanel();
    void setupSliderConnections();
    void updateSliderRanges();
    void setupMenuBar();
    void setupStatusBar();
    
    void updateWidgetInfo();
    void updateSettingsPanel();
    void clearSettingsPanel();
    void filterWidgetList(const QString& searchText, int statusFilter);
    
    QString getCurrentSelectedWidgetId() const;
    void restoreWidgetSelection(const QString& widgetId);
    void populateSettingsFromConfig(const WidgetConfig& config);
    WidgetConfig getConfigFromSettings() const;

private:
    WidgetManager* m_widgetManager;
    
    // UI组件
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    
    // 左侧Widget列表
    QGroupBox* m_listGroupBox;
    QListWidget* m_widgetListWidget;
    QVBoxLayout* m_listLayout;
    
    // 中间控制按钮
    QVBoxLayout* m_buttonLayout;
    QPushButton* m_createButton;
    QPushButton* m_removeButton;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_configureButton;
    
    // 右侧设置面板
    QGroupBox* m_settingsGroupBox;
    QVBoxLayout* m_settingsLayout;
    
    // 设置控件
    QLineEdit* m_nameLineEdit;
    QComboBox* m_typeComboBox;
    QSpinBox* m_xSpinBox;
    QSpinBox* m_ySpinBox;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
    QDoubleSpinBox* m_opacitySpinBox;
    QSpinBox* m_updateIntervalSpinBox;
    QCheckBox* m_alwaysOnTopCheckBox;
    QCheckBox* m_alwaysOnBottomCheckBox;    // 新增：始终置底选项
    QComboBox* m_windowLayerComboBox;       // 新增：窗口层级下拉选择
    QCheckBox* m_avoidMinimizeAllCheckBox;  // 新增：避免被显示桌面影响
    QCheckBox* m_clickThroughCheckBox;
    QCheckBox* m_lockedCheckBox;
    QCheckBox* m_autoStartCheckBox;
    QTextEdit* m_customSettingsTextEdit;
    
    // 滑条控件
    QSlider* m_xSlider;
    QSlider* m_ySlider;
    QSlider* m_widthSlider;
    QSlider* m_heightSlider;
    QSlider* m_opacitySlider;
    
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    
    // 状态栏标签
    QLabel* m_statusLabel{nullptr};
    QLabel* m_widgetCountLabel{nullptr};
}; 