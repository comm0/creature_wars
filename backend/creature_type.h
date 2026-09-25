#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "resource_reward.h"

enum class creature_behavior_t
{
    aggressive,
    fleeing
};

struct creature_attributes_t
{
    int health_;
    int attack_;
    int attack_range_;
    int vision_range_;
    double speed_;
    creature_behavior_t behavior_ = creature_behavior_t::aggressive;
    int area_radius_ = 0;
    std::uint32_t area_color_ = 0xffffff;
    resource_reward_t reward_;
};

class creature_type_t
{
public:
    creature_type_t(
        std::string identifier_p,
        std::string name_p,
        std::string group_p,
        std::uint32_t color_p,
        std::optional<std::uint32_t> marker_color_p,
        const creature_attributes_t& attributes_p
    );

    const std::string& identifier() const noexcept
    {
        return identifier_;
    }

    const std::string& name() const noexcept
    {
        return name_;
    }

    const std::string& group() const noexcept
    {
        return group_;
    }

    std::uint32_t color() const noexcept
    {
        return color_;
    }

    std::optional<std::uint32_t> marker_color() const noexcept
    {
        return marker_color_;
    }

    int health() const noexcept
    {
        return attributes_.health_;
    }

    int attack() const noexcept
    {
        return attributes_.attack_;
    }

    int attack_range() const noexcept
    {
        return attributes_.attack_range_;
    }

    int vision_range() const noexcept
    {
        return attributes_.vision_range_;
    }

    double speed() const noexcept
    {
        return attributes_.speed_;
    }

    creature_behavior_t behavior() const noexcept
    {
        return attributes_.behavior_;
    }

    int area_radius() const noexcept
    {
        return attributes_.area_radius_;
    }

    std::uint32_t area_color() const noexcept
    {
        return attributes_.area_color_;
    }

    const resource_reward_t& reward() const noexcept
    {
        return attributes_.reward_;
    }

private:
    std::string identifier_;
    std::string name_;
    std::string group_;
    std::uint32_t color_;
    std::optional<std::uint32_t> marker_color_;
    creature_attributes_t attributes_;
};
