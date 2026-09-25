#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "resource_cost.h"
#include "resource_reward.h"

struct base_level_stats_t
{
    int health_;
    int attack_;
};

struct unit_option_t
{
    std::string creature_;
    int tier_;
    timed_cost_t spawn_;
    std::optional<timed_cost_t> training_;
};

class base_type_t
{
public:
    base_type_t(
        std::string identifier_p,
        std::string name_p,
        std::string group_p,
        std::uint32_t color_p,
        int size_p,
        int range_p,
        bool match_base_p,
        resource_reward_t reward_p,
        std::vector<base_level_stats_t> levels_p,
        std::vector<unit_option_t> units_p
    )
        : identifier_(std::move(identifier_p))
        , name_(std::move(name_p))
        , group_(std::move(group_p))
        , color_(color_p)
        , size_(size_p)
        , range_(range_p)
        , match_base_(match_base_p)
        , reward_(reward_p)
        , levels_(std::move(levels_p))
        , units_(std::move(units_p))
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

    int size() const noexcept
    {
        return size_;
    }

    int range() const noexcept
    {
        return range_;
    }

    bool is_match_base() const noexcept
    {
        return match_base_;
    }

    const resource_reward_t& reward() const noexcept
    {
        return reward_;
    }

    int max_level() const noexcept
    {
        return static_cast<int>(levels_.size());
    }

    const base_level_stats_t& level_stats(int level_p) const
    {
        return levels_.at(static_cast<std::size_t>(level_p - 1));
    }

    const std::vector<unit_option_t>& units() const noexcept
    {
        return units_;
    }

private:
    std::string identifier_;
    std::string name_;
    std::string group_;
    std::uint32_t color_;
    int size_;
    int range_;
    bool match_base_;
    resource_reward_t reward_;
    std::vector<base_level_stats_t> levels_;
    std::vector<unit_option_t> units_;
};
