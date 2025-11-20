#pragma once

#include <vector>

#include <QObject>
#include <QQuickItem>
#include <QString>
#include <QDebug>

#include "abilities.h"
#include "baseComponent.h"
#include "effect.h"


struct BranchInfo;
class AbilityComponent;

using VarEffect = decltype(asn::Effect::effect);

struct TreeNodeInfo {
    QString id;
    QQuickItem* object;
    asn::Effect& effect;
    std::shared_ptr<BranchInfo> branchInfo;
    std::vector<std::vector<std::shared_ptr<TreeNodeInfo>>> subBranches;
};

using Branch = std::vector<std::shared_ptr<TreeNodeInfo>>;

struct BranchInfo {
    QString branchId;
    int sequenceNextVal{0};
    std::vector<asn::Effect>& effects;
    Branch& treeBranch;
};

class EffectsTree : public QQuickItem {
    Q_OBJECT
private:
    AbilityComponent *abilityComponent_{nullptr};
    std::unordered_map<QString, std::shared_ptr<TreeNodeInfo>> nodeMap_;
    std::vector<asn::Effect> effects_;
    std::vector<std::shared_ptr<TreeNodeInfo>> treeRoot_;
    QQuickItem *workingArea_;
    int maxOffset_{0};

public:
    Q_INVOKABLE void setWorkingArea(QObject *workingArea);
    Q_INVOKABLE void setAbilityComponent(QQuickItem *abilityComponent);

signals:
    void componentChanged(std::vector<asn::Effect> effects);

private:
    void renderTree();
    void renderBranch(int& currentHeight, int offset, const Branch& branch);
    void createSubBranch(std::vector<std::shared_ptr<TreeNodeInfo>>& subBranch, QString idSuffix, QString header, std::vector<asn::Effect>& effects);

private slots:
    void createEffect(QString,QString);
    void effectChanged(QString nodeId, asn::EffectType type, const VarEffect& effect);

protected:
    void componentComplete() override;
};
