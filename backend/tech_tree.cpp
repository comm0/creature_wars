#include "tech_tree.h"

#include "type_parsing.h"

#include <algorithm>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace
{
research_effect_t parse_effect(const std::string& effect_p)
{
    if (effect_p == "baseRegeneration") {
        return research_effect_t::base_regeneration;
    }

    if (effect_p == "baseHealthPercent") {
        return research_effect_t::base_health_percent;
    }

    if (effect_p == "healingAura") {
        return research_effect_t::healing_aura;
    }

    if (effect_p == "baseDamagePercent") {
        return research_effect_t::base_damage_percent;
    }

    if (effect_p == "shotCount") {
        return research_effect_t::shot_count;
    }

    if (effect_p == "baseRangeBonus") {
        return research_effect_t::base_range_bonus;
    }

    throw std::invalid_argument("Unknown research effect.");
}
}

tech_tree_t::tech_tree_t(std::string_view tech_tree_json_p)
{
    const auto document = nlohmann::json::parse(tech_tree_json_p);

    for (const auto& upgrade : document.at("baseUpgrades")) {
        upgrades_.push_back({upgrade.at("level").get<int>(), parse_timed_cost(upgrade)});
    }

    for (const auto& definition : document.at("researches")) {
        research_t research{
            definition.at("id").get<std::string>(),
            definition.at("name").get<std::string>(),
            parse_effect(definition.at("effect").get<std::string>()),
            {}
        };

        for (const auto& level : definition.at("levels")) {
            research.levels_.push_back({
                level.at("baseLevel").get<int>(),
                parse_timed_cost(level),
                level.at("value").get<int>()
            });
        }

        if (research.identifier_.empty() || research.levels_.empty()) {
            throw std::invalid_argument("Research needs an id and levels.");
        }

        if (find_research(research.identifier_) != nullptr) {
            throw std::invalid_argument("Research id must be unique.");
        }

        researches_.push_back(std::move(research));
    }
}

const research_t* tech_tree_t::find_research(std::string_view identifier_p) const noexcept
{
    const auto research = std::find_if(
        researches_.begin(),
        researches_.end(),
        [identifier_p](const auto& research_p) {
            return research_p.identifier_ == identifier_p;
        }
    );

    return research == researches_.end() ? nullptr : &*research;
}

const base_upgrade_t* tech_tree_t::find_upgrade(int level_p) const noexcept
{
    const auto upgrade = std::find_if(
        upgrades_.begin(),
        upgrades_.end(),
        [level_p](const auto& upgrade_p) {
            return upgrade_p.level_ == level_p;
        }
    );

    return upgrade == upgrades_.end() ? nullptr : &*upgrade;
}
