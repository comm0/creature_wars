#pragma once

#include <chrono>
#include <string>

struct resource_state_t
{
    int amount_ = 0;
    int income_ = 0;
    std::chrono::milliseconds income_interval_{0};
    int income_cycle_ = 0;
};

struct player_state_t
{
    std::string base_name_;
    int base_level_ = 1;
    resource_state_t gold_;
    resource_state_t food_;
};
