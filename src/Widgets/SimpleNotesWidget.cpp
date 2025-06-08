#include "Widgets/SimpleNotesWidget.h"
#include <QPainter>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QFontDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDateTime>

SimpleNotesWidget::SimpleNotesWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , m_mainLayout(nullptr)
    , m_textEdit(nullptr)
    , m_customContextMenu(nullptr)
    , m_clearAction(nullptr)
    , m_fontAction(nullptr)
    , m_textColorAction(nullptr)
    , m_backgroundColorAction(nullptr)
    , m_textChanged(false)
    , m_autoSave(true)
    , m_autoSaveInterval(30000) // 30秒
    , m_textFont("Arial", 12)
    , m_textColor(Qt::black)
    , m_backgroundColor(Qt::white)
    , m_widgetBackgroundColor(QColor(255, 255, 220)) // 浅黄色，类似便签纸
    , m_autoSaveTimer(nullptr)
{
    parseCustomSettings();
    setupUI();
    loadNote();
    
    // 设置自动保存定时器
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &SimpleNotesWidget::onAutoSave);
    if (m_autoSave) {
        m_autoSaveTimer->start(m_autoSaveInterval);
    }
    
    setMinimumSize(200, 150);
}

void SimpleNotesWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(0);
    
    // 创建文本编辑器
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("在此输入便签内容...");
    m_textEdit->setFrameStyle(QFrame::NoFrame);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    
    // 设置样式
    updateTextStyle();
    
    m_mainLayout->addWidget(m_textEdit);
    
    // 连接信号
    connect(m_textEdit, &QTextEdit::textChanged, this, &SimpleNotesWidget::onTextChanged);
}

void SimpleNotesWidget::parseCustomSettings() {
    const QJsonObject& settings = m_config.customSettings;
    
    if (settings.contains("autoSave")) {
        m_autoSave = settings["autoSave"].toBool();
    }
    
    if (settings.contains("autoSaveInterval")) {
        m_autoSaveInterval = settings["autoSaveInterval"].toInt();
    }
    
    if (settings.contains("noteFilePath")) {
        m_noteFilePath = settings["noteFilePath"].toString();
    } else {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_noteFilePath = dataDir + QString("/simple_note_%1.txt").arg(m_config.id);
    }
    
    // 字体设置
    if (settings.contains("fontFamily")) {
        m_textFont.setFamily(settings["fontFamily"].toString());
    }
    if (settings.contains("fontSize")) {
        m_textFont.setPointSize(settings["fontSize"].toInt());
    }
    
    // 颜色设置
    if (settings.contains("textColor")) {
        m_textColor = QColor(settings["textColor"].toString());
    }
    if (settings.contains("backgroundColor")) {
        m_backgroundColor = QColor(settings["backgroundColor"].toString());
    }
    if (settings.contains("widgetBackgroundColor")) {
        m_widgetBackgroundColor = QColor(settings["widgetBackgroundColor"].toString());
    }
}

void SimpleNotesWidget::updateTextStyle() {
    if (!m_textEdit) return;
    
    m_textEdit->setFont(m_textFont);
    
    // 设置文本编辑器的颜色
    QPalette palette = m_textEdit->palette();
    palette.setColor(QPalette::Text, m_textColor);
    palette.setColor(QPalette::Base, m_backgroundColor);
    m_textEdit->setPalette(palette);
    
    // 设置样式表以确保颜色正确应用
    QString styleSheet = QString(
        "QTextEdit {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: none;"
        "    selection-background-color: rgba(0, 123, 255, 100);"
        "}"
    ).arg(m_backgroundColor.name()).arg(m_textColor.name());
    
    m_textEdit->setStyleSheet(styleSheet);
}

QMenu* SimpleNotesWidget::createCustomContextMenu() {
    if (!m_customContextMenu) {
        m_customContextMenu = new QMenu(this);
        
        // 清空文本
        m_clearAction = new QAction("清空文本", this);
        connect(m_clearAction, &QAction::triggered, this, &SimpleNotesWidget::onClearText);
        m_customContextMenu->addAction(m_clearAction);
        
        m_customContextMenu->addSeparator();
        
        // 字体设置
        m_fontAction = new QAction("更改字体...", this);
        connect(m_fontAction, &QAction::triggered, this, &SimpleNotesWidget::onChangeFont);
        m_customContextMenu->addAction(m_fontAction);
        
        // 文本颜色
        m_textColorAction = new QAction("文本颜色...", this);
        connect(m_textColorAction, &QAction::triggered, this, &SimpleNotesWidget::onChangeTextColor);
        m_customContextMenu->addAction(m_textColorAction);
        
        // 背景颜色
        m_backgroundColorAction = new QAction("背景颜色...", this);
        connect(m_backgroundColorAction, &QAction::triggered, this, &SimpleNotesWidget::onChangeBackgroundColor);
        m_customContextMenu->addAction(m_backgroundColorAction);
    }
    
    return m_customContextMenu;
}

void SimpleNotesWidget::onTextChanged() {
    m_textChanged = true;
    m_noteContent = m_textEdit->toPlainText();
}

void SimpleNotesWidget::onAutoSave() {
    if (m_textChanged) {
        saveNote();
        m_textChanged = false;
    }
}

void SimpleNotesWidget::onClearText() {
    int ret = QMessageBox::question(this, "清空便签", 
        "确定要清空便签内容吗？",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        m_textEdit->clear();
        m_noteContent.clear();
        saveNote();
    }
}

void SimpleNotesWidget::onChangeFont() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_textFont, this, "选择字体");
    
    if (ok) {
        m_textFont = font;
        updateTextStyle();
        
        // 保存到配置
        QJsonObject settings = m_config.customSettings;
        settings["fontFamily"] = m_textFont.family();
        settings["fontSize"] = m_textFont.pointSize();
        m_config.customSettings = settings;
        
        m_textChanged = true;
    }
}

void SimpleNotesWidget::onChangeTextColor() {
    QColor color = QColorDialog::getColor(m_textColor, this, "选择文本颜色");
    
    if (color.isValid()) {
        m_textColor = color;
        updateTextStyle();
        
        // 保存到配置
        QJsonObject settings = m_config.customSettings;
        settings["textColor"] = m_textColor.name();
        m_config.customSettings = settings;
        
        m_textChanged = true;
    }
}

void SimpleNotesWidget::onChangeBackgroundColor() {
    QColor color = QColorDialog::getColor(m_backgroundColor, this, "选择背景颜色");
    
    if (color.isValid()) {
        m_backgroundColor = color;
        updateTextStyle();
        
        // 保存到配置
        QJsonObject settings = m_config.customSettings;
        settings["backgroundColor"] = m_backgroundColor.name();
        m_config.customSettings = settings;
        
        update(); // 触发重绘
        m_textChanged = true;
    }
}

void SimpleNotesWidget::saveNote() {
    QFile file(m_noteFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << m_noteContent;
        file.close();
    }
}

void SimpleNotesWidget::loadNote() {
    QFile file(m_noteFilePath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        m_noteContent = in.readAll();
        file.close();
        
        if (m_textEdit) {
            m_textEdit->setPlainText(m_noteContent);
        }
    }
}

void SimpleNotesWidget::updateContent() {
    // 极简便签不需要定期更新内容
    // 可以在这里检查文件变化等，但一般不需要
}

void SimpleNotesWidget::drawContent(QPainter& painter) {
    // 绘制小组件背景
    painter.fillRect(rect(), m_widgetBackgroundColor);
    
    // 绘制边框
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void SimpleNotesWidget::applyConfig() {
    BaseWidget::applyConfig();
    parseCustomSettings();
    updateTextStyle();
    
    if (m_autoSaveTimer) {
        if (m_autoSave) {
            m_autoSaveTimer->start(m_autoSaveInterval);
        } else {
            m_autoSaveTimer->stop();
        }
    }
    
    update();
}

void SimpleNotesWidget::resizeEvent(QResizeEvent* event) {
    BaseWidget::resizeEvent(event);
    // 可以在这里调整布局
}

void SimpleNotesWidget::contextMenuEvent(QContextMenuEvent* event) {
    // 如果点击在文本编辑器上，显示自定义菜单
    if (m_textEdit && m_textEdit->geometry().contains(event->pos())) {
        QMenu* customMenu = createCustomContextMenu();
        customMenu->exec(event->globalPos());
    } else {
        // 否则显示基础菜单
        BaseWidget::contextMenuEvent(event);
    }
} 