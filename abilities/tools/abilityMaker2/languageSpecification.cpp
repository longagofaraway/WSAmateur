#include "languageSpecification.h"

#include <sstream>

#include <QDebug>

#include "lang_spec.h"

namespace {
const QHash<QString, std::string> typeToStruct = {
    // Trigger
    {"OnZoneChange", "ZoneChangeTrigger"},
    {"OnPlay", "OnPlayTrigger"},
    {"OnStateChange", "StateChangeTrigger"},
    {"OnAttack", "OnAttackTrigger"},
    {"OnBackupOfThis", "Empty"},
    {"OnTriggerReveal", "TriggerRevealTrigger"},
    {"OnPhaseEvent", "PhaseTrigger"},
    {"OnEndOfThisCardsAttack", "Empty"},
    {"OnOppCharPlacedByStandbyTriggerReveal", "Empty"},
    {"OnBeingAttacked", "OnBeingAttackedTrigger"},
    {"OnDamageCancel", "OnDamageCancelTrigger"},
    {"OnDamageTakenCancel", "OnDamageTakenCancelTrigger"},
    {"OnPayingCost", "OnPayingCostTrigger"},
    {"OnActAbillity", "OnActAbillityTrigger"},

    // Effect
    {"AttributeGain", "AttributeGain"},
    {"ChooseCard", "ChooseCard"},
    {"RevealCard", "RevealCard"},
    {"MoveCard", "MoveCard"},
    {"SearchCard", "SearchCard"},
    {"PayCost", "PayCost"},
    {"AbilityGain", "AbilityGain"},
    {"MoveWrToDeck", "MoveWrToDeck"},
    {"FlipOver", "FlipOver"},
    {"Backup", "Backup"},
    {"TriggerCheckTwice", "Empty"},
    {"Look", "Look"},
    {"NonMandatory", "NonMandatory"},
    {"EarlyPlay", "Empty"},
    {"CannotPlay", "Empty"},
    {"PerformEffect", "PerformEffect"},
    {"ChangeState", "ChangeState"},
    {"DealDamage", "DealDamage"},
    {"CannotUseBackupOrEvent", "CannotUseBackupOrEvent"},
    {"DrawCard", "DrawCard"},
    {"SwapCards", "SwapCards"},
    {"CannotAttack", "CannotAttack"},
    {"CharAutoCannotDealDamage", ""},
    {"OpponentAutoCannotDealDamage", "OpponentAutoCannotDealDamage"},
    {"CannotBecomeReversed", "CannotBecomeReversed"},
    {"StockSwap", "StockSwap"},
    {"AddMarker", "AddMarker"},
    {"Bond", "Bond"},
    {"CannotMove", "CannotMove"},
    {"PerformReplay", "PerformReplay"},
    {"Replay", "Replay"},
    {"SideAttackWithoutPenalty", "SideAttackWithoutPenalty"},
    {"Standby", "Empty"},
    {"Shuffle", "Shuffle"},
    {"PutOnStageRested", "PutOnStageRested"},
    {"RemoveMarker", "RemoveMarker"},
    {"CannotStand", "CannotStand"},
    {"CannotBeChosen", "CannotBeChosen"},
    {"TriggerIconGain", "TriggerIconGain"},
    {"CanPlayWithoutColorRequirement", "CanPlayWithoutColorRequirement"},
    {"ShotTriggerDamage", "Empty"},
    {"DelayedAbility", "DelayedAbility"},
    {"CostSubstitution", "CostSubstitution"},
    {"SkipPhase", "SkipPhase"},
    {"ChooseTrait", "ChooseTrait"},
    {"TraitModification", "TraitModification"},

    // Card
    {"CardType", "CardType"},
    {"Owner", "Player"},
    {"Trait", "Trait"},
    {"ExactName", "ExactName"},
    {"NameContains", "NameContains"},
    {"Level", "Level"},
    {"LevelHigherThanOpp", "Empty"},
    {"Color", "Color"},
    {"Cost", "CostSpecifier"},
    {"TriggerIcon", "TriggerIcon"},
    {"HasMarker", "Empty"},
    {"Power", "Power"},
    {"StandbyTarget", "Empty"},
    {"LevelWithMultiplier", "LevelWithMultiplier"},
    {"SumOfLevelsLessThanDiffNamedEventsInMemory", "Empty"},
    {"State", "State"},
};
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
                    skipBlock();
                    continue;
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
            lastToken = line.substr(5, line.find(' '));
            parseEnum();
            skipBlock();
            continue;
        }

        lastToken = line.substr(0, line.find(' '));

        auto pos = line.find('{');
        if (pos == std::string::npos)
            continue;

        parseStruct();
    }
}

LanguageSpecification& LanguageSpecification::get() {
    static LanguageSpecification langSpec;
    return langSpec;
}

std::vector<LangComponent> LanguageSpecification::getComponents(const QString typeName) {
    if (!typeToStruct.contains(typeName)) {
        qDebug() << "typeName " << typeName << " not found";
        return {};
    }
    auto componentName = typeToStruct[typeName];
    if (parsed_.contains(componentName)) {
        return parsed_[componentName];
    }
    return {};
}
