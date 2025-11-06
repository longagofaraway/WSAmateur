#pragma once

#include <memory>
#include <vector>

#include <QQuickItem>

#include "baseComponent.h"


class ComponentManager : public QObject {
    Q_OBJECT
    std::unordered_map<std::string, std::unique_ptr<BaseComponent>> cppComponents_;
    QHash<QString, QQuickItem*> components_;
    QSet<QString> connections_;

public:
    QQuickItem* createComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject);
    void deleteComponent(QString id);
    void clear();

private:
    QQuickItem* createCppComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject);
    QQuickItem* createQmlComponent(QString name, QString displayName, QString id, QQuickItem *parent, BaseComponent* linkObject);
};
