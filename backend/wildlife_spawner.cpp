#include "wildlife_spawner.h"

#include <array>
#include <chrono>
#include <utility>

namespace
{
constexpr std::chrono::seconds deer_initial_delay{30};
constexpr std::chrono::minutes wildlife_spawn_interval{1};
constexpr std::chrono::minutes lair_spawn_interval{5};
constexpr std::chrono::minutes troll_spawn_interval{2};
constexpr std::array<std::string_view, 3> faction_groups{
    "minotaurs",
    "dwarves",
    "orcs"
};
}

wildlife_spawner_t::wildlife_spawner_t(
    game_scheduler_t& scheduler_p,
    wildlife_spawn_handlers_t handlers_p
)
    : scheduler_(scheduler_p)
    , handlers_(std::move(handlers_p))
{
}

void wildlife_spawner_t::start(game_scheduler_t::time_point_t time_p)
{
    reset();
    const auto generation = generation_;
    schedule_deer(time_p + deer_initial_delay, generation);
    schedule_wolf(time_p + wildlife_spawn_interval, generation);
    schedule_lair(time_p + lair_spawn_interval, generation);
}

void wildlife_spawner_t::reset() noexcept
{
    ++generation_;
    deer_group_index_ = 0;
    wolf_group_index_ = 0;
    lair_group_index_ = 0;
}

void wildlife_spawner_t::schedule_deer(
    game_scheduler_t::time_point_t time_p,
    std::uint64_t generation_p
)
{
    scheduler_.schedule(time_p, [this, generation_p](auto scheduled_time_p) {
        if (generation_p != generation_) {
            return;
        }

        handlers_.spawn_creature_near_group_("deer", next_group(deer_group_index_));
        schedule_deer(scheduled_time_p + wildlife_spawn_interval, generation_p);
    });
}

void wildlife_spawner_t::schedule_wolf(
    game_scheduler_t::time_point_t time_p,
    std::uint64_t generation_p
)
{
    scheduler_.schedule(time_p, [this, generation_p](auto scheduled_time_p) {
        if (generation_p != generation_) {
            return;
        }

        handlers_.spawn_creature_near_group_("wolf", next_group(wolf_group_index_));
        schedule_wolf(scheduled_time_p + wildlife_spawn_interval, generation_p);
    });
}

void wildlife_spawner_t::schedule_lair(
    game_scheduler_t::time_point_t time_p,
    std::uint64_t generation_p
)
{
    scheduler_.schedule(time_p, [this, generation_p](auto scheduled_time_p) {
        if (generation_p != generation_) {
            return;
        }

        const auto lair_id = handlers_.spawn_lair_near_group_(
            next_group(lair_group_index_)
        );

        if (lair_id.has_value()) {
            handlers_.spawn_troll_near_lair_(*lair_id);
            schedule_troll(
                *lair_id,
                scheduled_time_p + troll_spawn_interval,
                generation_p
            );
        }

        schedule_lair(scheduled_time_p + lair_spawn_interval, generation_p);
    });
}

void wildlife_spawner_t::schedule_troll(
    std::uint64_t lair_id_p,
    game_scheduler_t::time_point_t time_p,
    std::uint64_t generation_p
)
{
    scheduler_.schedule(
        time_p,
        [this, lair_id_p, generation_p](auto scheduled_time_p) {
            if (generation_p != generation_ || !handlers_.lair_exists_(lair_id_p)) {
                return;
            }

            handlers_.spawn_troll_near_lair_(lair_id_p);
            schedule_troll(
                lair_id_p,
                scheduled_time_p + troll_spawn_interval,
                generation_p
            );
        }
    );
}

std::string_view wildlife_spawner_t::next_group(std::size_t& index_p) noexcept
{
    const auto group = faction_groups[index_p];
    index_p = (index_p + 1) % faction_groups.size();
    return group;
}
