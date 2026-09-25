#include "game_clock.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

game_clock_t::game_clock_t(time_point_t wall_time_p) noexcept
    : game_time_(wall_time_p)
    , wall_time_(wall_time_p)
{
}

game_clock_t::time_point_t game_clock_t::now(time_point_t wall_time_p) const noexcept
{
    if (paused_) {
        return game_time_;
    }

    return game_time_ + scaled(wall_time_p - wall_time_, time_scale_);
}

game_clock_t::time_point_t game_clock_t::wall_time_for(
    time_point_t game_time_p,
    time_point_t wall_time_p
) const
{
    if (paused_) {
        throw std::logic_error("A paused game clock has no wall deadline.");
    }

    const auto game_delay = std::max(duration_t::zero(), game_time_p - now(wall_time_p));
    return wall_time_p + scaled(game_delay, 1.0 / time_scale_);
}

void game_clock_t::set_time_scale(double time_scale_p, time_point_t wall_time_p)
{
    if (!std::isfinite(time_scale_p) || time_scale_p <= 0.0) {
        throw std::invalid_argument("Game time scale must be finite and positive.");
    }

    game_time_ = now(wall_time_p);
    wall_time_ = wall_time_p;
    time_scale_ = time_scale_p;
}

void game_clock_t::pause(time_point_t wall_time_p) noexcept
{
    if (paused_) {
        return;
    }

    game_time_ = now(wall_time_p);
    wall_time_ = wall_time_p;
    paused_ = true;
}

void game_clock_t::resume(time_point_t wall_time_p) noexcept
{
    if (!paused_) {
        return;
    }

    wall_time_ = wall_time_p;
    paused_ = false;
}

game_clock_t::duration_t game_clock_t::scaled(
    duration_t duration_p,
    double multiplier_p
) noexcept
{
    return std::chrono::duration_cast<duration_t>(
        std::chrono::duration<double>(duration_p) * multiplier_p
    );
}
