#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

enum class ai_difficulty_t {
    easy,
    normal,
    hard
};

enum class ai_strategy_t {
    recover,
    grow,
    defend,
    hunt,
    rally,
    assault
};

struct ai_profile_t {
    std::chrono::milliseconds decision_interval_ {1500};
    int gold_reserve_ {15};
    int food_reserve_ {1};
    int minimum_army_for_growth_ {3};
    int minimum_army_for_assault_ {3};
    int maximum_spawn_queue_ {2};
    int hunt_gold_below_ {60};
    int hunt_food_below_ {5};
    int hunt_maximum_distance_ {14};
};

class ai_profile_registry_t {
public:
    explicit ai_profile_registry_t(const std::string& json_p);

    [[nodiscard]] const ai_profile_t& profile(ai_difficulty_t difficulty_p) const;

private:
    std::unordered_map<ai_difficulty_t, ai_profile_t> profiles_;
};

[[nodiscard]] ai_difficulty_t ai_difficulty_from_string(const std::string& value_p);
[[nodiscard]] std::string to_string(ai_difficulty_t difficulty_p);
[[nodiscard]] std::string to_string(ai_strategy_t strategy_p);
