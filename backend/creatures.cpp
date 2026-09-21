#include "creatures.h"

std::uint64_t creatures_t::next_uid_ = 1000;

creature_t& creatures_t::create(
    const creature_type_t& type_p,
    position_t position_p
)
{
    const auto id = next_uid_++;

    creatures_.push_back(
        std::make_unique<creature_t>(id, type_p, position_p)
    );

    return *creatures_.back();
}

creature_t* creatures_t::find(std::uint64_t id_p) noexcept
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr && creature->id() == id_p) {
            return creature.get();
        }
    }

    return nullptr;
}

void creatures_t::on_think(game_t& game_p)
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr) {
            creature->on_think(game_p);
        }
    }
}

void creatures_t::publish_creatures(
    const std::function<void(const creature_t&)>& creature_handler_p
) const
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr) {
            creature_handler_p(*creature);
        }
    }
}
