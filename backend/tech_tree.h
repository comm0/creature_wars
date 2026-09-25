#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "resource_cost.h"

enum class research_effect_t
{
    base_regeneration,
    base_health_percent,
    healing_aura,
    base_damage_percent,
    shot_count,
    base_range_bonus
};

struct research_level_t
{
    int base_level_;
    timed_cost_t cost_;
    int value_;
};

struct research_t
{
    std::string identifier_;
    std::string name_;
    research_effect_t effect_;
    std::vector<research_level_t> levels_;
};

struct base_upgrade_t
{
    int level_;
    timed_cost_t cost_;
};

class tech_tree_t
{
public:
    explicit tech_tree_t(std::string_view tech_tree_json_p);

    const std::vector<research_t>& researches() const noexcept
    {
        return researches_;
    }

    const research_t* find_research(std::string_view identifier_p) const noexcept;
    const base_upgrade_t* find_upgrade(int level_p) const noexcept;

private:
    std::vector<research_t> researches_;
    std::vector<base_upgrade_t> upgrades_;
};
