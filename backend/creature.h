#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "creature_type.h"

struct position_t
{
    int column_ = 0;
    int row_ = 0;

    bool operator==(const position_t&) const = default;
};

enum class creature_state_t
{
    idle,
    walking
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
    void update_movement(
        game_t& game_p,
        std::chrono::steady_clock::time_point now_p
    );

    bool walk_to(position_t destination_p) noexcept
    {
        const auto state_changed = !destination_.has_value();
        destination_ = destination_p;
        next_movement_time_ = std::chrono::steady_clock::now();
        blocked_path_retry_count_ = 0;
        return state_changed;
    }

    void move_to(position_t position_p) noexcept
    {
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

    const creature_type_t& type() const noexcept
    {
        return type_;
    }

    int health() const noexcept
    {
        return health_;
    }

    creature_state_t state() const noexcept
    {
        return destination_.has_value()
            ? creature_state_t::walking
            : creature_state_t::idle;
    }

    std::optional<std::chrono::steady_clock::time_point> next_movement_time()
        const noexcept
    {
        return next_movement_time_;
    }

private:
    void finish_walking(game_t& game_p);
    std::chrono::steady_clock::duration movement_interval() const;

    std::uint64_t id_;
    const creature_type_t& type_;
    position_t position_;
    std::optional<position_t> destination_;
    std::optional<std::chrono::steady_clock::time_point> next_movement_time_;
    int blocked_path_retry_count_ = 0;
    int health_;
};
