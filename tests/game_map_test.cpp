#include <gtest/gtest.h>

#include "creature.h"
#include "game_map.h"

namespace
{
const creature_type_t& test_creature_type()
{
    static const creature_type_t creature_type{
        "test",
        "Test",
        "none",
        0x000000,
        std::nullopt,
        1,
        1,
        1,
        1
    };

    return creature_type;
}
}

// Verifies that two creatures cannot be placed on one position (e.g. colliding spawns).
TEST(game_map_t_test, does_not_place_creature_on_occupied_position)
{
    game_map_t game_map;
    const auto& creature_type = test_creature_type();
    creature_t first_creature{1000, creature_type, {0, 0}};
    creature_t second_creature{1001, creature_type, {0, 0}};

    EXPECT_TRUE(game_map.place_creature(first_creature, {4, 5}));
    EXPECT_FALSE(game_map.place_creature(second_creature, {4, 5}));
    EXPECT_EQ(second_creature.position(), (position_t{0, 0}));
}

// Verifies that a creature cannot move onto another creature (e.g. a blocked step).
TEST(game_map_t_test, does_not_move_creature_to_occupied_position)
{
    game_map_t game_map;
    const auto& creature_type = test_creature_type();
    creature_t first_creature{1000, creature_type, {0, 0}};
    creature_t second_creature{1001, creature_type, {0, 0}};

    ASSERT_TRUE(game_map.place_creature(first_creature, {4, 5}));
    ASSERT_TRUE(game_map.place_creature(second_creature, {5, 5}));

    EXPECT_FALSE(game_map.move_creature(first_creature, {5, 5}));
    EXPECT_EQ(first_creature.position(), (position_t{4, 5}));
    EXPECT_TRUE(game_map.is_position_occupied({4, 5}));
    EXPECT_TRUE(game_map.is_position_occupied({5, 5}));
}

// Verifies that a creature can move to an unoccupied position (e.g. a normal step).
TEST(game_map_t_test, moves_creature_to_free_position)
{
    game_map_t game_map;
    const auto& creature_type = test_creature_type();
    creature_t creature{1000, creature_type, {0, 0}};

    ASSERT_TRUE(game_map.place_creature(creature, {4, 5}));

    EXPECT_TRUE(game_map.move_creature(creature, {5, 5}));
    EXPECT_EQ(creature.position(), (position_t{5, 5}));
    EXPECT_FALSE(game_map.is_position_occupied({4, 5}));
    EXPECT_TRUE(game_map.is_position_occupied({5, 5}));
}
