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
        auto spawn_creature = definition.at("spawnCreature").get<std::string>();
        const auto size = definition.at("size").get<int>();
        const auto health = definition.at("health").get<int>();
        const auto attack = definition.at("attack").get<int>();
        const auto attack_range = definition.at("attackRange").get<int>();

        if (identifier.empty() || name.empty() || group.empty()) {
            throw std::invalid_argument("Base id, name and group must not be empty.");
        }

        if (size <= 0 || health <= 0) {
            throw std::invalid_argument("Base size and health must be positive.");
        }

        require_non_negative(attack, "attack");
        require_non_negative(attack_range, "attackRange");

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
            std::move(spawn_creature),
            base_attributes_t{size, health, attack, attack_range}
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
