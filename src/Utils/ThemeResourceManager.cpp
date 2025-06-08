#include "Utils/ThemeResourceManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QUuid>

ThemeResourceManager& ThemeResourceManager::instance() {
    static ThemeResourceManager instance;
    return instance;
}

ThemeResourceManager::ThemeResourceManager(QObject* parent) : QObject(parent) {
    // 设置主题资源路径为应用程序目录下的theme_source
    m_themeResourcePath = QApplication::applicationDirPath() + "/theme_source";
    
    // 默认主题列表
    m_defaultThemes << "nature" << "city" << "space" << "minimal" << "gradient" << "custom";
    
    // 主题描述
    m_themeDescriptions["nature"] = "自然主题";
    m_themeDescriptions["city"] = "城市主题";
    m_themeDescriptions["space"] = "太空主题";
    m_themeDescriptions["minimal"] = "简约主题";
    m_themeDescriptions["gradient"] = "渐变主题";
    m_themeDescriptions["custom"] = "自定义主题";
    
    initializeThemeDirectories();
}

void ThemeResourceManager::initializeThemeDirectories() {
    QDir baseDir(m_themeResourcePath);
    if (!baseDir.exists()) {
        baseDir.mkpath(".");
        qDebug() << "创建主题资源目录:" << m_themeResourcePath;
    }
    
    createDefaultThemeStructure();
    createSampleImages();
}

void ThemeResourceManager::createDefaultThemeStructure() {
    for (const QString& theme : m_defaultThemes) {
        createThemeDirectory(theme);
        createWidgetDirectory(theme, "ClockWidget");
        // 为将来的其他小组件预留目录
        // createWidgetDirectory(theme, "WeatherWidget");
        // createWidgetDirectory(theme, "CalendarWidget");
    }
}

void ThemeResourceManager::createSampleImages() {
    // 为每个主题创建示例图片（如果不存在）
    QMap<QString, QColor> themeColors;
    themeColors["nature"] = QColor(34, 139, 34);    // 森林绿
    themeColors["city"] = QColor(70, 130, 180);     // 钢蓝色
    themeColors["space"] = QColor(25, 25, 112);     // 午夜蓝
    themeColors["minimal"] = QColor(128, 128, 128); // 灰色
    themeColors["gradient"] = QColor(255, 165, 0);  // 橙色
    
    for (auto it = themeColors.begin(); it != themeColors.end(); ++it) {
        QString themeName = it.key();
        QColor color = it.value();
        
        QString imagePath = getThemeImagePath(themeName, "ClockWidget") + "/background.png";
        QFileInfo imageFile(imagePath);
        
        if (!imageFile.exists()) {
            // 创建示例背景图片
            QPixmap sampleImage(400, 200);
            sampleImage.fill(color);
            
            QPainter painter(&sampleImage);
            painter.setRenderHint(QPainter::Antialiasing);
            
            // 添加渐变效果
            QLinearGradient gradient(0, 0, 400, 200);
            gradient.setColorAt(0, color.lighter(120));
            gradient.setColorAt(1, color.darker(120));
            painter.fillRect(sampleImage.rect(), gradient);
            
            // 添加主题名称
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 24, QFont::Bold));
            painter.drawText(sampleImage.rect(), Qt::AlignCenter, m_themeDescriptions[themeName]);
            
            // 保存图片
            QDir().mkpath(imageFile.absolutePath());
            sampleImage.save(imagePath);
            qDebug() << "创建示例图片:" << imagePath;
        }
    }
}

QString ThemeResourceManager::getThemeImagePath(const QString& themeName, const QString& widgetName) const {
    return QString("%1/%2/%3").arg(m_themeResourcePath, themeName, widgetName);
}

QString ThemeResourceManager::getCustomThemePath() const {
    return getThemeImagePath("custom", "");
}

QStringList ThemeResourceManager::getAvailableThemes() const {
    QDir themeDir(m_themeResourcePath);
    QStringList themes = themeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    return themes;
}

QStringList ThemeResourceManager::getThemeImages(const QString& themeName, const QString& widgetName) const {
    QString themePath = getThemeImagePath(themeName, widgetName);
    QDir themeDir(themePath);
    
    QStringList nameFilters;
    nameFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif";
    
    return themeDir.entryList(nameFilters, QDir::Files);
}

bool ThemeResourceManager::hasThemeImage(const QString& themeName, const QString& widgetName) const {
    QStringList images = getThemeImages(themeName, widgetName);
    return !images.isEmpty();
}

QString ThemeResourceManager::importCustomImage(const QString& sourceImagePath, const QString& widgetName, const QString& customName) {
    QFileInfo sourceFile(sourceImagePath);
    if (!sourceFile.exists() || !sourceFile.isFile()) {
        qWarning() << "源图片文件不存在:" << sourceImagePath;
        return QString();
    }
    
    // 确保自定义主题目录存在
    QString customWidgetPath = getThemeImagePath("custom", widgetName);
    QDir().mkpath(customWidgetPath);
    
    // 生成目标文件名
    QString targetFileName;
    if (customName.isEmpty()) {
        targetFileName = generateUniqueImageName(sourceFile.baseName(), customWidgetPath);
    } else {
        targetFileName = customName;
    }
    
    // 确保有正确的扩展名
    if (!targetFileName.contains('.')) {
        targetFileName += "." + sourceFile.suffix();
    }
    
    QString targetPath = customWidgetPath + "/" + targetFileName;
    
    // 复制文件
    if (QFile::copy(sourceImagePath, targetPath)) {
        qDebug() << "成功导入自定义图片:" << targetPath;
        emit customImageImported(widgetName, targetFileName);
        return targetFileName;
    } else {
        qWarning() << "导入自定义图片失败:" << sourceImagePath << "到" << targetPath;
        return QString();
    }
}

bool ThemeResourceManager::removeCustomImage(const QString& widgetName, const QString& imageName) {
    QString imagePath = getThemeImagePath("custom", widgetName) + "/" + imageName;
    QFile imageFile(imagePath);
    
    if (imageFile.exists() && imageFile.remove()) {
        qDebug() << "删除自定义图片:" << imagePath;
        emit themeResourceRemoved("custom", widgetName, imageName);
        return true;
    }
    
    qWarning() << "删除自定义图片失败:" << imagePath;
    return false;
}

QStringList ThemeResourceManager::getCustomImages(const QString& widgetName) const {
    return getThemeImages("custom", widgetName);
}

QPixmap ThemeResourceManager::getThemePreview(const QString& themeName, const QString& widgetName, const QSize& size) const {
    QStringList images = getThemeImages(themeName, widgetName);
    if (images.isEmpty()) {
        // 创建默认预览图
        QPixmap preview(size);
        preview.fill(Qt::lightGray);
        
        QPainter painter(&preview);
        painter.setPen(Qt::darkGray);
        painter.setFont(QFont("Arial", 12));
        painter.drawText(preview.rect(), Qt::AlignCenter, "无背景图片");
        
        return preview;
    }
    
    // 使用第一张图片作为预览
    QString imagePath = getThemeImagePath(themeName, widgetName) + "/" + images.first();
    QPixmap originalImage(imagePath);
    
    if (originalImage.isNull()) {
        QPixmap preview(size);
        preview.fill(Qt::red);
        return preview;
    }
    
    return originalImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

bool ThemeResourceManager::copyImageToTheme(const QString& sourceImagePath, const QString& themeName, const QString& widgetName, const QString& imageName) {
    QString targetDir = getThemeImagePath(themeName, widgetName);
    QDir().mkpath(targetDir);
    
    QString targetPath = targetDir + "/" + imageName;
    
    // 如果目标文件已存在，先删除
    if (QFile::exists(targetPath)) {
        QFile::remove(targetPath);
    }
    
    if (QFile::copy(sourceImagePath, targetPath)) {
        emit themeResourceAdded(themeName, widgetName, imageName);
        return true;
    }
    
    return false;
}

bool ThemeResourceManager::createThemeDirectory(const QString& themeName) {
    QString themePath = m_themeResourcePath + "/" + themeName;
    QDir themeDir;
    
    if (themeDir.mkpath(themePath)) {
        qDebug() << "创建主题目录:" << themePath;
        return true;
    }
    
    return false;
}

bool ThemeResourceManager::createWidgetDirectory(const QString& themeName, const QString& widgetName) {
    QString widgetPath = getThemeImagePath(themeName, widgetName);
    QDir widgetDir;
    
    if (widgetDir.mkpath(widgetPath)) {
        qDebug() << "创建小组件目录:" << widgetPath;
        return true;
    }
    
    return false;
}

bool ThemeResourceManager::validateThemeStructure() const {
    QDir baseDir(m_themeResourcePath);
    if (!baseDir.exists()) {
        return false;
    }
    
    for (const QString& theme : m_defaultThemes) {
        QString themePath = m_themeResourcePath + "/" + theme;
        if (!QDir(themePath).exists()) {
            qWarning() << "主题目录不存在:" << themePath;
            return false;
        }
    }
    
    return true;
}

void ThemeResourceManager::cleanupEmptyDirectories() {
    QDir baseDir(m_themeResourcePath);
    QStringList themes = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString& theme : themes) {
        QString themePath = m_themeResourcePath + "/" + theme;
        QDir themeDir(themePath);
        
        QStringList widgets = themeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& widget : widgets) {
            QString widgetPath = themePath + "/" + widget;
            QDir widgetDir(widgetPath);
            
            // 如果小组件目录为空，删除它
            if (widgetDir.entryList(QDir::Files).isEmpty()) {
                widgetDir.removeRecursively();
                qDebug() << "删除空的小组件目录:" << widgetPath;
            }
        }
        
        // 如果主题目录为空，删除它（除非是默认主题）
        if (themeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty() && 
            !m_defaultThemes.contains(theme)) {
            themeDir.removeRecursively();
            qDebug() << "删除空的主题目录:" << themePath;
        }
    }
}

QString ThemeResourceManager::getRelativeImagePath(const QString& themeName, const QString& widgetName, const QString& imageName) const {
    return QString("theme_source/%1/%2/%3").arg(themeName, widgetName, imageName);
}

QString ThemeResourceManager::generateUniqueImageName(const QString& originalName, const QString& targetDir) const {
    QString baseName = originalName;
    QString extension = "";
    
    int dotIndex = originalName.lastIndexOf('.');
    if (dotIndex > 0) {
        baseName = originalName.left(dotIndex);
        extension = originalName.mid(dotIndex);
    }
    
    QString uniqueName = originalName;
    int counter = 1;
    
    while (QFile::exists(targetDir + "/" + uniqueName)) {
        uniqueName = QString("%1_%2%3").arg(baseName).arg(counter).arg(extension);
        counter++;
    }
    
    return uniqueName;
} 