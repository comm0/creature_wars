#pragma once

#include <string>

struct player_state_t
{
    std::string base_name_;
    int base_level_ = 1;
    int gold_ = 0;
    int food_ = 0;
};
