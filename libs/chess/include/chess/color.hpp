#pragma once

#include <cstdint>

namespace chess {

enum class Color : std::uint8_t {
    NONE = 0,
    WHITE = 1 << 4,
    BLACK = 1 << 5,
    BOTH = WHITE | BLACK,
};

inline constexpr Color invert(Color color) {
    return static_cast<Color>(
        static_cast<std::uint8_t>(color) ^
        static_cast<std::uint8_t>(Color::BOTH)
    );
}

}  // namespace chess
