#pragma once
#include <QObject>

// WidgetRenderer - 高性能渲染器（待实现）
class WidgetRenderer : public QObject {
    Q_OBJECT
public:
    explicit WidgetRenderer(QObject* parent = nullptr);
}; 