#pragma once

#include <vector>
#include <unordered_map>

#include <QHash>
#include <QString>

struct LangComponent {
    QString type;
    QString name;
    bool isArray{false};
    bool isEnum{false};
    bool isOptional{false};
};


class LanguageSpecification {
    std::unordered_map<std::string, std::vector<LangComponent>> parsed_;

    QHash<QString, std::string> typeToStruct_;
    QHash<QString, std::string> triggersMap_;
    QHash<QString, std::string> effectsMap_;
    QHash<QString, std::string> conditionsMap_;
    QHash<QString, std::string> cardsMap_;
    QHash<QString, std::string> multiplierMap_;
public:
    std::vector<LangComponent> getComponentsByEnum(const QString typeName);
    std::vector<LangComponent> getComponentsByType(const QString typeName);

    static LanguageSpecification& get();
private:
    LanguageSpecification();
};
