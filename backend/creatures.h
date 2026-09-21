#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "creature.h"

class creatures_t
{
public:
    creature_t& create(position_t position_p = {});

    creature_t* find(std::uint64_t id_p) noexcept;

#ifndef NDEBUG
    std::size_t size() const noexcept
    {
        return creatures_.size();
    }
#endif

    void on_think(game_t& game_p);

    void publish_positions(
        const std::function<void(std::uint64_t, position_t)>& position_handler_p
    ) const;

private:
    static std::uint64_t next_uid_;

    std::vector<std::unique_ptr<creature_t>> creatures_;
};
