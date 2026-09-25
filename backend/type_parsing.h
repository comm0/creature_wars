#pragma once

#include <cstdint>
#include <string_view>

#include <nlohmann/json_fwd.hpp>

#include "resource_cost.h"

std::uint32_t parse_color(std::string_view color_p);
void require_non_negative(int value_p, const char* field_name_p);
timed_cost_t parse_timed_cost(const nlohmann::json& definition_p);
