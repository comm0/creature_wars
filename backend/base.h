#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "base_type.h"
#include "creature.h"

class game_t;

class base_t
{
public:
    base_t(
        std::uint64_t id_p,
        const base_type_t& type_p,
        const creature_type_t& spawn_type_p,
        position_t position_p,
        int level_p
    );

    void on_attacking(game_t& game_p, std::chrono::milliseconds interval_p);

    std::uint64_t id() const noexcept
    {
        return id_;
    }

    const base_type_t& type() const noexcept
    {
        return type_;
    }

    const creature_type_t& spawn_type() const noexcept
    {
        return spawn_type_;
    }

    position_t position() const noexcept
    {
        return position_;
    }

    int health() const noexcept
    {
        return health_;
    }

    int level() const noexcept
    {
        return level_;
    }

    bool is_dead() const noexcept
    {
        return health_ == 0;
    }

    void drain_health(int damage_p) noexcept;
    bool contains(position_t position_p) const noexcept;
    position_t closest_position_to(position_t position_p) const noexcept;
    int distance_to(position_t position_p) const noexcept;
    int distance_to(const base_t& base_p) const noexcept;

private:
    int last_column() const noexcept
    {
        return position_.column_ + type_.size() - 1;
    }

    int last_row() const noexcept
    {
        return position_.row_ + type_.size() - 1;
    }

    std::uint64_t id_;
    const base_type_t& type_;
    const creature_type_t& spawn_type_;
    position_t position_;
    std::optional<std::uint64_t> target_id_;
    std::chrono::milliseconds attack_elapsed_{0};
    int health_;
    int level_;
};
