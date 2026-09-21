#include "creature.h"

#include "game.h"

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

void creature_t::on_think(game_t& game_p)
{
    if (!destination_.has_value()) {
        return;
    }

    if (position_ == *destination_) {
        destination_.reset();
        blocked_path_retry_count_ = 0;
        game_p.notify_creature_state_changed(*this);
        return;
    }

    if (game_p.move_creature_towards(id_, *destination_)) {
        blocked_path_retry_count_ = 0;
        return;
    }

    if (blocked_path_retry_count_ < blocked_path_retry_count) {
        ++blocked_path_retry_count_;
        return;
    }

    destination_.reset();
    blocked_path_retry_count_ = 0;
    game_p.notify_creature_state_changed(*this);
}
