#pragma once
#include "Core/BaseWidget.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QFont>
#include <QColor>
#include <QJsonObject>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

class SimpleNotesWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit SimpleNotesWidget(const WidgetConfig& config, QWidget* parent = nullptr);
    ~SimpleNotesWidget() = default;

    void updateContent() override;

protected:
    void drawContent(QPainter& painter) override;
    void paintEvent(QPaintEvent* event) override;
    void applyConfig() override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onTextChanged();
    void onAutoSave();
    void onClearText();
    void onChangeFont();
    void onChangeTextColor();
    void onChangeBackgroundColor();

private:
    void setupUI();
    void parseCustomSettings();
    void saveNote();
    void loadNote();
    void updateTextStyle();
    QMenu* createCustomContextMenu();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QTextEdit* m_textEdit;
    
    // 右键菜单
    QMenu* m_customContextMenu;
    QAction* m_clearAction;
    QAction* m_fontAction;
    QAction* m_textColorAction;
    QAction* m_backgroundColorAction;
    
    // 数据
    QString m_noteContent;
    bool m_textChanged;
    
    // 配置
    bool m_autoSave;
    int m_autoSaveInterval;
    QString m_noteFilePath;
    
    // 样式
    QFont m_textFont;
    QColor m_textColor;
    QColor m_backgroundColor;
    QColor m_widgetBackgroundColor;
    
    // 定时器
    QTimer* m_autoSaveTimer;
}; 