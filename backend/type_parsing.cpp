#include "type_parsing.h"

#include <charconv>
#include <stdexcept>
#include <string>

std::uint32_t parse_color(std::string_view color_p)
{
    if (color_p.size() != 7 || color_p.front() != '#') {
        throw std::invalid_argument("Color must use #RRGGBB format.");
    }

    std::uint32_t color = 0;
    const auto result = std::from_chars(
        color_p.data() + 1,
        color_p.data() + color_p.size(),
        color,
        16
    );

    if (result.ec != std::errc{} || result.ptr != color_p.data() + color_p.size()) {
        throw std::invalid_argument("Color contains invalid characters.");
    }

    return color;
}

void require_non_negative(int value_p, const char* field_name_p)
{
    if (value_p < 0) {
        throw std::invalid_argument(
            std::string("Field must not be negative: ") + field_name_p
        );
    }
}
