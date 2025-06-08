#pragma once
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMap>
#include <QString>
#include "Common/Types.h"

struct ThemeSettings {
    QString name;
    QString description;
    QJsonObject settings;
    
    ThemeSettings() = default;
    ThemeSettings(const QString& n, const QString& desc, const QJsonObject& s)
        : name(n), description(desc), settings(s) {}
};

class ThemeManager : public QObject {
    Q_OBJECT

public:
    static ThemeManager& instance();
    
    // 主题管理
    bool loadThemesFromFile(const QString& filePath);
    bool saveThemesToFile(const QString& filePath);
    
    // 主题操作
    void addTheme(const QString& themeId, const ThemeSettings& theme);
    void removeTheme(const QString& themeId);
    bool hasTheme(const QString& themeId) const;
    
    // 获取主题
    ThemeSettings getTheme(const QString& themeId) const;
    QStringList getThemeIds() const;
    QMap<QString, ThemeSettings> getAllThemes() const;
    
    // 应用主题
    QJsonObject applyTheme(const QString& themeId, const QJsonObject& currentSettings = QJsonObject()) const;
    
    // 预设主题
    void loadDefaultThemes();
    
    // 主题验证
    bool validateTheme(const ThemeSettings& theme) const;
    bool validateImagePath(const QString& imagePath) const;

signals:
    void themeAdded(const QString& themeId);
    void themeRemoved(const QString& themeId);
    void themeChanged(const QString& themeId);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;
    
    // 禁用拷贝构造和赋值
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    void createDefaultThemes();

private:
    QMap<QString, ThemeSettings> m_themes;
    QString m_currentThemeId;
}; 