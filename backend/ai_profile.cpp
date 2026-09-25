#include "ai_profile.h"

#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {

ai_profile_t parse_profile(const nlohmann::json& profile_p)
{
    ai_profile_t result;
    result.decision_interval_ = std::chrono::milliseconds{
        profile_p.at("decisionIntervalMs").get<int>()
    };
    result.gold_reserve_ = profile_p.at("goldReserve").get<int>();
    result.food_reserve_ = profile_p.at("foodReserve").get<int>();
    result.minimum_army_for_growth_ = profile_p.at("minimumArmyForGrowth").get<int>();
    result.minimum_army_for_assault_ = profile_p.at("minimumArmyForAssault").get<int>();
    result.maximum_spawn_queue_ = profile_p.at("maximumSpawnQueue").get<int>();
    result.hunt_gold_below_ = profile_p.at("huntGoldBelow").get<int>();
    result.hunt_food_below_ = profile_p.at("huntFoodBelow").get<int>();
    result.hunt_maximum_distance_ = profile_p.at("huntMaximumDistance").get<int>();

    if (result.decision_interval_.count() <= 0
        || result.gold_reserve_ < 0
        || result.food_reserve_ < 0
        || result.minimum_army_for_growth_ < 1
        || result.minimum_army_for_assault_ < result.minimum_army_for_growth_
        || result.maximum_spawn_queue_ < 1
        || result.hunt_gold_below_ < 0
        || result.hunt_food_below_ < 0
        || result.hunt_maximum_distance_ < 1) {
        throw std::runtime_error("Invalid AI profile values.");
    }

    return result;
}

}

ai_profile_registry_t::ai_profile_registry_t(const std::string& json_p)
{
    const auto root = nlohmann::json::parse(json_p);
    const auto& profiles = root.at("profiles");

    for (const auto difficulty : {ai_difficulty_t::easy, ai_difficulty_t::normal, ai_difficulty_t::hard}) {
        profiles_.emplace(difficulty, parse_profile(profiles.at(to_string(difficulty))));
    }
}

const ai_profile_t& ai_profile_registry_t::profile(ai_difficulty_t difficulty_p) const
{
    return profiles_.at(difficulty_p);
}

ai_difficulty_t ai_difficulty_from_string(const std::string& value_p)
{
    if (value_p == "easy") {
        return ai_difficulty_t::easy;
    }
    if (value_p == "hard") {
        return ai_difficulty_t::hard;
    }
    return ai_difficulty_t::normal;
}

std::string to_string(ai_difficulty_t difficulty_p)
{
    switch (difficulty_p) {
    case ai_difficulty_t::easy:
        return "easy";
    case ai_difficulty_t::normal:
        return "normal";
    case ai_difficulty_t::hard:
        return "hard";
    }

    return "normal";
}

std::string to_string(ai_strategy_t strategy_p)
{
    switch (strategy_p) {
    case ai_strategy_t::recover:
        return "recover";
    case ai_strategy_t::grow:
        return "grow";
    case ai_strategy_t::defend:
        return "defend";
    case ai_strategy_t::hunt:
        return "hunt";
    case ai_strategy_t::rally:
        return "rally";
    case ai_strategy_t::assault:
        return "assault";
    }

    return "recover";
}
