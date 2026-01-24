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
    std::optional<asn::Effect> effect;
    std::shared_ptr<BranchInfo> branchInfo;
    std::vector<std::vector<std::shared_ptr<TreeNodeInfo>>> subBranches;
};

using Branch = std::vector<std::shared_ptr<TreeNodeInfo>>;

struct BranchInfo {
    QString branchId;
    int sequenceNextVal{0};
    Branch& treeBranch;
};

class EffectsTree : public QQuickItem {
    Q_OBJECT
private:
    AbilityComponent *abilityComponent_{nullptr};
    std::unordered_map<QString, std::shared_ptr<TreeNodeInfo>> nodeMap_;
    std::vector<std::shared_ptr<TreeNodeInfo>> treeRoot_;
    QQuickItem *workingArea_;
    QQuickItem *selectedItem_{nullptr};
    int maxOffset_{0};

public:
    Q_INVOKABLE void setWorkingArea(QObject *workingArea);
    Q_INVOKABLE void setAbilityComponent(QQuickItem *abilityComponent);
    Q_INVOKABLE void loseFocus();

signals:
    void componentChanged(std::vector<asn::Effect> effects);
    void gotFocus();
    void sizeChanged(qreal width, qreal height);

private:
    void renderTree();
    void setFocus(QQuickItem* node);
    void renderBranch(int& currentHeight, int offset, const Branch& branch);
    void createSubBranch(std::vector<std::shared_ptr<TreeNodeInfo>>& subBranch, QString idSuffix, QString header);
    void updateEffectsTree(TreeNodeInfo *node);
    void notifyOfChanges();
    void createEffectComponent(const TreeNodeInfo *nodeInfo);

private slots:
    void createEffect(QString,QString);
    void effectChanged(QString nodeId, asn::EffectType type, const VarEffect& effect);
    void effectSizeChanged(qreal width, qreal height);
    void selectEffect(QString);

protected:
    void componentComplete() override;
};
