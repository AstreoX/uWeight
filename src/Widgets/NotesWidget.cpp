#include "Widgets/NotesWidget.h"
#include <QPainter>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QUuid>
#include <QFontDialog>
#include <QColorDialog>
#include <QApplication>
#include <QListWidgetItem>
#include <QDateTime>

// Note结构体实现
QJsonObject Note::toJson() const {
    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["content"] = content;
    obj["created"] = created.toString(Qt::ISODate);
    obj["modified"] = modified.toString(Qt::ISODate);
    obj["fontFamily"] = font.family();
    obj["fontSize"] = font.pointSize();
    obj["fontWeight"] = font.weight();
    obj["fontItalic"] = font.italic();
    obj["textColor"] = textColor.name();
    obj["backgroundColor"] = backgroundColor.name();
    return obj;
}

Note Note::fromJson(const QJsonObject& json) {
    Note note;
    note.id = json["id"].toString();
    note.title = json["title"].toString();
    note.content = json["content"].toString();
    note.created = QDateTime::fromString(json["created"].toString(), Qt::ISODate);
    note.modified = QDateTime::fromString(json["modified"].toString(), Qt::ISODate);
    
    // 字体设置
    QFont font(json["fontFamily"].toString("Arial"), json["fontSize"].toInt(12));
    font.setWeight(static_cast<QFont::Weight>(json["fontWeight"].toInt(QFont::Normal)));
    font.setItalic(json["fontItalic"].toBool(false));
    note.font = font;
    
    note.textColor = QColor(json["textColor"].toString("#000000"));
    note.backgroundColor = QColor(json["backgroundColor"].toString("#ffffff"));
    
    return note;
}

NotesWidget::NotesWidget(const WidgetConfig& config, QWidget* parent)
    : BaseWidget(config, parent)
    , m_mainLayout(nullptr)
    , m_topLayout(nullptr)
    , m_splitter(nullptr)
    , m_leftPanel(nullptr)
    , m_leftLayout(nullptr)
    , m_searchEdit(nullptr)
    , m_notesList(nullptr)
    , m_newNoteButton(nullptr)
    , m_deleteNoteButton(nullptr)
    , m_rightPanel(nullptr)
    , m_rightLayout(nullptr)
    , m_titleEdit(nullptr)
    , m_textEdit(nullptr)
    , m_toolBar(nullptr)
    , m_fontCombo(nullptr)
    , m_fontSizeSpinBox(nullptr)
    , m_textColorButton(nullptr)
    , m_backgroundColorButton(nullptr)
    , m_textChanged(false)
    , m_autoSave(true)
    , m_autoSaveInterval(30000) // 30秒
    , m_autoSaveTimer(nullptr)
    , m_widgetBackgroundColor(QColor(240, 240, 240))
    , m_borderColor(QColor(200, 200, 200))
    , m_borderWidth(1)
{
    parseCustomSettings();
    setupUI();
    loadNotes();
    
    // 设置自动保存定时器
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &NotesWidget::onAutoSave);
    if (m_autoSave) {
        m_autoSaveTimer->start(m_autoSaveInterval);
    }
    
    setMinimumSize(400, 300);
}

void NotesWidget::setupUI() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(2);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);
    
    // 左侧面板 - 便签列表
    m_leftPanel = new QWidget();
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    m_leftLayout->setContentsMargins(2, 2, 2, 2);
    
    // 搜索框
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索便签...");
    m_leftLayout->addWidget(m_searchEdit);
    
    // 便签列表
    m_notesList = new QListWidget();
    m_notesList->setAlternatingRowColors(true);
    m_leftLayout->addWidget(m_notesList);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_newNoteButton = new QPushButton("新建");
    m_deleteNoteButton = new QPushButton("删除");
    m_deleteNoteButton->setEnabled(false);
    buttonLayout->addWidget(m_newNoteButton);
    buttonLayout->addWidget(m_deleteNoteButton);
    m_leftLayout->addLayout(buttonLayout);
    
    m_splitter->addWidget(m_leftPanel);
    
    // 右侧面板 - 编辑器
    m_rightPanel = new QWidget();
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    m_rightLayout->setContentsMargins(2, 2, 2, 2);
    
    // 工具栏
    setupToolBar();
    m_rightLayout->addWidget(m_toolBar);
    
    // 标题编辑框
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("便签标题...");
    m_titleEdit->setEnabled(false);
    m_rightLayout->addWidget(m_titleEdit);
    
    // 文本编辑器
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("在此输入便签内容...");
    m_textEdit->setEnabled(false);
    m_rightLayout->addWidget(m_textEdit);
    
    m_splitter->addWidget(m_rightPanel);
    
    // 设置分割器比例
    m_splitter->setSizes({120, 280});
    
    // 连接信号
    connect(m_newNoteButton, &QPushButton::clicked, this, &NotesWidget::onNewNote);
    connect(m_deleteNoteButton, &QPushButton::clicked, this, &NotesWidget::onDeleteNote);
    connect(m_notesList, &QListWidget::currentRowChanged, this, &NotesWidget::onNoteSelectionChanged);
    connect(m_titleEdit, &QLineEdit::textChanged, this, &NotesWidget::onTitleChanged);
    connect(m_textEdit, &QTextEdit::textChanged, this, &NotesWidget::onTextChanged);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &NotesWidget::onSearchTextChanged);
}

void NotesWidget::setupToolBar() {
    m_toolBar = new QToolBar();
    m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolBar->setIconSize(QSize(16, 16));
    
    // 字体选择
    m_fontCombo = new QFontComboBox();
    m_fontCombo->setMaximumWidth(120);
    m_fontCombo->setEnabled(false);
    m_toolBar->addWidget(m_fontCombo);
    
    // 字体大小
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(8, 72);
    m_fontSizeSpinBox->setValue(12);
    m_fontSizeSpinBox->setMaximumWidth(60);
    m_fontSizeSpinBox->setEnabled(false);
    m_toolBar->addWidget(m_fontSizeSpinBox);
    
    m_toolBar->addSeparator();
    
    // 文本颜色
    m_textColorButton = new QPushButton("A");
    m_textColorButton->setStyleSheet("QPushButton { color: black; font-weight: bold; }");
    m_textColorButton->setMaximumSize(24, 24);
    m_textColorButton->setEnabled(false);
    m_toolBar->addWidget(m_textColorButton);
    
    // 背景颜色
    m_backgroundColorButton = new QPushButton("■");
    m_backgroundColorButton->setStyleSheet("QPushButton { color: white; font-weight: bold; }");
    m_backgroundColorButton->setMaximumSize(24, 24);
    m_backgroundColorButton->setEnabled(false);
    m_toolBar->addWidget(m_backgroundColorButton);
    
    // 连接信号
    connect(m_fontCombo, QOverload<const QFont&>::of(&QFontComboBox::currentFontChanged),
            this, &NotesWidget::onFontChanged);
    connect(m_fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &NotesWidget::onFontSizeChanged);
    connect(m_textColorButton, &QPushButton::clicked, this, &NotesWidget::onTextColorChanged);
    connect(m_backgroundColorButton, &QPushButton::clicked, this, &NotesWidget::onBackgroundColorChanged);
}

void NotesWidget::parseCustomSettings() {
    const QJsonObject& settings = m_config.customSettings;
    
    if (settings.contains("autoSave")) {
        m_autoSave = settings["autoSave"].toBool();
    }
    
    if (settings.contains("autoSaveInterval")) {
        m_autoSaveInterval = settings["autoSaveInterval"].toInt();
    }
    
    if (settings.contains("notesFilePath")) {
        m_notesFilePath = settings["notesFilePath"].toString();
    } else {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        m_notesFilePath = dataDir + "/notes.json";
    }
    
    if (settings.contains("widgetBackgroundColor")) {
        m_widgetBackgroundColor = QColor(settings["widgetBackgroundColor"].toString());
    }
    
    if (settings.contains("borderColor")) {
        m_borderColor = QColor(settings["borderColor"].toString());
    }
    
    if (settings.contains("borderWidth")) {
        m_borderWidth = settings["borderWidth"].toInt();
    }
}

void NotesWidget::onNewNote() {
    Note newNote;
    newNote.id = generateNoteId();
    newNote.title = QString("新便签 %1").arg(m_notes.size() + 1);
    newNote.content = "";
    
    m_notes.append(newNote);
    updateNotesList();
    
    // 选择新建的便签
    for (int i = 0; i < m_notesList->count(); ++i) {
        QListWidgetItem* item = m_notesList->item(i);
        if (item->data(Qt::UserRole).toString() == newNote.id) {
            m_notesList->setCurrentItem(item);
            break;
        }
    }
    
    m_titleEdit->setFocus();
    m_titleEdit->selectAll();
}

void NotesWidget::onDeleteNote() {
    if (m_currentNoteId.isEmpty()) return;
    
    int result = QMessageBox::question(this, "删除便签", 
                                     "确定要删除当前便签吗？",
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        // 从列表中移除
        for (int i = 0; i < m_notes.size(); ++i) {
            if (m_notes[i].id == m_currentNoteId) {
                m_notes.removeAt(i);
                break;
            }
        }
        
        m_currentNoteId.clear();
        updateNotesList();
        
        // 清空编辑器
        m_titleEdit->clear();
        m_textEdit->clear();
        m_titleEdit->setEnabled(false);
        m_textEdit->setEnabled(false);
        m_fontCombo->setEnabled(false);
        m_fontSizeSpinBox->setEnabled(false);
        m_textColorButton->setEnabled(false);
        m_backgroundColorButton->setEnabled(false);
        m_deleteNoteButton->setEnabled(false);
        
        saveNotes();
    }
}

void NotesWidget::onNoteSelectionChanged() {
    int currentRow = m_notesList->currentRow();
    if (currentRow >= 0) {
        QListWidgetItem* item = m_notesList->item(currentRow);
        QString noteId = item->data(Qt::UserRole).toString();
        switchToNote(noteId);
    }
}

void NotesWidget::onTextChanged() {
    if (m_currentNoteId.isEmpty()) return;
    
    m_textChanged = true;
    updateCurrentNote();
}

void NotesWidget::onTitleChanged() {
    if (m_currentNoteId.isEmpty()) return;
    
    m_textChanged = true;
    updateCurrentNote();
    updateNotesList(); // 更新列表显示的标题
}

void NotesWidget::onFontChanged() {
    if (m_currentNoteId.isEmpty()) return;
    
    for (auto& note : m_notes) {
        if (note.id == m_currentNoteId) {
            note.font.setFamily(m_fontCombo->currentFont().family());
            note.modified = QDateTime::currentDateTime();
            break;
        }
    }
    
    // 应用字体到文本编辑器
    QFont font = m_textEdit->font();
    font.setFamily(m_fontCombo->currentFont().family());
    m_textEdit->setFont(font);
    
    m_textChanged = true;
}

void NotesWidget::onFontSizeChanged(int size) {
    if (m_currentNoteId.isEmpty()) return;
    
    for (auto& note : m_notes) {
        if (note.id == m_currentNoteId) {
            note.font.setPointSize(size);
            note.modified = QDateTime::currentDateTime();
            break;
        }
    }
    
    // 应用字体大小到文本编辑器
    QFont font = m_textEdit->font();
    font.setPointSize(size);
    m_textEdit->setFont(font);
    
    m_textChanged = true;
}

void NotesWidget::onTextColorChanged() {
    if (m_currentNoteId.isEmpty()) return;
    
    QColor color = QColorDialog::getColor(Qt::black, this, "选择文本颜色");
    if (color.isValid()) {
        for (auto& note : m_notes) {
            if (note.id == m_currentNoteId) {
                note.textColor = color;
                note.modified = QDateTime::currentDateTime();
                break;
            }
        }
        
        // 应用颜色到文本编辑器
        QPalette palette = m_textEdit->palette();
        palette.setColor(QPalette::Text, color);
        m_textEdit->setPalette(palette);
        
        // 更新按钮颜色
        m_textColorButton->setStyleSheet(QString("QPushButton { color: %1; font-weight: bold; }").arg(color.name()));
        
        m_textChanged = true;
    }
}

void NotesWidget::onBackgroundColorChanged() {
    if (m_currentNoteId.isEmpty()) return;
    
    QColor color = QColorDialog::getColor(Qt::white, this, "选择背景颜色");
    if (color.isValid()) {
        for (auto& note : m_notes) {
            if (note.id == m_currentNoteId) {
                note.backgroundColor = color;
                note.modified = QDateTime::currentDateTime();
                break;
            }
        }
        
        // 应用背景色到文本编辑器
        QPalette palette = m_textEdit->palette();
        palette.setColor(QPalette::Base, color);
        m_textEdit->setPalette(palette);
        
        // 更新按钮颜色
        m_backgroundColorButton->setStyleSheet(QString("QPushButton { background-color: %1; font-weight: bold; }").arg(color.name()));
        
        m_textChanged = true;
    }
}

void NotesWidget::onAutoSave() {
    if (m_textChanged) {
        saveNotes();
        m_textChanged = false;
    }
}

void NotesWidget::onSearchTextChanged(const QString& text) {
    filterNotes(text);
}

void NotesWidget::updateNotesList() {
    m_notesList->clear();
    
    for (const auto& note : m_notes) {
        QListWidgetItem* item = new QListWidgetItem(note.title.isEmpty() ? "无标题" : note.title);
        item->setData(Qt::UserRole, note.id);
        item->setToolTip(QString("创建: %1\n修改: %2")
                        .arg(note.created.toString("yyyy-MM-dd hh:mm"))
                        .arg(note.modified.toString("yyyy-MM-dd hh:mm")));
        m_notesList->addItem(item);
    }
}

void NotesWidget::updateCurrentNote() {
    if (m_currentNoteId.isEmpty()) return;
    
    for (auto& note : m_notes) {
        if (note.id == m_currentNoteId) {
            note.title = m_titleEdit->text();
            note.content = m_textEdit->toPlainText();
            note.modified = QDateTime::currentDateTime();
            break;
        }
    }
}

void NotesWidget::switchToNote(const QString& noteId) {
    if (noteId == m_currentNoteId) return;
    
    // 保存当前便签
    if (!m_currentNoteId.isEmpty()) {
        updateCurrentNote();
    }
    
    m_currentNoteId = noteId;
    
    // 加载新便签
    for (const auto& note : m_notes) {
        if (note.id == noteId) {
            m_titleEdit->setText(note.title);
            m_textEdit->setPlainText(note.content);
            
            // 设置字体
            m_fontCombo->setCurrentFont(note.font);
            m_fontSizeSpinBox->setValue(note.font.pointSize());
            m_textEdit->setFont(note.font);
            
            // 设置颜色
            QPalette palette = m_textEdit->palette();
            palette.setColor(QPalette::Text, note.textColor);
            palette.setColor(QPalette::Base, note.backgroundColor);
            m_textEdit->setPalette(palette);
            
            // 更新按钮颜色
            m_textColorButton->setStyleSheet(QString("QPushButton { color: %1; font-weight: bold; }").arg(note.textColor.name()));
            m_backgroundColorButton->setStyleSheet(QString("QPushButton { background-color: %1; font-weight: bold; }").arg(note.backgroundColor.name()));
            
            break;
        }
    }
    
    // 启用编辑器
    m_titleEdit->setEnabled(true);
    m_textEdit->setEnabled(true);
    m_fontCombo->setEnabled(true);
    m_fontSizeSpinBox->setEnabled(true);
    m_textColorButton->setEnabled(true);
    m_backgroundColorButton->setEnabled(true);
    m_deleteNoteButton->setEnabled(true);
    
    m_textChanged = false;
}

QString NotesWidget::generateNoteId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void NotesWidget::filterNotes(const QString& filter) {
    for (int i = 0; i < m_notesList->count(); ++i) {
        QListWidgetItem* item = m_notesList->item(i);
        QString noteId = item->data(Qt::UserRole).toString();
        
        bool visible = filter.isEmpty();
        if (!visible) {
            // 搜索标题和内容
            for (const auto& note : m_notes) {
                if (note.id == noteId) {
                    visible = note.title.contains(filter, Qt::CaseInsensitive) ||
                             note.content.contains(filter, Qt::CaseInsensitive);
                    break;
                }
            }
        }
        
        item->setHidden(!visible);
    }
}

void NotesWidget::saveNotes() {
    QJsonArray notesArray;
    for (const auto& note : m_notes) {
        notesArray.append(note.toJson());
    }
    
    QJsonObject rootObj;
    rootObj["notes"] = notesArray;
    rootObj["version"] = "1.0";
    rootObj["lastSaved"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(rootObj);
    
    QFile file(m_notesFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void NotesWidget::loadNotes() {
    QFile file(m_notesFilePath);
    if (!file.exists()) {
        // 创建默认便签
        Note defaultNote;
        defaultNote.id = generateNoteId();
        defaultNote.title = "欢迎使用便签";
        defaultNote.content = "这是您的第一个便签！\n\n您可以：\n• 创建多个便签\n• 搜索便签内容\n• 自定义字体和颜色\n• 自动保存您的修改";
        m_notes.append(defaultNote);
        saveNotes();
        return;
    }
    
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject rootObj = doc.object();
        QJsonArray notesArray = rootObj["notes"].toArray();
        
        m_notes.clear();
        for (const auto& value : notesArray) {
            m_notes.append(Note::fromJson(value.toObject()));
        }
        
        updateNotesList();
    }
}

void NotesWidget::updateContent() {
    // 便签小组件不需要定期更新内容
    // 但可以在这里检查文件变化等
}

void NotesWidget::drawContent(QPainter& painter) {
    // 使用默认绘制，因为我们使用了Qt的布局和控件
    painter.fillRect(rect(), m_widgetBackgroundColor);
    
    if (m_borderWidth > 0) {
        painter.setPen(QPen(m_borderColor, m_borderWidth));
        painter.drawRect(rect().adjusted(m_borderWidth/2, m_borderWidth/2, -m_borderWidth/2, -m_borderWidth/2));
    }
}

void NotesWidget::applyConfig() {
    BaseWidget::applyConfig();
    parseCustomSettings();
    
    if (m_autoSaveTimer) {
        if (m_autoSave) {
            m_autoSaveTimer->start(m_autoSaveInterval);
        } else {
            m_autoSaveTimer->stop();
        }
    }
    
    update();
}

void NotesWidget::resizeEvent(QResizeEvent* event) {
    BaseWidget::resizeEvent(event);
    // 可以在这里调整布局
} 