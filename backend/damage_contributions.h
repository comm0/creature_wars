#pragma once

#include <string>
#include <unordered_map>

class damage_contributions_t
{
public:
    void add(const std::string& group_p, int damage_p)
    {
        if (!group_p.empty() && damage_p > 0) {
            damage_by_group_[group_p] += damage_p;
        }
    }

    const std::unordered_map<std::string, int>& by_group() const noexcept
    {
        return damage_by_group_;
    }

private:
    std::unordered_map<std::string, int> damage_by_group_;
};
