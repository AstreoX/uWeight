#include "Widgets/CalendarWidget.h"
#include <QPainter>
#include <QJsonObject>
#include <QRect>
#include <QWheelEvent>
#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <cmath>

CalendarWidget::CalendarWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , m_currentDate(QDate::currentDate())
    , m_selectedDate(QDate::currentDate())
    , m_today(QDate::currentDate())
    , m_style(CalendarStyle::Modern)
    , m_weekStartDay(WeekStartDay::Monday)
    , m_showLunar(false)
    , m_showWeekNumbers(false)
    , m_highlightToday(true)
    , m_showOtherMonths(true)
    , m_headerColor(Qt::white)
    , m_weekHeaderColor(QColor(200, 200, 200))
    , m_dateColor(Qt::white)
    , m_lunarColor(QColor(150, 150, 150))
    , m_todayColor(QColor(0, 120, 215))
    , m_selectedColor(QColor(255, 140, 0))
    , m_otherMonthColor(QColor(100, 100, 100))
    , m_gridColor(QColor(60, 60, 60))
    , m_backgroundColor(QColor(30, 30, 30, 200))
    , m_backgroundScaleMode(BackgroundScaleMode::Stretch)
    , m_backgroundOpacity(1.0)
    , m_useBackgroundImage(false)
    , m_headerHeight(40)
    , m_weekHeaderHeight(25)
    , m_cellPadding(2)
    , m_borderRadius(8)
    , m_locale(QLocale::Chinese)
{
    setupDefaultConfig();
    parseCustomSettings();
    setMinimumSize(280, 320);
    setFixedSize(300, 350);
}

void CalendarWidget::setupDefaultConfig() {
    // 设置默认字体
    m_headerFont = QFont("Microsoft YaHei", 12, QFont::Bold);
    m_weekHeaderFont = QFont("Microsoft YaHei", 9);
    m_dateFont = QFont("Microsoft YaHei", 10);
    m_lunarFont = QFont("Microsoft YaHei", 7);
    
    // 设置周天名称
    if (m_locale.language() == QLocale::Chinese) {
        if (m_weekStartDay == WeekStartDay::Monday) {
            m_weekDayNames = {"一", "二", "三", "四", "五", "六", "日"};
        } else {
            m_weekDayNames = {"日", "一", "二", "三", "四", "五", "六"};
        }
    } else {
        if (m_weekStartDay == WeekStartDay::Monday) {
            m_weekDayNames = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        } else {
            m_weekDayNames = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        }
    }
}

void CalendarWidget::parseCustomSettings() {
    const QJsonObject& settings = m_config.customSettings;
    
    // 显示设置
    if (settings.contains("showLunar")) {
        m_showLunar = settings["showLunar"].toBool();
    }
    
    if (settings.contains("showWeekNumbers")) {
        m_showWeekNumbers = settings["showWeekNumbers"].toBool();
    }
    
    if (settings.contains("highlightToday")) {
        m_highlightToday = settings["highlightToday"].toBool();
    }
    
    if (settings.contains("showOtherMonths")) {
        m_showOtherMonths = settings["showOtherMonths"].toBool();
    }
    
    if (settings.contains("weekStartDay")) {
        int startDay = settings["weekStartDay"].toInt();
        m_weekStartDay = (startDay == 0) ? WeekStartDay::Sunday : WeekStartDay::Monday;
        setupDefaultConfig(); // 重新设置周天名称
    }
    
    // 样式设置
    if (settings.contains("calendarStyle")) {
        QString style = settings["calendarStyle"].toString();
        if (style == "modern") m_style = CalendarStyle::Modern;
        else if (style == "classic") m_style = CalendarStyle::Classic;
        else if (style == "minimal") m_style = CalendarStyle::Minimal;
        else if (style == "rounded") m_style = CalendarStyle::Rounded;
    }
    
    // 颜色设置
    if (settings.contains("headerColor")) {
        m_headerColor = QColor(settings["headerColor"].toString());
    }
    
    if (settings.contains("weekHeaderColor")) {
        m_weekHeaderColor = QColor(settings["weekHeaderColor"].toString());
    }
    
    if (settings.contains("dateColor")) {
        m_dateColor = QColor(settings["dateColor"].toString());
    }
    
    if (settings.contains("lunarColor")) {
        m_lunarColor = QColor(settings["lunarColor"].toString());
    }
    
    if (settings.contains("todayColor")) {
        m_todayColor = QColor(settings["todayColor"].toString());
    }
    
    if (settings.contains("selectedColor")) {
        m_selectedColor = QColor(settings["selectedColor"].toString());
    }
    
    if (settings.contains("otherMonthColor")) {
        m_otherMonthColor = QColor(settings["otherMonthColor"].toString());
    }
    
    if (settings.contains("gridColor")) {
        m_gridColor = QColor(settings["gridColor"].toString());
    }
    
    if (settings.contains("backgroundColor")) {
        m_backgroundColor = QColor(settings["backgroundColor"].toString());
    }
    
    // 字体设置
    if (settings.contains("headerFontSize")) {
        m_headerFont.setPointSize(settings["headerFontSize"].toInt());
    }
    
    if (settings.contains("weekHeaderFontSize")) {
        m_weekHeaderFont.setPointSize(settings["weekHeaderFontSize"].toInt());
    }
    
    if (settings.contains("dateFontSize")) {
        m_dateFont.setPointSize(settings["dateFontSize"].toInt());
    }
    
    if (settings.contains("lunarFontSize")) {
        m_lunarFont.setPointSize(settings["lunarFontSize"].toInt());
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
    
    // 布局参数
    if (settings.contains("headerHeight")) {
        m_headerHeight = settings["headerHeight"].toInt();
    }
    
    if (settings.contains("weekHeaderHeight")) {
        m_weekHeaderHeight = settings["weekHeaderHeight"].toInt();
    }
    
    if (settings.contains("cellPadding")) {
        m_cellPadding = settings["cellPadding"].toInt();
    }
    
    if (settings.contains("borderRadius")) {
        m_borderRadius = settings["borderRadius"].toInt();
    }
}

void CalendarWidget::loadBackgroundImage() {
    if (m_backgroundImagePath.isEmpty()) {
        m_backgroundImage = QPixmap();
        return;
    }
    
    QPixmap image(m_backgroundImagePath);
    if (!image.isNull()) {
        m_backgroundImage = image;
        m_useBackgroundImage = true;
    } else {
        m_backgroundImage = QPixmap();
        m_useBackgroundImage = false;
    }
}

void CalendarWidget::drawBackground(QPainter& painter) {
    if (!m_useBackgroundImage || m_backgroundImage.isNull()) {
        // 使用纯色背景
        if (m_style == CalendarStyle::Rounded) {
            painter.fillRect(rect(), Qt::transparent);
            painter.setBrush(m_backgroundColor);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), m_borderRadius, m_borderRadius);
        } else {
            painter.fillRect(rect(), m_backgroundColor);
        }
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

void CalendarWidget::updateContent() {
    m_today = QDate::currentDate();
    update(); // 触发重绘
}

void CalendarWidget::drawContent(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    drawBackground(painter);
    
    // 绘制边框
    if (m_style != CalendarStyle::Minimal) {
        painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
        if (m_style == CalendarStyle::Rounded) {
            painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), m_borderRadius, m_borderRadius);
        } else {
            painter.drawRect(rect().adjusted(1, 1, -1, -1));
        }
    }
    
    // 绘制日历各部分
    drawHeader(painter);
    drawWeekHeaders(painter);
    drawCalendarGrid(painter);
    drawDates(painter);
}

void CalendarWidget::drawHeader(QPainter& painter) {
    QRect headerRect = getHeaderRect();
    
    // 计算导航按钮区域
    int buttonSize = headerRect.height() - 8;
    m_prevButtonRect = QRect(headerRect.left() + 5, headerRect.top() + 4, buttonSize, buttonSize);
    m_nextButtonRect = QRect(headerRect.right() - buttonSize - 5, headerRect.top() + 4, buttonSize, buttonSize);
    m_headerTextRect = QRect(m_prevButtonRect.right() + 5, headerRect.top(), 
                           m_nextButtonRect.left() - m_prevButtonRect.right() - 10, headerRect.height());
    
    // 绘制标题文本
    painter.setFont(m_headerFont);
    painter.setPen(m_headerColor);
    QString headerText;
    if (m_locale.language() == QLocale::Chinese) {
        headerText = QString("%1年%2月").arg(m_currentDate.year()).arg(m_currentDate.month());
    } else {
        headerText = m_currentDate.toString("MMMM yyyy");
    }
    painter.drawText(m_headerTextRect, Qt::AlignCenter, headerText);
    
    // 绘制导航按钮
    painter.setPen(QPen(m_headerColor, 2));
    painter.setBrush(Qt::NoBrush);
    
    // 上个月按钮 (<)
    QRect prevTriangle = m_prevButtonRect.adjusted(6, 6, -6, -6);
    QPolygon prevArrow;
    prevArrow << QPoint(prevTriangle.right(), prevTriangle.top())
              << QPoint(prevTriangle.left() + prevTriangle.width()/3, prevTriangle.center().y())
              << QPoint(prevTriangle.right(), prevTriangle.bottom());
    painter.drawPolyline(prevArrow);
    
    // 下个月按钮 (>)
    QRect nextTriangle = m_nextButtonRect.adjusted(6, 6, -6, -6);
    QPolygon nextArrow;
    nextArrow << QPoint(nextTriangle.left(), nextTriangle.top())
              << QPoint(nextTriangle.right() - nextTriangle.width()/3, nextTriangle.center().y())
              << QPoint(nextTriangle.left(), nextTriangle.bottom());
    painter.drawPolyline(nextArrow);
}

void CalendarWidget::drawWeekHeaders(QPainter& painter) {
    QRect weekHeaderRect = getWeekHeaderRect();
    QSize cellSize = getCellSize();
    
    painter.setFont(m_weekHeaderFont);
    painter.setPen(m_weekHeaderColor);
    
    for (int i = 0; i < 7; ++i) {
        QRect cellRect(weekHeaderRect.left() + i * cellSize.width(), weekHeaderRect.top(),
                      cellSize.width(), weekHeaderRect.height());
        painter.drawText(cellRect, Qt::AlignCenter, m_weekDayNames[i]);
    }
    
    // 绘制分隔线
    if (m_style != CalendarStyle::Minimal) {
        painter.setPen(QPen(m_gridColor, 1));
        painter.drawLine(weekHeaderRect.bottomLeft(), weekHeaderRect.bottomRight());
    }
}

void CalendarWidget::drawCalendarGrid(QPainter& painter) {
    if (m_style == CalendarStyle::Minimal) {
        return; // 极简风格不绘制网格
    }
    
    QRect gridRect = getCalendarGridRect();
    QSize cellSize = getCellSize();
    int weeks = getWeeksInMonth();
    
    painter.setPen(QPen(m_gridColor, 1));
    
    // 绘制垂直线
    for (int i = 0; i <= 7; ++i) {
        int x = gridRect.left() + i * cellSize.width();
        painter.drawLine(x, gridRect.top(), x, gridRect.bottom());
    }
    
    // 绘制水平线
    for (int i = 0; i <= weeks; ++i) {
        int y = gridRect.top() + i * cellSize.height();
        painter.drawLine(gridRect.left(), y, gridRect.right(), y);
    }
}

void CalendarWidget::drawDates(QPainter& painter) {
    QDate firstDate = getFirstDateOfMonth();
    int weeks = getWeeksInMonth();
    
    painter.setFont(m_dateFont);
    
    for (int week = 0; week < weeks; ++week) {
        for (int day = 0; day < 7; ++day) {
            QDate currentDate = firstDate.addDays(week * 7 + day);
            QRect dateRect = getDateRect(week, day);
            
            // 判断日期类型
            bool isCurrentMonth = this->isCurrentMonth(currentDate);
            bool isTodayDate = isToday(currentDate);
            bool isSelected = (currentDate == m_selectedDate);
            
            // 如果不显示其他月份的日期，跳过
            if (!isCurrentMonth && !m_showOtherMonths) {
                continue;
            }
            
            // 绘制今天的高亮
            if (isTodayDate && m_highlightToday) {
                drawTodayHighlight(painter, dateRect);
            }
            
            // 绘制选中日期的高亮
            if (isSelected) {
                drawSelectedDate(painter, dateRect);
            }
            
            // 设置文本颜色
            if (isTodayDate && m_highlightToday) {
                painter.setPen(Qt::white);
            } else if (isCurrentMonth) {
                painter.setPen(m_dateColor);
            } else {
                painter.setPen(m_otherMonthColor);
            }
            
            // 绘制日期数字
            QString dateText = QString::number(currentDate.day());
            QRect textRect = dateRect.adjusted(m_cellPadding, m_cellPadding, -m_cellPadding, -m_cellPadding);
            
            if (m_showLunar) {
                // 如果显示农历，调整布局
                QRect dateNumRect = QRect(textRect.left(), textRect.top(), 
                                        textRect.width(), textRect.height() * 0.6);
                QRect lunarRect = QRect(textRect.left(), dateNumRect.bottom(), 
                                      textRect.width(), textRect.height() * 0.4);
                
                painter.drawText(dateNumRect, Qt::AlignCenter, dateText);
                
                // 绘制农历信息
                painter.setFont(m_lunarFont);
                painter.setPen(m_lunarColor);
                QString lunarText = getLunarDate(currentDate);
                painter.drawText(lunarRect, Qt::AlignCenter, lunarText);
                painter.setFont(m_dateFont);
            } else {
                painter.drawText(textRect, Qt::AlignCenter, dateText);
            }
        }
    }
}

void CalendarWidget::drawTodayHighlight(QPainter& painter, const QRect& dateRect) {
    painter.setBrush(m_todayColor);
    painter.setPen(Qt::NoPen);
    
    if (m_style == CalendarStyle::Rounded) {
        painter.drawRoundedRect(dateRect.adjusted(2, 2, -2, -2), 4, 4);
    } else {
        painter.drawRect(dateRect.adjusted(1, 1, -1, -1));
    }
}

void CalendarWidget::drawSelectedDate(QPainter& painter, const QRect& dateRect) {
    painter.setPen(QPen(m_selectedColor, 2));
    painter.setBrush(Qt::NoBrush);
    
    if (m_style == CalendarStyle::Rounded) {
        painter.drawRoundedRect(dateRect.adjusted(2, 2, -2, -2), 4, 4);
    } else {
        painter.drawRect(dateRect.adjusted(1, 1, -1, -1));
    }
}

QRect CalendarWidget::getHeaderRect() const {
    return QRect(0, 0, width(), m_headerHeight);
}

QRect CalendarWidget::getWeekHeaderRect() const {
    return QRect(0, m_headerHeight, width(), m_weekHeaderHeight);
}

QRect CalendarWidget::getCalendarGridRect() const {
    int topOffset = m_headerHeight + m_weekHeaderHeight;
    return QRect(0, topOffset, width(), height() - topOffset);
}

QRect CalendarWidget::getDateRect(int row, int col) const {
    QRect gridRect = getCalendarGridRect();
    QSize cellSize = getCellSize();
    
    return QRect(gridRect.left() + col * cellSize.width(),
                gridRect.top() + row * cellSize.height(),
                cellSize.width(), cellSize.height());
}

QSize CalendarWidget::getCellSize() const {
    QRect gridRect = getCalendarGridRect();
    int weeks = getWeeksInMonth();
    return QSize(gridRect.width() / 7, gridRect.height() / weeks);
}

QDate CalendarWidget::getFirstDateOfMonth() const {
    QDate firstOfMonth(m_currentDate.year(), m_currentDate.month(), 1);
    int dayOfWeek = getDayOfWeek(firstOfMonth);
    return firstOfMonth.addDays(-dayOfWeek);
}

int CalendarWidget::getWeeksInMonth() const {
    QDate firstOfMonth(m_currentDate.year(), m_currentDate.month(), 1);
    
    int firstDayOfWeek = getDayOfWeek(firstOfMonth);
    int daysInMonth = m_currentDate.daysInMonth();
    int totalDays = firstDayOfWeek + daysInMonth;
    
    return (totalDays + 6) / 7; // 向上取整
}

int CalendarWidget::getDayOfWeek(const QDate& date) const {
    int qtDayOfWeek = date.dayOfWeek(); // Qt: 1=Monday, 7=Sunday
    
    if (m_weekStartDay == WeekStartDay::Monday) {
        return qtDayOfWeek - 1; // 0=Monday, 6=Sunday
    } else {
        return qtDayOfWeek % 7; // 0=Sunday, 6=Saturday
    }
}

bool CalendarWidget::isToday(const QDate& date) const {
    return date == m_today;
}

bool CalendarWidget::isCurrentMonth(const QDate& date) const {
    return date.month() == m_currentDate.month() && date.year() == m_currentDate.year();
}

void CalendarWidget::navigateToNextMonth() {
    m_currentDate = m_currentDate.addMonths(1);
    onMonthChanged();
    update();
}

void CalendarWidget::navigateToPreviousMonth() {
    m_currentDate = m_currentDate.addMonths(-1);
    onMonthChanged();
    update();
}

void CalendarWidget::navigateToToday() {
    m_currentDate = QDate::currentDate();
    m_selectedDate = m_currentDate;
    onMonthChanged();
    update();
}

QDate CalendarWidget::getDateFromPosition(const QPoint& position) const {
    QRect gridRect = getCalendarGridRect();
    if (!gridRect.contains(position)) {
        return QDate();
    }
    
    QSize cellSize = getCellSize();
    int col = (position.x() - gridRect.left()) / cellSize.width();
    int row = (position.y() - gridRect.top()) / cellSize.height();
    
    if (col < 0 || col >= 7 || row < 0 || row >= getWeeksInMonth()) {
        return QDate();
    }
    
    QDate firstDate = getFirstDateOfMonth();
    return firstDate.addDays(row * 7 + col);
}

QString CalendarWidget::getLunarDate(const QDate& date) const {
    // 这里是一个简化的农历显示实现
    // 实际项目中可能需要使用专门的农历计算库
    static QStringList lunarDays = {
        "初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
        "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
        "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"
    };
    
    // 简单的模拟实现，实际应该使用真正的农历计算
    int dayIndex = (date.dayOfYear() + date.year()) % 30;
    return lunarDays[dayIndex];
}

bool CalendarWidget::shouldShowLunar() const {
    return m_showLunar && m_locale.language() == QLocale::Chinese;
}

void CalendarWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击了导航按钮
        if (m_prevButtonRect.contains(event->pos())) {
            navigateToPreviousMonth();
            return;
        } else if (m_nextButtonRect.contains(event->pos())) {
            navigateToNextMonth();
            return;
        } else if (m_headerTextRect.contains(event->pos())) {
            navigateToToday();
            return;
        }
        
        // 检查是否点击了日期
        QDate clickedDate = getDateFromPosition(event->pos());
        if (clickedDate.isValid()) {
            m_selectedDate = clickedDate;
            update();
        }
    }
    
    BaseWidget::mousePressEvent(event);
}

void CalendarWidget::wheelEvent(QWheelEvent* event) {
    // 鼠标滚轮切换月份
    if (event->angleDelta().y() > 0) {
        navigateToPreviousMonth();
    } else if (event->angleDelta().y() < 0) {
        navigateToNextMonth();
    }
    
    event->accept();
}

void CalendarWidget::onMonthChanged() {
    // 月份变化时的处理
    // 可以在这里添加月份变化的通知或其他逻辑
}

void CalendarWidget::applyConfig() {
    BaseWidget::applyConfig();
    parseCustomSettings();
    updateContent();
} 