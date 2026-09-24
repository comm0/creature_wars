#include "creature_type_registry.h"

#include "type_parsing.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

creature_type_registry_t::creature_type_registry_t(
    std::string_view creature_types_json_p
)
{
    const auto document = nlohmann::json::parse(creature_types_json_p);
    const auto& definitions = document.at("creatureTypes");

    if (!definitions.is_array()) {
        throw std::invalid_argument("creatureTypes must be an array.");
    }

    creature_types_.reserve(definitions.size());

    for (const auto& definition : definitions) {
        auto identifier = definition.at("id").get<std::string>();
        auto name = definition.at("name").get<std::string>();
        auto group = definition.value("group", std::string("none"));
        const auto color = parse_color(definition.at("color").get<std::string>());
        std::optional<std::uint32_t> marker_color;

        if (definition.contains("markerColor")
            && !definition.at("markerColor").is_null()) {
            marker_color = parse_color(
                definition.at("markerColor").get<std::string>()
            );
        }

        const auto health = definition.at("health").get<int>();
        const auto attack = definition.at("attack").get<int>();
        const auto attack_range = definition.at("attackRange").get<int>();
        const auto vision_range = definition.at("visionRange").get<int>();
        const auto speed = definition.at("speed").get<double>();

        if (identifier.empty() || name.empty()) {
            throw std::invalid_argument("Creature id and name must not be empty.");
        }

        if (group.empty()) {
            group = "none";
        }

        require_non_negative(health, "health");
        require_non_negative(attack, "attack");
        require_non_negative(attack_range, "attackRange");
        require_non_negative(vision_range, "visionRange");

        if (!std::isfinite(speed) || speed <= 0.0) {
            throw std::invalid_argument("Creature speed must be positive.");
        }

        const auto duplicate = std::find_if(
            creature_types_.begin(),
            creature_types_.end(),
            [&identifier](const auto& creature_type_p) {
                return creature_type_p.identifier() == identifier;
            }
        );

        if (duplicate != creature_types_.end()) {
            throw std::invalid_argument("Creature type id must be unique.");
        }

        creature_types_.emplace_back(
            std::move(identifier),
            std::move(name),
            std::move(group),
            color,
            marker_color,
            creature_attributes_t{
                health,
                attack,
                attack_range,
                vision_range,
                speed
            }
        );
    }
}

const creature_type_t& creature_type_registry_t::get(
    std::string_view identifier_p
) const
{
    const auto creature_type = std::find_if(
        creature_types_.begin(),
        creature_types_.end(),
        [identifier_p](const auto& creature_type_p) {
            return creature_type_p.identifier() == identifier_p;
        }
    );

    if (creature_type == creature_types_.end()) {
        throw std::out_of_range("Unknown creature type.");
    }

    return *creature_type;
}
