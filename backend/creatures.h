#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "creature.h"

class creatures_t
{
public:
    creature_t& create(
        const creature_type_t& type_p,
        position_t position_p = {}
    );

    creature_t* find(std::uint64_t id_p) noexcept;
    const creature_t* find(std::uint64_t id_p) const noexcept;
    bool remove(std::uint64_t id_p) noexcept;

#ifndef NDEBUG
    std::size_t size() const noexcept
    {
        return creatures_.size();
    }
#endif

    void on_think(game_t& game_p);
    void on_attacking(
        game_t& game_p,
        std::chrono::milliseconds interval_p
    );
    void update_movement(
        game_t& game_p,
        std::chrono::steady_clock::time_point now_p
    );
    std::optional<std::chrono::steady_clock::time_point> next_movement_time()
        const noexcept;

    void publish_creatures(
        const std::function<void(const creature_t&)>& creature_handler_p
    ) const;

private:
    static std::uint64_t next_uid_;

    std::vector<std::unique_ptr<creature_t>> creatures_;
};
