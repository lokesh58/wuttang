#pragma once

#include <cstdint>

namespace chess {

enum class CastlingRights : std::uint8_t {
    NONE = 0,
    WHITE_KINGSIDE = 1 << 0,
    WHITE_QUEENSIDE = 1 << 1,
    BLACK_KINGSIDE = 1 << 2,
    BLACK_QUEENSIDE = 1 << 3,
};

inline constexpr CastlingRights operator|(CastlingRights a, CastlingRights b) {
    return static_cast<CastlingRights>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b)
    );
}

inline constexpr CastlingRights& operator|=(
    CastlingRights& a,
    CastlingRights b
) {
    a = a | b;
    return a;
}

inline constexpr CastlingRights operator&(CastlingRights a, CastlingRights b) {
    return static_cast<CastlingRights>(
        static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b)
    );
}

inline constexpr CastlingRights& operator&=(
    CastlingRights& a,
    CastlingRights b
) {
    a = a & b;
    return a;
}

inline constexpr CastlingRights operator~(CastlingRights a) {
    return static_cast<CastlingRights>(~static_cast<std::uint8_t>(a));
}

}  // namespace chess
