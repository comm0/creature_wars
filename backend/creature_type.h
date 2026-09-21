#pragma once

#include <cstdint>
#include <optional>
#include <string>

class creature_type_t
{
public:
    creature_type_t(
        std::string identifier_p,
        std::string name_p,
        std::string group_p,
        std::uint32_t color_p,
        std::optional<std::uint32_t> marker_color_p,
        int health_p,
        int attack_p,
        int attack_range_p,
        int vision_range_p
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
        return health_;
    }

    int attack() const noexcept
    {
        return attack_;
    }

    int attack_range() const noexcept
    {
        return attack_range_;
    }

    int vision_range() const noexcept
    {
        return vision_range_;
    }

private:
    std::string identifier_;
    std::string name_;
    std::string group_;
    std::uint32_t color_;
    std::optional<std::uint32_t> marker_color_;
    int health_;
    int attack_;
    int attack_range_;
    int vision_range_;
};
