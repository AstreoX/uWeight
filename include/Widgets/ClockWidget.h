#pragma once
#include "Core/BaseWidget.h"
#include "Common/WidgetEnums.h"
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QPainter>
#include <QPixmap>

class ClockWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit ClockWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~ClockWidget() = default;

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void applyConfig() override;

private:
    void setupDefaultConfig();
    void parseCustomSettings();
    void loadBackgroundImage();
    void drawBackground(QPainter& painter);

private:
    QDateTime m_currentTime;
    QFont m_timeFont;
    QFont m_dateFont;
    QColor m_timeColor;
    QColor m_dateColor;
    QColor m_backgroundColor;
    bool m_showDate;
    bool m_show24Hour;
    bool m_showSeconds;
    QString m_timeFormat;
    QString m_dateFormat;
    
    // 背景图片相关
    QPixmap m_backgroundImage;
    QString m_backgroundImagePath;
    BackgroundScaleMode m_backgroundScaleMode;
    double m_backgroundOpacity;
    bool m_useBackgroundImage;
}; 