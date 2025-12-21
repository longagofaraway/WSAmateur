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

    std::unordered_map<QString, std::unique_ptr<BaseComponent>> cppComponents_;
    QHash<QString, QQuickItem*> components_;
    QSet<QString> connections_;

public:
    ~ComponentManager();
    QQuickItem* createComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject, gen::ComponentMediator *mediator);
    void deleteComponent(QString id);
    void clear();

private:
    QQuickItem* createCppComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
    QQuickItem* createQmlComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject, QObject *mediator);
};
