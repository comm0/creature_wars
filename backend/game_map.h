#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <random>

#include "base.h"
#include "creature.h"
#include "game_constants.h"
#include "pathfinder.h"

class game_map_t
{
public:
    static constexpr std::size_t position_count =
        static_cast<std::size_t>(game_constants::map_column_count)
        * static_cast<std::size_t>(game_constants::map_row_count);

    bool place_creature(creature_t& creature_p, position_t position_p) noexcept;
    bool move_creature(creature_t& creature_p, position_t position_p) noexcept;
    bool remove_creature(creature_t& creature_p) noexcept;
    bool place_base(base_t& base_p) noexcept;
    void remove_base(const base_t& base_p) noexcept;

    bool can_place_creature(position_t position_p) const noexcept;
    bool can_place_base(position_t position_p, int size_p) const noexcept;
    std::optional<position_t> free_position_around(const base_t& base_p);
    std::optional<position_t> random_position_near(
        const base_t& base_p,
        int minimum_distance_p,
        int maximum_distance_p,
        const std::function<bool(position_t)>& position_allowed_p
    );
    bool is_position_occupied(position_t position_p) const noexcept;
    creature_t* creature_at(position_t position_p) const noexcept;
    std::optional<position_t> next_step_towards(
        const creature_t& creature_p,
        position_t destination_p
    );
    std::optional<position_t> next_step_away(
        const creature_t& creature_p,
        position_t threat_position_p
    );
    std::optional<position_t> next_idle_step(
        const creature_t& creature_p,
        position_t idle_starting_position_p
    );

#ifndef NDEBUG
    std::size_t occupied_position_count() const noexcept
    {
        return occupied_position_count_;
    }
#endif

    std::optional<position_t> find_free_position(
        std::size_t first_position_index_p
    ) const noexcept;

private:
    static bool is_position_inside(position_t position_p) noexcept;
    static std::size_t position_index(position_t position_p) noexcept;

    std::array<creature_t*, position_count> creatures_{};
    std::array<const base_t*, position_count> bases_{};
    pathfinder_t pathfinder_{
        game_constants::map_column_count,
        game_constants::map_row_count
    };
    std::mt19937 random_generator_{std::random_device{}()};
#ifndef NDEBUG
    std::size_t occupied_position_count_ = 0;
#endif
};
