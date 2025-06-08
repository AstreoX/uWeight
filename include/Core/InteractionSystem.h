#pragma once
#include <QObject>

// InteractionSystem - 交互系统（待实现）
class InteractionSystem : public QObject {
    Q_OBJECT
public:
    explicit InteractionSystem(QObject* parent = nullptr);
}; 