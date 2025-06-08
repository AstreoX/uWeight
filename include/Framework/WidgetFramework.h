#pragma once
#include <QObject>

// WidgetFramework - Widget框架（待实现）
class WidgetFramework : public QObject {
    Q_OBJECT
public:
    explicit WidgetFramework(QObject* parent = nullptr);
}; 