#pragma once

#include <string>
#include <vector>

#include "ai_profile.h"
#include "base_action.h"

struct ai_context_t {
    int gold_ = 0;
    int food_ = 0;
    int base_level_ = 1;
    int army_size_ = 0;
    int nearby_enemy_count_ = 0;
    int base_health_ = 0;
    int base_max_health_ = 1;
    bool neutral_target_available_ = false;
};

struct ai_decision_t {
    ai_strategy_t strategy_ = ai_strategy_t::recover;
    std::string action_key_;
    bool launch_hunt_ = false;
    bool launch_assault_ = false;
};

class ai_planner_t {
public:
    [[nodiscard]] static ai_decision_t plan(
        const ai_profile_t& profile_p,
        const ai_context_t& context_p,
        const std::vector<base_action_state_t>& actions_p
    );
};
