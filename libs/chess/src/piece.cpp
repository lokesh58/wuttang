#include "chess/piece.hpp"

#include <cstdint>

#include "chess/color.hpp"

namespace chess {

const Piece Piece::WHITE_PAWN = Piece(Color::WHITE, PieceType::PAWN);
const Piece Piece::WHITE_KNIGHT = Piece(Color::WHITE, PieceType::KNIGHT);
const Piece Piece::WHITE_BISHOP = Piece(Color::WHITE, PieceType::BISHOP);
const Piece Piece::WHITE_ROOK = Piece(Color::WHITE, PieceType::ROOK);
const Piece Piece::WHITE_QUEEN = Piece(Color::WHITE, PieceType::QUEEN);
const Piece Piece::WHITE_KING = Piece(Color::WHITE, PieceType::KING);
const Piece Piece::BLACK_PAWN = Piece(Color::BLACK, PieceType::PAWN);
const Piece Piece::BLACK_KNIGHT = Piece(Color::BLACK, PieceType::KNIGHT);
const Piece Piece::BLACK_BISHOP = Piece(Color::BLACK, PieceType::BISHOP);
const Piece Piece::BLACK_ROOK = Piece(Color::BLACK, PieceType::ROOK);
const Piece Piece::BLACK_QUEEN = Piece(Color::BLACK, PieceType::QUEEN);
const Piece Piece::BLACK_KING = Piece(Color::BLACK, PieceType::KING);

std::uint8_t Piece::encode(Color color, PieceType type) {
  return (static_cast<std::uint8_t>(color) << COLOR_SHIFT) |
         static_cast<std::uint8_t>(type);
}

Piece::Piece(Color piece_color, PieceType piece_type)
    : encoded_piece_(encode(piece_color, piece_type)) {}

Color Piece::get_piece_color() const {
  return static_cast<Color>(encoded_piece_ >> COLOR_SHIFT);
}

PieceType Piece::get_piece_type() const {
  return static_cast<PieceType>(encoded_piece_ & TYPE_MASK);
}

}  // namespace chess
