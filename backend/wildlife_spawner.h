#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>

#include "game_scheduler.h"

struct wildlife_spawn_handlers_t
{
    std::function<void(std::string_view, std::string_view)> spawn_creature_near_group_;
    std::function<std::optional<std::uint64_t>(std::string_view)> spawn_lair_near_group_;
    std::function<bool(std::uint64_t)> spawn_troll_near_lair_;
    std::function<bool(std::uint64_t)> lair_exists_;
};

class wildlife_spawner_t
{
public:
    wildlife_spawner_t(
        game_scheduler_t& scheduler_p,
        wildlife_spawn_handlers_t handlers_p
    );

    void start(game_scheduler_t::time_point_t time_p);
    void reset() noexcept;

private:
    void schedule_deer(game_scheduler_t::time_point_t time_p, std::uint64_t generation_p);
    void schedule_wolf(game_scheduler_t::time_point_t time_p, std::uint64_t generation_p);
    void schedule_lair(game_scheduler_t::time_point_t time_p, std::uint64_t generation_p);
    void schedule_troll(
        std::uint64_t lair_id_p,
        game_scheduler_t::time_point_t time_p,
        std::uint64_t generation_p
    );
    std::string_view next_group(std::size_t& index_p) noexcept;

    game_scheduler_t& scheduler_;
    wildlife_spawn_handlers_t handlers_;
    std::size_t deer_group_index_ = 0;
    std::size_t wolf_group_index_ = 0;
    std::size_t lair_group_index_ = 0;
    std::uint64_t generation_ = 0;
};
