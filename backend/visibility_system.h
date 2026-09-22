#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_set>

#include "game_map.h"

class creatures_t;

class visibility_system_t
{
public:
    void add_creature(
        creature_t& creature_p,
        const game_map_t& game_map_p,
        creatures_t& creatures_p,
        const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
    );
    void move_creature(
        creature_t& creature_p,
        position_t previous_position_p,
        const game_map_t& game_map_p,
        creatures_t& creatures_p,
        const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
    );
    void remove_creature(
        creature_t& creature_p,
        creatures_t& creatures_p
    );

private:
    void register_full_range(
        creature_t& observer_p,
        const game_map_t& game_map_p,
        const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
    );
    void move_observer_range(
        creature_t& observer_p,
        position_t previous_position_p,
        const game_map_t& game_map_p,
        const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
    );
    void add_observed_position(
        creature_t& observer_p,
        position_t position_p,
        const game_map_t& game_map_p,
        const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
    );
    void remove_observed_position(
        creature_t& observer_p,
        position_t position_p,
        const game_map_t& game_map_p
    );
    static void spot_creature(
        creature_t& observer_p,
        creature_t& spotted_creature_p,
        const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
    );
    static int circle_extent(int range_p, int offset_p) noexcept;
    static bool is_position_inside(position_t position_p) noexcept;
    static std::size_t position_index(position_t position_p) noexcept;

    std::array<
        std::unordered_set<std::uint64_t>,
        game_map_t::position_count
    > observers_by_position_;
};
