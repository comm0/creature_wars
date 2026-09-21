#pragma once

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>

#include "creatures.h"
#include "game_event_dispatcher.h"

class game_t
{
public:
    game_t();
    ~game_t();

    game_t(const game_t&) = delete;
    game_t& operator=(const game_t&) = delete;

    void start(
        std::function<void()> heartbeat_handler_p,
        std::function<void(std::uint64_t, position_t)> creature_position_handler_p
    );
    void stop();
    void post(std::function<void()> event_p);
    void request_move(std::uint64_t id_p, direction_t direction_p);

private:
    void run(std::stop_token stop_token_p);
    void collect_actions();
    void dispatch_tick();
    void publish_creature_positions();

    game_event_dispatcher_t dispatcher_;
    creatures_t creatures_;
    std::function<void()> heartbeat_handler_;
    std::function<void(std::uint64_t, position_t)> creature_position_handler_;

    std::mutex actions_mutex_;
    std::condition_variable actions_available_;
    std::deque<std::function<void()>> actions_;
    bool thread_running_ = false;
    std::jthread thread_;
};
