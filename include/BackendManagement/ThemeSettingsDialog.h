#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include "Common/Types.h"

class ThemePreviewWidget;

class ThemeSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ThemeSettingsDialog(const WidgetConfig& config, QWidget* parent = nullptr);
    ~ThemeSettingsDialog() = default;

    // 获取更新后的配置
    WidgetConfig getUpdatedConfig() const { return m_config; }

private slots:
    void onThemeSelectionChanged();
    void onScaleModeChanged();
    void onOpacityChanged(int value);
    void onImportCustomImage();
    void onRemoveCustomImage();
    void onPreviewTheme();
    void onApplyTheme();
    void onResetToDefault();
    void onCustomImageSelected();

private:
    void setupUI();
    void setupThemeSelection();
    void setupThemeOptions();
    void setupCustomImageManagement();
    void setupPreviewArea();
    void setupButtons();
    
    void loadAvailableThemes();
    void loadThemeImages();
    void updatePreview();
    void updateThemeOptions();
    void applyCurrentTheme();
    
    QString getCurrentThemeName() const;
    QString getCurrentImageName() const;
    QString getWidgetTypeName() const;

private:
    WidgetConfig m_config;
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // 主题选择区域
    QGroupBox* m_themeSelectionGroup;
    QComboBox* m_themeCombo;
    QListWidget* m_imageList;
    QPushButton* m_previewBtn;
    
    // 主题选项区域
    QGroupBox* m_themeOptionsGroup;
    QComboBox* m_scaleModeCombo;
    QSlider* m_opacitySlider;
    QLabel* m_opacityLabel;
    
    // 自定义图片管理区域
    QGroupBox* m_customImageGroup;
    QPushButton* m_importImageBtn;
    QPushButton* m_removeImageBtn;
    QListWidget* m_customImageList;
    QProgressBar* m_importProgress;
    
    // 预览区域
    QGroupBox* m_previewGroup;
    ThemePreviewWidget* m_previewWidget;
    
    // 按钮区域
    QPushButton* m_applyBtn;
    QPushButton* m_resetBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    // 当前状态
    QString m_currentTheme;
    QString m_currentImage;
    bool m_hasChanges;
};

// 主题预览小组件
class ThemePreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ThemePreviewWidget(QWidget* parent = nullptr);
    
    void setThemePreview(const QString& themeName, const QString& widgetName, const QString& imageName = QString());
    void setScaleMode(const QString& scaleMode);
    void setOpacity(double opacity);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawBackground(QPainter& painter);
    void drawSampleContent(QPainter& painter);

private:
    QString m_themeName;
    QString m_widgetName;
    QString m_imageName;
    QString m_scaleMode;
    double m_opacity;
    QPixmap m_backgroundImage;
}; 