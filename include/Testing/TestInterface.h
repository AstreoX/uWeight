#pragma once
#include <QObject>

// TestInterface - 测试接口（待实现）
class TestInterface : public QObject {
    Q_OBJECT
public:
    explicit TestInterface(QObject* parent = nullptr);
}; 