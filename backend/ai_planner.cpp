#include "ai_planner.h"

#include <algorithm>

namespace {

bool can_afford(
    const base_action_state_t& action_p,
    const ai_context_t& context_p,
    int gold_reserve_p,
    int food_reserve_p
)
{
    return context_p.gold_ >= action_p.cost_.cost_.gold_ + gold_reserve_p
        && context_p.food_ >= action_p.cost_.cost_.food_ + food_reserve_p;
}

const base_action_state_t* select_spawn(
    const ai_profile_t& profile_p,
    const ai_context_t& context_p,
    const std::vector<base_action_state_t>& actions_p,
    bool strongest_p,
    bool keep_reserve_p
)
{
    const base_action_state_t* selected = nullptr;

    for (const auto& action : actions_p) {
        if (action.kind_ != base_action_kind_t::spawn
            || action.queued_ >= profile_p.maximum_spawn_queue_
            || !can_afford(
                action,
                context_p,
                keep_reserve_p ? profile_p.gold_reserve_ : 0,
                keep_reserve_p ? profile_p.food_reserve_ : 0
            )) {
            continue;
        }

        if (selected == nullptr
            || (strongest_p && action.level_ > selected->level_)
            || (!strongest_p
                && action.cost_.cost_.gold_ + action.cost_.cost_.food_
                    < selected->cost_.cost_.gold_ + selected->cost_.cost_.food_)) {
            selected = &action;
        }
    }

    return selected;
}

const base_action_state_t* select_action(
    base_action_kind_t kind_p,
    const ai_profile_t& profile_p,
    const ai_context_t& context_p,
    const std::vector<base_action_state_t>& actions_p
)
{
    const base_action_state_t* selected = nullptr;

    for (const auto& action : actions_p) {
        if (action.kind_ != kind_p
            || action.queued_ > 0
            || !can_afford(action, context_p, profile_p.gold_reserve_, profile_p.food_reserve_)) {
            continue;
        }

        if (selected == nullptr || action.cost_.cost_.gold_ < selected->cost_.cost_.gold_) {
            selected = &action;
        }
    }

    return selected;
}

}

ai_decision_t ai_planner_t::plan(
    const ai_profile_t& profile_p,
    const ai_context_t& context_p,
    const std::vector<base_action_state_t>& actions_p
)
{
    ai_decision_t decision;
    const auto under_attack = context_p.nearby_enemy_count_ > 0
        || context_p.base_health_ * 100 < context_p.base_max_health_ * 70;

    if (under_attack) {
        decision.strategy_ = ai_strategy_t::defend;
        const auto* spawn = select_spawn(profile_p, context_p, actions_p, true, false);
        if (spawn != nullptr) {
            decision.action_key_ = spawn->key_;
        }
        return decision;
    }

    const auto desired_army = profile_p.minimum_army_for_growth_
        + std::max(0, context_p.base_level_ - 1) * 2;
    if (context_p.army_size_ < desired_army) {
        decision.strategy_ = ai_strategy_t::rally;
        const auto* spawn = select_spawn(profile_p, context_p, actions_p, false, false);
        if (spawn != nullptr) {
            decision.action_key_ = spawn->key_;
        } else {
            decision.strategy_ = ai_strategy_t::recover;
        }
        return decision;
    }

    const auto resources_low = context_p.gold_ < profile_p.hunt_gold_below_
        || context_p.food_ < profile_p.hunt_food_below_;
    decision.launch_hunt_ = context_p.neutral_target_available_ && resources_low;
    decision.launch_assault_ = !decision.launch_hunt_
        && context_p.army_size_ >= profile_p.minimum_army_for_assault_;
    decision.strategy_ = decision.launch_hunt_
        ? ai_strategy_t::hunt
        : (decision.launch_assault_ ? ai_strategy_t::assault : ai_strategy_t::grow);

    if (const auto* training = select_action(
            base_action_kind_t::training,
            profile_p,
            context_p,
            actions_p
        )) {
        decision.action_key_ = training->key_;
        return decision;
    }

    if (const auto* upgrade = select_action(
            base_action_kind_t::upgrade,
            profile_p,
            context_p,
            actions_p
        )) {
        decision.action_key_ = upgrade->key_;
        return decision;
    }

    const auto has_upgrade = std::any_of(
        actions_p.begin(),
        actions_p.end(),
        [](const auto& action_p) {
            return action_p.kind_ == base_action_kind_t::upgrade;
        }
    );

    if (!has_upgrade) {
        if (const auto* research = select_action(
                base_action_kind_t::research,
                profile_p,
                context_p,
                actions_p
            )) {
            decision.action_key_ = research->key_;
            return decision;
        }
    }

    if (decision.launch_assault_) {
        const auto* spawn = select_spawn(profile_p, context_p, actions_p, true, true);
        if (spawn != nullptr) {
            decision.action_key_ = spawn->key_;
        }
    }

    return decision;
}
