#include "BackendManagement/CreateWidgetDialog.h"
#include <QDateTime>
#include <QMessageBox>
#include <QScreen>
#include <QApplication>

CreateWidgetDialog::CreateWidgetDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("创建新的小组件");
    setModal(true);
    setFixedSize(400, 600);
    
    setupUI();
    
    // 设置默认值
    m_config = WidgetConfig();
    m_config.id = generateUniqueId();
    
    // 连接信号槽
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CreateWidgetDialog::onWidgetTypeChanged);
    connect(m_okButton, &QPushButton::clicked, this, &CreateWidgetDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &CreateWidgetDialog::onReject);
    connect(m_previewButton, &QPushButton::clicked, this, &CreateWidgetDialog::updatePreview);
    
    // 初始化界面
    onWidgetTypeChanged();
}

void CreateWidgetDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 基本信息组
    QGroupBox* basicGroup = new QGroupBox("基本信息");
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    
    m_nameLineEdit = new QLineEdit;
    m_typeComboBox = new QComboBox;
    m_typeComboBox->addItem("时钟", static_cast<int>(WidgetType::Clock));
    m_typeComboBox->addItem("天气", static_cast<int>(WidgetType::Weather));
    m_typeComboBox->addItem("系统信息", static_cast<int>(WidgetType::SystemInfo));
    m_typeComboBox->addItem("日历", static_cast<int>(WidgetType::Calendar));
    m_typeComboBox->addItem("便签", static_cast<int>(WidgetType::Notes));
    m_typeComboBox->addItem("极简便签", static_cast<int>(WidgetType::SimpleNotes));
    m_typeComboBox->addItem("AI排行榜", static_cast<int>(WidgetType::AIRanking));
    m_typeComboBox->addItem("系统性能监测", static_cast<int>(WidgetType::SystemPerformance));
    
    basicLayout->addRow("名称:", m_nameLineEdit);
    basicLayout->addRow("类型:", m_typeComboBox);
    
    // 位置组
    QGroupBox* positionGroup = new QGroupBox("位置设置");
    QFormLayout* positionLayout = new QFormLayout(positionGroup);
    
    m_xSpinBox = new QSpinBox;
    m_xSpinBox->setRange(0, 9999);
    m_xSpinBox->setValue(100);
    
    m_ySpinBox = new QSpinBox;
    m_ySpinBox->setRange(0, 9999);
    m_ySpinBox->setValue(100);
    
    positionLayout->addRow("X坐标:", m_xSpinBox);
    positionLayout->addRow("Y坐标:", m_ySpinBox);
    
    // 大小组
    QGroupBox* sizeGroup = new QGroupBox("大小设置");
    QFormLayout* sizeLayout = new QFormLayout(sizeGroup);
    
    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setRange(Constants::MIN_SIZE, Constants::MAX_SIZE);
    m_widthSpinBox->setValue(200);
    
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setRange(Constants::MIN_SIZE, Constants::MAX_SIZE);
    m_heightSpinBox->setValue(150);
    
    sizeLayout->addRow("宽度:", m_widthSpinBox);
    sizeLayout->addRow("高度:", m_heightSpinBox);
    
    // 显示设置组
    QGroupBox* displayGroup = new QGroupBox("显示设置");
    QFormLayout* displayLayout = new QFormLayout(displayGroup);
    
    m_opacitySpinBox = new QDoubleSpinBox;
    m_opacitySpinBox->setRange(Constants::MIN_OPACITY, Constants::MAX_OPACITY);
    m_opacitySpinBox->setSingleStep(0.1);
    m_opacitySpinBox->setValue(1.0);
    
    m_updateIntervalSpinBox = new QSpinBox;
    m_updateIntervalSpinBox->setRange(100, 60000);
    m_updateIntervalSpinBox->setSuffix(" ms");
    m_updateIntervalSpinBox->setValue(Constants::DEFAULT_UPDATE_INTERVAL);
    
    m_alwaysOnTopCheckBox = new QCheckBox;
    m_alwaysOnTopCheckBox->setChecked(true);
    
    m_clickThroughCheckBox = new QCheckBox;
    m_clickThroughCheckBox->setChecked(false);
    
    m_autoStartCheckBox = new QCheckBox;
    m_autoStartCheckBox->setChecked(false);
    
    displayLayout->addRow("透明度:", m_opacitySpinBox);
    displayLayout->addRow("更新间隔:", m_updateIntervalSpinBox);
    displayLayout->addRow("始终置顶:", m_alwaysOnTopCheckBox);
    displayLayout->addRow("鼠标穿透:", m_clickThroughCheckBox);
    displayLayout->addRow("自动启动:", m_autoStartCheckBox);
    
    // 预览
    m_previewLabel = new QLabel("预览信息将在这里显示");
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setStyleSheet(
        "QLabel { "
        "background-color: #2b2b2b; "
        "color: #ffffff; "
        "padding: 10px; "
        "border: 1px solid #555555; "
        "border-radius: 5px; "
        "font-family: 'Consolas', 'Monaco', monospace; "
        "}"
    );
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_previewButton = new QPushButton("预览");
    m_okButton = new QPushButton("确定");
    m_cancelButton = new QPushButton("取消");
    
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_previewButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 主布局
    mainLayout->addWidget(basicGroup);
    mainLayout->addWidget(positionGroup);
    mainLayout->addWidget(sizeGroup);
    mainLayout->addWidget(displayGroup);
    mainLayout->addWidget(m_previewLabel);
    mainLayout->addLayout(buttonLayout);
}

void CreateWidgetDialog::onWidgetTypeChanged() {
    WidgetType type = static_cast<WidgetType>(m_typeComboBox->currentData().toInt());
    
    // 根据Widget类型设置默认值
    switch (type) {
        case WidgetType::Clock:
            m_nameLineEdit->setText("时钟");
            m_widthSpinBox->setValue(200);
            m_heightSpinBox->setValue(100);
            m_updateIntervalSpinBox->setValue(1000);
            break;
        case WidgetType::Weather:
            m_nameLineEdit->setText("天气");
            m_widthSpinBox->setValue(250);
            m_heightSpinBox->setValue(150);
            m_updateIntervalSpinBox->setValue(300000); // 5分钟
            break;
        case WidgetType::SystemInfo:
            m_nameLineEdit->setText("系统信息");
            m_widthSpinBox->setValue(300);
            m_heightSpinBox->setValue(200);
            m_updateIntervalSpinBox->setValue(2000);
            break;
        case WidgetType::Calendar:
            m_nameLineEdit->setText("日历");
            m_widthSpinBox->setValue(250);
            m_heightSpinBox->setValue(200);
            m_updateIntervalSpinBox->setValue(60000); // 1分钟
            break;
        case WidgetType::Notes:
            m_nameLineEdit->setText("便签");
            m_widthSpinBox->setValue(400);
            m_heightSpinBox->setValue(300);
            m_updateIntervalSpinBox->setValue(0); // 不需要自动更新
            break;
        case WidgetType::SimpleNotes:
            m_nameLineEdit->setText("极简便签");
            m_widthSpinBox->setValue(250);
            m_heightSpinBox->setValue(200);
            m_updateIntervalSpinBox->setValue(0); // 不需要自动更新
            break;
        case WidgetType::AIRanking:
            m_nameLineEdit->setText("AI排行榜");
            m_widthSpinBox->setValue(400);
            m_heightSpinBox->setValue(300);
            m_updateIntervalSpinBox->setValue(1000); // 1秒更新间隔
            break;
        case WidgetType::SystemPerformance:
            m_nameLineEdit->setText("系统性能监测");
            m_widthSpinBox->setValue(280);
            m_heightSpinBox->setValue(220);
            m_updateIntervalSpinBox->setValue(2000); // 2秒更新间隔
            break;
        default:
            m_nameLineEdit->setText("自定义组件");
            break;
    }
    
    updatePreview();
}

void CreateWidgetDialog::onAccept() {
    // 验证输入
    if (m_nameLineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入组件名称！");
        m_nameLineEdit->setFocus();
        return;
    }
    
    // 更新配置
    m_config.id = generateUniqueId();
    m_config.name = m_nameLineEdit->text().trimmed();
    m_config.type = static_cast<WidgetType>(m_typeComboBox->currentData().toInt());
    m_config.position = QPoint(m_xSpinBox->value(), m_ySpinBox->value());
    m_config.size = QSize(m_widthSpinBox->value(), m_heightSpinBox->value());
    m_config.opacity = m_opacitySpinBox->value();
    m_config.updateInterval = m_updateIntervalSpinBox->value();
    m_config.alwaysOnTop = m_alwaysOnTopCheckBox->isChecked();
    m_config.clickThrough = m_clickThroughCheckBox->isChecked();
    m_config.autoStart = m_autoStartCheckBox->isChecked();
    
    accept();
}

void CreateWidgetDialog::onReject() {
    reject();
}

void CreateWidgetDialog::updatePreview() {
    QString preview = QString(
        "组件预览:\n"
        "名称: %1\n"
        "类型: %2\n"
        "位置: (%3, %4)\n"
        "大小: %5 x %6\n"
        "透明度: %7\n"
        "更新间隔: %8 ms\n"
        "始终置顶: %9\n"
        "鼠标穿透: %10\n"
        "自动启动: %11"
    ).arg(m_nameLineEdit->text())
     .arg(m_typeComboBox->currentText())
     .arg(m_xSpinBox->value())
     .arg(m_ySpinBox->value())
     .arg(m_widthSpinBox->value())
     .arg(m_heightSpinBox->value())
     .arg(m_opacitySpinBox->value())
     .arg(m_updateIntervalSpinBox->value())
     .arg(m_alwaysOnTopCheckBox->isChecked() ? "是" : "否")
     .arg(m_clickThroughCheckBox->isChecked() ? "是" : "否")
     .arg(m_autoStartCheckBox->isChecked() ? "是" : "否");
    
    m_previewLabel->setText(preview);
}

WidgetConfig CreateWidgetDialog::getWidgetConfig() const {
    return m_config;
}

void CreateWidgetDialog::setWidgetConfig(const WidgetConfig& config) {
    m_config = config;
    
    // 更新界面
    m_nameLineEdit->setText(config.name);
    
    // 设置类型下拉框
    for (int i = 0; i < m_typeComboBox->count(); ++i) {
        if (static_cast<WidgetType>(m_typeComboBox->itemData(i).toInt()) == config.type) {
            m_typeComboBox->setCurrentIndex(i);
            break;
        }
    }
    
    m_xSpinBox->setValue(config.position.x());
    m_ySpinBox->setValue(config.position.y());
    m_widthSpinBox->setValue(config.size.width());
    m_heightSpinBox->setValue(config.size.height());
    m_opacitySpinBox->setValue(config.opacity);
    m_updateIntervalSpinBox->setValue(config.updateInterval);
    m_alwaysOnTopCheckBox->setChecked(config.alwaysOnTop);
    m_clickThroughCheckBox->setChecked(config.clickThrough);
    m_autoStartCheckBox->setChecked(config.autoStart);
    
    updatePreview();
}

QString CreateWidgetDialog::generateUniqueId() const {
    return QString("widget_%1").arg(QDateTime::currentMSecsSinceEpoch());
} 