#pragma once

#include <string_view>
#include <vector>

#include "creature_type.h"

class creature_type_registry_t
{
public:
    explicit creature_type_registry_t(std::string_view creature_types_json_p);

    const creature_type_t& get(std::string_view identifier_p) const;
    std::size_t group_size(std::string_view group_p) const noexcept;
    const creature_type_t& get_from_group(
        std::string_view group_p,
        std::size_t index_p
    ) const;

private:
    std::vector<creature_type_t> creature_types_;
};
