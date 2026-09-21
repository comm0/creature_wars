#include "creature.h"

#include "game.h"
#include "game_constants.h"

#include <chrono>

namespace
{
constexpr int blocked_path_retry_count = 10;
}

creature_t::creature_t(
    std::uint64_t id_p,
    const creature_type_t& type_p,
    position_t position_p
)
    : id_(id_p)
    , type_(type_p)
    , position_(position_p)
    , health_(type_p.health())
{
}

void creature_t::on_think(game_t&)
{
}

void creature_t::update_movement(
    game_t& game_p,
    std::chrono::steady_clock::time_point now_p
)
{
    if (!destination_.has_value()
        || !next_movement_time_.has_value()
        || now_p < *next_movement_time_) {
        return;
    }

    if (position_ == *destination_) {
        finish_walking(game_p);
        return;
    }

    if (game_p.move_creature_towards(id_, *destination_)) {
        blocked_path_retry_count_ = 0;
        next_movement_time_ = now_p + movement_interval();
        return;
    }

    if (blocked_path_retry_count_ < blocked_path_retry_count) {
        ++blocked_path_retry_count_;
        next_movement_time_ = now_p + game_constants::tick_interval;
        return;
    }

    finish_walking(game_p);
}

void creature_t::finish_walking(game_t& game_p)
{
    destination_.reset();
    next_movement_time_.reset();
    blocked_path_retry_count_ = 0;
    game_p.notify_creature_state_changed(*this);
}

std::chrono::steady_clock::duration creature_t::movement_interval() const
{
    const auto interval = std::chrono::duration_cast<
        std::chrono::steady_clock::duration
    >(
        std::chrono::duration<double>(1.0 / type_.speed())
    );

    if (interval < std::chrono::milliseconds{1}) {
        return std::chrono::milliseconds{1};
    }

    return interval;
}
