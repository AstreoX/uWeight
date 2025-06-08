#pragma once
#include "Core/BaseWidget.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QFontComboBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QColor>
#include <QJsonArray>
#include <QListWidget>
#include <QSplitter>
#include <QLineEdit>
#include <QTimer>

struct Note {
    QString id;
    QString title;
    QString content;
    QDateTime created;
    QDateTime modified;
    QFont font;
    QColor textColor;
    QColor backgroundColor;
    
    Note() : 
        created(QDateTime::currentDateTime()),
        modified(QDateTime::currentDateTime()),
        font(QFont("Arial", 12)),
        textColor(Qt::black),
        backgroundColor(Qt::white) {}
        
    QJsonObject toJson() const;
    static Note fromJson(const QJsonObject& json);
};

class NotesWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit NotesWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~NotesWidget() = default;

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void applyConfig() override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onNewNote();
    void onDeleteNote();
    void onNoteSelectionChanged();
    void onTextChanged();
    void onTitleChanged();
    void onFontChanged();
    void onFontSizeChanged(int size);
    void onTextColorChanged();
    void onBackgroundColorChanged();
    void onAutoSave();
    void onSearchTextChanged(const QString& text);

private:
    void setupUI();
    void setupToolBar();
    void parseCustomSettings();
    void saveNotes();
    void loadNotes();
    void updateNotesList();
    void updateCurrentNote();
    void switchToNote(const QString& noteId);
    QString generateNoteId();
    void filterNotes(const QString& filter);

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_topLayout;
    QSplitter* m_splitter;
    
    // 便签列表
    QWidget* m_leftPanel;
    QVBoxLayout* m_leftLayout;
    QLineEdit* m_searchEdit;
    QListWidget* m_notesList;
    QPushButton* m_newNoteButton;
    QPushButton* m_deleteNoteButton;
    
    // 便签编辑器
    QWidget* m_rightPanel;
    QVBoxLayout* m_rightLayout;
    QLineEdit* m_titleEdit;
    QTextEdit* m_textEdit;
    
    // 工具栏
    QToolBar* m_toolBar;
    QFontComboBox* m_fontCombo;
    QSpinBox* m_fontSizeSpinBox;
    QPushButton* m_textColorButton;
    QPushButton* m_backgroundColorButton;
    
    // 数据
    QList<Note> m_notes;
    QString m_currentNoteId;
    bool m_textChanged;
    
    // 配置
    bool m_autoSave;
    int m_autoSaveInterval;
    QString m_notesFilePath;
    
    // 定时器
    QTimer* m_autoSaveTimer;
    
    // 样式
    QColor m_widgetBackgroundColor;
    QColor m_borderColor;
    int m_borderWidth;
}; 