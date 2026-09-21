#include "creature_type.h"

#include <utility>

creature_type_t::creature_type_t(
    std::string identifier_p,
    std::string name_p,
    std::string group_p,
    std::uint32_t color_p,
    std::optional<std::uint32_t> marker_color_p,
    int health_p,
    int attack_p,
    int attack_range_p,
    int vision_range_p
)
    : identifier_(std::move(identifier_p))
    , name_(std::move(name_p))
    , group_(std::move(group_p))
    , color_(color_p)
    , marker_color_(marker_color_p)
    , health_(health_p)
    , attack_(attack_p)
    , attack_range_(attack_range_p)
    , vision_range_(vision_range_p)
{
}
