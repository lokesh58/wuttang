#include "chess/position.hpp"

#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <system_error>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/piece.hpp"
#include "chess/square.hpp"

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

Color Position::get_side_to_move() const noexcept {
    return side_to_move_;
}

bool Position::has_castling_right(
    CastlingRights castling_right
) const noexcept {
    return (castling_rights_ & castling_right) != CastlingRights::NONE;
}

bool Position::has_kingside_castling_rights(Color color) const noexcept {
    auto right = color == Color::WHITE ? CastlingRights::WHITE_KINGSIDE
                                       : CastlingRights::BLACK_KINGSIDE;
    return has_castling_right(right);
}

bool Position::has_queenside_castling_rights(Color color) const noexcept {
    auto right = color == Color::WHITE ? CastlingRights::WHITE_QUEENSIDE
                                       : CastlingRights::BLACK_QUEENSIDE;
    return has_castling_right(right);
}

std::optional<Square> Position::get_en_passant_square() const noexcept {
    return en_passant_square_;
}

std::uint8_t Position::get_halfmove_clock() const noexcept {
    return halfmove_clock_;
}

bool Position::is_valid_fen(std::string_view fen_string) noexcept {
    // Split the FEN string into parts using space as a delimiter
    auto it = fen_string.begin();
    auto end = fen_string.end();

    // Helper lambda to extract and validate a part until a delimiter
    auto extract_part = [&](char delimiter) -> std::string_view {
        auto start = it;
        while (it != end && *it != delimiter) {
            ++it;
        }
        std::string_view part(&*start, std::distance(start, it));
        if (it != end && *it == delimiter) {
            ++it;
        }
        return part;
    };

    // 1. Validate piece placement
    std::string_view piece_placement = extract_part(' ');
    std::uint8_t rank_count = 0;
    std::uint8_t file_count = 0;
    for (char c : piece_placement) {
        if (c == '/') {
            if (file_count != 8)
                return false;  // Each rank must have exactly 8 squares
            ++rank_count;
            file_count = 0;
        } else if (std::isdigit(c)) {
            file_count += c - '0';
        } else if (std::isalpha(c)) {
            if (std::string_view("prnbqkPRNBQK").find(c) ==
                std::string_view::npos)
                return false;
            ++file_count;
        } else {
            return false;  // Invalid character
        }
    }
    if (rank_count != 7 || file_count != 8)
        return false;  // Must have 8 ranks total

    // 2. Validate active color
    std::string_view active_color = extract_part(' ');
    if (active_color != "w" && active_color != "b")
        return false;

    // 3. Validate castling availability
    std::string_view castling_availability = extract_part(' ');
    if (castling_availability != "-" &&
        castling_availability.find_first_not_of("KQkq") !=
            std::string_view::npos)
        return false;

    // 4. Validate en passant target square
    std::string_view en_passant = extract_part(' ');
    if (en_passant != "-" &&
        (en_passant.size() != 2 || en_passant[0] < 'a' || en_passant[0] > 'h' ||
         en_passant[1] < '1' || en_passant[1] > '8'))
        return false;

    // 5. Validate halfmove clock
    std::string_view halfmove_clock_sv = extract_part(' ');
    int halfmove_clock;
    auto halfmove_clock_result = std::from_chars(
        halfmove_clock_sv.data(),
        halfmove_clock_sv.data() + halfmove_clock_sv.size(),
        halfmove_clock
    );
    if (halfmove_clock_result.ec != std::errc() ||
        halfmove_clock_result.ptr !=
            halfmove_clock_sv.data() + halfmove_clock_sv.size() ||
        halfmove_clock < 0 || halfmove_clock > 100)
        return false;

    return true;
}

Position Position::from_valid_fen(std::string_view fen_string) noexcept {
    Position position;
    auto it = fen_string.begin();
    auto end = fen_string.end();

    // Helper lambda to extract and validate a part until a delimiter
    auto extract_part = [&](char delimiter) -> std::string_view {
        auto start = it;
        while (it != end && *it != delimiter) {
            ++it;
        }
        std::string_view part(&*start, std::distance(start, it));
        if (it != end && *it == delimiter) {
            ++it;
        }
        return part;
    };

    // Helper lambda to get piece from char
    auto get_piece_type_from_fen_char = [](char piece_char) -> PieceType {
        switch (std::toupper(piece_char)) {
            case 'P':
                return PieceType::PAWN;
            case 'R':
                return PieceType::ROOK;
            case 'N':
                return PieceType::KNIGHT;
            case 'B':
                return PieceType::BISHOP;
            case 'Q':
                return PieceType::QUEEN;
            case 'K':
                return PieceType::KING;
            default:
                assert(false);
                // return a pawn by default to satisfy compiler
                return PieceType::PAWN;
        }
    };

    // 1. Piece placement
    std::string_view piece_placement = extract_part(' ');
    std::optional<Rank> placement_rank = Rank::RANK_8;
    std::optional<File> placement_file = File::FILE_A;
    for (char c : piece_placement) {
        if (c == '/') {
            placement_rank = shift(placement_rank, -1);
            placement_file = File::FILE_A;
        } else if (std::isdigit(c)) {
            std::size_t empty_squares = c - '0';
            placement_file = shift(placement_file, empty_squares);
        } else if (std::isalpha(c)) {
            Color piece_color = std::isupper(c) ? Color::WHITE : Color::BLACK;
            PieceType piece_type = get_piece_type_from_fen_char(c);
            position.board_[static_cast<std::size_t>(
                square_from_file_rank(*placement_file, *placement_rank)
            )] = get_piece(piece_color, piece_type);
            placement_file = shift(placement_file, 1);
        }
    }

    // 2. Active color
    std::string_view active_color = extract_part(' ');
    position.side_to_move_ =
        (active_color == "w") ? Color::WHITE : Color::BLACK;

    // 3. Castling availability
    std::string_view castling_availability = extract_part(' ');
    for (char c : castling_availability) {
        switch (c) {
            case 'K':
                position.castling_rights_ |= CastlingRights::WHITE_KINGSIDE;
                break;
            case 'Q':
                position.castling_rights_ |= CastlingRights::WHITE_QUEENSIDE;
                break;
            case 'k':
                position.castling_rights_ |= CastlingRights::BLACK_KINGSIDE;
                break;
            case 'q':
                position.castling_rights_ |= CastlingRights::BLACK_QUEENSIDE;
                break;
        }
    }

    // 4. En passant target square
    std::string_view en_passant = extract_part(' ');
    if (en_passant != "-") {
        const auto file = static_cast<File>(en_passant[0] - 'a');
        const auto rank = static_cast<Rank>(en_passant[1] - '1');
        position.en_passant_square_ = square_from_file_rank(file, rank);
    }

    // 5. Halfmove clock
    std::string_view halfmove_clock = extract_part(' ');
    std::from_chars(
        halfmove_clock.data(),
        halfmove_clock.data() + halfmove_clock.size(),
        position.halfmove_clock_
    );

    return position;
}

}  // namespace chess
