#include "game.h"

#ifndef NDEBUG
#include "debug_console.h"
#endif
#include "game_constants.h"

#include <stdexcept>
#include <utility>

namespace
{
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
game_t::game_t(const std::string& creature_types_json_p)
    : creature_type_registry_(creature_types_json_p)
{
}

game_t::~game_t()
{
    stop();
}

/*! Starts the game thread and reports state changes to \a observer_p. */
void game_t::start(igame_observer_t& observer_p)
{
    {
        std::lock_guard<std::mutex> lock(actions_mutex_);

        if (thread_running_) {
            throw std::logic_error("Game is already running.");
        }

        observer_ = &observer_p;
        thread_running_ = true;
    }

    try {
        thread_ = std::jthread([this](const std::stop_token& stop_token_p) {
            run(stop_token_p);
        });
    } catch (...) {
        std::lock_guard<std::mutex> lock(actions_mutex_);
        observer_ = nullptr;
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
        observer_ = nullptr;
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
    publish_creatures();

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

/*! Creates a creature of the requested type at a map position. */
void game_t::spawn_creature(
    std::string identifier_p,
    position_t position_p
)
{
    if (std::this_thread::get_id() != thread_.get_id()) {
        post([this, identifier = std::move(identifier_p), position_p]() mutable {
            spawn_creature(std::move(identifier), position_p);
        });
        return;
    }

    if (!game_map_.can_place_creature(position_p)) {
        return;
    }

    const auto& creature_type = creature_type_registry_.get(identifier_p);
    auto& creature = creatures_.create(creature_type, position_p);

    if (!game_map_.place_creature(creature, position_p)) {
        throw std::logic_error("Could not place a creature at the requested position.");
    }

    observer_->on_creature_created(creature);
}

void game_t::dispatch_tick()
{
#ifndef NDEBUG
    const auto tick_start = std::chrono::steady_clock::now();
#endif

    creatures_.on_think(*this);

    observer_->on_game_tick();

#ifndef NDEBUG
    update_debug_monitor(tick_start);
#endif
}

#ifndef NDEBUG
void game_t::update_debug_monitor(
    std::chrono::steady_clock::time_point tick_start_p
) const
{
    debug_console::update_monitor({
        .tick_duration_ = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - tick_start_p
        ),
        .creature_count_ = creatures_.size(),
        .occupied_position_count_ = game_map_.occupied_position_count(),
        .position_count_ = game_map_t::position_count,
        .pending_event_count_ = dispatcher_.pending_event_count(),
        .peak_pending_event_count_ = dispatcher_.peak_pending_event_count(),
        .dispatched_event_count_ = dispatcher_.dispatched_event_count()
    });
}
#endif

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

    const auto previous_position = creature->position();
    const auto destination = destination_position(previous_position, direction_p);

    if (!game_map_.move_creature(*creature, destination)) {
        return;
    }

    observer_->on_creature_moved(
        creature->id(),
        previous_position,
        creature->position()
    );
}

void game_t::publish_creatures()
{
    creatures_.publish_creatures([this](const creature_t& creature_p) {
        observer_->on_creature_created(creature_p);
    });
}
