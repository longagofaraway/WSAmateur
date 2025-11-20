#include "effectsTree.h"

#include <QQmlContext>

#include "ability.h"
#include "effect.h"
#include "effectInit.h"
#include "language_parser.h"

namespace {
const int kNodeHeight = 35;
const int kHeaderHeight = 15;
const int kOffsetStep = 30;
const int kDefaultTreeWidth = 180;
asn::Effect kEmptyEffect = asn::Effect{.type=asn::EffectType::NotSpecified};
const QString kBaseBranchId = "Main";
QQuickItem* createQmlObject(const QString &name, QQuickItem *parent, QString id, QString effectMode, QString effectName) {
    QQmlComponent component(qmlEngine(parent), "qrc:/qml/" + name + ".qml");
    QQmlContext *context = new QQmlContext(qmlContext(parent), parent);
    QObject *obj = component.create(context);
    QQuickItem *qmlObject = qobject_cast<QQuickItem*>(obj);
    qmlObject->setParentItem(parent);
    qmlObject->setParent(parent);
    qmlObject->setProperty("componentId", id);
    qmlObject->setProperty("effectMode", effectMode);
    qmlObject->setProperty("effectName", effectName);
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
std::vector<asn::Effect>& getEffects(asn::Effect& effect, int index) {
    if (effect.type == asn::EffectType::PayCost) {
        auto& ef = std::get<asn::PayCost>(effect.effect);
        if (index == 1) {
            return ef.ifYouDo;
        } else {
            return ef.ifYouDont;
        }
    }
    throw std::runtime_error("unsupported effect: "+toString(effect.type));
}
}

void EffectsTree::componentComplete() {
    QQuickItem::componentComplete();
    this->setProperty("width", kDefaultTreeWidth);
    QString nodeId = "New";
    auto node = createQmlObject("EffectsTree/Effect", this, nodeId, "createMode", "");
    connect(node, SIGNAL(createEffect(QString,QString)), this, SLOT(createEffect(QString,QString)));
    auto branchInfo = std::make_shared<BranchInfo>(BranchInfo {
        .branchId = kBaseBranchId,
        .sequenceNextVal = 0,
        .effects = effects_,
        .treeBranch = treeRoot_
    });
    TreeNodeInfo nodeInfo {
        .id = nodeId,
        .object = node,
        .effect = kEmptyEffect,
        .branchInfo = branchInfo,
    };
    auto treeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(nodeInfo));
    nodeMap_.emplace(std::make_pair(treeNodeInfo->id, treeNodeInfo));
    treeRoot_.push_back(treeNodeInfo);
    renderTree();
}

void EffectsTree::setWorkingArea(QObject *workingArea) {
    workingArea_ = qobject_cast<QQuickItem*>(workingArea);
}
void EffectsTree::setAbilityComponent(QQuickItem *abilityComponent) {
    abilityComponent_ = dynamic_cast<AbilityComponent*>(abilityComponent);
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
        if (node->effect.type == asn::EffectType::PayCost) {
            node->object->setProperty("x", offset);
            node->object->setProperty("y", currentHeight);
            currentHeight += kNodeHeight;
            for (size_t i = 0; i < node->subBranches.size(); i++) {
                renderBranch(currentHeight, offset + kOffsetStep, node->subBranches.at(i));
            }
            continue;
        } else if (node->effect.type == asn::EffectType::NonMandatory) {
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

void EffectsTree::createSubBranch(std::vector<std::shared_ptr<TreeNodeInfo>>& subBranch, QString parentNodeId, QString header, std::vector<asn::Effect>& effects) {
    QString nodeId = "New"+parentNodeId;
    auto qmlheader = createQmlBranchHeader(this, header);
    auto node = createQmlObject("EffectsTree/Effect", this, nodeId, "createMode", "");
    connect(node, SIGNAL(createEffect(QString,QString)), this, SLOT(createEffect(QString,QString)));

    auto branchInfo = std::make_shared<BranchInfo>(BranchInfo {
        .branchId = parentNodeId+header,
        .sequenceNextVal = 0,
        .effects = effects,
        .treeBranch = subBranch
    });
    TreeNodeInfo headerNodeInfo {
        .id = parentNodeId+"header",
        .object = qmlheader,
        .effect = kEmptyEffect,
        .branchInfo = branchInfo
    };
    TreeNodeInfo nodeInfo {
        .id = nodeId,
        .object = node,
        .effect = kEmptyEffect,
        .branchInfo = branchInfo
    };
    auto headerTreeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(headerNodeInfo));
    auto treeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(nodeInfo));
    nodeMap_.emplace(std::make_pair(treeNodeInfo->id, treeNodeInfo));
    subBranch.push_back(headerTreeNodeInfo);
    subBranch.push_back(treeNodeInfo);
}

void EffectsTree::createEffect(QString nodeId, QString effectId) {
    if (!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    auto effect = getEffectFromPreset(effectId);
    QString newNodeId = node->branchInfo->branchId + QString::number(node->branchInfo->sequenceNextVal++);
    auto newNode = createQmlObject("EffectsTree/Effect", this, newNodeId, "selectMode", QString::fromStdString(toString(effect.type)));
    node->branchInfo->effects.push_back(effect);
    TreeNodeInfo nodeInfo {
        .id = newNodeId,
        .object = newNode,
        .effect = node->branchInfo->effects.back(),
        .branchInfo = node->branchInfo
    };
    std::shared_ptr<EffectComponent> effectComponent;
    if (effect.type != asn::EffectType::NotSpecified) {
        //nodeInfo.effectComponent = std::make_shared<EffectComponent>(workingArea_, this, effect);
        effectComponent = std::make_shared<EffectComponent>(newNodeId, workingArea_, effect);
    } else {
        effectComponent = std::make_shared<EffectComponent>(newNodeId, workingArea_);
    }
    connect(&*effectComponent, &EffectComponent::componentChanged, this, &EffectsTree::effectChanged);
    abilityComponent_->setCurrentComponent(effectComponent);
    if (effect.type == asn::EffectType::PayCost) {
        nodeInfo.subBranches.resize(2);
        createSubBranch(nodeInfo.subBranches[0], newNodeId, "If you do", getEffects(nodeInfo.effect, 1));
        createSubBranch(nodeInfo.subBranches[1], newNodeId, "If you don't", getEffects(nodeInfo.effect, 2));
    }
    auto treeNodeInfo = std::make_shared<TreeNodeInfo>(std::move(nodeInfo));
    nodeMap_.emplace(std::make_pair(treeNodeInfo->id, treeNodeInfo));
    node->branchInfo->treeBranch.insert(node->branchInfo->treeBranch.end() - 1, treeNodeInfo);
    renderTree();
}

void EffectsTree::effectChanged(QString nodeId, asn::EffectType type, const VarEffect& effect) {
    if(!nodeMap_.contains(nodeId)) {
        qWarning() << nodeId << " node not found in effectChanged";
        return;
    }
    auto& node = nodeMap_.at(nodeId);
    node->effect.type = type;
    node->effect.effect = effect;

    emit componentChanged(effects_);
}

