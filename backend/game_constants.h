#pragma once

#include <chrono>

namespace game_constants
{
inline constexpr int map_column_count = 40;
inline constexpr int map_row_count = 20;
inline constexpr std::chrono::milliseconds tick_interval{1000};
}
