#pragma once

#include <string_view>
#include <vector>

#include "base_type.h"

class base_type_registry_t
{
public:
    explicit base_type_registry_t(std::string_view base_types_json_p);

    const base_type_t& get(std::string_view identifier_p) const;
    const std::vector<base_type_t>& all() const noexcept
    {
        return base_types_;
    }

private:
    std::vector<base_type_t> base_types_;
};
