#include "game.h"

#ifndef NDEBUG
#include "debug_console.h"
#endif
#include "game_constants.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
int position_distance(position_t left_p, position_t right_p) noexcept
{
    return std::max(
        std::abs(left_p.column_ - right_p.column_),
        std::abs(left_p.row_ - right_p.row_)
    );
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
        auto next_wake_time = next_tick;
        const auto next_movement_time = creatures_.next_movement_time();

        if (next_movement_time.has_value()
            && *next_movement_time < next_wake_time) {
            next_wake_time = *next_movement_time;
        }

        {
            std::unique_lock lock(actions_mutex_);
            actions_available_.wait_until(
                lock,
                next_wake_time,
                [this, &stop_token_p]() {
                    return stop_token_p.stop_requested() || !actions_.empty();
                }
            );
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

        const auto movement_time = std::chrono::steady_clock::now();
        const auto next_due_movement = creatures_.next_movement_time();

        if (next_due_movement.has_value()
            && *next_due_movement <= movement_time) {
            dispatcher_.enqueue([this, movement_time]() {
                dispatch_movement(movement_time);
            });
            dispatcher_.dispatch_pending();
        }
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

void game_t::request_spawn_creature(
    std::string identifier_p,
    position_t position_p
)
{
    post([this, identifier = std::move(identifier_p), position_p]() mutable {
        spawn_creature(std::move(identifier), position_p);
    });
}

/*! Creates a creature of the requested type at a map position. */
void game_t::spawn_creature(
    std::string identifier_p,
    position_t position_p
)
{
    if (!game_map_.can_place_creature(position_p)) {
        return;
    }

    const auto& creature_type = creature_type_registry_.get(identifier_p);
    auto& creature = creatures_.create(creature_type, position_p);

    if (!game_map_.place_creature(creature, position_p)) {
        throw std::logic_error("Could not place a creature at the requested position.");
    }

    observer_->on_creature_created(creature);
    visibility_system_.add_creature(
        creature,
        game_map_,
        creatures_,
        [this](std::uint64_t observer_id_p, std::uint64_t spotted_id_p) {
            observer_->on_creature_spotted(observer_id_p, spotted_id_p);
        }
    );
}

void game_t::dispatch_tick()
{
#ifndef NDEBUG
    const auto tick_start = std::chrono::steady_clock::now();
#endif

    creatures_.on_think(*this);
    creatures_.on_attacking(*this, game_constants::tick_interval);
    remove_dead_creatures();

    observer_->on_game_tick();

#ifndef NDEBUG
    update_debug_monitor(tick_start);
#endif
}

void game_t::dispatch_movement(
    std::chrono::steady_clock::time_point now_p
)
{
    creatures_.update_movement(*this, now_p);
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

void game_t::request_walk_to(
    std::uint64_t id_p,
    position_t destination_p
)
{
    post([this, id_p, destination_p]() {
        set_creature_destination(id_p, destination_p);
    });
}

void game_t::request_set_aggressive(bool aggressive_p)
{
    post([this, aggressive_p]() {
        set_aggressive(aggressive_p);
    });
}

void game_t::set_creature_destination(
    std::uint64_t id_p,
    position_t destination_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature != nullptr) {
        creature->request_walk(destination_p);
        creature->on_think(*this);
    }
}

void game_t::set_aggressive(bool aggressive_p)
{
    if (aggressive_ == aggressive_p) {
        return;
    }

    aggressive_ = aggressive_p;
    creatures_.on_think(*this);
}

const creature_t* game_t::find_nearest_visible_enemy(
    const creature_t& creature_p
) const noexcept
{
    const creature_t* nearest_enemy = nullptr;
    auto nearest_distance = std::numeric_limits<int>::max();

    for (const auto visible_id : creature_p.visible_creature_ids()) {
        const auto* visible_creature = creatures_.find(visible_id);

        if (visible_creature == nullptr
            || visible_creature->is_dead()
            || visible_creature->type().group() == creature_p.type().group()) {
            continue;
        }

        const auto distance = position_distance(
            creature_p.position(),
            visible_creature->position()
        );

        if (distance < nearest_distance) {
            nearest_enemy = visible_creature;
            nearest_distance = distance;
        }
    }

    return nearest_enemy;
}

const creature_t* game_t::find_visible_enemy(
    const creature_t& creature_p,
    std::uint64_t target_id_p
) const noexcept
{
    if (!creature_p.visible_creature_ids().contains(target_id_p)) {
        return nullptr;
    }

    const auto* target = creatures_.find(target_id_p);

    if (target == nullptr
        || target->is_dead()
        || target->type().group() == creature_p.type().group()) {
        return nullptr;
    }

    return target;
}

bool game_t::is_in_attack_range(
    const creature_t& creature_p,
    const creature_t& target_p
) const noexcept
{
    return position_distance(creature_p.position(), target_p.position())
        <= creature_p.type().attack_range();
}

bool game_t::move_creature_towards(
    std::uint64_t id_p,
    position_t destination_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature == nullptr) {
        return false;
    }

    const auto next_position = game_map_.next_step_towards(
        *creature,
        destination_p
    );

    if (!next_position.has_value()) {
        return false;
    }

    const auto previous_position = creature->position();

    if (!game_map_.move_creature(*creature, *next_position)) {
        return true;
    }

    observer_->on_creature_moved(
        creature->id(),
        previous_position,
        creature->position()
    );
    visibility_system_.move_creature(
        *creature,
        previous_position,
        game_map_,
        creatures_,
        [this](std::uint64_t observer_id_p, std::uint64_t spotted_id_p) {
            observer_->on_creature_spotted(observer_id_p, spotted_id_p);
        }
    );

    return true;
}

bool game_t::check_creature_attack(
    std::uint64_t attacker_id_p,
    std::uint64_t target_id_p
)
{
    auto* attacker = creatures_.find(attacker_id_p);
    auto* target = creatures_.find(target_id_p);

    if (attacker == nullptr
        || target == nullptr
        || attacker == target
        || attacker->is_dead()
        || target->is_dead()
        || attacker->type().group() == target->type().group()
        || !attacker->visible_creature_ids().contains(target_id_p)
        || !is_in_attack_range(*attacker, *target)) {
        return false;
    }

    const auto previous_health = target->health();
    target->drain_health(attacker->type().attack());

    if (target->health() != previous_health) {
        observer_->on_creature_health_changed(target->id(), target->health());
    }

    if (target->is_dead()) {
        dead_creature_ids_.push_back(target->id());
    }

    return true;
}

void game_t::remove_dead_creatures()
{
    for (const auto id : dead_creature_ids_) {
        auto* creature = creatures_.find(id);

        if (creature == nullptr || !creature->is_dead()) {
            continue;
        }

        if (!game_map_.remove_creature(*creature)) {
            continue;
        }

        visibility_system_.remove_creature(*creature, creatures_);
        observer_->on_creature_removed(id);
        creatures_.remove(id);
    }

    dead_creature_ids_.clear();
}

void game_t::notify_creature_state_changed(const creature_t& creature_p)
{
    observer_->on_creature_state_changed(
        creature_p.id(),
        creature_p.state()
    );
}

void game_t::publish_creatures()
{
    creatures_.publish_creatures([this](const creature_t& creature_p) {
        observer_->on_creature_created(creature_p);
    });
}
