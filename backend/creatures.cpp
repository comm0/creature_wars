#include "creatures.h"

std::uint64_t creatures_t::next_uid_ = 1000;

creature_t& creatures_t::create(
    const creature_type_t& type_p,
    position_t position_p
)
{
    const auto id = allocate_id();

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

const creature_t* creatures_t::find(std::uint64_t id_p) const noexcept
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr && creature->id() == id_p) {
            return creature.get();
        }
    }

    return nullptr;
}

bool creatures_t::remove(std::uint64_t id_p) noexcept
{
    for (auto creature = creatures_.begin(); creature != creatures_.end(); ++creature) {
        if (*creature != nullptr && (*creature)->id() == id_p) {
            creatures_.erase(creature);
            return true;
        }
    }

    return false;
}

void creatures_t::on_think(game_t& game_p)
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr) {
            creature->on_think(game_p);
        }
    }
}

void creatures_t::on_attacking(
    game_t& game_p,
    std::chrono::milliseconds interval_p
)
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr) {
            creature->on_attacking(game_p, interval_p);
        }
    }
}

void creatures_t::update_movement(
    game_t& game_p,
    std::chrono::steady_clock::time_point now_p
)
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr) {
            creature->update_movement(game_p, now_p);
        }
    }
}

std::optional<std::chrono::steady_clock::time_point>
creatures_t::next_movement_time() const noexcept
{
    std::optional<std::chrono::steady_clock::time_point> next_time;

    for (const auto& creature : creatures_) {
        if (creature == nullptr) {
            continue;
        }

        const auto creature_time = creature->next_movement_time();

        if (!creature_time.has_value()) {
            continue;
        }

        if (!next_time.has_value() || *creature_time < *next_time) {
            next_time = *creature_time;
        }
    }

    return next_time;
}

void creatures_t::for_each(
    const std::function<void(const creature_t&)>& creature_handler_p
) const
{
    for (const auto& creature : creatures_) {
        if (creature != nullptr) {
            creature_handler_p(*creature);
        }
    }
}
