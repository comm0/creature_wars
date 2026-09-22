#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "pathfinder.h"

namespace
{
bool contains_position(
    const std::vector<position_t>& positions_p,
    position_t position_p
)
{
    return std::find(
        positions_p.begin(),
        positions_p.end(),
        position_p
    ) != positions_p.end();
}
}

// Verifies the first step of a direct route (e.g. walking along an empty row).
TEST(pathfinder_t_test, finds_direct_next_step)
{
    pathfinder_t pathfinder{4, 1};

    const auto next_step = pathfinder.find_next_step(
        {0, 0},
        {3, 0},
        [](position_t) { return true; }
    );

    ASSERT_TRUE(next_step.has_value());
    EXPECT_EQ(*next_step, (position_t{1, 0}));
}

// Verifies that a blocked direct route is bypassed (e.g. walking around a creature).
TEST(pathfinder_t_test, finds_route_around_obstacle)
{
    pathfinder_t pathfinder{3, 2};
    const std::vector<position_t> blocked_positions{{1, 0}};

    const auto next_step = pathfinder.find_next_step(
        {0, 0},
        {2, 0},
        [&blocked_positions](position_t position_p) {
            return !contains_position(blocked_positions, position_p);
        }
    );

    ASSERT_TRUE(next_step.has_value());
    EXPECT_EQ(*next_step, (position_t{1, 1}));
}

// Verifies movement toward the closest free field (e.g. a destination occupied by another creature).
TEST(pathfinder_t_test, approaches_occupied_destination)
{
    pathfinder_t pathfinder{4, 1};
    const position_t destination{3, 0};

    const auto next_step = pathfinder.find_next_step(
        {0, 0},
        destination,
        [destination](position_t position_p) {
            return position_p != destination;
        }
    );

    ASSERT_TRUE(next_step.has_value());
    EXPECT_EQ(*next_step, (position_t{1, 0}));
}

// Verifies that no step is returned from a closed area (e.g. a surrounded creature).
TEST(pathfinder_t_test, returns_no_step_when_route_is_blocked)
{
    pathfinder_t pathfinder{3, 3};
    const std::vector<position_t> blocked_positions{
        {1, 0},
        {2, 0},
        {2, 1},
        {1, 2},
        {2, 2},
        {0, 2},
        {0, 1},
        {0, 0}
    };

    const auto next_step = pathfinder.find_next_step(
        {1, 1},
        {2, 2},
        [&blocked_positions](position_t position_p) {
            return !contains_position(blocked_positions, position_p);
        }
    );

    EXPECT_FALSE(next_step.has_value());
}

// Verifies a diagonal shortest step (e.g. moving across an open square).
TEST(pathfinder_t_test, finds_diagonal_next_step)
{
    pathfinder_t pathfinder{2, 2};

    const auto next_step = pathfinder.find_next_step(
        {0, 0},
        {1, 1},
        [](position_t) { return true; }
    );

    ASSERT_TRUE(next_step.has_value());
    EXPECT_EQ(*next_step, (position_t{1, 1}));
}

// Verifies that an already reached destination requires no step.
TEST(pathfinder_t_test, returns_no_step_at_destination)
{
    pathfinder_t pathfinder{3, 3};

    const auto next_step = pathfinder.find_next_step(
        {1, 1},
        {1, 1},
        [](position_t) { return true; }
    );

    EXPECT_FALSE(next_step.has_value());
}

// Verifies that positions outside the map are rejected (e.g. an invalid movement request).
TEST(pathfinder_t_test, rejects_positions_outside_map)
{
    pathfinder_t pathfinder{3, 3};
    const auto is_walkable = [](position_t) { return true; };

    EXPECT_FALSE(pathfinder.find_next_step(
        {-1, 0},
        {1, 1},
        is_walkable
    ).has_value());
    EXPECT_FALSE(pathfinder.find_next_step(
        {1, 1},
        {3, 1},
        is_walkable
    ).has_value());
}
