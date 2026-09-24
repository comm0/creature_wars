#pragma once

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <unordered_set>

#include "creature_type.h"

struct position_t
{
    int column_ = 0;
    int row_ = 0;

    bool operator==(const position_t&) const = default;
};

struct target_t
{
    std::uint64_t id_ = 0;
    position_t position_;
};

enum class creature_state_t
{
    idle,
    walking
};

enum class direction_t
{
    north,
    east,
    south,
    west
};

class game_t;

class creature_t
{
public:
    creature_t(
        std::uint64_t id_p,
        const creature_type_t& type_p,
        position_t position_p
    );

    void on_think(game_t& game_p);
    void on_attacking(
        game_t& game_p,
        std::chrono::milliseconds interval_p
    );
    void update_movement(
        game_t& game_p,
        std::chrono::steady_clock::time_point now_p
    );

    void request_walk(position_t destination_p) noexcept
    {
        const auto destination_changed = !manual_destination_.has_value()
            || *manual_destination_ != destination_p;
        manual_destination_ = destination_p;

        if (active_movement_goal_ == movement_goal_t::manual
            && destination_changed) {
            next_movement_time_ = std::chrono::steady_clock::now();
            blocked_path_retry_count_ = 0;
        }
    }

    void move_to(position_t position_p) noexcept
    {
        const auto column_change = position_p.column_ - position_.column_;
        const auto row_change = position_p.row_ - position_.row_;

        if (std::abs(column_change) >= std::abs(row_change)
            && column_change != 0) {
            direction_ = column_change > 0
                ? direction_t::east
                : direction_t::west;
        } else if (row_change != 0) {
            direction_ = row_change > 0
                ? direction_t::south
                : direction_t::north;
        }

        position_ = position_p;
    }

    std::uint64_t id() const noexcept
    {
        return id_;
    }

    position_t position() const noexcept
    {
        return position_;
    }

    direction_t direction() const noexcept
    {
        return direction_;
    }

    const creature_type_t& type() const noexcept
    {
        return type_;
    }

    int health() const noexcept
    {
        return health_;
    }

    bool is_dead() const noexcept
    {
        return health_ == 0;
    }

    void drain_health(int damage_p) noexcept;

    creature_state_t state() const noexcept
    {
        return active_movement_goal_ == movement_goal_t::manual
                || active_movement_goal_ == movement_goal_t::target
            ? creature_state_t::walking
            : creature_state_t::idle;
    }

    std::optional<std::chrono::steady_clock::time_point> next_movement_time()
        const noexcept
    {
        return next_movement_time_;
    }

    bool add_visible_creature(std::uint64_t id_p)
    {
        if (blocked_target_id_ == id_p) {
            blocked_target_id_.reset();
            blocked_target_position_.reset();
        }

        return visible_creature_ids_.insert(id_p).second;
    }

    void remove_visible_creature(std::uint64_t id_p)
    {
        visible_creature_ids_.erase(id_p);
    }

    const std::unordered_set<std::uint64_t>& visible_creature_ids() const noexcept
    {
        return visible_creature_ids_;
    }

private:
    enum class movement_goal_t
    {
        none,
        manual,
        target,
        idle
    };

    void activate_manual_movement(game_t& game_p);
    void activate_target_movement(game_t& game_p, const target_t& target_p);
    void activate_idle_movement(game_t& game_p);
    void stop_movement(game_t& game_p);
    void finish_manual_movement(game_t& game_p);
    void block_target(position_t target_position_p, game_t& game_p);
    void clear_target(game_t& game_p);
    bool target_was_blocked(const target_t& target_p) const noexcept;
    std::chrono::steady_clock::duration movement_interval() const;

    std::uint64_t id_;
    const creature_type_t& type_;
    position_t position_;
    std::optional<position_t> manual_destination_;
    std::optional<position_t> idle_starting_position_;
    std::optional<std::uint64_t> target_id_;
    std::optional<std::uint64_t> blocked_target_id_;
    std::optional<position_t> blocked_target_position_;
    std::optional<std::chrono::steady_clock::time_point> next_movement_time_;
    std::unordered_set<std::uint64_t> visible_creature_ids_;
    movement_goal_t active_movement_goal_ = movement_goal_t::none;
    direction_t direction_ = direction_t::south;
    std::chrono::milliseconds attack_elapsed_{0};
    int blocked_path_retry_count_ = 0;
    int health_;
};
