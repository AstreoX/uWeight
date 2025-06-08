#pragma once
#include "Core/BaseWidget.h"
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>

// 天气数据结构
struct WeatherData {
    QString location;           // 位置
    QString description;        // 天气描述
    QString iconCode;          // 天气图标代码
    double temperature;        // 当前温度
    double tempMin;           // 最低温度
    double tempMax;           // 最高温度
    int humidity;             // 湿度
    double windSpeed;         // 风速
    QString windDirection;    // 风向
    int pressure;             // 气压
    QDateTime lastUpdate;     // 最后更新时间
    bool isValid;             // 数据是否有效
    
    WeatherData() : temperature(0), tempMin(0), tempMax(0), humidity(0), 
                   windSpeed(0), pressure(0), isValid(false) {}
};

// 天气显示样式
enum class WeatherDisplayStyle {
    Compact,        // 紧凑模式
    Detailed,       // 详细模式
    Mini           // 迷你模式
};

// 温度单位
enum class TemperatureUnit {
    Celsius,        // 摄氏度
    Fahrenheit      // 华氏度
};

class WeatherWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit WeatherWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~WeatherWidget() = default;

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void applyConfig() override;

private slots:
    void onWeatherDataReceived();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    void setupDefaultConfig();
    void parseCustomSettings();
    void fetchWeatherData();
    void drawWeatherIcon(QPainter& painter, const QRect& iconRect);
    void drawTemperature(QPainter& painter);
    void drawWeatherInfo(QPainter& painter);
    void drawLocationAndTime(QPainter& painter);
    QString formatTemperature(double temp) const;
    QString getWeatherIconPath(const QString& iconCode) const;
    void loadWeatherIcons();
    double convertTemperature(double celsius) const;
    void drawMiniWeather(QPainter& painter, const QRect& rect);
    void drawCompactWeather(QPainter& painter, const QRect& rect);
    void drawDetailedWeather(QPainter& painter, const QRect& rect);
    
    // API数据解析方法
    void parseQWeatherData(const QJsonObject& json);
    void parseSeniverseData(const QJsonObject& json);
    void parseOpenWeatherMapData(const QJsonObject& json);

private:
    // 网络请求
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
    
    // 天气数据
    WeatherData m_weatherData;
    QString m_apiKey;
    QString m_apiProvider;
    QString m_apiHost;
    QString m_location;
    QString m_cityName;
    
    // 显示设置
    WeatherDisplayStyle m_displayStyle;
    TemperatureUnit m_temperatureUnit;
    bool m_showWeatherIcon;
    bool m_showHumidity;
    bool m_showWindSpeed;
    bool m_showPressure;
    bool m_showLastUpdate;
    bool m_autoUpdateLocation;
    
    // 字体和颜色
    QFont m_temperatureFont;
    QFont m_locationFont;
    QFont m_infoFont;
    QColor m_temperatureColor;
    QColor m_locationColor;
    QColor m_infoColor;
    QColor m_backgroundColor;
    
    // 图标
    QMap<QString, QPixmap> m_weatherIcons;
    
    // 布局设置
    int m_iconSize;
    int m_spacing;
    int m_padding;
    
    // 更新设置
    int m_weatherUpdateInterval;
    QDateTime m_lastWeatherUpdate;
    bool m_enableAutoRefresh;
}; 