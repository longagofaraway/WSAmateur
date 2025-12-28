#pragma once

#include <QQuickItem>

#include "abilities.h"
#include "baseComponent.h"
#include "componentManager.h"

namespace gen {
    class MultiplierHelper;
}

class MultiplierComponent : public BaseComponent
{
    Q_OBJECT
public:
    using VarMultiplier = decltype(asn::Multiplier::specifier);

private:
    std::vector<QQuickItem*> components_;
    ComponentManager componentManager_;

    asn::MultiplierType type_{asn::MultiplierType::ForEach};
    std::optional<VarMultiplier> multiplier_;
    std::shared_ptr<gen::MultiplierHelper> gen_helper;

public:
    MultiplierComponent(QQuickItem *parent, QString id, QString displayName);

    void setMultiplier(asn::Multiplier);
    void notifyOfChanges() override;
    void onValueTypeChanged(QString,QString);

    virtual asn::MultiplierType getLanguageComponentType(formats::To<asn::MultiplierType>) override { return type_; }
    virtual VarMultiplier& getLanguageComponent(formats::To<VarMultiplier>) override { return multiplier_.value(); }

public slots:
    void onMultiplierTypeChanged(QString);

private:
    void fitComponent(QQuickItem* object);
    void createMultiplier();
};
