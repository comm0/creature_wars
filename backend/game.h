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
#include <vector>

#include "base.h"
#include "base_type_registry.h"
#include "creatures.h"
#include "creature_type_registry.h"
#include "game_event_dispatcher.h"
#include "game_map.h"
#include "game_observer.h"
#include "visibility_system.h"

class game_t
{
public:
    game_t(
        const std::string& creature_types_json_p,
        const std::string& base_types_json_p
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
    void request_spawn_from_base(std::uint64_t base_id_p);
    void request_start_match(std::string player_base_identifier_p);
    void request_walk_to(
        std::uint64_t id_p,
        position_t destination_p
    );
    void request_set_aggressive(bool aggressive_p);
    bool aggressive() const noexcept
    {
        return aggressive_;
    }
    std::optional<target_t> find_nearest_visible_enemy(
        const creature_t& creature_p
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
    void spawn_creature(
        std::string identifier_p,
        position_t position_p
    );
    void spawn_base(std::string identifier_p, position_t center_p, int level_p = 1);
    void spawn_from_base(std::uint64_t base_id_p);
    void start_match(const std::string& player_base_identifier_p);
    void clear_world();
    base_t* find_base(std::uint64_t id_p) noexcept;
    const base_t* find_base(std::uint64_t id_p) const noexcept;
    void damage_target(std::uint64_t target_id_p, int damage_p);
    void remove_dead_bases();
    void set_creature_destination(
        std::uint64_t id_p,
        position_t destination_p
    );
    void set_aggressive(bool aggressive_p);
    void run(const std::stop_token& stop_token_p);
    void collect_actions();
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
    creature_type_registry_t creature_type_registry_;
    base_type_registry_t base_type_registry_;
    creatures_t creatures_;
    std::vector<std::unique_ptr<base_t>> bases_;
    game_map_t game_map_;
    visibility_system_t visibility_system_;
    igame_observer_t* observer_ = nullptr;
    player_state_t player_state_;

    std::mutex actions_mutex_;
    std::condition_variable actions_available_;
    std::deque<std::function<void()>> actions_;
    std::vector<std::uint64_t> dead_creature_ids_;
    bool thread_running_ = false;
    bool aggressive_ = true;
    std::jthread thread_;
};
