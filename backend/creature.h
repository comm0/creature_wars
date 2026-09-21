#pragma once

#include <cstdint>

#include "creature_type.h"

struct position_t
{
    int column_ = 0;
    int row_ = 0;

    bool operator==(const position_t&) const = default;
};

enum class direction_t
{
    north,
    east,
    south,
    west
};

class game_t;

class creature_t
{
public:
    creature_t(
        std::uint64_t id_p,
        const creature_type_t& type_p,
        position_t position_p
    );

    void on_think(game_t& game_p);

    void move_to(position_t position_p) noexcept
    {
        position_ = position_p;
    }

    std::uint64_t id() const noexcept
    {
        return id_;
    }

    position_t position() const noexcept
    {
        return position_;
    }

    const creature_type_t& type() const noexcept
    {
        return type_;
    }

    int health() const noexcept
    {
        return health_;
    }

private:
    direction_t choose_random_direction();

    std::uint64_t id_;
    const creature_type_t& type_;
    position_t position_;
    int health_;
};
