#pragma once

#include <functional>

#include <QQuickItem>

#include "baseComponent.h"

class ComponentOpener : public BaseComponent
{
    Q_OBJECT
private:
    std::function<QQuickItem*()> componentCreator_;
    QQuickItem *workingArea_;
    QQuickItem *object_{nullptr};
    std::optional<QMetaObject::Connection> connection_;

    std::optional<asn::Ability> ability_;
    QString id_;

public:
    ComponentOpener(QQuickItem *parent, QQuickItem *workingArea, QString componentId, std::function<QQuickItem*()>&& componentCreator);
    ~ComponentOpener();

    void initAbility(const asn::Ability& ability, QString idParam);

public slots:
    void openView();
    void abilityChanged(asn::Ability ability, QString id);

private:
    void closeView();
};
