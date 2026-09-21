#include "creature.h"

#include "game.h"

#include <random>

namespace
{
std::mt19937 random_generator{std::random_device{}()};
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
    game_p.request_move(id_, choose_random_direction());
}

direction_t creature_t::choose_random_direction()
{
    auto direction_distribution = std::uniform_int_distribution<int>(0, 3);
    return static_cast<direction_t>(direction_distribution(random_generator));
}
