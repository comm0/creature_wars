#pragma once

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

#ifndef NDEBUG
#include <chrono>
#endif

#include "creatures.h"
#include "creature_type_registry.h"
#include "game_event_dispatcher.h"
#include "game_map.h"

class game_t
{
public:
    explicit game_t(std::string creature_types_json_p);
    ~game_t();

    game_t(const game_t&) = delete;
    game_t& operator=(const game_t&) = delete;

    void start(
        std::function<void()> heartbeat_handler_p,
        std::function<void(const creature_t&)> creature_update_handler_p
    );
    void stop();
    void post(std::function<void()> event_p);
    void spawn_creature(std::string group_p);
    void request_move(std::uint64_t id_p, direction_t direction_p);

private:
    void run(const std::stop_token& stop_token_p);
    void collect_actions();
    void dispatch_tick();
    void publish_creature_positions();

#ifndef NDEBUG
    void update_debug_monitor(
        std::chrono::steady_clock::time_point tick_start_p
    ) const;
#endif

    game_event_dispatcher_t dispatcher_;
    creature_type_registry_t creature_type_registry_;
    creatures_t creatures_;
    game_map_t game_map_;
    std::function<void()> heartbeat_handler_;
    std::function<void(const creature_t&)> creature_update_handler_;

    std::mutex actions_mutex_;
    std::condition_variable actions_available_;
    std::deque<std::function<void()>> actions_;
    bool thread_running_ = false;
    std::jthread thread_;
};
