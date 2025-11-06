#pragma once

#include <vector>
#include <unordered_map>

#include <QString>

struct LangComponent {
    QString type;
    QString name;
    bool isArray{false};
    bool isEnum{false};
};


class LanguageSpecification {
    std::unordered_map<std::string, std::vector<LangComponent>> parsed_;
public:
    std::vector<LangComponent> getComponents(const QString typeName);

    static LanguageSpecification& get();
private:
    LanguageSpecification();
};
