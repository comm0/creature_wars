#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ai_profile.h"
#include "base_orders.h"
#include "player_state.h"

struct base_controller_t {
    player_state_t state_;
    bool human_controlled_ = false;
    ai_difficulty_t difficulty_ = ai_difficulty_t::normal;
    ai_strategy_t strategy_ = ai_strategy_t::recover;
    const ai_profile_t* profile_ = nullptr;
    std::unique_ptr<base_orders_t> orders_;
    std::unordered_map<std::string, int> research_levels_;
    std::unordered_set<std::string> trained_units_;
    std::unordered_set<std::uint64_t> commanded_units_;
    std::optional<std::uint64_t> strategic_target_id_;
};
