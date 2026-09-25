#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include <unordered_map>
#include "ai_planner.h"
#include "ai_profile.h"
#include "base.h"
#include "base_controller.h"
#include "base_orders.h"
#include "base_type_registry.h"
#include "creatures.h"
#include "creature_type_registry.h"
#include "economy_settings.h"
#include "game_clock.h"
#include "game_event_dispatcher.h"
#include "game_map.h"
#include "game_observer.h"
#include "game_scheduler.h"
#include "resource_reward.h"
#include "tech_tree.h"
#include "visibility_system.h"
#include "wildlife_spawner.h"

class game_t
{
public:
    game_t(
        const std::string& creature_types_json_p,
        const std::string& base_types_json_p,
        const std::string& economy_json_p,
        const std::string& tech_tree_json_p,
        const std::string& ai_profiles_json_p
    );
    ~game_t();

    game_t(const game_t&) = delete;
    game_t& operator=(const game_t&) = delete;

    void start(igame_observer_t& observer_p);
    void stop();
    void request_spawn_creature(
        std::string identifier_p,
        position_t position_p
    );
    void request_spawn_base(std::string identifier_p, position_t center_p);
    void request_order_base_action(std::string key_p);
    void request_start_match(
        std::string player_base_identifier_p,
        std::unordered_map<std::string, ai_difficulty_t> ai_difficulties_p
    );
    void request_attack_target(std::uint64_t id_p, std::uint64_t target_id_p);
    void request_walk_to(
        std::uint64_t id_p,
        position_t destination_p
    );
    void request_set_aggressive(bool aggressive_p);
    void request_set_time_scale(double time_scale_p);
    bool aggressive() const noexcept
    {
        return aggressive_;
    }
    game_clock_t::time_point_t current_time() const noexcept
    {
        return game_clock_.now();
    }
    std::optional<target_t> find_nearest_visible_enemy(
        const creature_t& creature_p
    ) const noexcept;
    std::optional<target_t> find_commanded_target(
        const creature_t& creature_p,
        std::uint64_t target_id_p
    ) const noexcept;
    std::optional<target_t> find_visible_enemy(
        const creature_t& creature_p,
        std::uint64_t target_id_p
    ) const noexcept;
    std::optional<target_t> find_nearest_visible_threat(
        const creature_t& creature_p
    ) const noexcept;
    std::optional<target_t> find_visible_threat(
        const creature_t& creature_p,
        std::uint64_t target_id_p
    ) const noexcept;
    bool is_in_attack_range(
        const creature_t& creature_p,
        const target_t& target_p
    ) const noexcept;
    std::optional<target_t> find_base_target(
        const base_t& base_p,
        std::uint64_t target_id_p
    ) const noexcept;
    std::optional<target_t> find_nearest_base_target(
        const base_t& base_p
    ) const noexcept;
    void perform_base_attack(const base_t& base_p, const target_t& target_p);
    bool move_creature_towards(
        std::uint64_t id_p,
        position_t destination_p
    );
    bool move_creature_away_from(
        std::uint64_t id_p,
        position_t threat_position_p
    );
    bool move_creature_idle(
        std::uint64_t id_p,
        position_t idle_starting_position_p
    );
    bool check_creature_attack(
        std::uint64_t attacker_id_p,
        std::uint64_t target_id_p
    );
    void notify_creature_state_changed(const creature_t& creature_p);

private:
    void post(std::function<void()> event_p);
    creature_t* spawn_creature(
        std::string identifier_p,
        position_t position_p
    );
    base_t* spawn_base(const std::string& identifier_p, position_t center_p);
    void spawn_creature_near_group(
        std::string_view identifier_p,
        std::string_view group_p
    );
    std::optional<std::uint64_t> spawn_lair_near_group(std::string_view group_p);
    bool spawn_troll_near_lair(std::uint64_t lair_id_p);
    bool lair_exists(std::uint64_t lair_id_p) const noexcept;
    std::vector<base_action_state_t> base_actions(
        const base_controller_t& controller_p
    ) const;
    bool order_base_action(base_controller_t& controller_p, const std::string& key_p);
    bool complete_base_order(base_controller_t& controller_p, const std::string& key_p);
    void publish_base_actions();
    void publish_controller(const base_controller_t& controller_p);
    int research_level(
        const base_controller_t& controller_p,
        const std::string& identifier_p
    ) const;
    int research_value(
        const base_controller_t& controller_p,
        research_effect_t effect_p
    ) const;
    void apply_base_stats(base_controller_t& controller_p, base_t& base_p);
    void update_income_intervals(base_controller_t& controller_p);
    void regenerate_bases();
    void heal_near_bases();
    void damage_area(
        const creature_t& attacker_p,
        position_t center_p,
        int radius_p
    );
    void start_match(
        const std::string& player_base_identifier_p,
        const std::unordered_map<std::string, ai_difficulty_t>& ai_difficulties_p
    );
    base_controller_t& create_controller(
        const base_t& base_p,
        bool human_controlled_p,
        ai_difficulty_t difficulty_p
    );
    base_controller_t* controller_for_base(std::uint64_t base_id_p) noexcept;
    const base_controller_t* controller_for_base(std::uint64_t base_id_p) const noexcept;
    base_controller_t* controller_for_group(std::string_view group_p) noexcept;
    base_controller_t* player_controller() noexcept;
    const base_controller_t* player_controller() const noexcept;
    void schedule_ai_decision(
        std::uint64_t base_id_p,
        game_scheduler_t::time_point_t time_p
    );
    void run_ai_decision(base_controller_t& controller_p);
    std::optional<std::uint64_t> find_neutral_target(
        const base_controller_t& controller_p
    ) const;
    void command_attack(
        base_controller_t& controller_p,
        std::uint64_t target_id_p,
        std::size_t maximum_unit_count_p
    );
    void launch_assault(base_controller_t& controller_p);
    void clear_world();
    base_t* find_base(std::uint64_t id_p) noexcept;
    const base_t* find_base(std::uint64_t id_p) const noexcept;
    const base_t* find_match_base(std::string_view group_p) const noexcept;
    void damage_target(
        std::uint64_t target_id_p,
        int damage_p,
        const std::string& attacker_group_p
    );
    void award_reward(
        const resource_reward_t& reward_p,
        const damage_contributions_t& contributions_p
    );
    void create_corpse(const creature_t& creature_p);
    void remove_corpse(std::uint64_t id_p);
    void remove_dead_bases();
    void eliminate_group(const std::string& group_p);
    void check_match_end();
    void end_match(bool victory_p);
    void set_attack_target(std::uint64_t id_p, std::uint64_t target_id_p);
    bool is_player_creature(const creature_t& creature_p) const noexcept;
    void report_target_changes();
    void set_creature_destination(
        std::uint64_t id_p,
        position_t destination_p
    );
    void set_aggressive(bool aggressive_p);
    void set_time_scale(double time_scale_p);
    void run(const std::stop_token& stop_token_p);
    void collect_actions();
    void schedule_tick(game_scheduler_t::time_point_t time_p);
    void schedule_income(
        std::uint64_t base_id_p,
        resource_state_t player_state_t::* resource_p,
        game_scheduler_t::time_point_t time_p
    );
    void dispatch_tick();
    void dispatch_movement(std::chrono::steady_clock::time_point now_p);
    void remove_dead_creatures();
    bool move_creature(creature_t& creature_p, position_t position_p);
    void notify_creature_spotted(
        std::uint64_t observer_id_p,
        std::uint64_t spotted_id_p
    );
    void publish_creatures();

#ifndef NDEBUG
    void update_debug_monitor(
        std::chrono::steady_clock::time_point tick_start_p
    ) const;
#endif

    game_event_dispatcher_t dispatcher_;
    game_scheduler_t scheduler_;
    game_clock_t game_clock_;
    wildlife_spawner_t wildlife_spawner_;
    bool tick_scheduled_ = false;
    std::uint64_t match_id_ = 0;
    bool match_active_ = false;
    game_clock_t::time_point_t match_started_at_{};
    creature_type_registry_t creature_type_registry_;
    base_type_registry_t base_type_registry_;
    economy_settings_t economy_;
    tech_tree_t tech_tree_;
    ai_profile_registry_t ai_profiles_;
    std::vector<std::unique_ptr<base_controller_t>> controllers_;
    std::vector<std::unique_ptr<base_controller_t>> retired_controllers_;
    std::unordered_map<std::string, resource_reward_t> earned_resources_;
    std::unordered_set<std::uint64_t> corpse_ids_;
    creatures_t creatures_;
    std::vector<std::unique_ptr<base_t>> bases_;
    game_map_t game_map_;
    visibility_system_t visibility_system_;
    igame_observer_t* observer_ = nullptr;

    std::mutex actions_mutex_;
    std::condition_variable actions_available_;
    std::deque<std::function<void()>> actions_;
    std::vector<std::uint64_t> dead_creature_ids_;
    bool thread_running_ = false;
    bool aggressive_ = true;
    std::jthread thread_;
};
