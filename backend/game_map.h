#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "creature.h"
#include "game_constants.h"

class game_map_t
{
public:
    static constexpr std::size_t position_count =
        static_cast<std::size_t>(game_constants::map_column_count)
        * static_cast<std::size_t>(game_constants::map_row_count);

    bool place_creature(creature_t& creature_p, position_t position_p) noexcept;
    bool move_creature(creature_t& creature_p, position_t position_p) noexcept;

    bool can_place_creature(position_t position_p) const noexcept;
    bool is_position_occupied(position_t position_p) const noexcept;

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
#ifndef NDEBUG
    std::size_t occupied_position_count_ = 0;
#endif
};
