#include "Widgets/WeatherWidget.h"
#include <QPainter>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QPainterPath>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QtMath>

WeatherWidget::WeatherWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , m_networkManager(nullptr)
    , m_currentReply(nullptr)
    , m_displayStyle(WeatherDisplayStyle::Compact)
    , m_temperatureUnit(TemperatureUnit::Celsius)
    , m_showWeatherIcon(true)
    , m_showHumidity(true)
    , m_showWindSpeed(true)
    , m_showPressure(false)
    , m_showLastUpdate(true)
    , m_autoUpdateLocation(false)
    , m_iconSize(48)
    , m_spacing(5)
    , m_padding(10)
    , m_weatherUpdateInterval(600000) // 10分钟
    , m_enableAutoRefresh(true)
{
    setupDefaultConfig();
    parseCustomSettings();
    
    qDebug() << "WeatherWidget构造函数: 解析配置完成";
    qDebug() << "  API Provider:" << m_apiProvider;
    qDebug() << "  API Host:" << (m_apiHost.isEmpty() ? "默认" : m_apiHost);
    qDebug() << "  API Key长度:" << m_apiKey.length();
    qDebug() << "  City Name:" << m_cityName;
    
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    // 配置SSL设置，忽略SSL错误（仅用于开发测试）
    connect(m_networkManager, &QNetworkAccessManager::sslErrors,
            [](QNetworkReply* reply, const QList<QSslError>& errors) {
                Q_UNUSED(errors)
                qDebug() << "WeatherWidget忽略SSL错误:" << errors;
                reply->ignoreSslErrors();
            });
    
    // 启用自动重定向跟随
    m_networkManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    
    // 加载天气图标
    loadWeatherIcons();
    
    // 初始化天气数据为无效状态
    m_weatherData.isValid = false;
    
    // 如果有有效的API配置，立即获取一次天气数据
    if (!m_apiKey.isEmpty() && m_apiKey != "your_api_key_here" && !m_cityName.isEmpty()) {
        qDebug() << "配置有效，立即获取天气数据";
        fetchWeatherData();
    } else {
        qDebug() << "配置无效，跳过天气数据获取";
        qDebug() << "  API Key是否为空:" << m_apiKey.isEmpty();
        qDebug() << "  API Key是否为默认值:" << (m_apiKey == "your_api_key_here");
        qDebug() << "  城市名称是否为空:" << m_cityName.isEmpty();
    }
    
    // 设置更新间隔
    setUpdateInterval(m_weatherUpdateInterval);
}

void WeatherWidget::setupDefaultConfig() {
    // 设置默认字体
    m_temperatureFont.setFamily("Arial");
    m_temperatureFont.setPointSize(18);
    m_temperatureFont.setBold(true);
    
    m_locationFont.setFamily("Arial");
    m_locationFont.setPointSize(10);
    
    m_infoFont.setFamily("Arial");
    m_infoFont.setPointSize(8);
    
    // 设置默认颜色
    m_temperatureColor = QColor(50, 50, 50);
    m_locationColor = QColor(100, 100, 100);
    m_infoColor = QColor(120, 120, 120);
    m_backgroundColor = QColor(255, 255, 255, 200);
}

void WeatherWidget::parseCustomSettings() {
    const QJsonObject& settings = m_config.customSettings;
    
    // 解析API设置
    m_apiKey = settings.value("apiKey").toString();
    m_apiHost = settings.value("apiHost").toString();
    m_cityName = settings.value("cityName").toString("北京");
    m_location = settings.value("location").toString();
    m_apiProvider = settings.value("apiProvider").toString("qweather"); // 默认使用和风天气
    
    // 解析显示设置
    QString styleStr = settings.value("displayStyle").toString("Compact");
    if (styleStr == "Detailed") {
        m_displayStyle = WeatherDisplayStyle::Detailed;
    } else if (styleStr == "Mini") {
        m_displayStyle = WeatherDisplayStyle::Mini;
    } else {
        m_displayStyle = WeatherDisplayStyle::Compact;
    }
    
    QString unitStr = settings.value("temperatureUnit").toString("Celsius");
    m_temperatureUnit = (unitStr == "Fahrenheit") ? 
        TemperatureUnit::Fahrenheit : TemperatureUnit::Celsius;
    
    m_showWeatherIcon = settings.value("showWeatherIcon").toBool(true);
    m_showHumidity = settings.value("showHumidity").toBool(true);
    m_showWindSpeed = settings.value("showWindSpeed").toBool(true);
    m_showPressure = settings.value("showPressure").toBool(false);
    m_showLastUpdate = settings.value("showLastUpdate").toBool(true);
    m_autoUpdateLocation = settings.value("autoUpdateLocation").toBool(false);
    
    // 解析颜色设置
    if (settings.contains("temperatureColor")) {
        m_temperatureColor = QColor::fromString(settings.value("temperatureColor").toString());
    }
    if (settings.contains("locationColor")) {
        m_locationColor = QColor::fromString(settings.value("locationColor").toString());
    }
    if (settings.contains("infoColor")) {
        m_infoColor = QColor::fromString(settings.value("infoColor").toString());
    }
    if (settings.contains("backgroundColor")) {
        m_backgroundColor = QColor::fromString(settings.value("backgroundColor").toString());
    }
    
    // 解析布局设置
    m_iconSize = settings.value("iconSize").toInt(48);
    m_spacing = settings.value("spacing").toInt(5);
    m_padding = settings.value("padding").toInt(10);
    
    // 解析更新设置
    m_weatherUpdateInterval = settings.value("updateInterval").toInt(600000);
    m_enableAutoRefresh = settings.value("enableAutoRefresh").toBool(true);
}

void WeatherWidget::applyConfig() {
    BaseWidget::applyConfig();
    
    qDebug() << "WeatherWidget::applyConfig: 开始应用配置";
    qDebug() << "配置ID:" << m_config.id;
    qDebug() << "配置名称:" << m_config.name;
    
    parseCustomSettings();
    
    qDebug() << "解析后的API设置:";
    qDebug() << "  API Provider:" << m_apiProvider;
    qDebug() << "  API Host:" << (m_apiHost.isEmpty() ? "默认" : m_apiHost);
    qDebug() << "  API Key:" << m_apiKey;
    qDebug() << "  City Name:" << m_cityName;
    
    // 如果配置有变化，重新获取天气数据
    fetchWeatherData();
    
    update();
}

void WeatherWidget::updateContent() {
    if (m_enableAutoRefresh) {
        QDateTime now = QDateTime::currentDateTime();
        if (m_lastWeatherUpdate.isNull() || 
            m_lastWeatherUpdate.secsTo(now) >= m_weatherUpdateInterval / 1000) {
            fetchWeatherData();
        }
    }
}

void WeatherWidget::fetchWeatherData() {
    qDebug() << "WeatherWidget::fetchWeatherData: 开始获取天气数据";
    qDebug() << "API Provider:" << m_apiProvider;
    qDebug() << "API Key:" << (m_apiKey.isEmpty() ? "empty" : "configured");
    qDebug() << "City Name:" << m_cityName;
    
    if (m_apiKey.isEmpty() || m_apiKey == "your_api_key_here") {
        qDebug() << "Weather API key not configured";
        m_weatherData.isValid = false;
        update(); // 触发重绘以显示错误信息
        return;
    }
    
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    QUrl url;
    QUrlQuery query;
    
    // 根据API提供商构建不同的请求
    if (m_apiProvider == "qweather" || m_apiProvider.isEmpty()) {
        // 和风天气API
        QString host = m_apiHost.isEmpty() ? "devapi.qweather.com" : m_apiHost;
        url.setUrl(QString("https://%1/v7/weather/now").arg(host));
        
        // 优化城市参数 - 如果输入的是中文城市名，尝试转换为LocationID
        QString location = m_cityName;
        if (m_cityName == "北京" || m_cityName.toLower() == "beijing") {
            location = "101010100";  // 北京的LocationID
        } else if (m_cityName == "上海" || m_cityName.toLower() == "shanghai") {
            location = "101020100";  // 上海的LocationID  
        } else if (m_cityName == "广州" || m_cityName.toLower() == "guangzhou") {
            location = "101280101";  // 广州的LocationID
        } else if (m_cityName == "深圳" || m_cityName.toLower() == "shenzhen") {
            location = "101280601";  // 深圳的LocationID
        } else if (m_cityName == "西安" || m_cityName.toLower() == "xian" || m_cityName.toLower() == "xi'an") {
            location = "101110101";  // 西安的LocationID
        }
        
        query.addQueryItem("location", location);
        qDebug() << "WeatherWidget使用位置参数:" << m_cityName << "->" << location;
        
        // 检查是否为JWT token（包含点号）还是传统API key
        bool isJWT = m_apiKey.contains('.');
        if (!isJWT) {
            // 传统API key，使用URL参数
            query.addQueryItem("key", m_apiKey);
        }
        // JWT token将在请求头中设置
    } else if (m_apiProvider == "seniverse") {
        // 心知天气API
        url.setUrl("https://api.seniverse.com/v3/weather/now.json");
        query.addQueryItem("location", m_cityName);
        query.addQueryItem("key", m_apiKey);
        query.addQueryItem("language", "zh-Hans");
        query.addQueryItem("unit", "c");
    } else if (m_apiProvider == "openweathermap") {
        // OpenWeatherMap API (保留支持)
        url.setUrl("https://api.openweathermap.org/data/2.5/weather");
        query.addQueryItem("q", m_cityName);
        query.addQueryItem("appid", m_apiKey);
        query.addQueryItem("units", "metric");
        query.addQueryItem("lang", "zh_cn");
    } else {
        qDebug() << "Unsupported API provider:" << m_apiProvider;
        return;
    }
    
    url.setQuery(query);
    
    qDebug() << "发送天气API请求到:" << url.toString();
    
    QNetworkRequest request(url);
    
    // 设置重定向策略
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    // 设置完整的请求头来模拟浏览器请求
    request.setHeader(QNetworkRequest::UserAgentHeader, 
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8");
    // 暂时不要压缩，避免解压问题
    // request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Pragma", "no-cache");
    
    // 根据API提供商设置特定的请求头
    if (m_apiProvider == "qweather" || m_apiProvider.isEmpty()) {
        QString hostName = m_apiHost.isEmpty() ? "dev.qweather.com" : m_apiHost;
        QString refererUrl = QString("https://%1/").arg(hostName);
        request.setRawHeader("Referer", refererUrl.toUtf8());
        request.setRawHeader("Origin", QString("https://%1").arg(hostName).toUtf8());
        
        // 只有JWT token才需要Authorization头
        bool isJWT = m_apiKey.contains('.');
        if (isJWT) {
            QString authHeader = QString("Bearer %1").arg(m_apiKey);
            request.setRawHeader("Authorization", authHeader.toUtf8());
        }
    }
    
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::finished, 
            this, &WeatherWidget::onWeatherDataReceived);
    connect(m_currentReply, &QNetworkReply::errorOccurred,
            this, &WeatherWidget::onNetworkError);
    
    m_lastWeatherUpdate = QDateTime::currentDateTime();
}

void WeatherWidget::onWeatherDataReceived() {
    if (!m_currentReply) {
        qDebug() << "WeatherWidget::onWeatherDataReceived: reply为空";
        return;
    }
    
    qDebug() << "WeatherWidget::onWeatherDataReceived: 收到响应";
    qDebug() << "HTTP状态码:" << m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "网络错误:" << m_currentReply->error();
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray data = m_currentReply->readAll();
        qDebug() << "响应数据长度:" << data.length();
        qDebug() << "响应数据内容:" << QString::fromUtf8(data.left(500)); // 只显示前500字符
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "JSON解析错误:" << parseError.errorString();
            m_weatherData.isValid = false;
        } else {
            QJsonObject json = doc.object();
            qDebug() << "JSON解析成功，开始解析天气数据";
            
            // 根据不同API提供商解析数据
            if (m_apiProvider == "qweather" || m_apiProvider.isEmpty()) {
                qDebug() << "使用和风天气解析器";
                parseQWeatherData(json);
            } else if (m_apiProvider == "seniverse") {
                qDebug() << "使用心知天气解析器";
                parseSeniverseData(json);
            } else if (m_apiProvider == "openweathermap") {
                qDebug() << "使用OpenWeatherMap解析器";
                parseOpenWeatherMapData(json);
            }
            
            qDebug() << "天气数据解析完成:";
            qDebug() << "  有效性:" << m_weatherData.isValid;
            qDebug() << "  位置:" << m_weatherData.location;
            qDebug() << "  温度:" << m_weatherData.temperature;
            qDebug() << "  描述:" << m_weatherData.description;
        }
    } else {
        qDebug() << "Weather API error:" << m_currentReply->errorString();
        qDebug() << "Error details:" << m_currentReply->readAll();
        m_weatherData.isValid = false;
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    
    // 更新显示
    update();
}

void WeatherWidget::onNetworkError(QNetworkReply::NetworkError error) {
    qDebug() << "Network error:" << error;
    m_weatherData.isValid = false;
    update();
}

void WeatherWidget::drawContent(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), m_backgroundColor);
    
    qDebug() << "WeatherWidget::drawContent: 开始绘制";
    qDebug() << "天气数据有效性:" << m_weatherData.isValid;
    qDebug() << "显示样式:" << static_cast<int>(m_displayStyle);
    
    if (!m_weatherData.isValid) {
        // 显示错误信息
        qDebug() << "绘制错误信息";
        painter.setPen(m_infoColor);
        painter.setFont(m_infoFont);
        painter.drawText(rect(), Qt::AlignCenter, 
                        "天气数据获取失败\n\n请检查:\n• 网络连接\n• API密钥\n• 城市名称");
        return;
    }
    
    qDebug() << "绘制天气数据:" << m_weatherData.location << m_weatherData.temperature << "°C";
    
    QRect contentRect = rect().adjusted(m_padding, m_padding, -m_padding, -m_padding);
    
    switch (m_displayStyle) {
    case WeatherDisplayStyle::Mini:
        drawMiniWeather(painter, contentRect);
        break;
    case WeatherDisplayStyle::Detailed:
        drawDetailedWeather(painter, contentRect);
        break;
    case WeatherDisplayStyle::Compact:
    default:
        drawCompactWeather(painter, contentRect);
        break;
    }
}

void WeatherWidget::drawMiniWeather(QPainter& painter, const QRect& rect) {
    // 迷你模式：只显示温度和图标
    int iconSize = qMin(rect.width(), rect.height()) / 2;
    QRect iconRect(rect.left(), rect.top(), iconSize, iconSize);
    
    if (m_showWeatherIcon) {
        drawWeatherIcon(painter, iconRect);
    }
    
    // 绘制温度
    painter.setPen(m_temperatureColor);
    painter.setFont(m_temperatureFont);
    QString tempText = formatTemperature(convertTemperature(m_weatherData.temperature));
    QRect tempRect(iconRect.right() + m_spacing, rect.top(), 
                   rect.width() - iconSize - m_spacing, rect.height());
    painter.drawText(tempRect, Qt::AlignCenter, tempText);
}

void WeatherWidget::drawCompactWeather(QPainter& painter, const QRect& rect) {
    int currentY = rect.top();
    
    // 绘制位置
    painter.setPen(m_locationColor);
    painter.setFont(m_locationFont);
    QRect locationRect(rect.left(), currentY, rect.width(), m_locationFont.pointSize() + 5);
    painter.drawText(locationRect, Qt::AlignCenter, m_weatherData.location);
    currentY += locationRect.height() + m_spacing;
    
    // 绘制图标和温度
    int iconSize = qMin(m_iconSize, rect.width() / 3);
    QRect iconRect(rect.left(), currentY, iconSize, iconSize);
    if (m_showWeatherIcon) {
        drawWeatherIcon(painter, iconRect);
    }
    
    painter.setPen(m_temperatureColor);
    painter.setFont(m_temperatureFont);
    QString tempText = formatTemperature(convertTemperature(m_weatherData.temperature));
    QRect tempRect(iconRect.right() + m_spacing, currentY, 
                   rect.width() - iconSize - m_spacing, iconSize);
    painter.drawText(tempRect, Qt::AlignVCenter, tempText);
    
    currentY += iconSize + m_spacing;
    
    // 绘制天气描述
    painter.setPen(m_infoColor);
    painter.setFont(m_infoFont);
    QRect descRect(rect.left(), currentY, rect.width(), m_infoFont.pointSize() + 3);
    painter.drawText(descRect, Qt::AlignCenter, m_weatherData.description);
    currentY += descRect.height() + m_spacing;
    
    // 绘制额外信息
    if (m_showHumidity || m_showWindSpeed) {
        QString infoText;
        if (m_showHumidity) {
            infoText += QString("湿度: %1%").arg(m_weatherData.humidity);
        }
        if (m_showWindSpeed) {
            if (!infoText.isEmpty()) infoText += " | ";
            infoText += QString("风速: %1m/s").arg(m_weatherData.windSpeed, 0, 'f', 1);
        }
        
        QRect infoRect(rect.left(), currentY, rect.width(), m_infoFont.pointSize() + 3);
        painter.drawText(infoRect, Qt::AlignCenter, infoText);
    }
}

void WeatherWidget::drawDetailedWeather(QPainter& painter, const QRect& rect) {
    int currentY = rect.top();
    int rowHeight = 20;
    
    // 绘制位置和时间
    painter.setPen(m_locationColor);
    painter.setFont(m_locationFont);
    QRect locationRect(rect.left(), currentY, rect.width(), rowHeight);
    painter.drawText(locationRect, Qt::AlignCenter, m_weatherData.location);
    currentY += rowHeight + m_spacing;
    
    // 绘制主要天气信息（图标+温度）
    int iconSize = qMin(m_iconSize, rect.width() / 3);
    QRect iconRect(rect.left(), currentY, iconSize, iconSize);
    if (m_showWeatherIcon) {
        drawWeatherIcon(painter, iconRect);
    }
    
    painter.setPen(m_temperatureColor);
    painter.setFont(m_temperatureFont);
    QString tempText = formatTemperature(convertTemperature(m_weatherData.temperature));
    QRect tempRect(iconRect.right() + m_spacing, currentY, 
                   rect.width() - iconSize - m_spacing, iconSize / 2);
    painter.drawText(tempRect, Qt::AlignVCenter, tempText);
    
    // 绘制温度范围
    painter.setPen(m_infoColor);
    painter.setFont(m_infoFont);
    QString rangeText = QString("%1 / %2")
        .arg(formatTemperature(convertTemperature(m_weatherData.tempMin)))
        .arg(formatTemperature(convertTemperature(m_weatherData.tempMax)));
    QRect rangeRect(iconRect.right() + m_spacing, currentY + iconSize/2, 
                    rect.width() - iconSize - m_spacing, iconSize / 2);
    painter.drawText(rangeRect, Qt::AlignVCenter, rangeText);
    
    currentY += iconSize + m_spacing;
    
    // 绘制天气描述
    painter.setPen(m_infoColor);
    painter.setFont(m_infoFont);
    QRect descRect(rect.left(), currentY, rect.width(), rowHeight);
    painter.drawText(descRect, Qt::AlignCenter, m_weatherData.description);
    currentY += rowHeight + m_spacing;
    
    // 绘制详细信息
    QStringList details;
    if (m_showHumidity) {
        details << QString("湿度: %1%").arg(m_weatherData.humidity);
    }
    if (m_showWindSpeed) {
        details << QString("风速: %1m/s").arg(m_weatherData.windSpeed, 0, 'f', 1);
    }
    if (m_showPressure) {
        details << QString("气压: %1hPa").arg(m_weatherData.pressure);
    }
    
    for (const QString& detail : details) {
        QRect detailRect(rect.left(), currentY, rect.width(), rowHeight);
        painter.drawText(detailRect, Qt::AlignCenter, detail);
        currentY += rowHeight;
    }
    
    // 绘制最后更新时间
    if (m_showLastUpdate && m_weatherData.lastUpdate.isValid()) {
        currentY += m_spacing;
        QString updateText = QString("更新: %1")
            .arg(m_weatherData.lastUpdate.toString("hh:mm"));
        QRect updateRect(rect.left(), currentY, rect.width(), rowHeight);
        painter.drawText(updateRect, Qt::AlignCenter, updateText);
    }
}

void WeatherWidget::drawWeatherIcon(QPainter& painter, const QRect& iconRect) {
    QString iconKey = m_weatherData.iconCode;
    if (m_weatherIcons.contains(iconKey)) {
        QPixmap icon = m_weatherIcons[iconKey];
        painter.drawPixmap(iconRect, icon);
    } else {
        // 绘制默认图标（简单的太阳）
        painter.setPen(QPen(QColor(255, 165, 0), 2));
        painter.setBrush(QColor(255, 165, 0));
        QRect sunRect = iconRect.adjusted(iconRect.width()/4, iconRect.height()/4, 
                                         -iconRect.width()/4, -iconRect.height()/4);
        painter.drawEllipse(sunRect);
        
        // 绘制太阳光线
        int centerX = iconRect.center().x();
        int centerY = iconRect.center().y();
        int radius = sunRect.width() / 2 + 5;
        for (int i = 0; i < 8; i++) {
            double angle = i * M_PI / 4;
            int x1 = centerX + (radius - 3) * qCos(angle);
            int y1 = centerY + (radius - 3) * qSin(angle);
            int x2 = centerX + (radius + 3) * qCos(angle);
            int y2 = centerY + (radius + 3) * qSin(angle);
            painter.drawLine(x1, y1, x2, y2);
        }
    }
}

QString WeatherWidget::formatTemperature(double temp) const {
    QString unit = (m_temperatureUnit == TemperatureUnit::Celsius) ? "°C" : "°F";
    return QString("%1%2").arg(qRound(temp)).arg(unit);
}

double WeatherWidget::convertTemperature(double celsius) const {
    if (m_temperatureUnit == TemperatureUnit::Fahrenheit) {
        return celsius * 9.0 / 5.0 + 32.0;
    }
    return celsius;
}

void WeatherWidget::loadWeatherIcons() {
    // 这里可以加载天气图标
    // 为简单起见，我们使用默认绘制
    // 在实际项目中，可以加载对应的PNG图标文件
}

QString WeatherWidget::getWeatherIconPath(const QString& iconCode) const {
    return QString(":/icons/weather/%1.png").arg(iconCode);
}

void WeatherWidget::parseQWeatherData(const QJsonObject& json) {
    // 和风天气API响应格式
    QString code = json["code"].toString();
    if (code != "200") {
        qDebug() << "QWeather API error, code:" << code;
        m_weatherData.isValid = false;
        return;
    }
    
    QJsonObject now = json["now"].toObject();
    m_weatherData.isValid = true;
    m_weatherData.location = m_cityName; // 和风天气不返回城市名，使用输入的城市名
    m_weatherData.temperature = now["temp"].toString().toDouble();
    m_weatherData.description = now["text"].toString();
    m_weatherData.iconCode = now["icon"].toString();
    m_weatherData.humidity = now["humidity"].toString().toInt();
    m_weatherData.pressure = now["pressure"].toString().toInt();
    m_weatherData.windSpeed = now["windSpeed"].toString().toDouble();
    m_weatherData.windDirection = now["windDir"].toString();
    
    // 和风天气没有直接提供温度范围，使用当前温度
    m_weatherData.tempMin = m_weatherData.temperature;
    m_weatherData.tempMax = m_weatherData.temperature;
    
    m_weatherData.lastUpdate = QDateTime::currentDateTime();
}

void WeatherWidget::parseSeniverseData(const QJsonObject& json) {
    // 心知天气API响应格式
    QJsonArray results = json["results"].toArray();
    if (results.isEmpty()) {
        qDebug() << "Seniverse API error: no results";
        m_weatherData.isValid = false;
        return;
    }
    
    QJsonObject result = results[0].toObject();
    QJsonObject now = result["now"].toObject();
    QJsonObject location = result["location"].toObject();
    
    m_weatherData.isValid = true;
    m_weatherData.location = location["name"].toString();
    m_weatherData.temperature = now["temperature"].toString().toDouble();
    m_weatherData.description = now["text"].toString();
    m_weatherData.iconCode = now["code"].toString();
    
    // 心知天气的基础版本没有湿度、气压等信息
    m_weatherData.humidity = 0;
    m_weatherData.pressure = 0;
    m_weatherData.windSpeed = 0;
    m_weatherData.windDirection = "";
    
    // 没有温度范围，使用当前温度
    m_weatherData.tempMin = m_weatherData.temperature;
    m_weatherData.tempMax = m_weatherData.temperature;
    
    m_weatherData.lastUpdate = QDateTime::currentDateTime();
}

void WeatherWidget::parseOpenWeatherMapData(const QJsonObject& json) {
    // OpenWeatherMap API响应格式（原有的解析逻辑）
    m_weatherData.isValid = true;
    m_weatherData.location = json["name"].toString();
    m_weatherData.temperature = json["main"].toObject()["temp"].toDouble();
    m_weatherData.tempMin = json["main"].toObject()["temp_min"].toDouble();
    m_weatherData.tempMax = json["main"].toObject()["temp_max"].toDouble();
    m_weatherData.humidity = json["main"].toObject()["humidity"].toInt();
    m_weatherData.pressure = json["main"].toObject()["pressure"].toInt();
    m_weatherData.windSpeed = json["wind"].toObject()["speed"].toDouble();
    
    // 解析天气描述和图标
    QJsonArray weatherArray = json["weather"].toArray();
    if (!weatherArray.isEmpty()) {
        QJsonObject weather = weatherArray[0].toObject();
        m_weatherData.description = weather["description"].toString();
        m_weatherData.iconCode = weather["icon"].toString();
    }
    
    m_weatherData.lastUpdate = QDateTime::currentDateTime();
} 