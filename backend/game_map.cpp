#include "game_map.h"

#ifndef NDEBUG
#include "debug_console.h"
#endif

bool game_map_t::place_creature(
    creature_t& creature_p,
    position_t position_p
) noexcept
{
    if (!is_position_inside(position_p) || is_position_occupied(position_p)) {
        return false;
    }

    creatures_[position_index(position_p)] = &creature_p;
#ifndef NDEBUG
    ++occupied_position_count_;
#endif
    creature_p.move_to(position_p);
    return true;
}

bool game_map_t::move_creature(
    creature_t& creature_p,
    position_t position_p
) noexcept
{
    if (!is_position_inside(position_p) || is_position_occupied(position_p)) {
        return false;
    }

    const auto current_position = creature_p.position();

    if (!is_position_inside(current_position)
        || creatures_[position_index(current_position)] != &creature_p) {
#ifndef NDEBUG
        debug_console::print_warning("Creature position does not match the game map.");
#endif
        return false;
    }

    creatures_[position_index(current_position)] = nullptr;
    creatures_[position_index(position_p)] = &creature_p;
    creature_p.move_to(position_p);
    return true;
}

bool game_map_t::is_position_occupied(position_t position_p) const noexcept
{
    return is_position_inside(position_p)
        && creatures_[position_index(position_p)] != nullptr;
}

bool game_map_t::can_place_creature(position_t position_p) const noexcept
{
    return is_position_inside(position_p)
        && !is_position_occupied(position_p);
}

std::optional<position_t> game_map_t::find_free_position(
    std::size_t first_position_index_p
) const noexcept
{
    const auto first_position_index = first_position_index_p % position_count;

    for (std::size_t offset = 0; offset < position_count; ++offset) {
        const auto index = (first_position_index + offset) % position_count;
        const position_t position{
            static_cast<int>(index % game_constants::map_column_count),
            static_cast<int>(index / game_constants::map_column_count)
        };

        if (!is_position_occupied(position)) {
            return position;
        }
    }

    return std::nullopt;
}

bool game_map_t::is_position_inside(position_t position_p) noexcept
{
    return position_p.column_ >= 0
        && position_p.column_ < game_constants::map_column_count
        && position_p.row_ >= 0
        && position_p.row_ < game_constants::map_row_count;
}

std::size_t game_map_t::position_index(position_t position_p) noexcept
{
    return static_cast<std::size_t>(position_p.row_)
        * static_cast<std::size_t>(game_constants::map_column_count)
        + static_cast<std::size_t>(position_p.column_);
}
