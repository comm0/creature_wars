#pragma once

#include <chrono>
#include <string_view>

struct income_settings_t
{
    int amount_;
    std::chrono::milliseconds interval_;
};

struct economy_settings_t
{
    int starting_gold_;
    int starting_food_;
    income_settings_t gold_income_;
    income_settings_t food_income_;
};

economy_settings_t parse_economy_settings(std::string_view economy_json_p);
