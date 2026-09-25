#include <gtest/gtest.h>

#include <chrono>
#include <limits>
#include <stdexcept>

#include "game_clock.h"

namespace
{
const auto start_time = game_clock_t::time_point_t{};

game_clock_t::time_point_t at(int milliseconds_p)
{
    return start_time + std::chrono::milliseconds{milliseconds_p};
}
}

TEST(game_clock_t_test, stays_still_while_paused)
{
    game_clock_t clock(at(100));

    EXPECT_EQ(clock.now(at(500)), at(100));
}

TEST(game_clock_t_test, advances_at_selected_scale_without_a_jump)
{
    game_clock_t clock(at(0));
    clock.resume(at(0));

    EXPECT_EQ(clock.now(at(1000)), at(1000));

    clock.set_time_scale(2.0, at(1000));

    EXPECT_EQ(clock.now(at(1000)), at(1000));
    EXPECT_EQ(clock.now(at(2500)), at(4000));
}

TEST(game_clock_t_test, converts_game_deadline_to_wall_time)
{
    game_clock_t clock(at(0));
    clock.set_time_scale(5.0, at(0));
    clock.resume(at(0));

    EXPECT_EQ(clock.wall_time_for(at(10000), at(1000)), at(2000));
}

TEST(game_clock_t_test, pause_and_resume_preserve_game_time)
{
    game_clock_t clock(at(0));
    clock.resume(at(0));
    clock.pause(at(1000));

    EXPECT_EQ(clock.now(at(5000)), at(1000));

    clock.resume(at(5000));

    EXPECT_EQ(clock.now(at(6000)), at(2000));
}

TEST(game_clock_t_test, rejects_invalid_scale)
{
    game_clock_t clock(at(0));

    EXPECT_THROW(clock.set_time_scale(0.0, at(0)), std::invalid_argument);
    EXPECT_THROW(
        clock.set_time_scale(std::numeric_limits<double>::infinity(), at(0)),
        std::invalid_argument
    );
}
