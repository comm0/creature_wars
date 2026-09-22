#include "visibility_system.h"

#include "creatures.h"
#include "game_constants.h"

#include <cstdlib>

void visibility_system_t::add_creature(
    creature_t& creature_p,
    const game_map_t& game_map_p,
    creatures_t& creatures_p,
    const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
)
{
    const auto position = creature_p.position();
    const auto& current_observers = observers_by_position_[
        position_index(position)
    ];

    for (const auto observer_id : current_observers) {
        auto* observer = creatures_p.find(observer_id);

        if (observer != nullptr) {
            spot_creature(*observer, creature_p, spotted_p);
        }
    }

    register_full_range(creature_p, game_map_p, spotted_p);
}

void visibility_system_t::move_creature(
    creature_t& creature_p,
    position_t previous_position_p,
    const game_map_t& game_map_p,
    creatures_t& creatures_p,
    const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
)
{
    const auto position = creature_p.position();
    const auto& previous_observers = observers_by_position_[
        position_index(previous_position_p)
    ];
    const auto& current_observers = observers_by_position_[
        position_index(position)
    ];

    for (const auto observer_id : previous_observers) {
        if (observer_id == creature_p.id()
            || current_observers.contains(observer_id)) {
            continue;
        }

        auto* observer = creatures_p.find(observer_id);

        if (observer != nullptr) {
            observer->remove_visible_creature(creature_p.id());
        }
    }

    for (const auto observer_id : current_observers) {
        if (observer_id == creature_p.id()
            || previous_observers.contains(observer_id)) {
            continue;
        }

        auto* observer = creatures_p.find(observer_id);

        if (observer != nullptr) {
            spot_creature(*observer, creature_p, spotted_p);
        }
    }

    move_observer_range(
        creature_p,
        previous_position_p,
        game_map_p,
        spotted_p
    );
}

void visibility_system_t::register_full_range(
    creature_t& observer_p,
    const game_map_t& game_map_p,
    const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
)
{
    const auto position = observer_p.position();
    const auto range = observer_p.type().vision_range();

    for (auto row = position.row_ - range; row <= position.row_ + range; ++row) {
        for (
            auto column = position.column_ - range;
            column <= position.column_ + range;
            ++column
        ) {
            add_observed_position(
                observer_p,
                {column, row},
                game_map_p,
                spotted_p
            );
        }
    }
}

void visibility_system_t::move_observer_range(
    creature_t& observer_p,
    position_t previous_position_p,
    const game_map_t& game_map_p,
    const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
)
{
    const auto position = observer_p.position();
    const auto range = observer_p.type().vision_range();
    const auto column_change = position.column_ - previous_position_p.column_;
    const auto row_change = position.row_ - previous_position_p.row_;

    if (std::abs(column_change) + std::abs(row_change) != 1) {
        for (
            auto row = previous_position_p.row_ - range;
            row <= previous_position_p.row_ + range;
            ++row
        ) {
            for (
                auto column = previous_position_p.column_ - range;
                column <= previous_position_p.column_ + range;
                ++column
            ) {
                remove_observed_position(
                    observer_p,
                    {column, row},
                    game_map_p
                );
            }
        }

        register_full_range(observer_p, game_map_p, spotted_p);
        return;
    }

    if (column_change != 0) {
        const auto removed_column = column_change > 0
            ? previous_position_p.column_ - range
            : previous_position_p.column_ + range;
        const auto added_column = column_change > 0
            ? position.column_ + range
            : position.column_ - range;

        for (auto row = position.row_ - range; row <= position.row_ + range; ++row) {
            remove_observed_position(
                observer_p,
                {removed_column, row},
                game_map_p
            );
            add_observed_position(
                observer_p,
                {added_column, row},
                game_map_p,
                spotted_p
            );
        }

        return;
    }

    const auto removed_row = row_change > 0
        ? previous_position_p.row_ - range
        : previous_position_p.row_ + range;
    const auto added_row = row_change > 0
        ? position.row_ + range
        : position.row_ - range;

    for (
        auto column = position.column_ - range;
        column <= position.column_ + range;
        ++column
    ) {
        remove_observed_position(
            observer_p,
            {column, removed_row},
            game_map_p
        );
        add_observed_position(
            observer_p,
            {column, added_row},
            game_map_p,
            spotted_p
        );
    }
}

void visibility_system_t::add_observed_position(
    creature_t& observer_p,
    position_t position_p,
    const game_map_t& game_map_p,
    const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
)
{
    if (!is_position_inside(position_p)) {
        return;
    }

    observers_by_position_[position_index(position_p)].insert(observer_p.id());
    auto* spotted_creature = game_map_p.creature_at(position_p);

    if (spotted_creature != nullptr) {
        spot_creature(observer_p, *spotted_creature, spotted_p);
    }
}

void visibility_system_t::remove_observed_position(
    creature_t& observer_p,
    position_t position_p,
    const game_map_t& game_map_p
)
{
    if (!is_position_inside(position_p)) {
        return;
    }

    observers_by_position_[position_index(position_p)].erase(observer_p.id());
    const auto* spotted_creature = game_map_p.creature_at(position_p);

    if (spotted_creature != nullptr
        && spotted_creature->id() != observer_p.id()) {
        observer_p.remove_visible_creature(spotted_creature->id());
    }
}

void visibility_system_t::spot_creature(
    creature_t& observer_p,
    creature_t& spotted_creature_p,
    const std::function<void(std::uint64_t, std::uint64_t)>& spotted_p
)
{
    if (observer_p.id() == spotted_creature_p.id()
        || !observer_p.add_visible_creature(spotted_creature_p.id())) {
        return;
    }

    spotted_p(observer_p.id(), spotted_creature_p.id());
}

bool visibility_system_t::is_position_inside(position_t position_p) noexcept
{
    return position_p.column_ >= 0
        && position_p.column_ < game_constants::map_column_count
        && position_p.row_ >= 0
        && position_p.row_ < game_constants::map_row_count;
}

std::size_t visibility_system_t::position_index(position_t position_p) noexcept
{
    return static_cast<std::size_t>(position_p.row_)
        * static_cast<std::size_t>(game_constants::map_column_count)
        + static_cast<std::size_t>(position_p.column_);
}
