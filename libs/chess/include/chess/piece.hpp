#pragma once

#include <array>
#include <cstdint>

#include "chess/color.hpp"

namespace chess {

enum class PieceType : std::uint8_t {
    NONE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
};

namespace detail {

    constexpr std::uint8_t PIECE_COLOR_MASK = 0xF0;
    constexpr std::uint8_t PIECE_TYPE_MASK = 0x0F;

    inline constexpr std::uint8_t get_piece_encoded_value(
        Color piece_color,
        PieceType piece_type
    ) noexcept {
        return static_cast<std::uint8_t>(piece_color) |
               static_cast<std::uint8_t>(piece_type);
    }

}  // namespace detail

enum class Piece : std::uint8_t {
    NONE = detail::get_piece_encoded_value(Color::NONE, PieceType::NONE),
    WHITE_PAWN = detail::get_piece_encoded_value(Color::WHITE, PieceType::PAWN),
    WHITE_KNIGHT =
        detail::get_piece_encoded_value(Color::WHITE, PieceType::KNIGHT),
    WHITE_BISHOP =
        detail::get_piece_encoded_value(Color::WHITE, PieceType::BISHOP),
    WHITE_ROOK = detail::get_piece_encoded_value(Color::WHITE, PieceType::ROOK),
    WHITE_QUEEN =
        detail::get_piece_encoded_value(Color::WHITE, PieceType::QUEEN),
    WHITE_KING = detail::get_piece_encoded_value(Color::WHITE, PieceType::KING),
    BLACK_PAWN = detail::get_piece_encoded_value(Color::BLACK, PieceType::PAWN),
    BLACK_KNIGHT =
        detail::get_piece_encoded_value(Color::BLACK, PieceType::KNIGHT),
    BLACK_BISHOP =
        detail::get_piece_encoded_value(Color::BLACK, PieceType::BISHOP),
    BLACK_ROOK = detail::get_piece_encoded_value(Color::BLACK, PieceType::ROOK),
    BLACK_QUEEN =
        detail::get_piece_encoded_value(Color::BLACK, PieceType::QUEEN),
    BLACK_KING = detail::get_piece_encoded_value(Color::BLACK, PieceType::KING),
};

namespace detail {

    inline constexpr auto PIECE_CHARS = [] {
        std::array<char, 64> arr;
        arr.fill('?');

        arr[static_cast<std::uint8_t>(Piece::NONE)] = '.';

        arr[static_cast<std::uint8_t>(Piece::WHITE_PAWN)] = 'P';
        arr[static_cast<std::uint8_t>(Piece::WHITE_KNIGHT)] = 'N';
        arr[static_cast<std::uint8_t>(Piece::WHITE_BISHOP)] = 'B';
        arr[static_cast<std::uint8_t>(Piece::WHITE_ROOK)] = 'R';
        arr[static_cast<std::uint8_t>(Piece::WHITE_QUEEN)] = 'Q';
        arr[static_cast<std::uint8_t>(Piece::WHITE_KING)] = 'K';

        arr[static_cast<std::uint8_t>(Piece::BLACK_PAWN)] = 'p';
        arr[static_cast<std::uint8_t>(Piece::BLACK_KNIGHT)] = 'n';
        arr[static_cast<std::uint8_t>(Piece::BLACK_BISHOP)] = 'b';
        arr[static_cast<std::uint8_t>(Piece::BLACK_ROOK)] = 'r';
        arr[static_cast<std::uint8_t>(Piece::BLACK_QUEEN)] = 'q';
        arr[static_cast<std::uint8_t>(Piece::BLACK_KING)] = 'k';

        return arr;
    }();

}  // namespace detail

inline constexpr Piece get_piece(
    Color piece_color,
    PieceType piece_type
) noexcept {
    return static_cast<Piece>(
        detail::get_piece_encoded_value(piece_color, piece_type)
    );
}

inline constexpr Color get_piece_color(Piece piece) noexcept {
    return static_cast<Color>(
        static_cast<std::uint8_t>(piece) & detail::PIECE_COLOR_MASK
    );
}

inline constexpr PieceType get_piece_type(Piece piece) noexcept {
    return static_cast<PieceType>(
        static_cast<std::uint8_t>(piece) & detail::PIECE_TYPE_MASK
    );
}

inline constexpr char get_piece_char(Piece piece) noexcept {
    const auto index = static_cast<std::uint8_t>(piece);
    return detail::PIECE_CHARS[index];
}

inline constexpr Piece piece_from_char(char piece_char) noexcept {
    switch (piece_char) {
        case 'P':
            return Piece::WHITE_PAWN;
        case 'N':
            return Piece::WHITE_KNIGHT;
        case 'B':
            return Piece::WHITE_BISHOP;
        case 'R':
            return Piece::WHITE_ROOK;
        case 'Q':
            return Piece::WHITE_QUEEN;
        case 'K':
            return Piece::WHITE_KING;
        case 'p':
            return Piece::BLACK_PAWN;
        case 'n':
            return Piece::BLACK_KNIGHT;
        case 'b':
            return Piece::BLACK_BISHOP;
        case 'r':
            return Piece::BLACK_ROOK;
        case 'q':
            return Piece::BLACK_QUEEN;
        case 'k':
            return Piece::BLACK_KING;
        default:
            return Piece::NONE;
    }
}

}  // namespace chess
