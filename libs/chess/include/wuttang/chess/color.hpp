#pragma once

#include <cstdint>

namespace wuttang::chess {

enum class Color : std::uint8_t {
    WHITE = 0,
    BLACK = 1,
    NONE = 2,
};

inline constexpr Color invert(Color color) {
    return static_cast<Color>(static_cast<std::uint8_t>(color) ^ 1);
}

}  // namespace wuttang::chess
