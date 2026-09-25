#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_set>

#include "game.h"

namespace
{
std::string load_data(const std::string& name_p)
{
    const auto path = std::filesystem::path{CREATURE_WARS_SOURCE_DIR}
        / "data"
        / name_p;
    std::ifstream input{path};
    std::ostringstream output;
    output << input.rdbuf();
    return output.str();
}

class ai_test_observer_t final : public igame_observer_t
{
public:
    bool wait_for_ai_base(std::chrono::milliseconds timeout_p)
    {
        std::unique_lock lock{mutex_};
        return changed_.wait_for(lock, timeout_p, [this]() {
            return ai_base_position_.has_value();
        });
    }

    position_t ai_base_position()
    {
        std::lock_guard lock{mutex_};
        return *ai_base_position_;
    }

    bool wait_for_ai_hunt(std::chrono::milliseconds timeout_p)
    {
        std::unique_lock lock{mutex_};
        return changed_.wait_for(lock, timeout_p, [this]() {
            return ai_creature_created_
                && hard_difficulty_observed_
                && hunt_strategy_observed_
                && neutral_target_observed_;
        });
    }

    bool wait_for_restarted_match(std::chrono::milliseconds timeout_p)
    {
        std::unique_lock lock{mutex_};
        return changed_.wait_for(lock, timeout_p, [this]() {
            return human_base_ids_.size() >= 2;
        });
    }

    void on_creature_created(const creature_t& creature_p) override
    {
        {
            std::lock_guard lock{mutex_};
            if (creature_p.type().group() == "orcs") {
                ai_creature_ids_.insert(creature_p.id());
                ai_creature_created_ = true;
            } else if (creature_p.type().group() == "deer") {
                neutral_creature_ids_.insert(creature_p.id());
            }
        }
        changed_.notify_one();
    }

    void on_base_created(const base_t& base_p) override
    {
        if (base_p.type().group() != "orcs") {
            return;
        }

        {
            std::lock_guard lock{mutex_};
            ai_base_position_ = base_p.position();
        }
        changed_.notify_one();
    }

    void on_base_controller_changed(
        const base_controller_state_t& state_p
    ) override
    {
        {
            std::lock_guard lock{mutex_};
            if (state_p.human_controlled_) {
                human_base_ids_.insert(state_p.resources_.base_id_);
            } else if (state_p.resources_.base_group_ == "orcs") {
                hard_difficulty_observed_ = state_p.difficulty_ == "hard"
                    || hard_difficulty_observed_;
                hunt_strategy_observed_ = state_p.strategy_ == "hunt"
                    || hunt_strategy_observed_;
            }
        }
        changed_.notify_one();
    }

    void on_creature_target_changed(
        std::uint64_t id_p,
        std::uint64_t target_id_p
    ) override
    {
        {
            std::lock_guard lock{mutex_};
            if (ai_creature_ids_.contains(id_p)
                && neutral_creature_ids_.contains(target_id_p)) {
                neutral_target_observed_ = true;
            }
        }
        changed_.notify_one();
    }

    void on_game_tick() override {}
    void on_creature_moved(std::uint64_t, position_t, direction_t) override {}
    void on_creature_health_changed(std::uint64_t, int) override {}
    void on_creature_attack_performed(std::uint64_t) override {}
    void on_creature_walk_requested(std::uint64_t) override {}
    void on_creature_state_changed(std::uint64_t, creature_state_t) override {}
    void on_creature_spotted(std::uint64_t, std::uint64_t) override {}
    void on_creature_removed(std::uint64_t) override {}
    void on_corpse_created(std::uint64_t, position_t, const std::string&) override {}
    void on_corpse_removed(std::uint64_t) override {}
    void on_base_health_changed(std::uint64_t, int) override {}
    void on_base_attack_performed(std::uint64_t, position_t) override {}
    void on_base_removed(std::uint64_t) override {}
    void on_player_state_changed(const player_state_t&) override {}
    void on_base_changed(const base_t&) override {}
    void on_base_actions_changed(
        std::uint64_t,
        const std::vector<base_action_state_t>&
    ) override {}
    void on_area_attack(position_t, int, std::uint32_t) override {}
    void on_match_ended(bool, std::chrono::milliseconds) override {}

private:
    std::mutex mutex_;
    std::condition_variable changed_;
    std::optional<position_t> ai_base_position_;
    std::unordered_set<std::uint64_t> ai_creature_ids_;
    std::unordered_set<std::uint64_t> neutral_creature_ids_;
    std::unordered_set<std::uint64_t> human_base_ids_;
    bool ai_creature_created_ = false;
    bool hard_difficulty_observed_ = false;
    bool hunt_strategy_observed_ = false;
    bool neutral_target_observed_ = false;
};

TEST(game_ai_test_t, ai_produces_units_hunts_and_survives_match_restart)
{
    ai_test_observer_t observer;
    game_t game{
        load_data("creature_types.json"),
        load_data("base_types.json"),
        load_data("economy.json"),
        load_data("tech_tree.json"),
        load_data("ai_profiles.json")
    };
    game.start(observer);
    game.request_set_time_scale(100.0);
    game.request_start_match(
        "minotaur_base",
        {
            {"orc_base", ai_difficulty_t::hard},
            {"dwarf_base", ai_difficulty_t::easy}
        }
    );

    ASSERT_TRUE(observer.wait_for_ai_base(std::chrono::seconds{1}));
    const auto position = observer.ai_base_position();
    game.request_spawn_creature("deer", {position.column_ + 4, position.row_});
    game.request_spawn_creature("deer", {position.column_ - 2, position.row_});
    game.request_spawn_creature("deer", {position.column_, position.row_ + 4});
    game.request_spawn_creature("deer", {position.column_, position.row_ - 2});

    const auto hunt_observed = observer.wait_for_ai_hunt(std::chrono::seconds{2});
    game.request_start_match(
        "minotaur_base",
        {
            {"orc_base", ai_difficulty_t::normal},
            {"dwarf_base", ai_difficulty_t::normal}
        }
    );
    const auto restarted_match_observed = observer.wait_for_restarted_match(
        std::chrono::seconds{2}
    );
    game.stop();

    EXPECT_TRUE(hunt_observed);
    EXPECT_TRUE(restarted_match_observed);
}
}
