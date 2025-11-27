#include "chess/position.hpp"

#include <string>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <utils/optional.hpp>

#include "chess/castling_rights.hpp"
#include "chess/color.hpp"
#include "chess/move.hpp"
#include "chess/move_list.hpp"
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

void Position::set_piece_at(
    Square square,
    std::optional<Piece> piece
) noexcept {
    board_[static_cast<std::size_t>(square)] = piece;
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
            position.board_[static_cast<std::size_t>(square_from_file_rank(
                utils::unwrap(placement_file),
                utils::unwrap(placement_rank)
            ))] = get_piece(piece_color, piece_type);
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

void Position::make_move(const Move& move) {
    if (!is_valid_move(move)) {
        throw std::invalid_argument("Invalid Move");
    }

    const auto moved_piece =
        utils::unwrap(get_piece_at(move.get_from_square()));

    const History new_history_entry{
        .move = move,
        .en_passant_square = en_passant_square_,
        .castling_rights = castling_rights_,
        .halfmove_clock = halfmove_clock_,
    };

    switch (move.get_type()) {
        case MoveType::QUIET:
            move_piece(move.get_from_square(), move.get_to_square());
            en_passant_square_ = std::nullopt;
            halfmove_clock_ += 1;
            break;
        case MoveType::CAPTURE:
            remove_piece(move.get_to_square());
            move_piece(move.get_from_square(), move.get_to_square());
            en_passant_square_ = std::nullopt;
            halfmove_clock_ = 0;
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            move_piece(move.get_from_square(), move.get_to_square());
            en_passant_square_ = square_from_file_rank(
                get_square_file(move.get_to_square()),
                side_to_move_ == Color::WHITE ? Rank::RANK_3 : Rank::RANK_6
            );
            halfmove_clock_ = 0;
            break;
        case MoveType::EN_PASSANT:
            move_piece(move.get_from_square(), move.get_to_square());
            remove_piece(square_from_file_rank(
                get_square_file(move.get_to_square()),
                side_to_move_ == Color::WHITE ? Rank::RANK_5 : Rank::RANK_4
            ));
            en_passant_square_ = std::nullopt;
            halfmove_clock_ = 0;
            break;
        case MoveType::PROMOTION:
            remove_piece(move.get_from_square());
            add_piece(
                move.get_to_square(),
                utils::unwrap(move.get_promotion_piece())
            );
            en_passant_square_ = std::nullopt;
            halfmove_clock_ = 0;
            break;
        case MoveType::PROMOTION_CAPTURE:
            remove_piece(move.get_from_square());
            remove_piece(move.get_to_square());
            add_piece(
                move.get_to_square(),
                utils::unwrap(move.get_promotion_piece())
            );
            en_passant_square_ = std::nullopt;
            halfmove_clock_ = 0;
            break;
        case MoveType::CASTLE_KINGSIDE:
            move_piece(move.get_from_square(), move.get_to_square());
            move_piece(
                square_from_file_rank(
                    File::FILE_H,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                square_from_file_rank(
                    File::FILE_F,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            en_passant_square_ = std::nullopt;
            halfmove_clock_ += 1;
            break;
        case MoveType::CASTLE_QUEENSIDE:
            move_piece(move.get_from_square(), move.get_to_square());
            move_piece(
                square_from_file_rank(
                    File::FILE_A,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                square_from_file_rank(
                    File::FILE_D,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            en_passant_square_ = std::nullopt;
            halfmove_clock_ += 1;
            break;
        case MoveType::NULL_MOVE:
            en_passant_square_ = std::nullopt;
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

void Position::undo_last_move() {
    if (!history_.size()) {
        throw std::runtime_error("No moves to undo");
    }

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
            add_piece(
                move.get_to_square(),
                utils::unwrap(move.get_captured_piece())
            );
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::EN_PASSANT:
            add_piece(
                square_from_file_rank(
                    get_square_file(move.get_to_square()),
                    side_to_move_ == Color::WHITE ? Rank::RANK_5 : Rank::RANK_4
                ),
                get_piece(invert(side_to_move_), PieceType::PAWN)
            );
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::PROMOTION:
            remove_piece(move.get_to_square());
            add_piece(
                move.get_from_square(),
                get_piece(side_to_move_, PieceType::PAWN)
            );
            break;
        case MoveType::PROMOTION_CAPTURE:
            remove_piece(move.get_to_square());
            add_piece(
                move.get_to_square(),
                utils::unwrap(move.get_captured_piece())
            );
            add_piece(
                move.get_from_square(),
                get_piece(side_to_move_, PieceType::PAWN)
            );
            break;
        case MoveType::CASTLE_KINGSIDE:
            move_piece(
                square_from_file_rank(
                    File::FILE_F,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                square_from_file_rank(
                    File::FILE_H,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            move_piece(move.get_to_square(), move.get_from_square());
            break;
        case MoveType::CASTLE_QUEENSIDE:
            move_piece(
                square_from_file_rank(
                    File::FILE_D,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                square_from_file_rank(
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

bool Position::is_valid_move(const Move& move) const noexcept {
    const auto moving_piece = get_piece_at(move.get_from_square());
    if (!moving_piece)
        return false;
    if (get_piece_color(*moving_piece) != side_to_move_)
        return false;
    if (get_piece_at(move.get_to_square()) != move.get_captured_piece())
        return false;
    return true;
}

void Position::add_piece(Square square, Piece piece) noexcept {
    set_piece_at(square, piece);
}

void Position::remove_piece(Square square) noexcept {
    set_piece_at(square, std::nullopt);
}

void Position::move_piece(Square from_square, Square to_square) noexcept {
    const auto moving_piece = get_piece_at(from_square);
    set_piece_at(from_square, std::nullopt);
    set_piece_at(to_square, moving_piece);
}

MoveList Position::generate_legal_moves() const {
    MoveList pseudo_legal_moves = generate_pseudo_legal_moves();
    MoveList legal_moves;

    for (const auto& move : pseudo_legal_moves) {
        if (is_move_legal(move)) {
            legal_moves.push_back(move);
        }
    }

    return legal_moves;
}

void Position::generate_pawn_moves(MoveList& moves, Square start_square) const {
    const int direction = (side_to_move_ == Color::WHITE) ? 1 : -1;
    const Rank promotion_rank =
        (side_to_move_ == Color::WHITE) ? Rank::RANK_8 : Rank::RANK_1;
    const Rank start_rank =
        (side_to_move_ == Color::WHITE) ? Rank::RANK_2 : Rank::RANK_7;

    // Single push
    auto target_rank_ = static_cast<Rank>(
        static_cast<int>(get_square_rank(start_square)) + direction
    );
    auto one_step_square =
        square_from_file_rank(get_square_file(start_square), target_rank_);

    if (!get_piece_at(one_step_square)) {
        if (get_square_rank(one_step_square) == promotion_rank) {
            moves.push_back(
                Move::promotion(
                    start_square,
                    one_step_square,
                    get_piece(side_to_move_, PieceType::QUEEN)
                )
            );
            moves.push_back(
                Move::promotion(
                    start_square,
                    one_step_square,
                    get_piece(side_to_move_, PieceType::ROOK)
                )
            );
            moves.push_back(
                Move::promotion(
                    start_square,
                    one_step_square,
                    get_piece(side_to_move_, PieceType::BISHOP)
                )
            );
            moves.push_back(
                Move::promotion(
                    start_square,
                    one_step_square,
                    get_piece(side_to_move_, PieceType::KNIGHT)
                )
            );
        } else {
            moves.push_back(Move::quiet(start_square, one_step_square));
        }

        // Double push
        if (get_square_rank(start_square) == start_rank) {
            auto two_steps_rank = static_cast<Rank>(
                static_cast<int>(get_square_rank(start_square)) + 2 * direction
            );
            auto two_steps_square = square_from_file_rank(
                get_square_file(start_square),
                two_steps_rank
            );
            if (!get_piece_at(two_steps_square)) {
                moves.push_back(
                    Move::double_pawn_push(start_square, two_steps_square)
                );
            }
        }
    }

    // Captures
    for (int capture_direction : {-1, 1}) {
        auto target_file =
            shift(get_square_file(start_square), capture_direction);
        if (!target_file)
            continue;

        auto target_square = square_from_file_rank(*target_file, target_rank_);

        if (auto captured_piece = get_piece_at(target_square);
            captured_piece &&
            get_piece_color(*captured_piece) != side_to_move_) {
            if (get_square_rank(target_square) == promotion_rank) {
                moves.push_back(
                    Move::promotion_capture(
                        start_square,
                        target_square,
                        *captured_piece,
                        get_piece(side_to_move_, PieceType::QUEEN)
                    )
                );
                moves.push_back(
                    Move::promotion_capture(
                        start_square,
                        target_square,
                        *captured_piece,
                        get_piece(side_to_move_, PieceType::ROOK)
                    )
                );
                moves.push_back(
                    Move::promotion_capture(
                        start_square,
                        target_square,
                        *captured_piece,
                        get_piece(side_to_move_, PieceType::BISHOP)
                    )
                );
                moves.push_back(
                    Move::promotion_capture(
                        start_square,
                        target_square,
                        *captured_piece,
                        get_piece(side_to_move_, PieceType::KNIGHT)
                    )
                );
            } else {
                moves.push_back(
                    Move::capture(start_square, target_square, *captured_piece)
                );
            }
        } else if (target_square == en_passant_square_) {
            moves.push_back(Move::en_passant(start_square, target_square));
        }
    }
}

void Position::generate_knight_moves(
    MoveList& moves,
    Square start_square
) const {
    const std::pair<int, int> knight_moves[] = {
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2},
        {2, 1},
        {2, -1},
        {-2, 1},
        {-2, -1}
    };
    for (const auto& move : knight_moves) {
        const auto target_file =
            shift(get_square_file(start_square), move.first);
        const auto target_rank =
            shift(get_square_rank(start_square), move.second);

        if (target_file && target_rank) {
            const auto target_square =
                square_from_file_rank(*target_file, *target_rank);
            if (const auto& target_piece = get_piece_at(target_square);
                !target_piece) {
                moves.push_back(Move::quiet(start_square, target_square));
            } else if (get_piece_color(*target_piece) != side_to_move_) {
                moves.push_back(
                    Move::capture(start_square, target_square, *target_piece)
                );
            }
        }
    }
}

void Position::generate_bishop_moves(
    MoveList& moves,
    Square start_square
) const {
    const std::pair<int, int> directions[] =
        {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto& dir : directions) {
        for (int i = 1; i < 8; ++i) {
            const auto target_file =
                shift(get_square_file(start_square), dir.first * i);
            const auto target_rank =
                shift(get_square_rank(start_square), dir.second * i);

            if (target_file && target_rank) {
                const auto target_square =
                    square_from_file_rank(*target_file, *target_rank);
                if (const auto& target_piece = get_piece_at(target_square);
                    !target_piece) {
                    moves.push_back(Move::quiet(start_square, target_square));
                } else {
                    if (get_piece_color(*target_piece) != side_to_move_) {
                        moves.push_back(
                            Move::capture(
                                start_square,
                                target_square,
                                *target_piece
                            )
                        );
                    }
                    break;
                }
            } else {
                break;
            }
        }
    }
}

void Position::generate_rook_moves(MoveList& moves, Square start_square) const {
    const std::pair<int, int> directions[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto& dir : directions) {
        for (int i = 1; i < 8; ++i) {
            const auto target_file =
                shift(get_square_file(start_square), dir.first * i);
            const auto target_rank =
                shift(get_square_rank(start_square), dir.second * i);

            if (target_file && target_rank) {
                const auto target_square =
                    square_from_file_rank(*target_file, *target_rank);
                if (const auto& target_piece = get_piece_at(target_square);
                    !target_piece) {
                    moves.push_back(Move::quiet(start_square, target_square));
                } else {
                    if (get_piece_color(*target_piece) != side_to_move_) {
                        moves.push_back(
                            Move::capture(
                                start_square,
                                target_square,
                                *target_piece
                            )
                        );
                    }
                    break;
                }
            } else {
                break;
            }
        }
    }
}

void Position::generate_queen_moves(
    MoveList& moves,
    Square start_square
) const {
    generate_bishop_moves(moves, start_square);
    generate_rook_moves(moves, start_square);
}

void Position::generate_king_moves(MoveList& moves, Square start_square) const {
    const std::pair<int, int> king_moves[] =
        {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto& move : king_moves) {
        const auto target_file =
            shift(get_square_file(start_square), move.first);
        const auto target_rank =
            shift(get_square_rank(start_square), move.second);

        if (target_file && target_rank) {
            const auto target_square =
                square_from_file_rank(*target_file, *target_rank);
            if (const auto& target_piece = get_piece_at(target_square);
                !target_piece) {
                moves.push_back(Move::quiet(start_square, target_square));
            } else if (get_piece_color(*target_piece) != side_to_move_) {
                moves.push_back(
                    Move::capture(start_square, target_square, *target_piece)
                );
            }
        }
    }

    // Castling
    const Rank base_rank =
        (side_to_move_ == Color::WHITE) ? Rank::RANK_1 : Rank::RANK_8;
    if (get_square_rank(start_square) == base_rank &&
        get_square_file(start_square) == File::FILE_E) {
        // Kingside
        if (has_kingside_castling_rights(side_to_move_) &&
            !get_piece_at(square_from_file_rank(File::FILE_F, base_rank)) &&
            !get_piece_at(square_from_file_rank(File::FILE_G, base_rank)) &&
            !is_square_attacked(
                square_from_file_rank(File::FILE_E, base_rank),
                invert(side_to_move_)
            ) &&
            !is_square_attacked(
                square_from_file_rank(File::FILE_F, base_rank),
                invert(side_to_move_)
            ) &&
            !is_square_attacked(
                square_from_file_rank(File::FILE_G, base_rank),
                invert(side_to_move_)
            )) {
            moves.push_back(
                Move::castle_kingside(
                    start_square,
                    square_from_file_rank(File::FILE_G, base_rank)
                )
            );
        }

        // Queenside
        if (has_queenside_castling_rights(side_to_move_) &&
            !get_piece_at(square_from_file_rank(File::FILE_D, base_rank)) &&
            !get_piece_at(square_from_file_rank(File::FILE_C, base_rank)) &&
            !get_piece_at(square_from_file_rank(File::FILE_B, base_rank)) &&
            !is_square_attacked(
                square_from_file_rank(File::FILE_E, base_rank),
                invert(side_to_move_)
            ) &&
            !is_square_attacked(
                square_from_file_rank(File::FILE_D, base_rank),
                invert(side_to_move_)
            ) &&
            !is_square_attacked(
                square_from_file_rank(File::FILE_C, base_rank),
                invert(side_to_move_)
            )) {
            moves.push_back(
                Move::castle_queenside(
                    start_square,
                    square_from_file_rank(File::FILE_C, base_rank)
                )
            );
        }
    }
}

bool Position::is_move_legal(const Move& move) const {
    Board next_board = board_;

    const auto from_sq = move.get_from_square();
    const auto to_sq = move.get_to_square();

    auto move_piece_internal = [&](Square from, Square to) {
        next_board[static_cast<std::size_t>(to)] =
            next_board[static_cast<std::size_t>(from)];
        next_board[static_cast<std::size_t>(from)] = std::nullopt;
    };

    auto remove_piece_internal = [&](Square sq) {
        next_board[static_cast<std::size_t>(sq)] = std::nullopt;
    };

    auto add_piece_internal = [&](Square sq, Piece p) {
        next_board[static_cast<std::size_t>(sq)] = p;
    };

    switch (move.get_type()) {
        case MoveType::QUIET:
        case MoveType::DOUBLE_PAWN_PUSH:
            move_piece_internal(from_sq, to_sq);
            break;
        case MoveType::CAPTURE:
            move_piece_internal(from_sq, to_sq);
            break;
        case MoveType::EN_PASSANT:
            move_piece_internal(from_sq, to_sq);
            remove_piece_internal(square_from_file_rank(
                get_square_file(to_sq),
                side_to_move_ == Color::WHITE ? Rank::RANK_5 : Rank::RANK_4
            ));
            break;
        case MoveType::PROMOTION:
            remove_piece_internal(from_sq);
            add_piece_internal(
                to_sq,
                utils::unwrap(move.get_promotion_piece())
            );
            break;
        case MoveType::PROMOTION_CAPTURE:
            remove_piece_internal(from_sq);
            add_piece_internal(
                to_sq,
                utils::unwrap(move.get_promotion_piece())
            );
            break;
        case MoveType::CASTLE_KINGSIDE:
            move_piece_internal(from_sq, to_sq);
            move_piece_internal(
                square_from_file_rank(
                    File::FILE_H,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                square_from_file_rank(
                    File::FILE_F,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            break;
        case MoveType::CASTLE_QUEENSIDE:
            move_piece_internal(from_sq, to_sq);
            move_piece_internal(
                square_from_file_rank(
                    File::FILE_A,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                ),
                square_from_file_rank(
                    File::FILE_D,
                    side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
                )
            );
            break;
        case MoveType::NULL_MOVE:
            break;
    }

    return !is_king_in_check_internal(next_board, side_to_move_);
}

bool Position::is_king_in_check(Color king_color) const {
    return is_king_in_check_internal(board_, king_color);
}

bool Position::is_king_in_check_internal(const Board& board, Color king_color) {
    const auto king_square = find_king_internal(board, king_color);
    return king_square &&
           is_square_attacked_internal(board, *king_square, invert(king_color));
}

std::optional<Square> Position::find_king(Color king_color) const {
    return find_king_internal(board_, king_color);
}

std::optional<Square> Position::find_king_internal(
    const Board& board,
    Color king_color
) {
    for (const auto& sq : SquareRange{}) {
        const auto p = board[static_cast<std::size_t>(sq)];
        if (p && get_piece_type(*p) == PieceType::KING &&
            get_piece_color(*p) == king_color) {
            return sq;
        }
    }
    return std::nullopt;
}

bool Position::is_square_attacked(Square s, Color attacker_color) const {
    return is_square_attacked_internal(board_, s, attacker_color);
}

bool Position::is_square_attacked_internal(
    const Board& board,
    Square s,
    Color attacker_color
) {
    // Check for pawn attacks
    const int dir = (attacker_color == Color::WHITE) ? 1 : -1;
    for (int capture_dir : {-1, 1}) {
        const auto file = shift(get_square_file(s), capture_dir);
        const auto rank = shift(get_square_rank(s), -dir);
        if (file && rank) {
            const auto from_sq = square_from_file_rank(*file, *rank);
            if (const auto p = board[static_cast<std::size_t>(from_sq)];
                p && get_piece_color(*p) == attacker_color &&
                get_piece_type(*p) == PieceType::PAWN) {
                return true;
            }
        }
    }

    // Check for knight attacks
    const std::pair<int, int> knight_moves[] = {
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2},
        {2, 1},
        {2, -1},
        {-2, 1},
        {-2, -1}
    };
    for (const auto& m : knight_moves) {
        const auto file = shift(get_square_file(s), m.first);
        const auto rank = shift(get_square_rank(s), m.second);
        if (file && rank) {
            const auto from_sq = square_from_file_rank(*file, *rank);
            if (const auto p = board[static_cast<std::size_t>(from_sq)];
                p && get_piece_color(*p) == attacker_color &&
                get_piece_type(*p) == PieceType::KNIGHT) {
                return true;
            }
        }
    }

    // Check for sliding attacks (bishop, rook, queen)
    const std::pair<int, int> bishop_dirs[] =
        {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto& d : bishop_dirs) {
        for (int i = 1; i < 8; ++i) {
            const auto file = shift(get_square_file(s), d.first * i);
            const auto rank = shift(get_square_rank(s), d.second * i);
            if (file && rank) {
                const auto from_sq = square_from_file_rank(*file, *rank);
                if (const auto p = board[static_cast<std::size_t>(from_sq)]) {
                    if (get_piece_color(*p) == attacker_color &&
                        (get_piece_type(*p) == PieceType::BISHOP ||
                         get_piece_type(*p) == PieceType::QUEEN)) {
                        return true;
                    }
                    break;
                }
            } else {
                break;
            }
        }
    }

    const std::pair<int, int> rook_dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto& d : rook_dirs) {
        for (int i = 1; i < 8; ++i) {
            const auto file = shift(get_square_file(s), d.first * i);
            const auto rank = shift(get_square_rank(s), d.second * i);
            if (file && rank) {
                const auto from_sq = square_from_file_rank(*file, *rank);
                if (const auto p = board[static_cast<std::size_t>(from_sq)]) {
                    if (get_piece_color(*p) == attacker_color &&
                        (get_piece_type(*p) == PieceType::ROOK ||
                         get_piece_type(*p) == PieceType::QUEEN)) {
                        return true;
                    }
                    break;
                }
            } else {
                break;
            }
        }
    }

    // Check for king attacks
    const std::pair<int, int> king_moves[] =
        {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto& m : king_moves) {
        const auto file = shift(get_square_file(s), m.first);
        const auto rank = shift(get_square_rank(s), m.second);
        if (file && rank) {
            const auto from_sq = square_from_file_rank(*file, *rank);
            if (const auto p = board[static_cast<std::size_t>(from_sq)];
                p && get_piece_color(*p) == attacker_color &&
                get_piece_type(*p) == PieceType::KING) {
                return true;
            }
        }
    }

    return false;
}

MoveList Position::generate_pseudo_legal_moves() const {
    MoveList moves;
    for (const auto& square : SquareRange{}) {
        auto piece_opt = get_piece_at(square);
        if (!piece_opt.has_value()) {
            continue;
        }
        auto piece = piece_opt.value();
        auto piece_color = get_piece_color(piece);
        if (piece_color != side_to_move_) {
            continue;
        }
        auto piece_type = get_piece_type(piece);
        switch (piece_type) {
            case PieceType::PAWN:
                generate_pawn_moves(moves, square);
                break;
            case PieceType::KNIGHT:
                generate_knight_moves(moves, square);
                break;
            case PieceType::BISHOP:
                generate_bishop_moves(moves, square);
                break;
            case PieceType::ROOK:
                generate_rook_moves(moves, square);
                break;
            case PieceType::QUEEN:
                generate_queen_moves(moves, square);
                break;
            case PieceType::KING:
                generate_king_moves(moves, square);
                break;
        }
    }
    return moves;
}

std::string Position::get_fen() const noexcept {
    std::string fen;

    // 1. Piece placement
    for (auto rank : std::views::reverse(RankRange{})) {
        int empty_squares = 0;
        for (auto file : FileRange{}) {
            const auto piece = get_piece_at(square_from_file_rank(file, rank));
            if (piece) {
                if (empty_squares > 0) {
                    fen += std::to_string(empty_squares);
                    empty_squares = 0;
                }
                fen += get_piece_char(*piece);
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
    if (has_kingside_castling_rights(Color::WHITE)) castling_str += 'K';
    if (has_queenside_castling_rights(Color::WHITE)) castling_str += 'Q';
    if (has_kingside_castling_rights(Color::BLACK)) castling_str += 'k';
    if (has_queenside_castling_rights(Color::BLACK)) castling_str += 'q';
    fen += " " + (castling_str.empty() ? "-" : castling_str);

    // 4. En passant target square
    if (en_passant_square_) {
        fen += " " + to_string(*en_passant_square_);
    } else {
        fen += " -";
    }

    // 5. Halfmove clock
    fen += " " + std::to_string(halfmove_clock_);

    // 6. Fullmove number
    fen += " " + std::to_string(history_.size() / 2 + 1);

    return fen;
}

}  // namespace chess

