#include "economy_settings.h"

#include "type_parsing.h"

#include <stdexcept>

#include <nlohmann/json.hpp>

namespace
{
income_settings_t parse_income(const nlohmann::json& income_p, const char* name_p)
{
    const auto amount = income_p.at("amount").get<int>();
    const auto interval = income_p.at("intervalMs").get<int>();

    require_non_negative(amount, name_p);

    if (interval <= 0) {
        throw std::invalid_argument("Income interval must be positive.");
    }

    return {amount, std::chrono::milliseconds{interval}};
}
}

economy_settings_t parse_economy_settings(std::string_view economy_json_p)
{
    const auto document = nlohmann::json::parse(economy_json_p);
    const auto starting_gold = document.at("startingGold").get<int>();
    const auto starting_food = document.at("startingFood").get<int>();
    const auto& income = document.at("income");

    require_non_negative(starting_gold, "startingGold");
    require_non_negative(starting_food, "startingFood");

    return {
        starting_gold,
        starting_food,
        parse_income(income.at("gold"), "gold"),
        parse_income(income.at("food"), "food"),
        document.value("levelIncomeBonusPercent", 0)
    };
}
