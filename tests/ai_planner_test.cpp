#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <utility>

#include "ai_planner.h"

namespace
{
base_action_state_t action(
    std::string key_p,
    base_action_kind_t kind_p,
    int level_p,
    int gold_p,
    int food_p = 0,
    int queued_p = 0
)
{
    return {
        std::move(key_p),
        kind_p,
        {},
        {},
        0,
        level_p,
        {{gold_p, food_p}, std::chrono::seconds{1}},
        queued_p,
        std::chrono::milliseconds{0}
    };
}

ai_profile_t profile()
{
    return {
        std::chrono::milliseconds{1500},
        15,
        1,
        3,
        7,
        2,
        60,
        5,
        14
    };
}

TEST(ai_planner_test_t, recovers_while_it_cannot_afford_initial_army)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 0,
            .food_ = 0,
            .base_level_ = 1,
            .army_size_ = 0,
            .base_health_ = 100,
            .base_max_health_ = 100
        },
        {action("spawn:minotaur", base_action_kind_t::spawn, 1, 10, 1)}
    );

    EXPECT_EQ(decision.strategy_, ai_strategy_t::recover);
    EXPECT_TRUE(decision.action_key_.empty());
}

TEST(ai_planner_test_t, builds_the_cheapest_unit_for_initial_army)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 30,
            .food_ = 2,
            .base_level_ = 1,
            .army_size_ = 1,
            .base_health_ = 100,
            .base_max_health_ = 100
        },
        {
            action("spawn:expensive", base_action_kind_t::spawn, 1, 20, 2),
            action("spawn:cheap", base_action_kind_t::spawn, 1, 10, 1)
        }
    );

    EXPECT_EQ(decision.strategy_, ai_strategy_t::rally);
    EXPECT_EQ(decision.action_key_, "spawn:cheap");
}

TEST(ai_planner_test_t, defense_ignores_reserve_and_selects_strongest_unit)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 20,
            .food_ = 2,
            .base_level_ = 2,
            .army_size_ = 4,
            .nearby_enemy_count_ = 1,
            .base_health_ = 100,
            .base_max_health_ = 100
        },
        {
            action("spawn:tier1", base_action_kind_t::spawn, 1, 10, 1),
            action("spawn:tier2", base_action_kind_t::spawn, 2, 20, 2)
        }
    );

    EXPECT_EQ(decision.strategy_, ai_strategy_t::defend);
    EXPECT_EQ(decision.action_key_, "spawn:tier2");
}

TEST(ai_planner_test_t, hunts_neutral_creatures_when_resources_are_low)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 40,
            .food_ = 4,
            .base_level_ = 1,
            .army_size_ = 3,
            .base_health_ = 100,
            .base_max_health_ = 100,
            .neutral_target_available_ = true
        },
        {}
    );

    EXPECT_EQ(decision.strategy_, ai_strategy_t::hunt);
    EXPECT_TRUE(decision.launch_hunt_);
    EXPECT_FALSE(decision.launch_assault_);
}

TEST(ai_planner_test_t, trains_a_new_tier_before_upgrading)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 250,
            .food_ = 6,
            .base_level_ = 2,
            .army_size_ = 6,
            .base_health_ = 100,
            .base_max_health_ = 100
        },
        {
            action("training:tier2", base_action_kind_t::training, 2, 30),
            action("upgrade", base_action_kind_t::upgrade, 3, 200)
        }
    );

    EXPECT_EQ(decision.action_key_, "training:tier2");
}

TEST(ai_planner_test_t, upgrades_after_gathering_the_required_reserve)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 220,
            .food_ = 5,
            .base_level_ = 1,
            .army_size_ = 3,
            .base_health_ = 100,
            .base_max_health_ = 100
        },
        {action("upgrade", base_action_kind_t::upgrade, 2, 200)}
    );

    EXPECT_EQ(decision.strategy_, ai_strategy_t::grow);
    EXPECT_EQ(decision.action_key_, "upgrade");
}

TEST(ai_planner_test_t, assaults_after_reaching_the_profile_threshold)
{
    const auto decision = ai_planner_t::plan(
        profile(),
        {
            .gold_ = 60,
            .food_ = 5,
            .base_level_ = 1,
            .army_size_ = 7,
            .base_health_ = 100,
            .base_max_health_ = 100
        },
        {action("upgrade", base_action_kind_t::upgrade, 2, 200)}
    );

    EXPECT_EQ(decision.strategy_, ai_strategy_t::assault);
    EXPECT_TRUE(decision.launch_assault_);
}
}
