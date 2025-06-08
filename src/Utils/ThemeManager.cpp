/**
 * @file ThemeManager.cpp
 * @brief 小组件主题系统管理器实现
 * @details 提供主题的加载、保存、应用和管理功能
 * @author 李佩文 (PAPAPIG-SUN)
 * @date 2025-5
 * @version 1.0.0
 * 
 * 主题系统核心功能：
 * - 主题文件的加载和保存（JSON格式）
 * - 内置默认主题集合（自然、城市、太空等）
 * - 主题配置的验证和合并
 * - 动态主题切换支持
 * - 自定义主题创建和管理
 * - 主题资源路径管理
 * - 主题兼容性检查
 * 
 * 支持的主题元素：
 * - 背景图片和颜色
 * - 文字颜色和字体
 * - 透明度和特效
 * - 布局和尺寸参数
 * - 动画和过渡效果
 * 
 * 设计特点：
 * - 单例模式确保全局主题一致性
 * - JSON配置格式便于扩展和维护
 * - 完整的错误处理和验证机制
 * - 支持热更新和实时预览
 */

#include "Utils/ThemeManager.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QDebug>

/**
 * @brief 获取ThemeManager单例实例
 * @return ThemeManager的全局唯一实例引用
 * 
 * 使用Meyers Singleton模式实现线程安全的单例：
 * - 保证全局只有一个主题管理器实例
 * - 延迟初始化，提高启动性能
 * - 线程安全的实例创建
 */
ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

/**
 * @brief ThemeManager构造函数
 * @param parent 父对象指针
 * 
 * 初始化主题管理系统：
 * - 加载内置默认主题
 * - 初始化主题容器
 * - 设置默认配置参数
 */
ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
    // 加载系统预设的默认主题
    loadDefaultThemes();
}

/**
 * @brief 从文件加载主题配置
 * @param filePath 主题文件路径
 * @return 加载成功返回true，失败返回false
 * 
 * 主题文件加载流程：
 * 1. 验证文件存在性和可读性
 * 2. 解析JSON格式的主题配置
 * 3. 验证主题配置的完整性
 * 4. 加载有效主题到内存
 * 5. 跳过无效主题并记录错误
 * 
 * JSON文件格式：
 * {
 *   "themes": {
 *     "themeId": {
 *       "name": "主题名称",
 *       "description": "主题描述", 
 *       "settings": { /* 主题配置 */ }
 *     }
 *   }
 * }
 * 
 * 错误处理：
 * - 文件无法打开：记录警告并返回false
 * - JSON解析错误：记录错误详情并返回false
 * - 格式错误：检查必需字段并报告错误
 * - 主题验证失败：跳过无效主题继续处理
 */
bool ThemeManager::loadThemesFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开主题文件:" << filePath;
        return false;
    }
    
    // 解析JSON文档
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "主题文件JSON解析错误:" << error.errorString();
        return false;
    }
    
    // 验证JSON格式
    QJsonObject root = doc.object();
    if (!root.contains("themes")) {
        qWarning() << "主题文件格式错误：缺少themes字段";
        return false;
    }
    
    // 解析各个主题配置
    QJsonObject themes = root["themes"].toObject();
    for (auto it = themes.begin(); it != themes.end(); ++it) {
        QString themeId = it.key();
        QJsonObject themeObj = it.value().toObject();
        
        // 构建主题设置对象
        ThemeSettings theme;
        theme.name = themeObj["name"].toString();
        theme.description = themeObj["description"].toString();
        theme.settings = themeObj["settings"].toObject();
        
        // 验证主题有效性并添加到管理器
        if (validateTheme(theme)) {
            m_themes[themeId] = theme;
        } else {
            qWarning() << "主题验证失败:" << themeId;
        }
    }
    
    return true;
}

bool ThemeManager::saveThemesToFile(const QString& filePath) {
    QJsonObject root;
    QJsonObject themes;
    
    for (auto it = m_themes.begin(); it != m_themes.end(); ++it) {
        QJsonObject themeObj;
        themeObj["name"] = it.value().name;
        themeObj["description"] = it.value().description;
        themeObj["settings"] = it.value().settings;
        themes[it.key()] = themeObj;
    }
    
    root["themes"] = themes;
    
    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入主题文件:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

void ThemeManager::addTheme(const QString& themeId, const ThemeSettings& theme) {
    if (validateTheme(theme)) {
        m_themes[themeId] = theme;
        emit themeAdded(themeId);
    } else {
        qWarning() << "添加主题失败，验证不通过:" << themeId;
    }
}

void ThemeManager::removeTheme(const QString& themeId) {
    if (m_themes.remove(themeId) > 0) {
        emit themeRemoved(themeId);
    }
}

bool ThemeManager::hasTheme(const QString& themeId) const {
    return m_themes.contains(themeId);
}

ThemeSettings ThemeManager::getTheme(const QString& themeId) const {
    return m_themes.value(themeId);
}

QStringList ThemeManager::getThemeIds() const {
    return m_themes.keys();
}

QMap<QString, ThemeSettings> ThemeManager::getAllThemes() const {
    return m_themes;
}

QJsonObject ThemeManager::applyTheme(const QString& themeId, const QJsonObject& currentSettings) const {
    if (!hasTheme(themeId)) {
        qWarning() << "主题不存在:" << themeId;
        return currentSettings;
    }
    
    ThemeSettings theme = getTheme(themeId);
    QJsonObject newSettings = currentSettings;
    
    // 合并主题设置到当前设置
    for (auto it = theme.settings.begin(); it != theme.settings.end(); ++it) {
        newSettings[it.key()] = it.value();
    }
    
    return newSettings;
}

void ThemeManager::loadDefaultThemes() {
    createDefaultThemes();
}

bool ThemeManager::validateTheme(const ThemeSettings& theme) const {
    if (theme.name.isEmpty()) {
        return false;
    }
    
    // 验证背景图片路径（如果存在）
    if (theme.settings.contains("backgroundImagePath")) {
        QString imagePath = theme.settings["backgroundImagePath"].toString();
        if (!imagePath.isEmpty() && !validateImagePath(imagePath)) {
            qWarning() << "背景图片路径无效:" << imagePath;
            // 注意：这里不返回false，因为图片可能稍后添加
        }
    }
    
    // 验证透明度值
    if (theme.settings.contains("backgroundOpacity")) {
        double opacity = theme.settings["backgroundOpacity"].toDouble();
        if (opacity < 0.0 || opacity > 1.0) {
            qWarning() << "背景透明度值无效:" << opacity;
            return false;
        }
    }
    
    return true;
}

bool ThemeManager::validateImagePath(const QString& imagePath) const {
    if (imagePath.isEmpty()) {
        return true; // 空路径是有效的（表示不使用背景图片）
    }
    
    QFileInfo fileInfo(imagePath);
    return fileInfo.exists() && fileInfo.isFile();
}

void ThemeManager::createDefaultThemes() {
    // 自然主题
    {
        QJsonObject settings;
        settings["useBackgroundImage"] = true;
        settings["backgroundImagePath"] = "theme_source/nature/ClockWidget/background.png";
        settings["backgroundScaleMode"] = "keepAspectRatioByExpanding";
        settings["backgroundOpacity"] = 0.8;
        settings["timeColor"] = "#FFFFFF";
        settings["dateColor"] = "#E0E0E0";
        
        ThemeSettings theme("自然主题", "清新自然的森林背景", settings);
        m_themes["nature"] = theme;
    }
    
    // 城市主题
    {
        QJsonObject settings;
        settings["useBackgroundImage"] = true;
        settings["backgroundImagePath"] = "theme_source/city/ClockWidget/background.png";
        settings["backgroundScaleMode"] = "stretch";
        settings["backgroundOpacity"] = 0.7;
        settings["timeColor"] = "#00FFFF";
        settings["dateColor"] = "#80FFFF";
        
        ThemeSettings theme("城市主题", "现代城市夜景主题", settings);
        m_themes["city"] = theme;
    }
    
    // 太空主题
    {
        QJsonObject settings;
        settings["useBackgroundImage"] = true;
        settings["backgroundImagePath"] = "theme_source/space/ClockWidget/background.png";
        settings["backgroundScaleMode"] = "tile";
        settings["backgroundOpacity"] = 0.9;
        settings["timeColor"] = "#FFFF00";
        settings["dateColor"] = "#FFCC00";
        
        ThemeSettings theme("太空主题", "神秘太空星空主题", settings);
        m_themes["space"] = theme;
    }
    
    // 简约主题
    {
        QJsonObject settings;
        settings["useBackgroundImage"] = false;
        settings["backgroundColor"] = "#2C3E50AA";
        settings["timeColor"] = "#ECF0F1";
        settings["dateColor"] = "#BDC3C7";
        
        ThemeSettings theme("简约主题", "简洁的纯色背景主题", settings);
        m_themes["minimal"] = theme;
    }
    
    // 渐变主题
    {
        QJsonObject settings;
        settings["useBackgroundImage"] = true;
        settings["backgroundImagePath"] = "theme_source/gradient/ClockWidget/background.png";
        settings["backgroundScaleMode"] = "stretch";
        settings["backgroundOpacity"] = 0.6;
        settings["timeColor"] = "#FFFFFF";
        settings["dateColor"] = "#F0F0F0";
        
        ThemeSettings theme("渐变主题", "彩色渐变背景主题", settings);
        m_themes["gradient"] = theme;
    }
    
    // 经典主题（默认）
    {
        QJsonObject settings;
        settings["useBackgroundImage"] = false;
        settings["backgroundColor"] = "#000000AA";
        settings["timeColor"] = "#FFFFFF";
        settings["dateColor"] = "#CCCCCC";
        
        ThemeSettings theme("经典主题", "传统的黑色背景主题", settings);
        m_themes["classic"] = theme;
    }
} 