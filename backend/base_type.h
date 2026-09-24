#pragma once

#include <cstdint>
#include <string>
#include <utility>

struct base_attributes_t
{
    int size_;
    int health_;
    int attack_;
    int attack_range_;
};

class base_type_t
{
public:
    base_type_t(
        std::string identifier_p,
        std::string name_p,
        std::string group_p,
        std::uint32_t color_p,
        std::string spawn_creature_p,
        base_attributes_t attributes_p
    )
        : identifier_(std::move(identifier_p))
        , name_(std::move(name_p))
        , group_(std::move(group_p))
        , color_(color_p)
        , spawn_creature_(std::move(spawn_creature_p))
        , attributes_(attributes_p)
    {
    }

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

    const std::string& spawn_creature() const noexcept
    {
        return spawn_creature_;
    }

    int size() const noexcept
    {
        return attributes_.size_;
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

private:
    std::string identifier_;
    std::string name_;
    std::string group_;
    std::uint32_t color_;
    std::string spawn_creature_;
    base_attributes_t attributes_;
};
