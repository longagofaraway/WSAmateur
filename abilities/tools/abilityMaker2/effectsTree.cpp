#include "effectsTree.h"

#include <QQmlContext>

#include "ability.h"
#include "applicationData.h"
#include "effect.h"
#include "effectInit.h"
#include "condition.h"
#include "conditionInit.h"
#include "language_parser.h"

namespace {
const int kNodeHeight = 35;
const int kHeaderHeight = 15;
const int kOffsetStep = 30;
const int kDefaultTreeWidth = 190;
asn::Effect kEmptyEffect = asn::Effect{.type=asn::EffectType::NotSpecified};
const QString kBaseBranchId = "Main";
QQuickItem* createQmlObject(const QString &name, QQuickItem *parent, QString id) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setProperty("componentId", id);
    return qmlObject;
}
QQuickItem* createQmlBranchHeader(QQuickItem *parent, QString header) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/EffectsTree/BranchHeader.qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setProperty("header", header);
    return qmlObject;
}

std::vector<asn::Effect> constructEffects(const std::vector<std::shared_ptr<TreeNodeInfo>>& nodes) {
    std::vector<asn::Effect> effects;
    for (const auto &node: nodes) {
        if (!node->effect.has_value())
            continue;
        const auto& effect = node->effect.value();
        if (effect.type == asn::EffectType::PayCost) {
            if (node->subBranches.size() != 2) {
                qWarning() << "Wrong number of subbranches";
                return {};
            }
            asn::Effect eff{.type = effect.type, .cond = effect.cond};
            asn::PayCost payCost;
            payCost.ifYouDo = constructEffects(node->subBranches[0]);
            payCost.ifYouDont = constructEffects(node->subBranches[1]);
            eff.effect = payCost;
            effects.push_back(eff);
        } else {
            effects.push_back(effect);
        }
    }
    return effects;
}

bool hasSubBranches(asn::EffectType type) {
    return type == asn::EffectType::PayCost || type == asn::EffectType::NonMandatory;
}
} // namespace

void EffectsTree::componentComplete() {
    QQuickItem::componentComplete();
    this->setProperty("width", kDefaultTreeWidth);
    QString nodeId = "New";
    auto node = createQmlObject("EffectsTree/Effect", this, nodeId);
    node->setProperty("effectMode", "createMode");
    node->setProperty("conditionMode", "createMode");
    connect(node, SIGNAL(createEffect(QString,QString)), this, SLOT(createEffect(QString,QString)));
    connect(node, SIGNAL(createCondition(QString,QString)), this, SLOT(createCondition(QString,QString)));
    auto branchInfo = std::make_shared<BranchInfo>(BranchInfo {
        .branchId = kBaseBranchId,
        .sequenceNextVal = 0,
        .treeBranch = treeRoot_
    });
    TreeNodeInfo nodeInfo {
        .id = nodeId,
        .object = node,
        .branchInfo = branchInfo,
    };
    auto treeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(nodeInfo));
    nodeMap_.emplace(std::make_pair(treeNodeInfo->id, treeNodeInfo));
    treeRoot_.push_back(treeNodeInfo);
    renderTree();
}

void EffectsTree::setWorkingArea(QObject *workingArea) {
    workingArea_ = qobject_cast<QQuickItem*>(workingArea);
    ApplicationData::get().setWorkingArea(workingArea_);
}

void EffectsTree::setAbilityComponent(QQuickItem *abilityComponent) {
    abilityComponent_ = dynamic_cast<AbilityComponent*>(abilityComponent);
    abilityComponent_->subscribeToEffectsChange(this);
}

void EffectsTree::renderTree() {
    int totalHeight{0};
    maxOffset_ = 0;

    renderBranch(totalHeight, 0, treeRoot_);
    this->setProperty("height", totalHeight);
    this->setProperty("width", kDefaultTreeWidth + maxOffset_);
}

void EffectsTree::renderBranch(int& currentHeight, int offset, const Branch& branch) {
    maxOffset_ = std::max(offset, maxOffset_);
    for (const auto& node: branch) {
        if (node->effect.has_value() && node->effect.value().type == asn::EffectType::PayCost) {
            node->object->setProperty("x", offset);
            node->object->setProperty("y", currentHeight);
            currentHeight += kNodeHeight;
            for (size_t i = 0; i < node->subBranches.size(); i++) {
                renderBranch(currentHeight, offset + kOffsetStep, node->subBranches.at(i));
            }
            continue;
        } else if (node->effect.has_value() && node->effect.value().type == asn::EffectType::NonMandatory) {
            for (size_t i = 0; i < node->subBranches.size(); i++) {
                renderBranch(currentHeight, offset + kOffsetStep, node->subBranches.at(i));
            }
            continue;
        }
        if (node->object) {
            node->object->setProperty("x", offset);
            node->object->setProperty("y", currentHeight);
            if (node->id.endsWith("header")) {
                currentHeight += kHeaderHeight;
            } else {
                currentHeight += kNodeHeight;
            }
        }
    }
}

void EffectsTree::createSubBranch(std::vector<std::shared_ptr<TreeNodeInfo>>& subBranch, QString parentNodeId, QString header) {
    QString nodeId = "New"+parentNodeId+header;
    auto qmlheader = createQmlBranchHeader(this, header);
    auto node = createQmlObject("EffectsTree/Effect", this, nodeId);
    node->setProperty("effectMode", "createMode");
    node->setProperty("conditionMode", "createMode");
    connect(node, SIGNAL(createEffect(QString,QString)), this, SLOT(createEffect(QString,QString)));
    connect(node, SIGNAL(createCondition(QString,QString)), this, SLOT(createCondition(QString,QString)));

    auto branchInfo = std::make_shared<BranchInfo>(BranchInfo {
        .branchId = parentNodeId+header,
        .sequenceNextVal = 0,
        .treeBranch = subBranch
    });
    TreeNodeInfo headerNodeInfo {
        .id = parentNodeId+"header",
        .object = qmlheader,
        .branchInfo = branchInfo
    };
    TreeNodeInfo nodeInfo {
        .id = nodeId,
        .object = node,
        .branchInfo = branchInfo
    };
    auto headerTreeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(headerNodeInfo));
    auto treeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(nodeInfo));
    nodeMap_.emplace(std::make_pair(treeNodeInfo->id, treeNodeInfo));
    subBranch.push_back(headerTreeNodeInfo);
    subBranch.push_back(treeNodeInfo);
}

void EffectsTree::updateEffectsTree(TreeNodeInfo *node) {
    if (!node->effect.has_value()) {
        return;
    }
    const auto &effect = node->effect.value();
    if (hasSubBranches(effect.type)) {
        node->subBranches.clear();
        node->subBranches.resize(2);
        createSubBranch(node->subBranches[0], node->id, "If you do");
        createSubBranch(node->subBranches[1], node->id, "If you don't");
    }
}

void EffectsTree::createEffectComponent(const TreeNodeInfo *nodeInfo) {
    std::shared_ptr<EffectComponent> effectComponent;
    if (nodeInfo->effect.value().type != asn::EffectType::NotSpecified) {
        effectComponent = std::make_shared<EffectComponent>(nodeInfo->id, workingArea_, nodeInfo->effect.value());
    } else {
        effectComponent = std::make_shared<EffectComponent>(nodeInfo->id, workingArea_);
    }
    connect(&*effectComponent, &EffectComponent::componentChanged, this, &EffectsTree::effectChanged);
    connect(&*effectComponent, &EffectComponent::sizeChanged, this, &EffectsTree::effectSizeChanged);
    abilityComponent_->setCurrentComponent(effectComponent);
    setFocus(nodeInfo->object, /*isCondition*/ false);
}

void EffectsTree:: createConditionComponent(const TreeNodeInfo *nodeInfo) {
    std::shared_ptr<ConditionComponent> conditionComponent;
    conditionComponent = std::make_shared<ConditionComponent>(nodeInfo->id, workingArea_, nodeInfo->effect.value().cond);
    connect(&*conditionComponent, &ConditionComponent::componentChanged, this, &EffectsTree::conditionChanged);
    connect(&*conditionComponent, &ConditionComponent::sizeChanged, this, &EffectsTree::effectSizeChanged);
    abilityComponent_->setCurrentComponent(conditionComponent);
    setFocus(nodeInfo->object, /*isCondition*/ true);
}

TreeNodeInfo* EffectsTree::createNode(TreeNodeInfo *creatingNode, const asn::Effect& effect) {
    QString newNodeId = creatingNode->branchInfo->branchId + QString::number(creatingNode->branchInfo->sequenceNextVal++);
    auto newNode = createQmlObject("EffectsTree/Effect", this, newNodeId);
    connect(newNode, SIGNAL(selectEffect(QString)), this, SLOT(selectEffect(QString)));
    connect(newNode, SIGNAL(setEffect(QString,QString)), this, SLOT(setEffect(QString,QString)));
    connect(newNode, SIGNAL(deleteEffect(QString)), this, SLOT(deleteEffect(QString)));
    connect(newNode, SIGNAL(selectCondition(QString)), this, SLOT(selectCondition(QString)));
    connect(newNode, SIGNAL(setCondition(QString,QString)), this, SLOT(setCondition(QString,QString)));
    TreeNodeInfo nodeInfo {
        .id = newNodeId,
        .object = newNode,
        .effect = effect,
        .branchInfo = creatingNode->branchInfo
    };
    auto treeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(nodeInfo));
    nodeMap_.emplace(std::make_pair(treeNodeInfo->id, treeNodeInfo));
    return creatingNode->branchInfo->treeBranch.insert(creatingNode->branchInfo->treeBranch.end() - 1, treeNodeInfo)->get();
}

void EffectsTree::createEffect(QString nodeId, QString effectId) {
    if (!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    auto effect = getEffectFromPreset(effectId);
    auto newNode = createNode(node.get(), effect);
    newNode->object->setProperty("effectMode", "selectMode");
    newNode->object->setProperty("effectName", QString::fromStdString(toString(effect.type)));
    newNode->object->setProperty("conditionMode", "setMode");
    createEffectComponent(newNode);
    updateEffectsTree(newNode);
    renderTree();

    notifyOfChanges();
}

void EffectsTree::setEffect(QString nodeId, QString effectId) {
    if (!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    auto newEffect = getEffectFromPreset(effectId);
    node->effect.value().type = newEffect.type;
    node->effect.value().effect = newEffect.effect;
    node->object->setProperty("effectMode", "selectMode");
    node->object->setProperty("effectName", QString::fromStdString(toString(node->effect.value().type)));
    createEffectComponent(node.get());
    updateEffectsTree(node.get());
    renderTree();

    notifyOfChanges();
}

void EffectsTree::effectChanged(QString nodeId, asn::EffectType type, const VarEffect& effect) {
    if(!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found in effectChanged";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    if (!node->effect.has_value()) {
        qWarning() << "changing effect on a wrong node";
        return;
    }

    auto oldType = node->effect.value().type;
    node->effect.value().type = type;
    node->effect.value().effect = effect;
    notifyOfChanges();
    if (oldType == type) {
        return;
    }
    // need to delete subbranches here
    if (hasSubBranches(oldType)) {
        for (auto& subBranch: node->subBranches) {
            for (auto& subNode: subBranch) {
                deleteNode(subNode);
            }
        }
    }

    node->object->setProperty("effectName", QString::fromStdString(toString(type)));
    updateEffectsTree(node.get());
    renderTree();
}

void EffectsTree::effectSizeChanged(qreal width, qreal height) {
    emit sizeChanged(width+this->width(), height+this->height());
}

void EffectsTree::selectEffect(QString componentId) {
    auto treeNodeInfo = nodeMap_.at(componentId);
    createEffectComponent(treeNodeInfo.get());
}

void EffectsTree::deleteNode(std::shared_ptr<TreeNodeInfo> node) {
    auto currentComponent = abilityComponent_->getCurrentComponent();
    if (currentComponent && currentComponent->getComponentId() == node->id) {
        std::shared_ptr<EffectComponent> effectComponent;
        abilityComponent_->setCurrentComponent(effectComponent);
    }
    if (selectedItem_ == node->object) {
        selectedItem_ = nullptr;
    }
    for (auto& subBranch: node->subBranches) {
        for (auto& subNode: subBranch) {
            deleteNode(subNode);
        }
    }

    nodeMap_.erase(node->id);
    node->object->deleteLater();
}

void EffectsTree::createCondition(QString nodeId, QString conditionId) {
    if (!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    auto condition = getConditionFromPreset(conditionId);
    asn::Effect effect{.type=asn::EffectType::NotSpecified};
    effect.cond = condition;
    auto newNode = createNode(node.get(), effect);
    newNode->object->setProperty("effectMode", "setMode");
    newNode->object->setProperty("conditionName", QString::fromStdString(toString(condition.type)));
    newNode->object->setProperty("conditionMode", "selectMode");
    createConditionComponent(newNode);
    renderTree();

    notifyOfChanges();
}

void EffectsTree::setCondition(QString nodeId, QString conditionId) {
    if (!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    node->effect.value().cond = getConditionFromPreset(conditionId);
    node->object->setProperty("conditionMode", "selectMode");
    node->object->setProperty("conditionName", QString::fromStdString(toString(node->effect.value().cond.type)));
    createConditionComponent(node.get());
    renderTree();

    notifyOfChanges();
}

void EffectsTree::selectCondition(QString componentId) {
    auto treeNodeInfo = nodeMap_.at(componentId);
    createConditionComponent(treeNodeInfo.get());
}

void EffectsTree::conditionChanged(QString nodeId, asn::ConditionType type, const VarCondition &condition) {
    if(!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found in effectChanged";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    if (!node->effect.has_value()) {
        qWarning() << "changing effect on a wrong node";
        return;
    }

    node->effect.value().cond.type = type;
    node->effect.value().cond.cond = condition;
    renderTree();

    notifyOfChanges();
}

void EffectsTree::deleteEffect(QString componentId) {
    auto treeNodeInfo = nodeMap_.at(componentId);
    deleteNode(treeNodeInfo);
    auto& branch = treeNodeInfo->branchInfo->treeBranch;
    for (auto it = branch.begin(); it != branch.end(); ++it) {
        if (it->get()->id == treeNodeInfo->id) {
            branch.erase(it);
            break;
        }
    }

    renderTree();

    notifyOfChanges();
}

void EffectsTree::notifyOfChanges() {
    emit componentChanged(constructEffects(treeRoot_));
}

void EffectsTree::setFocus(QQuickItem* node, bool isCondition) {
    if (selectedItem_ != nullptr) {
        selectedItem_->setProperty("selected", false);
        selectedItem_->setProperty("conditionSelected", false);
    }
    selectedItem_ = node;
    if (isCondition)
        selectedItem_->setProperty("conditionSelected", true);
    else
        selectedItem_->setProperty("selected", true);
    emit gotFocus();
}

void EffectsTree::loseFocus() {
    if (selectedItem_ == nullptr) {
        return;
    }
    selectedItem_->setProperty("selected", false);
}

