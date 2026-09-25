#include "base_type_registry.h"

#include "type_parsing.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

base_type_registry_t::base_type_registry_t(std::string_view base_types_json_p)
{
    const auto document = nlohmann::json::parse(base_types_json_p);
    const auto& definitions = document.at("baseTypes");

    if (!definitions.is_array()) {
        throw std::invalid_argument("baseTypes must be an array.");
    }

    base_types_.reserve(definitions.size());

    for (const auto& definition : definitions) {
        auto identifier = definition.at("id").get<std::string>();
        auto name = definition.at("name").get<std::string>();
        auto group = definition.at("group").get<std::string>();
        const auto color = parse_color(definition.at("color").get<std::string>());
        const auto size = definition.at("size").get<int>();
        const auto range = definition.at("range").get<int>();
        const auto match_base = definition.value("matchBase", true);
        const auto& reward = definition.value(
            "reward",
            nlohmann::json::object()
        );
        const auto reward_gold = reward.value("gold", 0);
        const auto reward_food = reward.value("food", 0);

        if (identifier.empty() || name.empty() || group.empty()) {
            throw std::invalid_argument("Base id, name and group must not be empty.");
        }

        if (size <= 0) {
            throw std::invalid_argument("Base size must be positive.");
        }

        require_non_negative(range, "range");
        require_non_negative(reward_gold, "reward.gold");
        require_non_negative(reward_food, "reward.food");

        std::vector<base_level_stats_t> levels;

        for (const auto& level : definition.at("levels")) {
            const auto health = level.at("health").get<int>();
            const auto attack = level.at("attack").get<int>();

            if (health <= 0) {
                throw std::invalid_argument("Base health must be positive.");
            }

            require_non_negative(attack, "attack");
            levels.push_back({health, attack});
        }

        if (levels.empty()) {
            throw std::invalid_argument("Base needs at least one level.");
        }

        std::vector<unit_option_t> units;

        for (const auto& unit : definition.at("units")) {
            std::optional<timed_cost_t> training;

            if (unit.contains("training")) {
                training = parse_timed_cost(unit.at("training"));
            }

            units.push_back({
                unit.at("creature").get<std::string>(),
                unit.at("tier").get<int>(),
                parse_timed_cost(unit),
                training
            });
        }

        const auto duplicate = std::find_if(
            base_types_.begin(),
            base_types_.end(),
            [&identifier](const auto& base_type_p) {
                return base_type_p.identifier() == identifier;
            }
        );

        if (duplicate != base_types_.end()) {
            throw std::invalid_argument("Base type id must be unique.");
        }

        base_types_.emplace_back(
            std::move(identifier),
            std::move(name),
            std::move(group),
            color,
            size,
            range,
            match_base,
            resource_reward_t{reward_gold, reward_food},
            std::move(levels),
            std::move(units)
        );
    }
}

const base_type_t& base_type_registry_t::get(std::string_view identifier_p) const
{
    const auto base_type = std::find_if(
        base_types_.begin(),
        base_types_.end(),
        [identifier_p](const auto& base_type_p) {
            return base_type_p.identifier() == identifier_p;
        }
    );

    if (base_type == base_types_.end()) {
        throw std::out_of_range("Unknown base type.");
    }

    return *base_type;
}
