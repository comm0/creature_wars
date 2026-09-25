#include "base.h"

#include "game.h"
#include "game_constants.h"

#include <algorithm>

namespace
{
int axis_distance(int value_p, int first_p, int last_p) noexcept
{
    return std::max({0, first_p - value_p, value_p - last_p});
}
}

base_t::base_t(
    std::uint64_t id_p,
    const base_type_t& type_p,
    const creature_type_t& spawn_type_p,
    position_t position_p,
    int level_p
)
    : id_(id_p)
    , type_(type_p)
    , spawn_type_(spawn_type_p)
    , position_(position_p)
    , health_(type_p.health())
    , level_(level_p)
{
}

void base_t::on_attacking(
    game_t& game_p,
    std::chrono::milliseconds interval_p
)
{
    if (is_dead()) {
        return;
    }

    auto target = target_id_.has_value()
        ? game_p.find_base_target(*this, *target_id_)
        : std::nullopt;

    if (!target.has_value()) {
        target = game_p.find_nearest_base_target(*this);
        target_id_ = target.has_value()
            ? std::optional<std::uint64_t>(target->id_)
            : std::nullopt;
    }

    if (!target.has_value()) {
        attack_elapsed_ = std::chrono::milliseconds{0};
        return;
    }

    attack_elapsed_ = std::min(
        game_constants::attack_interval,
        attack_elapsed_ + interval_p
    );

    if (attack_elapsed_ < game_constants::attack_interval) {
        return;
    }

    game_p.perform_base_attack(*this, *target);
    attack_elapsed_ = std::chrono::milliseconds{0};
}

void base_t::drain_health(int damage_p) noexcept
{
    health_ = std::max(0, health_ - std::max(0, damage_p));
}

bool base_t::contains(position_t position_p) const noexcept
{
    return distance_to(position_p) == 0;
}

position_t base_t::closest_position_to(position_t position_p) const noexcept
{
    return {
        std::clamp(position_p.column_, position_.column_, last_column()),
        std::clamp(position_p.row_, position_.row_, last_row())
    };
}

int base_t::distance_to(position_t position_p) const noexcept
{
    return std::max(
        axis_distance(position_p.column_, position_.column_, last_column()),
        axis_distance(position_p.row_, position_.row_, last_row())
    );
}

int base_t::distance_to(const base_t& base_p) const noexcept
{
    return std::max(
        std::max({
            0,
            base_p.position_.column_ - last_column(),
            position_.column_ - base_p.last_column()
        }),
        std::max({
            0,
            base_p.position_.row_ - last_row(),
            position_.row_ - base_p.last_row()
        })
    );
}
