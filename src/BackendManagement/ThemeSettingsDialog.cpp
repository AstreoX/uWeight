#include "BackendManagement/ThemeSettingsDialog.h"
#include "Utils/ThemeResourceManager.h"
#include "Utils/ThemeManager.h"
#include <QPainter>
#include <QDateTime>
#include <QApplication>
#include <QThread>
#include <QDebug>

ThemeSettingsDialog::ThemeSettingsDialog(const WidgetConfig& config, QWidget* parent)
    : QDialog(parent)
    , m_config(config)
    , m_hasChanges(false)
{
    setWindowTitle("主题设置 - " + config.name);
    setMinimumSize(800, 600);
    setModal(true);
    
    setupUI();
    loadAvailableThemes();
    updatePreview();
}

void ThemeSettingsDialog::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    
    setupThemeSelection();
    setupThemeOptions();
    setupCustomImageManagement();
    setupPreviewArea();
    setupButtons();
    
    // 连接信号
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ThemeSettingsDialog::onThemeSelectionChanged);
    connect(m_imageList, &QListWidget::currentItemChanged,
            this, &ThemeSettingsDialog::onThemeSelectionChanged);
    connect(m_scaleModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ThemeSettingsDialog::onScaleModeChanged);
    connect(m_opacitySlider, &QSlider::valueChanged,
            this, &ThemeSettingsDialog::onOpacityChanged);
    connect(m_importImageBtn, &QPushButton::clicked,
            this, &ThemeSettingsDialog::onImportCustomImage);
    connect(m_removeImageBtn, &QPushButton::clicked,
            this, &ThemeSettingsDialog::onRemoveCustomImage);
    connect(m_customImageList, &QListWidget::currentItemChanged,
            this, &ThemeSettingsDialog::onCustomImageSelected);
    connect(m_previewBtn, &QPushButton::clicked,
            this, &ThemeSettingsDialog::onPreviewTheme);
    connect(m_applyBtn, &QPushButton::clicked,
            this, &ThemeSettingsDialog::onApplyTheme);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &ThemeSettingsDialog::onResetToDefault);
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ThemeSettingsDialog::setupThemeSelection() {
    m_themeSelectionGroup = new QGroupBox("主题选择");
    QVBoxLayout* layout = new QVBoxLayout(m_themeSelectionGroup);
    
    // 主题下拉框
    QHBoxLayout* themeLayout = new QHBoxLayout;
    themeLayout->addWidget(new QLabel("主题:"));
    m_themeCombo = new QComboBox;
    m_themeCombo->setMinimumWidth(200);
    themeLayout->addWidget(m_themeCombo);
    themeLayout->addStretch();
    
    m_previewBtn = new QPushButton("预览主题");
    themeLayout->addWidget(m_previewBtn);
    
    layout->addLayout(themeLayout);
    
    // 图片列表
    layout->addWidget(new QLabel("可用背景图片:"));
    m_imageList = new QListWidget;
    m_imageList->setMaximumHeight(120);
    m_imageList->setViewMode(QListView::IconMode);
    m_imageList->setIconSize(QSize(80, 60));
    m_imageList->setResizeMode(QListView::Adjust);
    layout->addWidget(m_imageList);
    
    m_mainLayout->addWidget(m_themeSelectionGroup);
}

void ThemeSettingsDialog::setupThemeOptions() {
    m_themeOptionsGroup = new QGroupBox("主题选项");
    QGridLayout* layout = new QGridLayout(m_themeOptionsGroup);
    
    // 缩放模式
    layout->addWidget(new QLabel("缩放模式:"), 0, 0);
    m_scaleModeCombo = new QComboBox;
    m_scaleModeCombo->addItem("拉伸填充", "stretch");
    m_scaleModeCombo->addItem("保持宽高比", "keepAspectRatio");
    m_scaleModeCombo->addItem("保持宽高比并裁剪", "keepAspectRatioByExpanding");
    m_scaleModeCombo->addItem("居中显示", "center");
    m_scaleModeCombo->addItem("平铺", "tile");
    layout->addWidget(m_scaleModeCombo, 0, 1);
    
    // 透明度
    layout->addWidget(new QLabel("透明度:"), 1, 0);
    QHBoxLayout* opacityLayout = new QHBoxLayout;
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(80);
    opacityLayout->addWidget(m_opacitySlider);
    
    m_opacityLabel = new QLabel("80%");
    m_opacityLabel->setMinimumWidth(40);
    opacityLayout->addWidget(m_opacityLabel);
    
    layout->addLayout(opacityLayout, 1, 1);
    
    m_mainLayout->addWidget(m_themeOptionsGroup);
}

void ThemeSettingsDialog::setupCustomImageManagement() {
    m_customImageGroup = new QGroupBox("自定义图片管理");
    QVBoxLayout* layout = new QVBoxLayout(m_customImageGroup);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_importImageBtn = new QPushButton("导入图片");
    m_removeImageBtn = new QPushButton("删除图片");
    m_removeImageBtn->setEnabled(false);
    
    buttonLayout->addWidget(m_importImageBtn);
    buttonLayout->addWidget(m_removeImageBtn);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    // 进度条
    m_importProgress = new QProgressBar;
    m_importProgress->setVisible(false);
    layout->addWidget(m_importProgress);
    
    // 自定义图片列表
    layout->addWidget(new QLabel("自定义图片:"));
    m_customImageList = new QListWidget;
    m_customImageList->setMaximumHeight(100);
    m_customImageList->setViewMode(QListView::IconMode);
    m_customImageList->setIconSize(QSize(60, 45));
    m_customImageList->setResizeMode(QListView::Adjust);
    layout->addWidget(m_customImageList);
    
    m_mainLayout->addWidget(m_customImageGroup);
}

void ThemeSettingsDialog::setupPreviewArea() {
    m_previewGroup = new QGroupBox("预览");
    QVBoxLayout* layout = new QVBoxLayout(m_previewGroup);
    
    m_previewWidget = new ThemePreviewWidget;
    m_previewWidget->setMinimumSize(300, 150);
    layout->addWidget(m_previewWidget);
    
    m_mainLayout->addWidget(m_previewGroup);
}

void ThemeSettingsDialog::setupButtons() {
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    m_applyBtn = new QPushButton("应用");
    m_resetBtn = new QPushButton("重置");
    m_okBtn = new QPushButton("确定");
    m_cancelBtn = new QPushButton("取消");
    
    buttonLayout->addWidget(m_applyBtn);
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addLayout(buttonLayout);
}

void ThemeSettingsDialog::loadAvailableThemes() {
    m_themeCombo->clear();
    
    ThemeResourceManager& resourceManager = ThemeResourceManager::instance();
    QStringList themes = resourceManager.getAvailableThemes();
    
    for (const QString& theme : themes) {
        QString displayName = theme;
        if (theme == "nature") displayName = "自然主题";
        else if (theme == "city") displayName = "城市主题";
        else if (theme == "space") displayName = "太空主题";
        else if (theme == "minimal") displayName = "简约主题";
        else if (theme == "gradient") displayName = "渐变主题";
        else if (theme == "custom") displayName = "自定义主题";
        
        m_themeCombo->addItem(displayName, theme);
    }
    
    // 设置当前主题
    QString currentTheme = m_config.customSettings.contains("currentTheme") ? 
                          m_config.customSettings.value("currentTheme").toString() : "minimal";
    int index = m_themeCombo->findData(currentTheme);
    if (index >= 0) {
        m_themeCombo->setCurrentIndex(index);
    }
    
    loadThemeImages();
}

void ThemeSettingsDialog::loadThemeImages() {
    m_imageList->clear();
    m_customImageList->clear();
    
    QString themeName = getCurrentThemeName();
    QString widgetName = getWidgetTypeName();
    
    ThemeResourceManager& resourceManager = ThemeResourceManager::instance();
    
    // 加载主题图片
    QStringList images = resourceManager.getThemeImages(themeName, widgetName);
    for (const QString& imageName : images) {
        QPixmap preview = resourceManager.getThemePreview(themeName, widgetName, QSize(80, 60));
        
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(imageName);
        item->setIcon(QIcon(preview));
        item->setData(Qt::UserRole, imageName);
        m_imageList->addItem(item);
    }
    
    // 加载自定义图片
    if (themeName == "custom") {
        QStringList customImages = resourceManager.getCustomImages(widgetName);
        for (const QString& imageName : customImages) {
            QPixmap preview = resourceManager.getThemePreview("custom", widgetName, QSize(60, 45));
            
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(imageName);
            item->setIcon(QIcon(preview));
            item->setData(Qt::UserRole, imageName);
            m_customImageList->addItem(item);
        }
    }
    
    // 选择当前图片
    QString currentImage = m_config.customSettings.value("backgroundImagePath").toString();
    if (!currentImage.isEmpty()) {
        QFileInfo fileInfo(currentImage);
        QString imageName = fileInfo.fileName();
        
        for (int i = 0; i < m_imageList->count(); ++i) {
            if (m_imageList->item(i)->data(Qt::UserRole).toString() == imageName) {
                m_imageList->setCurrentRow(i);
                break;
            }
        }
    }
}

void ThemeSettingsDialog::onThemeSelectionChanged() {
    loadThemeImages();
    updatePreview();
    m_hasChanges = true;
}

void ThemeSettingsDialog::onScaleModeChanged() {
    updatePreview();
    m_hasChanges = true;
}

void ThemeSettingsDialog::onOpacityChanged(int value) {
    m_opacityLabel->setText(QString("%1%").arg(value));
    updatePreview();
    m_hasChanges = true;
}

void ThemeSettingsDialog::onImportCustomImage() {
    QString imagePath = QFileDialog::getOpenFileName(
        this,
        "选择背景图片",
        "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"
    );
    
    if (imagePath.isEmpty()) {
        return;
    }
    
    m_importProgress->setVisible(true);
    m_importProgress->setRange(0, 0); // 不确定进度
    
    ThemeResourceManager& resourceManager = ThemeResourceManager::instance();
    QString widgetName = getWidgetTypeName();
    
    QString importedName = resourceManager.importCustomImage(imagePath, widgetName);
    
    m_importProgress->setVisible(false);
    
    if (!importedName.isEmpty()) {
        QMessageBox::information(this, "成功", "图片导入成功: " + importedName);
        
        // 切换到自定义主题
        int customIndex = m_themeCombo->findData("custom");
        if (customIndex >= 0) {
            m_themeCombo->setCurrentIndex(customIndex);
        }
        
        loadThemeImages();
    } else {
        QMessageBox::warning(this, "错误", "图片导入失败");
    }
}

void ThemeSettingsDialog::onRemoveCustomImage() {
    QListWidgetItem* currentItem = m_customImageList->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString imageName = currentItem->data(Qt::UserRole).toString();
    
    int ret = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除图片 \"" + imageName + "\" 吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        ThemeResourceManager& resourceManager = ThemeResourceManager::instance();
        QString widgetName = getWidgetTypeName();
        
        if (resourceManager.removeCustomImage(widgetName, imageName)) {
            QMessageBox::information(this, "成功", "图片删除成功");
            loadThemeImages();
        } else {
            QMessageBox::warning(this, "错误", "图片删除失败");
        }
    }
}

void ThemeSettingsDialog::onCustomImageSelected() {
    QListWidgetItem* item = m_customImageList->currentItem();
    m_removeImageBtn->setEnabled(item != nullptr);
    
    if (item) {
        // 清除主题图片选择
        m_imageList->clearSelection();
        updatePreview();
    }
}

void ThemeSettingsDialog::onPreviewTheme() {
    updatePreview();
}

void ThemeSettingsDialog::onApplyTheme() {
    applyCurrentTheme();
    m_hasChanges = false;
    QMessageBox::information(this, "成功", "主题已应用");
}

void ThemeSettingsDialog::onResetToDefault() {
    // 重置为默认设置
    m_themeCombo->setCurrentIndex(0);
    m_scaleModeCombo->setCurrentIndex(0);
    m_opacitySlider->setValue(80);
    
    updatePreview();
    m_hasChanges = true;
}

void ThemeSettingsDialog::updatePreview() {
    QString themeName = getCurrentThemeName();
    QString imageName = getCurrentImageName();
    QString widgetName = getWidgetTypeName();
    
    m_previewWidget->setThemePreview(themeName, widgetName, imageName);
    m_previewWidget->setScaleMode(m_scaleModeCombo->currentData().toString());
    m_previewWidget->setOpacity(m_opacitySlider->value() / 100.0);
}

void ThemeSettingsDialog::applyCurrentTheme() {
    QString themeName = getCurrentThemeName();
    QString imageName = getCurrentImageName();
    
    // 更新配置
    m_config.customSettings["useBackgroundImage"] = !imageName.isEmpty();
    
    if (!imageName.isEmpty()) {
        ThemeResourceManager& resourceManager = ThemeResourceManager::instance();
        QString relativePath = resourceManager.getRelativeImagePath(themeName, getWidgetTypeName(), imageName);
        m_config.customSettings["backgroundImagePath"] = relativePath;
    } else {
        m_config.customSettings.remove("backgroundImagePath");
    }
    
    m_config.customSettings["backgroundScaleMode"] = m_scaleModeCombo->currentData().toString();
    m_config.customSettings["backgroundOpacity"] = m_opacitySlider->value() / 100.0;
    m_config.customSettings["currentTheme"] = themeName;
}

QString ThemeSettingsDialog::getCurrentThemeName() const {
    return m_themeCombo->currentData().toString();
}

QString ThemeSettingsDialog::getCurrentImageName() const {
    // 优先检查自定义图片选择
    QListWidgetItem* customItem = m_customImageList->currentItem();
    if (customItem) {
        return customItem->data(Qt::UserRole).toString();
    }
    
    // 检查主题图片选择
    QListWidgetItem* themeItem = m_imageList->currentItem();
    if (themeItem) {
        return themeItem->data(Qt::UserRole).toString();
    }
    
    return QString();
}

QString ThemeSettingsDialog::getWidgetTypeName() const {
    switch (m_config.type) {
        case WidgetType::Clock:
            return "ClockWidget";
        case WidgetType::Weather:
            return "WeatherWidget";
        case WidgetType::Calendar:
            return "CalendarWidget";
        default:
            return "ClockWidget";
    }
}

// ThemePreviewWidget 实现
ThemePreviewWidget::ThemePreviewWidget(QWidget* parent)
    : QWidget(parent)
    , m_scaleMode("stretch")
    , m_opacity(0.8)
{
    setMinimumSize(300, 150);
}

void ThemePreviewWidget::setThemePreview(const QString& themeName, const QString& widgetName, const QString& imageName) {
    m_themeName = themeName;
    m_widgetName = widgetName;
    m_imageName = imageName;
    
    // 加载背景图片
    if (!imageName.isEmpty()) {
        ThemeResourceManager& resourceManager = ThemeResourceManager::instance();
        QString imagePath = resourceManager.getThemeImagePath(themeName, widgetName) + "/" + imageName;
        m_backgroundImage = QPixmap(imagePath);
    } else {
        m_backgroundImage = QPixmap();
    }
    
    update();
}

void ThemePreviewWidget::setScaleMode(const QString& scaleMode) {
    m_scaleMode = scaleMode;
    update();
}

void ThemePreviewWidget::setOpacity(double opacity) {
    m_opacity = opacity;
    update();
}

void ThemePreviewWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    drawBackground(painter);
    drawSampleContent(painter);
}

void ThemePreviewWidget::drawBackground(QPainter& painter) {
    if (m_backgroundImage.isNull()) {
        // 绘制默认背景
        painter.fillRect(rect(), QColor(64, 64, 64));
        return;
    }
    
    painter.setOpacity(m_opacity);
    
    QRect targetRect = rect();
    
    if (m_scaleMode == "stretch") {
        QPixmap scaledImage = m_backgroundImage.scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(targetRect, scaledImage);
    } else if (m_scaleMode == "keepAspectRatio") {
        QPixmap scaledImage = m_backgroundImage.scaled(targetRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect imageRect = scaledImage.rect();
        imageRect.moveCenter(targetRect.center());
        painter.drawPixmap(imageRect, scaledImage);
    } else if (m_scaleMode == "keepAspectRatioByExpanding") {
        QPixmap scaledImage = m_backgroundImage.scaled(targetRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QRect imageRect = scaledImage.rect();
        imageRect.moveCenter(targetRect.center());
        painter.drawPixmap(targetRect, scaledImage, imageRect);
    } else if (m_scaleMode == "center") {
        QRect imageRect = m_backgroundImage.rect();
        imageRect.moveCenter(targetRect.center());
        painter.drawPixmap(imageRect, m_backgroundImage);
    } else if (m_scaleMode == "tile") {
        QSize imageSize = m_backgroundImage.size();
        for (int x = 0; x < targetRect.width(); x += imageSize.width()) {
            for (int y = 0; y < targetRect.height(); y += imageSize.height()) {
                QRect tileRect(x, y, imageSize.width(), imageSize.height());
                tileRect = tileRect.intersected(targetRect);
                if (!tileRect.isEmpty()) {
                    painter.drawPixmap(tileRect, m_backgroundImage, 
                        QRect(0, 0, tileRect.width(), tileRect.height()));
                }
            }
        }
    }
    
    painter.setOpacity(1.0);
}

void ThemePreviewWidget::drawSampleContent(QPainter& painter) {
    // 绘制示例时钟内容
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeText = currentTime.toString("hh:mm:ss");
    QString dateText = currentTime.toString("yyyy-MM-dd");
    
    QRect timeRect = rect();
    timeRect.setHeight(rect().height() * 0.6);
    
    QRect dateRect = rect();
    dateRect.setTop(timeRect.bottom());
    
    painter.drawText(timeRect, Qt::AlignCenter, timeText);
    
    painter.setFont(QFont("Arial", 12));
    painter.drawText(dateRect, Qt::AlignCenter, dateText);
} 