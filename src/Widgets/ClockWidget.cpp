/**
 * @file ClockWidget.cpp
 * @brief 时钟小组件实现
 * @details 显示当前时间和日期的桌面小组件
 * @author 陈佳燕 (Aveline)
 * @date 2025-5
 * @version 1.0.0
 * 
 * 时钟小组件功能特性：
 * - 实时显示当前时间（支持12/24小时制）
 * - 可选显示日期和星期
 * - 可自定义时间和日期格式
 * - 支持背景图片和颜色自定义
 * - 多种背景缩放模式
 * - 字体大小和颜色配置
 * - 透明度调节
 * - 主题系统集成
 * 
 * 设计特点：
 * - 继承自BaseWidget，具备所有基础功能
 * - 采用QPainter进行高效绘制
 * - 支持多种背景渲染模式
 * - 实时更新机制，确保时间准确性
 * - 丰富的自定义选项
 */

#include "Widgets/ClockWidget.h"
#include <QPainter>
#include <QJsonObject>
#include <QRect>
#include <QApplication>
#include <QFileInfo>
#include <QDebug>

/**
 * @brief ClockWidget构造函数
 * @param config 小组件配置信息
 * @param parent 父窗口指针
 * 
 * 初始化时钟小组件：
 * - 设置默认颜色和字体
 * - 配置时间显示格式
 * - 解析自定义配置项
 * - 设置最小尺寸限制
 * 
 * 默认配置：
 * - 时间颜色：白色
 * - 日期颜色：浅灰色
 * - 背景色：半透明黑色
 * - 显示模式：24小时制，显示秒数和日期
 * - 背景图片：拉伸模式，完全不透明
 * 
 * 最小尺寸限制确保文本可读性
 */
ClockWidget::ClockWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , m_timeColor(Qt::white)              // 默认时间文字颜色
    , m_dateColor(Qt::lightGray)          // 默认日期文字颜色
    , m_backgroundColor(QColor(0, 0, 0, 100))  // 默认半透明黑色背景
    , m_showDate(true)                    // 默认显示日期
    , m_show24Hour(true)                  // 默认24小时制
    , m_showSeconds(true)                 // 默认显示秒数
    , m_backgroundScaleMode(BackgroundScaleMode::Stretch)  // 默认拉伸背景
    , m_backgroundOpacity(1.0)            // 默认背景完全不透明
    , m_useBackgroundImage(false)         // 默认不使用背景图片
{
    // 设置默认字体和格式
    setupDefaultConfig();
    
    // 解析用户自定义配置
    parseCustomSettings();
    
    // 设置最小尺寸以确保文本可读性
    setMinimumSize(150, 60);
}

void ClockWidget::setupDefaultConfig() {
    m_timeFont = QFont("Arial", 14, QFont::Bold);
    m_dateFont = QFont("Arial", 10);
    m_timeFormat = m_show24Hour ? "hh:mm:ss" : "h:mm:ss AP";
    m_dateFormat = "yyyy-MM-dd dddd";
}

void ClockWidget::parseCustomSettings() {
    const QJsonObject& settings = m_config.customSettings;
    
    if (settings.contains("showDate")) {
        m_showDate = settings["showDate"].toBool();
    }
    
    if (settings.contains("show24Hour")) {
        m_show24Hour = settings["show24Hour"].toBool();
        m_timeFormat = m_show24Hour ? "hh:mm:ss" : "h:mm:ss AP";
    }
    
    if (settings.contains("showSeconds")) {
        m_showSeconds = settings["showSeconds"].toBool();
        m_timeFormat = m_show24Hour ? 
            (m_showSeconds ? "hh:mm:ss" : "hh:mm") : 
            (m_showSeconds ? "h:mm:ss AP" : "h:mm AP");
    }
    
    if (settings.contains("timeColor")) {
        m_timeColor = QColor(settings["timeColor"].toString());
    }
    
    if (settings.contains("dateColor")) {
        m_dateColor = QColor(settings["dateColor"].toString());
    }
    
    if (settings.contains("backgroundColor")) {
        m_backgroundColor = QColor(settings["backgroundColor"].toString());
    }
    
    if (settings.contains("timeFontSize")) {
        m_timeFont.setPointSize(settings["timeFontSize"].toInt());
    }
    
    if (settings.contains("dateFontSize")) {
        m_dateFont.setPointSize(settings["dateFontSize"].toInt());
    }
    
    // 背景图片相关配置
    if (settings.contains("useBackgroundImage")) {
        m_useBackgroundImage = settings["useBackgroundImage"].toBool();
    }
    
    if (settings.contains("backgroundImagePath")) {
        m_backgroundImagePath = settings["backgroundImagePath"].toString();
        if (!m_backgroundImagePath.isEmpty()) {
            loadBackgroundImage();
        }
    }
    
    if (settings.contains("backgroundScaleMode")) {
        QString scaleMode = settings["backgroundScaleMode"].toString();
        if (scaleMode == "stretch") {
            m_backgroundScaleMode = BackgroundScaleMode::Stretch;
        } else if (scaleMode == "keepAspectRatio") {
            m_backgroundScaleMode = BackgroundScaleMode::KeepAspectRatio;
        } else if (scaleMode == "keepAspectRatioByExpanding") {
            m_backgroundScaleMode = BackgroundScaleMode::KeepAspectRatioByExpanding;
        } else if (scaleMode == "center") {
            m_backgroundScaleMode = BackgroundScaleMode::Center;
        } else if (scaleMode == "tile") {
            m_backgroundScaleMode = BackgroundScaleMode::Tile;
        }
    }
    
    if (settings.contains("backgroundOpacity")) {
        m_backgroundOpacity = settings["backgroundOpacity"].toDouble();
        if (m_backgroundOpacity < 0.0) m_backgroundOpacity = 0.0;
        if (m_backgroundOpacity > 1.0) m_backgroundOpacity = 1.0;
    }
}

void ClockWidget::loadBackgroundImage() {
    if (m_backgroundImagePath.isEmpty()) {
        m_backgroundImage = QPixmap();
        return;
    }
    
    QString imagePath = m_backgroundImagePath;
    
    // 如果是相对路径，转换为绝对路径
    if (!QFileInfo(imagePath).isAbsolute()) {
        imagePath = QApplication::applicationDirPath() + "/" + imagePath;
    }
    
    QPixmap image(imagePath);
    if (!image.isNull()) {
        m_backgroundImage = image;
        m_useBackgroundImage = true;
        qDebug() << "ClockWidget: 成功加载背景图片:" << imagePath;
    } else {
        m_backgroundImage = QPixmap();
        m_useBackgroundImage = false;
        qDebug() << "ClockWidget: 加载背景图片失败:" << imagePath;
    }
}

void ClockWidget::drawBackground(QPainter& painter) {
    if (!m_useBackgroundImage || m_backgroundImage.isNull()) {
        // 使用纯色背景
        painter.fillRect(rect(), m_backgroundColor);
        return;
    }
    
    // 设置背景图片透明度
    painter.setOpacity(m_backgroundOpacity);
    
    QRect targetRect = rect();
    QPixmap scaledImage;
    
    switch (m_backgroundScaleMode) {
        case BackgroundScaleMode::Stretch:
            scaledImage = m_backgroundImage.scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(targetRect, scaledImage);
            break;
            
        case BackgroundScaleMode::KeepAspectRatio:
            scaledImage = m_backgroundImage.scaled(targetRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            {
                QRect imageRect = scaledImage.rect();
                imageRect.moveCenter(targetRect.center());
                painter.drawPixmap(imageRect, scaledImage);
            }
            break;
            
        case BackgroundScaleMode::KeepAspectRatioByExpanding:
            scaledImage = m_backgroundImage.scaled(targetRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            {
                QRect imageRect = scaledImage.rect();
                imageRect.moveCenter(targetRect.center());
                painter.drawPixmap(targetRect, scaledImage, imageRect);
            }
            break;
            
        case BackgroundScaleMode::Center:
            {
                QRect imageRect = m_backgroundImage.rect();
                imageRect.moveCenter(targetRect.center());
                painter.drawPixmap(imageRect, m_backgroundImage);
            }
            break;
            
        case BackgroundScaleMode::Tile:
            {
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
            break;
    }
    
    // 恢复透明度
    painter.setOpacity(1.0);
}

void ClockWidget::updateContent() {
    m_currentTime = QDateTime::currentDateTime();
    update(); // 触发重绘
}

void ClockWidget::drawContent(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景（主题图片或纯色背景）
    drawBackground(painter);
    
    // 绘制边框（仅在没有背景图片时显示）
    if (!m_useBackgroundImage || m_backgroundImage.isNull()) {
        painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 5, 5);
    }
    
    // 计算文本区域
    QRect timeRect = rect();
    QRect dateRect = rect();
    
    if (m_showDate) {
        int totalHeight = rect().height();
        int timeHeight = totalHeight * 0.6;
        int dateHeight = totalHeight * 0.4;
        
        timeRect = QRect(0, 0, rect().width(), timeHeight);
        dateRect = QRect(0, timeHeight, rect().width(), dateHeight);
    }
    
    // 绘制时间
    painter.setFont(m_timeFont);
    painter.setPen(m_timeColor);
    QString timeText = m_currentTime.toString(m_timeFormat);
    painter.drawText(timeRect, Qt::AlignCenter, timeText);
    
    // 绘制日期
    if (m_showDate) {
        painter.setFont(m_dateFont);
        painter.setPen(m_dateColor);
        QString dateText = m_currentTime.toString(m_dateFormat);
        painter.drawText(dateRect, Qt::AlignCenter, dateText);
    }
}

void ClockWidget::applyConfig() {
    BaseWidget::applyConfig();
    parseCustomSettings();
    updateContent();
} 