#include "creature.h"

#include "game.h"
#include "game_constants.h"

#include <algorithm>
#include <chrono>

namespace
{
constexpr int manual_path_retry_count = 10;
constexpr int target_path_retry_count = 2;
}

creature_t::creature_t(
    std::uint64_t id_p,
    const creature_type_t& type_p,
    position_t position_p
)
    : id_(id_p)
    , type_(type_p)
    , position_(position_p)
    , health_(type_p.health())
{
}

void creature_t::on_think(game_t& game_p)
{
    if (is_dead()) {
        return;
    }

    const auto* enemy = target_id_.has_value()
        ? game_p.find_visible_enemy(*this, *target_id_)
        : nullptr;

    if (enemy == nullptr) {
        clear_target(game_p);
    }

    if (!game_p.aggressive() && manual_destination_.has_value()) {
        activate_manual_movement(game_p);
        return;
    }

    if (enemy == nullptr) {
        enemy = game_p.find_nearest_visible_enemy(*this);

        if (enemy != nullptr) {
            target_id_ = enemy->id();
        }
    }

    if (enemy == nullptr) {
        if (manual_destination_.has_value()) {
            activate_manual_movement(game_p);
        } else {
            stop_movement(game_p);
        }

        return;
    }

    if (game_p.is_in_attack_range(*this, *enemy)
        || target_was_blocked(*enemy)) {
        stop_movement(game_p);
        return;
    }

    activate_target_movement(game_p, *enemy);
}

void creature_t::on_attacking(
    game_t& game_p,
    std::chrono::milliseconds interval_p
)
{
    if (is_dead() || !target_id_.has_value()) {
        attack_elapsed_ = std::chrono::milliseconds{0};
        return;
    }

    const auto* target = game_p.find_visible_enemy(*this, *target_id_);

    if (target == nullptr) {
        clear_target(game_p);
        return;
    }

    attack_elapsed_ += interval_p;

    if (attack_elapsed_ < game_constants::attack_interval
        || !game_p.is_in_attack_range(*this, *target)) {
        return;
    }

    if (game_p.check_creature_attack(id_, target->id())) {
        attack_elapsed_ = std::chrono::milliseconds{0};
    }
}

void creature_t::drain_health(int damage_p) noexcept
{
    health_ = std::max(0, health_ - std::max(0, damage_p));
}

void creature_t::update_movement(
    game_t& game_p,
    std::chrono::steady_clock::time_point now_p
)
{
    if (active_movement_goal_ == movement_goal_t::none
        || !next_movement_time_.has_value()
        || now_p < *next_movement_time_) {
        return;
    }

    std::optional<position_t> destination;
    auto retry_count = manual_path_retry_count;

    if (active_movement_goal_ == movement_goal_t::manual) {
        destination = manual_destination_;

        if (!destination.has_value() || position_ == *destination) {
            finish_manual_movement(game_p);
            return;
        }
    } else {
        const auto* target = target_id_.has_value()
            ? game_p.find_visible_enemy(*this, *target_id_)
            : nullptr;

        if (target == nullptr || game_p.is_in_attack_range(*this, *target)) {
            stop_movement(game_p);
            return;
        }

        destination = target->position();
        retry_count = target_path_retry_count;
    }

    if (game_p.move_creature_towards(id_, *destination)) {
        blocked_path_retry_count_ = 0;
        next_movement_time_ = now_p + movement_interval();
        return;
    }

    if (blocked_path_retry_count_ < retry_count) {
        ++blocked_path_retry_count_;
        next_movement_time_ = now_p + game_constants::tick_interval;
        return;
    }

    if (active_movement_goal_ == movement_goal_t::target) {
        block_target(*destination, game_p);
        return;
    }

    finish_manual_movement(game_p);
}

void creature_t::activate_manual_movement(game_t& game_p)
{
    if (active_movement_goal_ == movement_goal_t::manual) {
        return;
    }

    const auto previous_state = state();
    active_movement_goal_ = movement_goal_t::manual;
    next_movement_time_ = std::chrono::steady_clock::now();
    blocked_path_retry_count_ = 0;

    if (previous_state != state()) {
        game_p.notify_creature_state_changed(*this);
    }
}

void creature_t::activate_target_movement(
    game_t& game_p,
    const creature_t& target_p
)
{
    if (active_movement_goal_ == movement_goal_t::target
        && target_id_ == target_p.id()) {
        return;
    }

    const auto previous_state = state();
    active_movement_goal_ = movement_goal_t::target;
    target_id_ = target_p.id();
    blocked_target_id_.reset();
    blocked_target_position_.reset();
    next_movement_time_ = std::chrono::steady_clock::now();
    blocked_path_retry_count_ = 0;

    if (previous_state != state()) {
        game_p.notify_creature_state_changed(*this);
    }
}

void creature_t::stop_movement(game_t& game_p)
{
    if (active_movement_goal_ == movement_goal_t::none) {
        return;
    }

    active_movement_goal_ = movement_goal_t::none;
    next_movement_time_.reset();
    blocked_path_retry_count_ = 0;
    game_p.notify_creature_state_changed(*this);
}

void creature_t::finish_manual_movement(game_t& game_p)
{
    manual_destination_.reset();
    stop_movement(game_p);
}

void creature_t::block_target(position_t target_position_p, game_t& game_p)
{
    blocked_target_id_ = target_id_;
    blocked_target_position_ = target_position_p;
    stop_movement(game_p);
}

void creature_t::clear_target(game_t& game_p)
{
    target_id_.reset();
    attack_elapsed_ = std::chrono::milliseconds{0};

    if (active_movement_goal_ == movement_goal_t::target) {
        stop_movement(game_p);
    }
}

bool creature_t::target_was_blocked(const creature_t& target_p) const noexcept
{
    return blocked_target_id_ == target_p.id()
        && blocked_target_position_ == target_p.position();
}

std::chrono::steady_clock::duration creature_t::movement_interval() const
{
    const auto interval = std::chrono::duration_cast<
        std::chrono::steady_clock::duration
    >(
        std::chrono::duration<double>(1.0 / type_.speed())
    );

    if (interval < std::chrono::milliseconds{1}) {
        return std::chrono::milliseconds{1};
    }

    return interval;
}
