#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDateTime>
#include "Common/Types.h"

class CreateWidgetDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateWidgetDialog(QWidget* parent = nullptr);
    
    WidgetConfig getWidgetConfig() const;
    void setWidgetConfig(const WidgetConfig& config);

private slots:
    void onWidgetTypeChanged();
    void onAccept();
    void onReject();

private:
    void setupUI();
    void updatePreview();
    QString generateUniqueId() const;

private:
    // 基本设置
    QLineEdit* m_nameLineEdit;
    QComboBox* m_typeComboBox;
    
    // 位置设置
    QSpinBox* m_xSpinBox;
    QSpinBox* m_ySpinBox;
    
    // 大小设置
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
    
    // 显示设置
    QDoubleSpinBox* m_opacitySpinBox;
    QSpinBox* m_updateIntervalSpinBox;
    QCheckBox* m_alwaysOnTopCheckBox;
    QCheckBox* m_clickThroughCheckBox;
    QCheckBox* m_autoStartCheckBox;
    
    // 按钮
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_previewButton;
    
    // 预览标签
    QLabel* m_previewLabel;
    
    WidgetConfig m_config;
}; 