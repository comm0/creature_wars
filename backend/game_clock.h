#pragma once

#include <chrono>

class game_clock_t
{
public:
    using clock_t = std::chrono::steady_clock;
    using time_point_t = clock_t::time_point;
    using duration_t = clock_t::duration;

    explicit game_clock_t(time_point_t wall_time_p = clock_t::now()) noexcept;

    time_point_t now(time_point_t wall_time_p = clock_t::now()) const noexcept;
    time_point_t wall_time_for(
        time_point_t game_time_p,
        time_point_t wall_time_p = clock_t::now()
    ) const;
    void set_time_scale(
        double time_scale_p,
        time_point_t wall_time_p = clock_t::now()
    );
    void pause(time_point_t wall_time_p = clock_t::now()) noexcept;
    void resume(time_point_t wall_time_p = clock_t::now()) noexcept;

    double time_scale() const noexcept
    {
        return time_scale_;
    }

private:
    static duration_t scaled(duration_t duration_p, double multiplier_p) noexcept;

    time_point_t game_time_;
    time_point_t wall_time_;
    double time_scale_ = 1.0;
    bool paused_ = true;
};
