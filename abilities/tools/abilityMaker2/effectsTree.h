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

struct TreeNodeInfo {
    QString id;
    QQuickItem* object;
    asn::Effect& effect;
    std::shared_ptr<BranchInfo> branchInfo;
    std::shared_ptr<EffectComponent> effectComponent;
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
    std::unordered_map<QString, std::shared_ptr<TreeNodeInfo>> nodeMap_;
    std::vector<asn::Effect> effects_;
    std::vector<std::shared_ptr<TreeNodeInfo>> treeRoot_;
    QQuickItem *workingArea_;
    int maxOffset_{0};

public:
    Q_INVOKABLE void setWorkingArea(QObject *workingArea);

private:
    void renderTree();
    void renderBranch(int& currentHeight, int offset, const Branch& branch);
    void createSubBranch(std::vector<std::shared_ptr<TreeNodeInfo>>& subBranch, QString idSuffix, QString header, std::vector<asn::Effect>& effects);

private slots:
    void createEffect(QString,QString);

protected:
    void componentComplete() override;
};
