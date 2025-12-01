#pragma once

#include <memory>
#include <vector>

#include <QQuickItem>

#include "baseComponent.h"

namespace gen {
    class ComponentMediator;
}

class ComponentManager : public QObject {
    Q_OBJECT
    struct QmlComponentPack {
        QQuickItem* component;
        std::shared_ptr<gen::ComponentMediator> creator;
    };
    struct CppComponentPack {
        std::unique_ptr<BaseComponent> component;
        std::shared_ptr<gen::ComponentMediator> creator;
    };

    std::unordered_map<QString, CppComponentPack> cppComponents_;
    QHash<QString, QmlComponentPack> components_;
    QSet<QString> connections_;

public:
    ~ComponentManager();
    QQuickItem* createComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject);
    void deleteComponent(QString id);
    void clear();

private:
    QQuickItem* createCppComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject);
    QQuickItem* createQmlComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject);
};
