#include "pathfinder.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <queue>
#include <vector>

namespace
{
struct path_node_t
{
    std::size_t index_;
    std::size_t estimated_total_cost_;
    std::uint32_t random_priority_;
};

struct path_node_compare_t
{
    bool operator()(const path_node_t& left_p, const path_node_t& right_p) const
    {
        if (left_p.estimated_total_cost_ != right_p.estimated_total_cost_) {
            return left_p.estimated_total_cost_ > right_p.estimated_total_cost_;
        }

        return left_p.random_priority_ > right_p.random_priority_;
    }
};
}

pathfinder_t::pathfinder_t(int column_count_p, int row_count_p)
    : column_count_(column_count_p)
    , row_count_(row_count_p)
    , random_generator_(std::random_device{}())
{
}

std::optional<position_t> pathfinder_t::find_next_step(
    position_t start_p,
    position_t destination_p,
    const std::function<bool(position_t)>& is_walkable_p
)
{
    if (!is_position_inside(start_p)
        || !is_position_inside(destination_p)) {
        return std::nullopt;
    }

    const auto position_count = static_cast<std::size_t>(column_count_)
        * static_cast<std::size_t>(row_count_);
    const auto no_position = position_count;
    const auto start_index = position_index(start_p);
    std::vector<std::size_t> path_costs(
        position_count,
        std::numeric_limits<std::size_t>::max()
    );
    std::vector<std::size_t> previous_positions(position_count, no_position);
    std::vector<bool> closed_positions(position_count, false);
    std::priority_queue<
        path_node_t,
        std::vector<path_node_t>,
        path_node_compare_t
    > open_positions;
    auto random_priority = std::uniform_int_distribution<std::uint32_t>();
    auto choose_equal_path = std::bernoulli_distribution(0.5);

    path_costs[start_index] = 0;
    open_positions.push({
        start_index,
        distance(start_p, destination_p),
        random_priority(random_generator_)
    });

    auto best_index = start_index;
    auto best_distance = distance(start_p, destination_p);

    constexpr std::array<position_t, 4> base_offsets{
        position_t{0, -1},
        position_t{1, 0},
        position_t{0, 1},
        position_t{-1, 0}
    };

    while (!open_positions.empty()) {
        const auto current_node = open_positions.top();
        open_positions.pop();

        if (closed_positions[current_node.index_]) {
            continue;
        }

        closed_positions[current_node.index_] = true;
        const auto current_position = position_from_index(current_node.index_);
        const auto current_distance = distance(current_position, destination_p);

        if (current_distance < best_distance
            || (current_distance == best_distance
                && path_costs[current_node.index_] < path_costs[best_index])
            || (current_distance == best_distance
                && path_costs[current_node.index_] == path_costs[best_index]
                && choose_equal_path(random_generator_))) {
            best_index = current_node.index_;
            best_distance = current_distance;
        }

        if (current_position == destination_p) {
            best_index = current_node.index_;
            break;
        }

        auto offsets = base_offsets;
        std::shuffle(offsets.begin(), offsets.end(), random_generator_);

        for (const auto offset : offsets) {
            const position_t neighbour_position{
                current_position.column_ + offset.column_,
                current_position.row_ + offset.row_
            };

            if (!is_position_inside(neighbour_position)
                || !is_walkable_p(neighbour_position)) {
                continue;
            }

            const auto neighbour_index = position_index(neighbour_position);

            if (closed_positions[neighbour_index]) {
                continue;
            }

            const auto candidate_cost = path_costs[current_node.index_] + 1;

            if (candidate_cost > path_costs[neighbour_index]
                || (candidate_cost == path_costs[neighbour_index]
                    && !choose_equal_path(random_generator_))) {
                continue;
            }

            path_costs[neighbour_index] = candidate_cost;
            previous_positions[neighbour_index] = current_node.index_;
            open_positions.push({
                neighbour_index,
                candidate_cost + distance(neighbour_position, destination_p),
                random_priority(random_generator_)
            });
        }
    }

    if (best_index == start_index) {
        return std::nullopt;
    }

    auto next_index = best_index;

    while (previous_positions[next_index] != start_index) {
        next_index = previous_positions[next_index];

        if (next_index == no_position) {
            return std::nullopt;
        }
    }

    return position_from_index(next_index);
}

bool pathfinder_t::is_position_inside(position_t position_p) const noexcept
{
    return position_p.column_ >= 0
        && position_p.column_ < column_count_
        && position_p.row_ >= 0
        && position_p.row_ < row_count_;
}

std::size_t pathfinder_t::position_index(position_t position_p) const noexcept
{
    return static_cast<std::size_t>(position_p.row_)
        * static_cast<std::size_t>(column_count_)
        + static_cast<std::size_t>(position_p.column_);
}

position_t pathfinder_t::position_from_index(std::size_t index_p) const noexcept
{
    return {
        static_cast<int>(index_p % static_cast<std::size_t>(column_count_)),
        static_cast<int>(index_p / static_cast<std::size_t>(column_count_))
    };
}

std::size_t pathfinder_t::distance(
    position_t position_p,
    position_t destination_p
) noexcept
{
    return static_cast<std::size_t>(
        std::abs(position_p.column_ - destination_p.column_)
        + std::abs(position_p.row_ - destination_p.row_)
    );
}
