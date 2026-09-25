#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "base.h"
#include "base_action.h"
#include "creature.h"
#include "player_state.h"

class igame_observer_t
{
public:
    virtual ~igame_observer_t() = default;

    virtual void on_game_tick() = 0;
    virtual void on_creature_created(const creature_t& creature_p) = 0;
    virtual void on_creature_moved(
        std::uint64_t id_p,
        position_t position_p,
        direction_t direction_p
    ) = 0;
    virtual void on_creature_health_changed(
        std::uint64_t id_p,
        int health_p
    ) = 0;
    virtual void on_creature_attack_performed(std::uint64_t id_p) = 0;
    virtual void on_creature_walk_requested(std::uint64_t id_p) = 0;
    virtual void on_creature_state_changed(
        std::uint64_t id_p,
        creature_state_t state_p
    ) = 0;
    virtual void on_creature_spotted(
        std::uint64_t observer_id_p,
        std::uint64_t spotted_id_p
    ) = 0;
    virtual void on_creature_removed(std::uint64_t id_p) = 0;
    virtual void on_corpse_created(
        std::uint64_t id_p,
        position_t position_p,
        const std::string& identifier_p
    ) = 0;
    virtual void on_corpse_removed(std::uint64_t id_p) = 0;
    virtual void on_base_created(const base_t& base_p) = 0;
    virtual void on_base_health_changed(std::uint64_t id_p, int health_p) = 0;
    virtual void on_base_attack_performed(
        std::uint64_t id_p,
        position_t target_position_p
    ) = 0;
    virtual void on_base_removed(std::uint64_t id_p) = 0;
    virtual void on_player_state_changed(const player_state_t& state_p) = 0;
    virtual void on_base_controller_changed(
        const base_controller_state_t& state_p
    ) = 0;
    virtual void on_base_changed(const base_t& base_p) = 0;
    virtual void on_base_actions_changed(
        std::uint64_t base_id_p,
        const std::vector<base_action_state_t>& actions_p
    ) = 0;
    virtual void on_area_attack(
        position_t center_p,
        int radius_p,
        std::uint32_t color_p
    ) = 0;
    virtual void on_match_ended(bool victory_p, std::chrono::milliseconds duration_p) = 0;
    virtual void on_creature_target_changed(
        std::uint64_t id_p,
        std::uint64_t target_id_p
    ) = 0;
};
