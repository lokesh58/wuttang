#include "chess/position.hpp"

#include <stdexcept>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"

namespace chess {

Position Position::standard() {
    return Position::from_fen(STANDARD_STARTING_FEN);
}

Position Position::from_fen(std::string_view) {
    // TODO: implement FEN parsing
    throw std::runtime_error("Not implemented");
}

Position::Position() noexcept:
        side_to_move_(Color::WHITE),
        castling_rights_(CastlingRights::NONE) {};

}  // namespace chess

