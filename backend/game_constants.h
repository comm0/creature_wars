#pragma once

#include <chrono>

namespace game_constants
{
inline constexpr int map_column_count = CREATURE_WARS_MAP_COLUMN_COUNT;
inline constexpr int map_row_count = CREATURE_WARS_MAP_ROW_COUNT;
inline constexpr std::chrono::milliseconds tick_interval{1000};
inline constexpr std::chrono::milliseconds attack_interval{1000};
}
