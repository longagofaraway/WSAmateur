#include "languageSpecification.h"

#include <set>
#include <sstream>

#include <QDebug>

#include "lang_spec.h"

namespace {
std::string strip(const std::string &inpt)
{
    auto start_it = inpt.begin();
    auto end_it = inpt.rbegin();
    while (std::isspace(*start_it))
        ++start_it;
    if (start_it != inpt.end()) {
       while (std::isspace(*end_it))
          ++end_it;
     }
    return std::string(start_it, end_it.base());
}
}

LanguageSpecification::LanguageSpecification() {
    std::istringstream ss(kLangSpec);
    std::string line;

    std::string currentBlock;
    std::string lastToken;

    auto skipBlock = [&]() {
         int braces_count = 1;
         while (std::getline(ss, line)) {
             for (auto ch: line) {
                 if (ch == '{') braces_count++;
                 else if (ch == '}') {
                     braces_count--;
                     if (!braces_count) return;
                 }
             }
         }
    };

    auto parseChoice = [&](QString name) -> LangComponent {
        int braces_count = 1;
        LangComponent comp;
        comp.name = name;
        std::string choiceLine;
        while (std::getline(ss, choiceLine)) {
            if (choiceLine.empty()) continue;
            for (auto ch: choiceLine) {
                if (ch == '{') braces_count++;
                else if (ch == '}') braces_count--;
            }
            if (braces_count <= 0) break;
            choiceLine = strip(choiceLine);
            if (choiceLine.back() == ',') choiceLine.pop_back();
            if (choiceLine == "Nothing") continue;
            comp.type = QString::fromStdString(choiceLine);
        }
        return comp;
    };

    auto parseStruct = [&, this]() {
        int braces_count = 1;
        std::vector<LangComponent> components;
        while (std::getline(ss, line))  {
            std::istringstream linestream(line);
            std::string token;
            std::vector<std::string> tokens;
            while(std::getline(linestream, token, ' ')) {
                if (token.empty()) continue;
                if (token.back() == ',') token.pop_back();
                if (token == "}") braces_count--;
                tokens.push_back(token);
            }

            if (tokens.size() > 1) {
                LangComponent comp{};
                if (tokens[1] == "Array") {
                    comp.type = QString::fromStdString(tokens[3]);
                    comp.name = QString::fromStdString(tokens[0]);
                    comp.name.front() = comp.name.front().toUpper();
                    comp.isArray = true;
                } else if (tokens[1] == "Choice") {
                    comp = parseChoice(QString::fromStdString(tokens[0]));
                } else {
                    comp.type = QString::fromStdString(tokens[1]);
                    comp.name = QString::fromStdString(tokens[0]);
                    comp.name.front() = comp.name.front().toUpper();
                }
                components.push_back(comp);
            }

            if (braces_count == 0) break;
        }
        parsed_[lastToken] = components;
    };

    auto parseRelation = [&, this]() {
        int braces_count = 1;
        std::set<char> punctuation{',',':'};
        while (std::getline(ss, line))  {
            std::istringstream linestream(line);
            std::string token;
            std::vector<std::string> tokens;
            while(std::getline(linestream, token, ' ')) {
                if (token.empty()) continue;
                if (punctuation.contains(token.back())) token.pop_back();
                if (token == "{") braces_count++;
                else if (token == "}") braces_count--;
                tokens.push_back(token);
            }

            if (braces_count == 0) break;

            if (tokens.size() <= 1) {
                continue;
            }

            auto type = QString::fromStdString(tokens[0]);
            type.front() = type.front().toUpper();
            auto structName = tokens[1];
            if (lastToken == "Trigger") {
                triggersMap_[type] = structName;
            } else if (lastToken == "Effect") {
                effectsMap_[type] = structName;
            } else if (lastToken == "Condition") {
                conditionsMap_[type] = structName;
            } else if (lastToken == "CardSpecifier") {
                cardsMap_[type] = structName;
            }
        }
    };

    auto parseEnum = [&, this]() {
        std::vector<LangComponent> components;
        LangComponent enum_component{
            .type = QString::fromStdString(lastToken),
            .isArray = false,
            .isEnum = true
        };
        components.push_back(enum_component);
        parsed_[lastToken] = components;
    };

    while (std::getline(ss, line)) {
        if (line.starts_with("Trigger ")) {
            currentBlock = "Trigger";
            skipBlock();
            continue;
        } else if (line.starts_with("Effect ")) {
            currentBlock = "Effect";
            skipBlock();
            continue;
        } else if (line.starts_with("CardSpecifier ")) {
            currentBlock = "CardSpecifier";
            skipBlock();
            continue;
        }

        if (line.starts_with("enum")) {
            lastToken = line.substr(5, line.find(' ', 5)-5);
            parseEnum();
            skipBlock();
            continue;
        }
        if (line.starts_with("relation")) {
            lastToken = line.substr(9, line.find(' ', 9)-9);
            parseRelation();
            continue;
        }

        lastToken = line.substr(0, line.find(' '));

        auto pos = line.find('{');
        if (pos == std::string::npos)
            continue;

        parseStruct();
    }
    typeToStruct_.insert(triggersMap_);
    typeToStruct_.insert(effectsMap_);
    typeToStruct_.insert(conditionsMap_);
    typeToStruct_.insert(cardsMap_);
}

LanguageSpecification& LanguageSpecification::get() {
    static LanguageSpecification langSpec;
    return langSpec;
}

std::vector<LangComponent> LanguageSpecification::getComponentsByEnum(const QString typeName) {
    if (!typeToStruct_.contains(typeName)) {
        qDebug() << "typeName " << typeName << " not found";
        return {};
    }
    auto componentName = typeToStruct_[typeName];
    if (parsed_.contains(componentName)) {
        return parsed_[componentName];
    }
    return {};
}

std::vector<LangComponent> LanguageSpecification::getComponentsByType(const QString typeName) {
    if (parsed_.contains(typeName.toStdString())) {
        return parsed_[typeName.toStdString()];
    }
    return {};
}

