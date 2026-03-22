#include "componentOpener.h"

#include "language_parser.h"

ComponentOpener::ComponentOpener(QQuickItem *parent, QQuickItem *workingArea, QString componentId, std::function<QQuickItem*()>&& componentCreator)
    : BaseComponent("ComponentOpener", parent, componentId), componentCreator_(componentCreator), workingArea_(workingArea) {
    connect(qmlObject_, SIGNAL(openView()), this, SLOT(openView()));
}

ComponentOpener::~ComponentOpener() {
    closeView();
}

void ComponentOpener::initAbility(const asn::Ability &ability, QString idParam) {
    ability_ = ability;
    id_ = idParam;
    abilityChanged(ability, idParam);
}

void ComponentOpener::openView() {
    object_ = componentCreator_();
    connection_ = connect(this, SIGNAL(setAbility(asn::Ability,QString)), object_, SLOT(setAbility(asn::Ability,QString)));
    connect(object_, SIGNAL(componentChanged(asn::Ability,QString)), this, SLOT(abilityChanged(asn::Ability,QString)));
    qvariant_cast<QObject*>(object_->property("anchors"))->setProperty("fill", QVariant::fromValue(workingArea_));

    if (ability_.has_value())
        emit setAbility(ability_.value(), id_);
}

void ComponentOpener::abilityChanged(asn::Ability ability, QString id) {
    switch(ability.type) {
    case asn::AbilityType::Auto: {
        auto& ab = std::get<asn::AutoAbility>(ability.ability);
        if (ab.effects.size() > 0)
            QMetaObject::invokeMethod(qmlObject_, "setDescription", Q_ARG(QString, QString::fromStdString(toString(ab.effects.front().type))));
        break;
    }
    case asn::AbilityType::Cont: {
        auto& ab = std::get<asn::ContAbility>(ability.ability);
        if (ab.effects.size() > 0)
            QMetaObject::invokeMethod(qmlObject_, "setDescription", Q_ARG(QString, QString::fromStdString(toString(ab.effects.front().type))));
        break;
    }
    case asn::AbilityType::Act: {
        auto& ab = std::get<asn::ActAbility>(ability.ability);
        if (ab.effects.size() > 0)
            QMetaObject::invokeMethod(qmlObject_, "setDescription", Q_ARG(QString, QString::fromStdString(toString(ab.effects.front().type))));
        break;
    }
        // initialize ability
    }
}

void ComponentOpener::closeView() {
    if (connection_) {
        QObject::disconnect(connection_.value());
        connection_.reset();
    }
    if (object_)
        object_->deleteLater();
    object_ = nullptr;
}
