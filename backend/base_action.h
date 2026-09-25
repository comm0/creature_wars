#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include "resource_cost.h"

enum class base_action_kind_t
{
    spawn,
    training,
    research,
    upgrade
};

struct base_action_state_t
{
    std::string key_;
    base_action_kind_t kind_;
    std::string name_;
    std::string subject_;
    std::uint32_t color_;
    int level_;
    timed_cost_t cost_;
    int queued_;
    std::chrono::milliseconds remaining_{0};
};
