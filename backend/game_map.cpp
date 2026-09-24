#include "game_map.h"

#ifndef NDEBUG
#include "debug_console.h"
#endif

#include <algorithm>
#include <cstdlib>
#include <vector>

namespace
{
constexpr int guaranteed_idle_return_distance = 3;

int position_distance(position_t left_p, position_t right_p) noexcept
{
    return std::max(
        std::abs(left_p.column_ - right_p.column_),
        std::abs(left_p.row_ - right_p.row_)
    );
}
}

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

bool game_map_t::remove_creature(creature_t& creature_p) noexcept
{
    const auto position = creature_p.position();

    if (!is_position_inside(position)
        || creatures_[position_index(position)] != &creature_p) {
#ifndef NDEBUG
        debug_console::print_warning("Could not remove creature from the game map.");
#endif
        return false;
    }

    creatures_[position_index(position)] = nullptr;
#ifndef NDEBUG
    --occupied_position_count_;
#endif
    return true;
}

bool game_map_t::place_base(base_t& base_p) noexcept
{
    const auto position = base_p.position();
    const auto size = base_p.type().size();

    if (!can_place_base(position, size)) {
        return false;
    }

    for (auto row = position.row_; row < position.row_ + size; ++row) {
        for (auto column = position.column_; column < position.column_ + size; ++column) {
            bases_[position_index({column, row})] = &base_p;
        }
    }

    return true;
}

void game_map_t::remove_base(const base_t& base_p) noexcept
{
    for (auto& base : bases_) {
        if (base == &base_p) {
            base = nullptr;
        }
    }
}

bool game_map_t::can_place_base(position_t position_p, int size_p) const noexcept
{
    for (auto row = position_p.row_; row < position_p.row_ + size_p; ++row) {
        for (auto column = position_p.column_; column < position_p.column_ + size_p; ++column) {
            if (!can_place_creature({column, row})) {
                return false;
            }
        }
    }

    return true;
}

std::optional<position_t> game_map_t::free_position_around(const base_t& base_p)
{
    const auto position = base_p.position();
    const auto size = base_p.type().size();
    std::vector<position_t> free_positions;

    for (auto row = position.row_ - 1; row <= position.row_ + size; ++row) {
        for (auto column = position.column_ - 1; column <= position.column_ + size; ++column) {
            const position_t candidate{column, row};

            if (base_p.distance_to(candidate) == 1 && can_place_creature(candidate)) {
                free_positions.push_back(candidate);
            }
        }
    }

    if (free_positions.empty()) {
        return std::nullopt;
    }

    auto index_distribution = std::uniform_int_distribution<std::size_t>(
        0,
        free_positions.size() - 1
    );

    return free_positions[index_distribution(random_generator_)];
}

bool game_map_t::is_position_occupied(position_t position_p) const noexcept
{
    if (!is_position_inside(position_p)) {
        return false;
    }

    const auto index = position_index(position_p);
    return creatures_[index] != nullptr || bases_[index] != nullptr;
}

creature_t* game_map_t::creature_at(position_t position_p) const noexcept
{
    if (!is_position_inside(position_p)) {
        return nullptr;
    }

    return creatures_[position_index(position_p)];
}

bool game_map_t::can_place_creature(position_t position_p) const noexcept
{
    return is_position_inside(position_p)
        && !is_position_occupied(position_p);
}

std::optional<position_t> game_map_t::next_step_towards(
    const creature_t& creature_p,
    position_t destination_p
)
{
    const auto start_position = creature_p.position();

    if (!is_position_inside(start_position)
        || !is_position_inside(destination_p)) {
        return std::nullopt;
    }

    const auto start_index = position_index(start_position);

    if (creatures_[start_index] != &creature_p) {
        return std::nullopt;
    }

    return pathfinder_.find_next_step(
        start_position,
        destination_p,
        [this](position_t position_p) {
            return can_place_creature(position_p);
        }
    );
}

std::optional<position_t> game_map_t::next_idle_step(
    const creature_t& creature_p,
    position_t idle_starting_position_p
)
{
    const auto position = creature_p.position();
    const auto distance = position_distance(position, idle_starting_position_p);
    const auto return_probability = std::min(
        1.0,
        static_cast<double>(distance) / guaranteed_idle_return_distance
    );
    auto return_to_start = std::bernoulli_distribution(return_probability);

    constexpr std::array<position_t, 4> offsets{
        position_t{0, -1},
        position_t{1, 0},
        position_t{0, 1},
        position_t{-1, 0}
    };
    std::array<position_t, offsets.size()> available_positions{};
    std::size_t available_position_count = 0;

    for (const auto offset : offsets) {
        const position_t next_position{
            position.column_ + offset.column_,
            position.row_ + offset.row_
        };

        if (can_place_creature(next_position)) {
            available_positions[available_position_count] = next_position;
            ++available_position_count;
        }
    }

    if (available_position_count == 0) {
        return std::nullopt;
    }

    if (return_to_start(random_generator_)) {
        std::array<position_t, offsets.size()> closest_positions{};
        std::size_t closest_position_count = 0;
        auto closest_distance = position_distance(
            available_positions[0],
            idle_starting_position_p
        );

        for (
            std::size_t index = 0;
            index < available_position_count;
            ++index
        ) {
            const auto next_distance = position_distance(
                available_positions[index],
                idle_starting_position_p
            );

            if (next_distance < closest_distance) {
                closest_distance = next_distance;
                closest_position_count = 0;
            }

            if (next_distance == closest_distance) {
                closest_positions[closest_position_count] =
                    available_positions[index];
                ++closest_position_count;
            }
        }

        auto closest_position_distribution =
            std::uniform_int_distribution<std::size_t>(
                0,
                closest_position_count - 1
            );

        return closest_positions[
            closest_position_distribution(random_generator_)
        ];
    }

    auto position_index_distribution = std::uniform_int_distribution<std::size_t>(
        0,
        available_position_count - 1
    );

    return available_positions[position_index_distribution(random_generator_)];
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
