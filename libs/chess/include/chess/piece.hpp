#pragma once

#include <cstdint>

#include "chess/color.hpp"

namespace chess {

enum class PieceType : std::uint8_t {
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
};

class Piece {
 public:
  static const Piece WHITE_PAWN;
  static const Piece WHITE_KNIGHT;
  static const Piece WHITE_BISHOP;
  static const Piece WHITE_ROOK;
  static const Piece WHITE_QUEEN;
  static const Piece WHITE_KING;
  static const Piece BLACK_PAWN;
  static const Piece BLACK_KNIGHT;
  static const Piece BLACK_BISHOP;
  static const Piece BLACK_ROOK;
  static const Piece BLACK_QUEEN;
  static const Piece BLACK_KING;

  Color get_piece_color() const;

  PieceType get_piece_type() const;

 private:
  Piece(Color piece_color, PieceType piece_type);

  static constexpr std::uint8_t COLOR_SHIFT = 4;
  static constexpr std::uint8_t TYPE_MASK = 0x0F;

  static std::uint8_t encode(Color color, PieceType type);

  std::uint8_t encoded_piece_;
};

}  // namespace chess
