#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "base_type.h"
#include "creature.h"

class game_t;

struct base_stats_t
{
    int max_health_;
    int attack_;
    int range_;
    int shots_ = 1;
    int regeneration_ = 0;
    int healing_aura_ = 0;
};

class base_t
{
public:
    base_t(std::uint64_t id_p, const base_type_t& type_p, position_t position_p);

    void on_attacking(game_t& game_p, std::chrono::milliseconds interval_p);

    std::uint64_t id() const noexcept
    {
        return id_;
    }

    const base_type_t& type() const noexcept
    {
        return type_;
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

    const base_stats_t& stats() const noexcept
    {
        return stats_;
    }

    bool is_dead() const noexcept
    {
        return health_ == 0;
    }

    void set_level(int level_p) noexcept
    {
        level_ = level_p;
    }

    void apply_stats(const base_stats_t& stats_p) noexcept;
    void drain_health(int damage_p) noexcept;
    bool regenerate() noexcept;
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
    position_t position_;
    std::optional<std::uint64_t> target_id_;
    std::chrono::milliseconds attack_elapsed_{0};
    base_stats_t stats_;
    int health_;
    int level_ = 1;
};
