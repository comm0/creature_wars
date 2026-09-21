#include "game.h"

#include "game_constants.h"

#include <random>
#include <stdexcept>
#include <utility>

namespace
{
std::mt19937 random_generator{std::random_device{}()};

position_t destination_position(position_t position_p, direction_t direction_p)
{
    switch (direction_p) {
    case direction_t::north:
        --position_p.row_;
        break;
    case direction_t::east:
        ++position_p.column_;
        break;
    case direction_t::south:
        ++position_p.row_;
        break;
    case direction_t::west:
        --position_p.column_;
        break;
    }

    return position_p;
}

}

/*!
    \class game_t
    \inmodule CreatureWars
    \brief Owns the game thread and serializes all game actions.
*/

/*! Creates a stopped game. */
game_t::game_t()
{

}

game_t::~game_t()
{
    stop();
}

/*! Starts the game thread with \a heartbeat_handler_p and \a creature_position_handler_p. */
void game_t::start(
    std::function<void()> heartbeat_handler_p,
    std::function<void(std::uint64_t, position_t)> creature_position_handler_p
)
{
    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (thread_running_) {
            throw std::logic_error("Game is already running.");
        }

        heartbeat_handler_ = std::move(heartbeat_handler_p);
        creature_position_handler_ = std::move(creature_position_handler_p);
        thread_running_ = true;
    }

    try {
        thread_ = std::jthread([this](const std::stop_token& stop_token_p) {
            run(stop_token_p);
        });
    } catch (...) {
        std::lock_guard<std::mutex> lock(actions_mutex_);
        thread_running_ = false;
        throw;
    }
}

/*! Stops the game thread after its current action. */
void game_t::stop()
{
    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (!thread_running_) {
            return;
        }

        thread_running_ = false;
    }

    thread_.request_stop();
    actions_available_.notify_one();
    thread_.join();

    {
        std::lock_guard<std::mutex> lock(actions_mutex_);
        actions_.clear();
    }
}

/*! Queues \a event_p for the game thread. */
void game_t::post(std::function<void()> event_p)
{
    if (!event_p) {
        throw std::invalid_argument("Game event must be callable.");
    }

    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (!thread_running_) {
            throw std::logic_error("Game is not running.");
        }

        actions_.push_back(std::move(event_p));
    }

    actions_available_.notify_one();
}

void game_t::run(const std::stop_token& stop_token_p)
{
    auto next_tick = std::chrono::steady_clock::now() + game_constants::tick_interval;
    publish_creature_positions();

    while (!stop_token_p.stop_requested()) {
        {
            std::unique_lock lock(actions_mutex_);
            actions_available_.wait_until(lock, next_tick, [this, &stop_token_p]() {
                return stop_token_p.stop_requested() || !actions_.empty();
            });
        }

        if (stop_token_p.stop_requested()) {
            break;
        }

        collect_actions();

        const auto now = std::chrono::steady_clock::now();

        if (now >= next_tick) {
            dispatcher_.enqueue([this]() { dispatch_tick(); });
            next_tick = now + game_constants::tick_interval;
        }

        dispatcher_.dispatch_pending();
    }
}

void game_t::collect_actions()
{
    std::deque<std::function<void()>> collected_actions;

    {
        std::lock_guard<std::mutex> lock(actions_mutex_);
        collected_actions.swap(actions_);
    }

    while (!collected_actions.empty()) {
        dispatcher_.enqueue(std::move(collected_actions.front()));
        collected_actions.pop_front();
    }
}

/*! Creates a creature at a random map position. */
void game_t::spawn_creature()
{
    if (std::this_thread::get_id() != thread_.get_id()) {
        post([this]() { spawn_creature(); });
        return;
    }

    auto position_distribution = std::uniform_int_distribution<std::size_t>(
        0,
        game_map_t::position_count - 1
    );
    const auto position = game_map_.find_free_position(
        position_distribution(random_generator)
    );

    if (!position.has_value()) {
        return;
    }

    auto& creature = creatures_.create(*position);

    if (!game_map_.place_creature(creature, *position)) {
        throw std::logic_error("Could not place a creature on a free position.");
    }

    if (creature_position_handler_) {
        creature_position_handler_(creature.id(), creature.position());
    }
}

void game_t::dispatch_tick()
{
    creatures_.on_think(*this);

    if (heartbeat_handler_) {
        heartbeat_handler_();
    }
}

void game_t::request_move(std::uint64_t id_p, direction_t direction_p)
{
    if (std::this_thread::get_id() != thread_.get_id()) {
        post([this, id_p, direction_p]() {
            request_move(id_p, direction_p);
        });
        return;
    }

    auto* creature = creatures_.find(id_p);

    if (creature == nullptr) {
        return;
    }

    const auto destination = destination_position(creature->position(), direction_p);

    if (!game_map_.move_creature(*creature, destination)) {
        return;
    }

    if (creature_position_handler_) {
        creature_position_handler_(creature->id(), creature->position());
    }
}

void game_t::publish_creature_positions()
{
    if (!creature_position_handler_) {
        return;
    }

    creatures_.publish_positions(creature_position_handler_);
}
