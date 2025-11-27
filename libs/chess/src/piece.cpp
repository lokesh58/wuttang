#include "chess/piece.hpp"

#include <cctype>

namespace chess {

char get_piece_char(Piece piece) noexcept {
    const auto piece_type = get_piece_type(piece);
    const auto piece_color = get_piece_color(piece);

    char c;
    switch (piece_type) {
        case PieceType::PAWN:
            c = 'p';
            break;
        case PieceType::KNIGHT:
            c = 'n';
            break;
        case PieceType::BISHOP:
            c = 'b';
            break;
        case PieceType::ROOK:
            c = 'r';
            break;
        case PieceType::QUEEN:
            c = 'q';
            break;
        case PieceType::KING:
            c = 'k';
            break;
    }

    if (piece_color == Color::WHITE) {
        return std::toupper(c);
    }

    return c;
}

}  // namespace chess
