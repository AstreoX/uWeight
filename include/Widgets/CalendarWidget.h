#pragma once
#include "Core/BaseWidget.h"
#include "Common/WidgetEnums.h"
#include <QDate>
#include <QFont>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QCalendar>
#include <QLocale>

class CalendarWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit CalendarWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~CalendarWidget() = default;

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void applyConfig() override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setupDefaultConfig();
    void parseCustomSettings();
    void loadBackgroundImage();
    void drawBackground(QPainter& painter);
    void drawCalendarGrid(QPainter& painter);
    void drawHeader(QPainter& painter);
    void drawWeekHeaders(QPainter& painter);
    void drawDates(QPainter& painter);
    void drawSelectedDate(QPainter& painter, const QRect& dateRect);
    void drawTodayHighlight(QPainter& painter, const QRect& dateRect);
    
    // 布局计算
    QRect getHeaderRect() const;
    QRect getWeekHeaderRect() const;
    QRect getCalendarGridRect() const;
    QRect getDateRect(int row, int col) const;
    QSize getCellSize() const;
    
    // 日期相关
    QDate getFirstDateOfMonth() const;
    int getWeeksInMonth() const;
    int getDayOfWeek(const QDate& date) const;
    bool isToday(const QDate& date) const;
    bool isCurrentMonth(const QDate& date) const;
    
    // 导航相关
    void navigateToNextMonth();
    void navigateToPreviousMonth();
    void navigateToToday();
    QDate getDateFromPosition(const QPoint& position) const;
    
    // 农历相关（可选）
    QString getLunarDate(const QDate& date) const;
    bool shouldShowLunar() const;

private slots:
    void onMonthChanged();

private:
    // 当前显示的日期
    QDate m_currentDate;
    QDate m_selectedDate;
    QDate m_today;
    
    // 显示配置
    CalendarStyle m_style;
    WeekStartDay m_weekStartDay;
    bool m_showLunar;
    bool m_showWeekNumbers;
    bool m_highlightToday;
    bool m_showOtherMonths;
    
    // 字体和颜色
    QFont m_headerFont;
    QFont m_weekHeaderFont;
    QFont m_dateFont;
    QFont m_lunarFont;
    
    QColor m_headerColor;
    QColor m_weekHeaderColor;
    QColor m_dateColor;
    QColor m_lunarColor;
    QColor m_todayColor;
    QColor m_selectedColor;
    QColor m_otherMonthColor;
    QColor m_gridColor;
    QColor m_backgroundColor;
    
    // 背景相关
    QPixmap m_backgroundImage;
    QString m_backgroundImagePath;
    BackgroundScaleMode m_backgroundScaleMode;
    double m_backgroundOpacity;
    bool m_useBackgroundImage;
    
    // 布局参数
    int m_headerHeight;
    int m_weekHeaderHeight;
    int m_cellPadding;
    int m_borderRadius;
    
    // 导航区域
    QRect m_prevButtonRect;
    QRect m_nextButtonRect;
    QRect m_headerTextRect;
    
    // 语言支持
    QLocale m_locale;
    QStringList m_weekDayNames;
    QStringList m_monthNames;
}; 