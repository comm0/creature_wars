#pragma once

#include <cstdint>
#include <string_view>

std::uint32_t parse_color(std::string_view color_p);
void require_non_negative(int value_p, const char* field_name_p);
