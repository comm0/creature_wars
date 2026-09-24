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
game_t::game_t(
    const std::string& creature_types_json_p,
    const std::string& base_types_json_p
)
    : creature_type_registry_(creature_types_json_p)
    , base_type_registry_(base_types_json_p)
{
    for (const auto& base_type : base_type_registry_.all()) {
        static_cast<void>(creature_type_registry_.get(base_type.spawn_creature()));
    }
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

void game_t::request_spawn_base(std::string identifier_p, position_t center_p)
{
    post([this, identifier = std::move(identifier_p), center_p]() mutable {
        spawn_base(std::move(identifier), center_p);
    });
}

void game_t::request_spawn_from_base(std::uint64_t base_id_p)
{
    post([this, base_id_p]() {
        spawn_from_base(base_id_p);
    });
}

void game_t::spawn_base(std::string identifier_p, position_t center_p)
{
    const auto& base_type = base_type_registry_.get(identifier_p);
    const auto group_has_base = std::any_of(
        bases_.begin(),
        bases_.end(),
        [&base_type](const auto& base_p) {
            return base_p->type().group() == base_type.group();
        }
    );

    if (group_has_base) {
        return;
    }

    const auto half_size = base_type.size() / 2;
    const position_t position{
        center_p.column_ - half_size,
        center_p.row_ - half_size
    };

    if (!game_map_.can_place_base(position, base_type.size())) {
        return;
    }

    auto& base = *bases_.emplace_back(std::make_unique<base_t>(
        creatures_t::allocate_id(),
        base_type,
        creature_type_registry_.get(base_type.spawn_creature()),
        position
    ));
    game_map_.place_base(base);
    observer_->on_base_created(base);
}

void game_t::spawn_from_base(std::uint64_t base_id_p)
{
    const auto* base = find_base(base_id_p);

    if (base == nullptr || base->is_dead()) {
        return;
    }

    const auto position = game_map_.free_position_around(*base);

    if (position.has_value()) {
        spawn_creature(base->spawn_type().identifier(), *position);
    }
}

base_t* game_t::find_base(std::uint64_t id_p) noexcept
{
    const auto base = std::find_if(
        bases_.begin(),
        bases_.end(),
        [id_p](const auto& base_p) {
            return base_p->id() == id_p;
        }
    );

    return base == bases_.end() ? nullptr : base->get();
}

const base_t* game_t::find_base(std::uint64_t id_p) const noexcept
{
    const auto base = std::find_if(
        bases_.begin(),
        bases_.end(),
        [id_p](const auto& base_p) {
            return base_p->id() == id_p;
        }
    );

    return base == bases_.end() ? nullptr : base->get();
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
            notify_creature_spotted(observer_id_p, spotted_id_p);
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

    for (const auto& base : bases_) {
        base->on_attacking(*this, game_constants::tick_interval);
    }

    remove_dead_creatures();
    remove_dead_bases();

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
        observer_->on_creature_walk_requested(id_p);
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

std::optional<target_t> game_t::find_nearest_visible_enemy(
    const creature_t& creature_p
) const noexcept
{
    std::optional<target_t> nearest_enemy;
    auto nearest_distance = std::numeric_limits<int>::max();
    const auto consider = [&](std::uint64_t id_p) {
        const auto enemy = find_visible_enemy(creature_p, id_p);

        if (!enemy.has_value()) {
            return;
        }

        const auto distance = position_distance(
            creature_p.position(),
            enemy->position_
        );

        if (distance < nearest_distance) {
            nearest_enemy = enemy;
            nearest_distance = distance;
        }
    };

    for (const auto visible_id : creature_p.visible_creature_ids()) {
        consider(visible_id);
    }

    for (const auto& base : bases_) {
        consider(base->id());
    }

    return nearest_enemy;
}

std::optional<target_t> game_t::find_visible_enemy(
    const creature_t& creature_p,
    std::uint64_t target_id_p
) const noexcept
{
    const auto& group = creature_p.type().group();

    if (creature_p.visible_creature_ids().contains(target_id_p)) {
        const auto* target = creatures_.find(target_id_p);

        if (target == nullptr
            || target->is_dead()
            || target->type().group() == group) {
            return std::nullopt;
        }

        return target_t{target->id(), target->position()};
    }

    const auto* base = find_base(target_id_p);

    if (base == nullptr
        || base->is_dead()
        || base->type().group() == group
        || base->distance_to(creature_p.position())
            > creature_p.type().vision_range()) {
        return std::nullopt;
    }

    return target_t{base->id(), base->closest_position_to(creature_p.position())};
}

bool game_t::is_in_attack_range(
    const creature_t& creature_p,
    const target_t& target_p
) const noexcept
{
    return position_distance(creature_p.position(), target_p.position_)
        <= creature_p.type().attack_range();
}

std::optional<target_t> game_t::find_base_target(
    const base_t& base_p,
    std::uint64_t target_id_p
) const noexcept
{
    const auto& group = base_p.type().group();
    const auto range = base_p.type().attack_range();

    if (const auto* creature = creatures_.find(target_id_p); creature != nullptr) {
        if (creature->is_dead()
            || creature->type().group() == group
            || base_p.distance_to(creature->position()) > range) {
            return std::nullopt;
        }

        return target_t{creature->id(), creature->position()};
    }

    const auto* base = find_base(target_id_p);

    if (base == nullptr
        || base->is_dead()
        || base->type().group() == group
        || base_p.distance_to(*base) > range) {
        return std::nullopt;
    }

    return target_t{
        base->id(),
        base->closest_position_to(base_p.closest_position_to(base->position()))
    };
}

std::optional<target_t> game_t::find_nearest_base_target(
    const base_t& base_p
) const noexcept
{
    std::optional<target_t> nearest_target;
    auto nearest_distance = std::numeric_limits<int>::max();
    const auto consider = [&](std::uint64_t id_p) {
        const auto target = find_base_target(base_p, id_p);

        if (!target.has_value()) {
            return;
        }

        const auto distance = base_p.distance_to(target->position_);

        if (distance < nearest_distance) {
            nearest_target = target;
            nearest_distance = distance;
        }
    };

    creatures_.for_each([&consider](const creature_t& creature_p) {
        consider(creature_p.id());
    });

    for (const auto& base : bases_) {
        consider(base->id());
    }

    return nearest_target;
}

void game_t::perform_base_attack(const base_t& base_p, const target_t& target_p)
{
    observer_->on_base_attack_performed(base_p.id(), target_p.position_);
    damage_target(target_p.id_, base_p.type().attack());
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

    return move_creature(*creature, *next_position);
}

bool game_t::move_creature_idle(
    std::uint64_t id_p,
    position_t idle_starting_position_p
)
{
    auto* creature = creatures_.find(id_p);

    if (creature == nullptr) {
        return false;
    }

    const auto next_position = game_map_.next_idle_step(
        *creature,
        idle_starting_position_p
    );

    if (!next_position.has_value()) {
        return false;
    }

    return move_creature(*creature, *next_position);
}

bool game_t::move_creature(
    creature_t& creature_p,
    position_t position_p
)
{
    const auto previous_position = creature_p.position();

    if (!game_map_.move_creature(creature_p, position_p)) {
        return true;
    }

    observer_->on_creature_moved(
        creature_p.id(),
        creature_p.position(),
        creature_p.direction()
    );
    visibility_system_.move_creature(
        creature_p,
        previous_position,
        game_map_,
        creatures_,
        [this](std::uint64_t observer_id_p, std::uint64_t spotted_id_p) {
            notify_creature_spotted(observer_id_p, spotted_id_p);
        }
    );

    return true;
}

void game_t::notify_creature_spotted(
    std::uint64_t observer_id_p,
    std::uint64_t spotted_id_p
)
{
    const auto* observing_creature = creatures_.find(observer_id_p);
    const auto* spotted_creature = creatures_.find(spotted_id_p);

    if (observing_creature == nullptr
        || spotted_creature == nullptr
        || observing_creature->type().group()
            == spotted_creature->type().group()) {
        return;
    }

    observer_->on_creature_spotted(observer_id_p, spotted_id_p);
}

bool game_t::check_creature_attack(
    std::uint64_t attacker_id_p,
    std::uint64_t target_id_p
)
{
    const auto* attacker = creatures_.find(attacker_id_p);

    if (attacker == nullptr || attacker->is_dead()) {
        return false;
    }

    const auto target = find_visible_enemy(*attacker, target_id_p);

    if (!target.has_value() || !is_in_attack_range(*attacker, *target)) {
        return false;
    }

    observer_->on_creature_attack_performed(attacker->id());
    damage_target(target_id_p, attacker->type().attack());
    return true;
}

void game_t::damage_target(std::uint64_t target_id_p, int damage_p)
{
    if (auto* creature = creatures_.find(target_id_p); creature != nullptr) {
        const auto previous_health = creature->health();
        creature->drain_health(damage_p);

        if (creature->health() != previous_health) {
            observer_->on_creature_health_changed(creature->id(), creature->health());
        }

        if (creature->is_dead()) {
            dead_creature_ids_.push_back(creature->id());
        }

        return;
    }

    if (auto* base = find_base(target_id_p); base != nullptr) {
        const auto previous_health = base->health();
        base->drain_health(damage_p);

        if (base->health() != previous_health) {
            observer_->on_base_health_changed(base->id(), base->health());
        }
    }
}

void game_t::remove_dead_bases()
{
    std::erase_if(bases_, [this](const auto& base_p) {
        if (!base_p->is_dead()) {
            return false;
        }

        game_map_.remove_base(*base_p);
        observer_->on_base_removed(base_p->id());
        return true;
    });
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
    creatures_.for_each([this](const creature_t& creature_p) {
        observer_->on_creature_created(creature_p);
    });

    for (const auto& base : bases_) {
        observer_->on_base_created(*base);
    }
}
