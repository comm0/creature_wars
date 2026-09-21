#pragma once

#include <cstdint>

#include "creature.h"

class igame_observer_t
{
public:
    virtual ~igame_observer_t() = default;

    virtual void on_game_tick() = 0;
    virtual void on_creature_created(const creature_t& creature_p) = 0;
    virtual void on_creature_moved(
        std::uint64_t id_p,
        position_t previous_position_p,
        position_t position_p
    ) = 0;
    virtual void on_creature_health_changed(
        std::uint64_t id_p,
        int health_p
    ) = 0;
    virtual void on_creature_removed(std::uint64_t id_p) = 0;
};
