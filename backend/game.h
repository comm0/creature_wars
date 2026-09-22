#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

#include "creatures.h"
#include "creature_type_registry.h"
#include "game_event_dispatcher.h"
#include "game_map.h"
#include "game_observer.h"
#include "visibility_system.h"

class game_t
{
public:
    explicit game_t(const std::string& creature_types_json_p);
    ~game_t();

    game_t(const game_t&) = delete;
    game_t& operator=(const game_t&) = delete;

    void start(igame_observer_t& observer_p);
    void stop();
    void request_spawn_creature(
        std::string identifier_p,
        position_t position_p
    );
    void request_walk_to(
        std::uint64_t id_p,
        position_t destination_p
    );
    void request_set_aggressive(bool aggressive_p);
    bool aggressive() const noexcept
    {
        return aggressive_;
    }
    const creature_t* find_nearest_visible_enemy(
        const creature_t& creature_p
    ) const noexcept;
    const creature_t* find_followed_enemy(
        const creature_t& creature_p,
        std::uint64_t target_id_p
    ) const noexcept;
    bool is_in_attack_range(
        const creature_t& creature_p,
        const creature_t& target_p
    ) const noexcept;
    bool move_creature_towards(
        std::uint64_t id_p,
        position_t destination_p
    );
    void notify_creature_state_changed(const creature_t& creature_p);

private:
    void post(std::function<void()> event_p);
    void spawn_creature(
        std::string identifier_p,
        position_t position_p
    );
    void set_creature_destination(
        std::uint64_t id_p,
        position_t destination_p
    );
    void set_aggressive(bool aggressive_p);
    void run(const std::stop_token& stop_token_p);
    void collect_actions();
    void dispatch_tick();
    void dispatch_movement(std::chrono::steady_clock::time_point now_p);
    void publish_creatures();

#ifndef NDEBUG
    void update_debug_monitor(
        std::chrono::steady_clock::time_point tick_start_p
    ) const;
#endif

    game_event_dispatcher_t dispatcher_;
    creature_type_registry_t creature_type_registry_;
    creatures_t creatures_;
    game_map_t game_map_;
    visibility_system_t visibility_system_;
    igame_observer_t* observer_ = nullptr;

    std::mutex actions_mutex_;
    std::condition_variable actions_available_;
    std::deque<std::function<void()>> actions_;
    bool thread_running_ = false;
    bool aggressive_ = false;
    std::jthread thread_;
};
