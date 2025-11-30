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

namespace chess {

Position::Position() noexcept :
        board_{},
        side_to_move_(Color::NONE),
        en_passant_square_(Square::NO_SQ),
        castling_rights_(CastlingRights::NONE),
        halfmove_clock_(0),
        initial_fullmove_number_(1) {
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
            auto piece = get_piece_from_char(c);
            position.set_piece_at(
                get_square_from_file_rank(placement_file, placement_rank),
                piece
            );
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

    // 4. En passant target square
    std::string_view en_passant = extract_part(' ');
    if (en_passant != "-") {
        const auto file = static_cast<File>(en_passant[0] - 'a');
        const auto rank = static_cast<Rank>(en_passant[1] - '1');
        position.en_passant_square_ = get_square_from_file_rank(file, rank);
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

void Position::make_move(const Move& move) {
    if (!is_valid_move(move)) {
        throw std::invalid_argument("Invalid Move");
    }
    do_make_move(move);
}

void Position::undo_last_move() {
    if (history_.empty()) {
        throw std::logic_error("No moves present in history");
    }
    do_undo_last_move();
}

bool Position::is_valid_move(const Move& move) const noexcept {
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

void Position::do_make_move(const Move& move) noexcept {
    const auto moved_piece = get_piece_at(move.get_from_square());

    const History new_history_entry{
        .move = move,
        .en_passant_square = en_passant_square_,
        .castling_rights = castling_rights_,
        .halfmove_clock = halfmove_clock_,
    };

    switch (move.get_type()) {
        case MoveType::QUIET:
            move_piece(move.get_from_square(), move.get_to_square());
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ += 1;
            break;
        case MoveType::CAPTURE:
            remove_piece(move.get_to_square());
            move_piece(move.get_from_square(), move.get_to_square());
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ = 0;
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            move_piece(move.get_from_square(), move.get_to_square());
            en_passant_square_ = get_square_from_file_rank(
                get_square_file(move.get_to_square()),
                side_to_move_ == Color::WHITE ? Rank::RANK_3 : Rank::RANK_6
            );
            halfmove_clock_ = 0;
            break;
        case MoveType::EN_PASSANT:
            move_piece(move.get_from_square(), move.get_to_square());
            remove_piece(get_square_from_file_rank(
                get_square_file(move.get_to_square()),
                side_to_move_ == Color::WHITE ? Rank::RANK_5 : Rank::RANK_4
            ));
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ = 0;
            break;
        case MoveType::PROMOTION:
            remove_piece(move.get_from_square());
            add_piece(move.get_to_square(), move.get_promotion_piece());
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ = 0;
            break;
        case MoveType::PROMOTION_CAPTURE:
            remove_piece(move.get_from_square());
            remove_piece(move.get_to_square());
            add_piece(move.get_to_square(), move.get_promotion_piece());
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ = 0;
            break;
        case MoveType::CASTLE_KINGSIDE:
            move_piece(move.get_from_square(), move.get_to_square());
            move_piece(
                get_square_from_file_rank(
                    File::FILE_H,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                get_square_from_file_rank(
                    File::FILE_F,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ += 1;
            break;
        case MoveType::CASTLE_QUEENSIDE:
            move_piece(move.get_from_square(), move.get_to_square());
            move_piece(
                get_square_from_file_rank(
                    File::FILE_A,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                get_square_from_file_rank(
                    File::FILE_D,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ += 1;
            break;
        case MoveType::NULL_MOVE:
            en_passant_square_ = Square::NO_SQ;
            halfmove_clock_ += 1;
            break;
    }

    const auto moved_piece_type = get_piece_type(moved_piece);
    switch (moved_piece_type) {
        case PieceType::KING: {
            const auto right_to_remove = side_to_move_ == Color::WHITE
                                             ? CastlingRights::WHITE_ALL
                                             : CastlingRights::BLACK_ALL;
            castling_rights_ &= ~right_to_remove;
        } break;
        case PieceType::ROOK: {
            const auto right_to_remove = [&]() {
                const auto rook_file = get_square_file(move.get_from_square());
                if (rook_file == File::FILE_A) {
                    return side_to_move_ == Color::WHITE
                               ? CastlingRights::WHITE_QUEENSIDE
                               : CastlingRights::BLACK_QUEENSIDE;
                } else if (rook_file == File::FILE_H) {
                    return side_to_move_ == Color::WHITE
                               ? CastlingRights::WHITE_KINGSIDE
                               : CastlingRights::BLACK_KINGSIDE;
                }
                return CastlingRights::NONE;
            }();
            castling_rights_ &= ~right_to_remove;
        } break;
        default:
            // No updates to castling rights
            break;
    }

    side_to_move_ = invert(side_to_move_);

    history_.push_back(new_history_entry);
}

void Position::do_undo_last_move() noexcept {
    const auto& last_history_entry = history_.back();

    side_to_move_ = invert(side_to_move_);
    en_passant_square_ = last_history_entry.en_passant_square;
    castling_rights_ = last_history_entry.castling_rights;
    halfmove_clock_ = last_history_entry.halfmove_clock;

    const auto& move = last_history_entry.move;
    switch (move.get_type()) {
        case MoveType::QUIET:
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::CAPTURE:
            move_piece(move.get_to_square(), move.get_from_square());
            add_piece(move.get_to_square(), move.get_captured_piece());
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::EN_PASSANT:
            add_piece(
                get_square_from_file_rank(
                    get_square_file(move.get_to_square()),
                    side_to_move_ == Color::WHITE ? Rank::RANK_5 : Rank::RANK_4
                ),
                get_piece_from_color_type(
                    invert(side_to_move_),
                    PieceType::PAWN
                )
            );
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::PROMOTION:
            remove_piece(move.get_to_square());
            add_piece(
                move.get_from_square(),
                get_piece_from_color_type(side_to_move_, PieceType::PAWN)
            );
            break;
        case MoveType::PROMOTION_CAPTURE:
            remove_piece(move.get_to_square());
            add_piece(move.get_to_square(), move.get_captured_piece());
            add_piece(
                move.get_from_square(),
                get_piece_from_color_type(side_to_move_, PieceType::PAWN)
            );
            break;
        case MoveType::CASTLE_KINGSIDE:
            move_piece(
                get_square_from_file_rank(
                    File::FILE_F,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                get_square_from_file_rank(
                    File::FILE_H,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::CASTLE_QUEENSIDE:
            move_piece(
                get_square_from_file_rank(
                    File::FILE_D,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                get_square_from_file_rank(
                    File::FILE_A,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::NULL_MOVE:
            break;
    }

    history_.pop_back();
}

void Position::add_piece(Square square, Piece piece) noexcept {
    set_piece_at(square, piece);
}

void Position::remove_piece(Square square) noexcept {
    set_piece_at(square, Piece::NONE);
}

void Position::move_piece(Square from_square, Square to_square) noexcept {
    const auto moving_piece = get_piece_at(from_square);
    set_piece_at(from_square, Piece::NONE);
    set_piece_at(to_square, moving_piece);
}

}  // namespace chess
