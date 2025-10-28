#include "chess/position.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/piece.hpp"

namespace chess {

Position::Position() noexcept:
        side_to_move_(Color::WHITE),
        castling_rights_(CastlingRights::NONE) {};

Position Position::standard() noexcept {
    return from_valid_fen(STANDARD_STARTING_FEN);
}

Position Position::from_fen(std::string_view fen_string) {
    if (!is_valid_fen(fen_string)) {
        throw std::invalid_argument("Invalid FEN string");
    }
    return from_valid_fen(fen_string);
}

std::optional<Piece> Position::get_piece_at(Square square) const noexcept {
    return board_[static_cast<std::size_t>(square)];
}

bool Position::is_valid_fen(std::string_view fen_string) noexcept {
    return true;
}

Position Position::from_valid_fen(std::string_view fen_string) noexcept {
    Position position;
    return position;
}

}  // namespace chess
