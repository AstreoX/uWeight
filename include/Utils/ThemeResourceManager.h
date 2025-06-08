#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QMap>

class ThemeResourceManager : public QObject {
    Q_OBJECT

public:
    static ThemeResourceManager& instance();
    
    // 初始化主题资源目录
    void initializeThemeDirectories();
    
    // 获取主题资源路径
    QString getThemeResourcePath() const { return m_themeResourcePath; }
    QString getThemeImagePath(const QString& themeName, const QString& widgetName) const;
    QString getCustomThemePath() const;
    
    // 主题资源操作
    QStringList getAvailableThemes() const;
    QStringList getThemeImages(const QString& themeName, const QString& widgetName) const;
    bool hasThemeImage(const QString& themeName, const QString& widgetName) const;
    
    // 自定义图片管理
    QString importCustomImage(const QString& sourceImagePath, const QString& widgetName, const QString& customName = QString());
    bool removeCustomImage(const QString& widgetName, const QString& imageName);
    QStringList getCustomImages(const QString& widgetName) const;
    
    // 主题预览
    QPixmap getThemePreview(const QString& themeName, const QString& widgetName, const QSize& size = QSize(200, 100)) const;
    
    // 文件操作
    bool copyImageToTheme(const QString& sourceImagePath, const QString& themeName, const QString& widgetName, const QString& imageName);
    bool createThemeDirectory(const QString& themeName);
    bool createWidgetDirectory(const QString& themeName, const QString& widgetName);
    
    // 验证和清理
    bool validateThemeStructure() const;
    void cleanupEmptyDirectories();
    
    // 获取相对路径（用于配置文件）
    QString getRelativeImagePath(const QString& themeName, const QString& widgetName, const QString& imageName) const;

signals:
    void themeResourceAdded(const QString& themeName, const QString& widgetName, const QString& imageName);
    void themeResourceRemoved(const QString& themeName, const QString& widgetName, const QString& imageName);
    void customImageImported(const QString& widgetName, const QString& imageName);

private:
    explicit ThemeResourceManager(QObject* parent = nullptr);
    ~ThemeResourceManager() = default;
    
    // 禁用拷贝构造和赋值
    ThemeResourceManager(const ThemeResourceManager&) = delete;
    ThemeResourceManager& operator=(const ThemeResourceManager&) = delete;
    
    void createDefaultThemeStructure();
    void createSampleImages();
    QString generateUniqueImageName(const QString& originalName, const QString& targetDir) const;

private:
    QString m_themeResourcePath;
    QStringList m_defaultThemes;
    QMap<QString, QString> m_themeDescriptions;
}; 