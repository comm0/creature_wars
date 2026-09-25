#pragma once

#include <chrono>

struct resource_cost_t
{
    int gold_ = 0;
    int food_ = 0;
};

struct timed_cost_t
{
    resource_cost_t cost_;
    std::chrono::milliseconds duration_{0};
};
