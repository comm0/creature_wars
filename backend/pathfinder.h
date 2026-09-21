#pragma once

#include <functional>
#include <optional>
#include <random>

#include "creature.h"

class pathfinder_t
{
public:
    pathfinder_t(int column_count_p, int row_count_p);

    std::optional<position_t> find_next_step(
        position_t start_p,
        position_t destination_p,
        const std::function<bool(position_t)>& is_walkable_p
    );

private:
    bool is_position_inside(position_t position_p) const noexcept;
    std::size_t position_index(position_t position_p) const noexcept;
    position_t position_from_index(std::size_t index_p) const noexcept;
    static std::size_t distance(
        position_t position_p,
        position_t destination_p
    ) noexcept;

    int column_count_;
    int row_count_;
    std::mt19937 random_generator_;
};
