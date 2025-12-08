#include "chess/position.hpp"

#include <cassert>
#include <cctype>
#include <charconv>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/move.hpp"
#include "chess/piece.hpp"
#include "chess/square.hpp"
#include "chess/zobrist_hash.hpp"

namespace chess {

Position::Position() noexcept :
        board_{},
        side_to_move_(Color::NONE),
        en_passant_square_(Square::NO_SQ),
        castling_rights_(CastlingRights::NONE),
        halfmove_clock_(0),
        initial_fullmove_number_(1),
        hash_(0),
        color_bitboards_{},
        piece_type_bitboards_{} {
    board_.fill(Piece::NONE);
    history_.reserve(100);
};

Position Position::standard() noexcept {
    return from_valid_fen(STANDARD_STARTING_FEN);
}

Position Position::from_fen(std::string_view fen_string) {
    if (!is_valid_fen(fen_string)) {
        throw std::invalid_argument("Invalid FEN string");
    }
    return from_valid_fen(fen_string);
}

bool Position::is_valid_fen(std::string_view fen_string) noexcept {
    // Split the FEN string into parts using space as a delimiter
    auto it = fen_string.begin();
    auto end = fen_string.end();

    // Helper lambda to extract and validate a part until a delimiter
    auto extract_part = [&](char delimiter) {
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
        halfmove_clock < 0 || halfmove_clock > 150)
        return false;

    // 6. Validate fullmove number
    std::string_view fullmove_number_sv = extract_part(' ');
    int fullmove_number;
    auto fullmove_number_result = std::from_chars(
        fullmove_number_sv.data(),
        fullmove_number_sv.data() + fullmove_number_sv.size(),
        fullmove_number
    );
    if (fullmove_number_result.ec != std::errc() ||
        fullmove_number_result.ptr !=
            fullmove_number_sv.data() + fullmove_number_sv.size() ||
        fullmove_number < 1 || fullmove_number > 9999)
        return false;
    return true;
}

Position Position::from_valid_fen(std::string_view fen_string) noexcept {
    Position position;
    auto it = fen_string.begin();
    auto end = fen_string.end();

    // Helper lambda to extract and validate a part until a delimiter
    auto extract_part = [&](char delimiter) {
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

    // 1. Piece placement
    std::string_view piece_placement = extract_part(' ');
    Rank placement_rank = Rank::RANK_8;
    File placement_file = File::FILE_A;
    for (char c : piece_placement) {
        if (c == '/') {
            placement_rank = shift(placement_rank, -1);
            placement_file = File::FILE_A;
        } else if (std::isdigit(c)) {
            std::int8_t empty_squares = c - '0';
            placement_file = shift(placement_file, empty_squares);
        } else {
            const auto piece = get_piece_from_char(c);
            const auto square =
                get_square_from_file_rank(placement_file, placement_rank);
            position.add_piece(square, piece);
            placement_file = shift(placement_file, 1);
        }
    }

    // 2. Active color
    std::string_view active_color = extract_part(' ');
    if (active_color == "w") {
        position.side_to_move_ = Color::WHITE;
    } else {
        position.side_to_move_ = Color::BLACK;
        position.hash_ ^= ZobristHash::get_side_to_move_key();
    }

    // 3. Castling availability
    std::string_view castling_availability = extract_part(' ');
    for (char c : castling_availability) {
        switch (c) {
            case 'K':
                position.add_castling_rights(CastlingRights::WHITE_KINGSIDE);
                break;
            case 'Q':
                position.add_castling_rights(CastlingRights::WHITE_QUEENSIDE);
                break;
            case 'k':
                position.add_castling_rights(CastlingRights::BLACK_KINGSIDE);
                break;
            case 'q':
                position.add_castling_rights(CastlingRights::BLACK_QUEENSIDE);
                break;
        }
    }
    position.hash_ ^=
        ZobristHash::get_castling_rights_key(position.castling_rights_);

    // 4. En passant target square
    std::string_view en_passant = extract_part(' ');
    if (en_passant != "-") {
        const auto file = static_cast<File>(en_passant[0] - 'a');
        const auto rank = static_cast<Rank>(en_passant[1] - '1');
        const auto ep_square = get_square_from_file_rank(file, rank);
        position.en_passant_square_ = ep_square;
        position.hash_ ^= ZobristHash::get_en_passant_key(ep_square);
    } else {
        position.en_passant_square_ = Square::NO_SQ;
    }

    // 5. Halfmove clock
    std::string_view halfmove_clock = extract_part(' ');
    std::from_chars(
        halfmove_clock.data(),
        halfmove_clock.data() + halfmove_clock.size(),
        position.halfmove_clock_
    );

    // 6. Fullmove number
    std::string_view fullmove_number = extract_part(' ');
    std::from_chars(
        fullmove_number.data(),
        fullmove_number.data() + fullmove_number.size(),
        position.initial_fullmove_number_
    );

    return position;
}

std::string Position::get_fen() const noexcept {
    std::string fen;
    fen.reserve(90);

    // 1. Piece placement
    for (auto rank : std::views::reverse(RankRange{})) {
        int empty_squares = 0;
        for (auto file : FileRange{}) {
            const auto piece =
                get_piece_at(get_square_from_file_rank(file, rank));
            if (piece != Piece::NONE) {
                if (empty_squares > 0) {
                    fen += std::to_string(empty_squares);
                    empty_squares = 0;
                }
                fen += get_piece_char(piece);
            } else {
                empty_squares++;
            }
        }
        if (empty_squares > 0) {
            fen += std::to_string(empty_squares);
        }
        if (rank != Rank::RANK_1) {
            fen += '/';
        }
    }

    // 2. Active color
    fen += side_to_move_ == Color::WHITE ? " w" : " b";

    // 3. Castling availability
    std::string castling_str;
    if (has_kingside_castling_rights(Color::WHITE))
        castling_str += 'K';
    if (has_queenside_castling_rights(Color::WHITE))
        castling_str += 'Q';
    if (has_kingside_castling_rights(Color::BLACK))
        castling_str += 'k';
    if (has_queenside_castling_rights(Color::BLACK))
        castling_str += 'q';
    fen += " " + (castling_str.empty() ? "-" : castling_str);

    // 4. En passant target square
    if (en_passant_square_ != Square::NO_SQ) {
        fen += " " + to_string(en_passant_square_);
    } else {
        fen += " -";
    }

    // 5. Halfmove clock
    fen += " " + std::to_string(halfmove_clock_);

    // 6. Fullmove number
    fen += " " + std::to_string(get_fullmove_number());

    return fen;
}

void Position::make_move(const Move& move) noexcept {
    assert(is_well_formed_move(move));
    make_well_formed_move(move);
}

void Position::undo_last_move() noexcept {
    assert(history_.size() > 0);
    undo_last_move_with_non_empty_history();
}

bool Position::is_well_formed_move(const Move& move) const noexcept {
    if (move.get_type() == MoveType::NULL_MOVE)
        return true;
    const auto moving_piece = get_piece_at(move.get_from_square());
    if (moving_piece == Piece::NONE)
        return false;
    if (get_piece_color(moving_piece) != side_to_move_)
        return false;
    if (get_piece_at(move.get_to_square()) != move.get_captured_piece())
        return false;
    return true;
}

void Position::make_well_formed_move(const Move& move) noexcept {
    history_.push_back({
        .move = move,
        .en_passant_square = en_passant_square_,
        .castling_rights = castling_rights_,
        .halfmove_clock = halfmove_clock_,
        .hash = hash_,
    });

    if (en_passant_square_ != Square::NO_SQ) {
        hash_ ^= ZobristHash::get_en_passant_key(en_passant_square_);
    }
    hash_ ^= ZobristHash::get_castling_rights_key(castling_rights_);

    switch (move.get_type()) {
        case MoveType::QUIET:
            make_quiet_move(move);
            break;
        case MoveType::CAPTURE:
            make_capture_move(move);
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            make_double_pawn_push_move(move);
            break;
        case MoveType::EN_PASSANT:
            make_en_passant_move(move);
            break;
        case MoveType::PROMOTION:
            make_promotion_move(move);
            break;
        case MoveType::PROMOTION_CAPTURE:
            make_promotion_capture_move(move);
            break;
        case MoveType::CASTLE_KINGSIDE:
            make_castle_kingside_move(move);
            break;
        case MoveType::CASTLE_QUEENSIDE:
            make_castle_queenside_move(move);
            break;
        case MoveType::NULL_MOVE:
            make_null_move();
            break;
    }

    if (move.get_type() != MoveType::NULL_MOVE) {
        castling_rights_ &= CASTLING_RIGHTS_MASK[static_cast<std::size_t>(
                                move.get_from_square()
                            )] &
                            CASTLING_RIGHTS_MASK[static_cast<std::size_t>(
                                move.get_to_square()
                            )];
    }

    if (en_passant_square_ != Square::NO_SQ) {
        hash_ ^= ZobristHash::get_en_passant_key(en_passant_square_);
    }
    hash_ ^= ZobristHash::get_castling_rights_key(castling_rights_);

    side_to_move_ = invert(side_to_move_);
    hash_ ^= ZobristHash::get_side_to_move_key();
}

void Position::undo_last_move_with_non_empty_history() noexcept {
    const auto& last_history_entry = history_.back();

    if (en_passant_square_ != Square::NO_SQ) {
        hash_ ^= ZobristHash::get_en_passant_key(en_passant_square_);
    }
    hash_ ^= ZobristHash::get_castling_rights_key(castling_rights_);

    en_passant_square_ = last_history_entry.en_passant_square;
    castling_rights_ = last_history_entry.castling_rights;
    halfmove_clock_ = last_history_entry.halfmove_clock;

    if (en_passant_square_ != Square::NO_SQ) {
        hash_ ^= ZobristHash::get_en_passant_key(en_passant_square_);
    }
    hash_ ^= ZobristHash::get_castling_rights_key(castling_rights_);

    side_to_move_ = invert(side_to_move_);
    hash_ ^= ZobristHash::get_side_to_move_key();

    const auto& move = last_history_entry.move;
    switch (move.get_type()) {
        case MoveType::QUIET:
            undo_quiet_move(move);
            break;
        case MoveType::CAPTURE:
            undo_capture_move(move);
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            undo_double_pawn_push_move(move);
            break;
        case MoveType::EN_PASSANT:
            undo_en_passant_move(move);
            break;
        case MoveType::PROMOTION:
            undo_promotion_move(move);
            break;
        case MoveType::PROMOTION_CAPTURE:
            undo_promotion_capture_move(move);
            break;
        case MoveType::CASTLE_KINGSIDE:
            undo_castle_kingside_move(move);
            break;
        case MoveType::CASTLE_QUEENSIDE:
            undo_castle_queenside_move(move);
            break;
        case MoveType::NULL_MOVE:
            undo_null_move();
            break;
    }

    history_.pop_back();
}

void Position::make_quiet_move(const Move& move) noexcept {
    if (get_piece_type(get_piece_at(move.get_from_square())) ==
        PieceType::PAWN) {
        halfmove_clock_ = 0;
    } else {
        halfmove_clock_ += 1;
    }
    move_piece(move.get_from_square(), move.get_to_square());
    en_passant_square_ = Square::NO_SQ;
}

void Position::undo_quiet_move(const Move& move) noexcept {
    move_piece(move.get_to_square(), move.get_from_square());
}

void Position::make_capture_move(const Move& move) noexcept {
    remove_piece(move.get_to_square());
    move_piece(move.get_from_square(), move.get_to_square());
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ = 0;
}

void Position::undo_capture_move(const Move& move) noexcept {
    move_piece(move.get_to_square(), move.get_from_square());
    add_piece(move.get_to_square(), move.get_captured_piece());
}

void Position::make_double_pawn_push_move(const Move& move) noexcept {
    move_piece(move.get_from_square(), move.get_to_square());
    const auto offset = side_to_move_ == Color::WHITE ? -8 : 8;
    en_passant_square_ = shift(move.get_to_square(), offset);
    halfmove_clock_ = 0;
}

void Position::undo_double_pawn_push_move(const Move& move) noexcept {
    move_piece(move.get_to_square(), move.get_from_square());
}

void Position::make_en_passant_move(const Move& move) noexcept {
    move_piece(move.get_from_square(), move.get_to_square());
    const auto offset = side_to_move_ == Color::WHITE ? -8 : 8;
    remove_piece(shift(move.get_to_square(), offset));
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ = 0;
}

void Position::undo_en_passant_move(const Move& move) noexcept {
    const auto offset = side_to_move_ == Color::WHITE ? -8 : 8;
    add_piece(
        shift(move.get_to_square(), offset),
        get_piece_from_color_type(invert(side_to_move_), PieceType::PAWN)
    );
    move_piece(move.get_to_square(), move.get_from_square());
}

void Position::make_promotion_move(const Move& move) noexcept {
    remove_piece(move.get_from_square());
    add_piece(move.get_to_square(), move.get_promotion_piece());
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ = 0;
}

void Position::undo_promotion_move(const Move& move) noexcept {
    remove_piece(move.get_to_square());
    add_piece(
        move.get_from_square(),
        get_piece_from_color_type(side_to_move_, PieceType::PAWN)
    );
}

void Position::make_promotion_capture_move(const Move& move) noexcept {
    remove_piece(move.get_from_square());
    remove_piece(move.get_to_square());
    add_piece(move.get_to_square(), move.get_promotion_piece());
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ = 0;
}

void Position::undo_promotion_capture_move(const Move& move) noexcept {
    remove_piece(move.get_to_square());
    add_piece(move.get_to_square(), move.get_captured_piece());
    add_piece(
        move.get_from_square(),
        get_piece_from_color_type(side_to_move_, PieceType::PAWN)
    );
}

void Position::make_castle_kingside_move(const Move& move) noexcept {
    move_piece(move.get_from_square(), move.get_to_square());
    move_piece(
        get_square_from_file_rank(
            KINGSIDE_ROOK_FILE,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        ),
        get_square_from_file_rank(
            File::FILE_F,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        )
    );
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ += 1;
}

void Position::undo_castle_kingside_move(const Move& move) noexcept {
    move_piece(
        get_square_from_file_rank(
            File::FILE_F,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        ),
        get_square_from_file_rank(
            KINGSIDE_ROOK_FILE,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        )
    );
    move_piece(move.get_to_square(), move.get_from_square());
}

void Position::make_castle_queenside_move(const Move& move) noexcept {
    move_piece(move.get_from_square(), move.get_to_square());
    move_piece(
        get_square_from_file_rank(
            QUEENSIDE_ROOK_FILE,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        ),
        get_square_from_file_rank(
            File::FILE_D,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        )
    );
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ += 1;
}

void Position::undo_castle_queenside_move(const Move& move) noexcept {
    move_piece(
        get_square_from_file_rank(
            File::FILE_D,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        ),
        get_square_from_file_rank(
            QUEENSIDE_ROOK_FILE,
            side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
        )
    );
    move_piece(move.get_to_square(), move.get_from_square());
}

void Position::make_null_move() noexcept {
    en_passant_square_ = Square::NO_SQ;
    halfmove_clock_ += 1;
}

void Position::undo_null_move() noexcept {
    // No board changes for null move
}

void Position::add_piece(Square square, Piece piece) noexcept {
    set_piece_at(square, piece);
    hash_ ^= ZobristHash::get_piece_square_key(piece, square);

    bitboard_of(get_piece_color(piece)).set(square);
    bitboard_of(get_piece_type(piece)).set(square);
}

void Position::remove_piece(Square square) noexcept {
    const auto piece = get_piece_at(square);
    assert(piece != Piece::NONE);
    set_piece_at(square, Piece::NONE);
    hash_ ^= ZobristHash::get_piece_square_key(piece, square);

    bitboard_of(get_piece_color(piece)).clear(square);
    bitboard_of(get_piece_type(piece)).clear(square);
}

void Position::move_piece(Square from_square, Square to_square) noexcept {
    const auto moving_piece = get_piece_at(from_square);
    set_piece_at(from_square, Piece::NONE);
    set_piece_at(to_square, moving_piece);
    hash_ ^= ZobristHash::get_piece_square_key(moving_piece, from_square);
    hash_ ^= ZobristHash::get_piece_square_key(moving_piece, to_square);

    const auto move_mask =
        Bitboard::from_square(from_square) | Bitboard::from_square(to_square);
    bitboard_of(get_piece_color(moving_piece)) ^= move_mask;
    bitboard_of(get_piece_type(moving_piece)) ^= move_mask;
}

}  // namespace chess
