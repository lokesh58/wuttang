#include <cassert>
#include <charconv>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <wuttang/chess/attacks.hpp>
#include <wuttang/chess/castling_rights.hpp>
#include <wuttang/chess/color.hpp>
#include <wuttang/chess/move.hpp>
#include <wuttang/chess/piece.hpp>
#include <wuttang/chess/position.hpp>
#include <wuttang/chess/square.hpp>

#include "zobrist_hash.hpp"

namespace wuttang::chess {

Position::Position() noexcept :
        board_{},
        side_to_move_(Color::NONE),
        en_passant_square_(Square::NO_SQ),
        castling_rights_(CastlingRights::NONE),
        halfmove_clock_(0),
        initial_fullmove_number_(1),
        hash_(0),
        color_bitboards_{},
        piece_type_bitboards_{},
        castling_rights_mask_{},
        king_file_(File::FILE_INVALID),
        kingside_rook_file_(File::FILE_INVALID),
        queenside_rook_file_(File::FILE_INVALID) {
    board_.fill(Piece::NONE);
    history_.reserve(100);
};

Position Position::standard() {
    return from_fen(STANDARD_STARTING_FEN);
}

Position Position::from_fen(std::string_view fen_string) {
    Position position;
    auto it = fen_string.begin();
    auto end = fen_string.end();

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
    std::uint8_t rank_count = 0;
    std::uint8_t file_count = 0;
    std::uint8_t white_kings = 0;
    std::uint8_t black_kings = 0;

    for (char c : piece_placement) {
        if (c == '/') {
            if (file_count != 8) {
                throw std::invalid_argument(
                    "Invalid FEN: Rank does not have 8 squares."
                );
            }
            placement_rank = shift(placement_rank, -1);
            placement_file = File::FILE_A;
            file_count = 0;
            ++rank_count;
        } else if (std::isdigit(c)) {
            std::int8_t empty_squares = c - '0';
            file_count += empty_squares;
            if (file_count > 8) {
                throw std::invalid_argument(
                    "Invalid FEN: Rank exceeds 8 squares."
                );
            }
            placement_file = shift(placement_file, empty_squares);
        } else if (std::string_view("prnbqkPRNBQK").find(c) !=
                   std::string_view::npos) {
            if (file_count >= 8) {
                throw std::invalid_argument(
                    "Invalid FEN: Rank exceeds 8 squares."
                );
            }
            const auto piece = get_piece_from_char(c);

            if (piece == Piece::WHITE_KING)
                ++white_kings;
            else if (piece == Piece::BLACK_KING)
                ++black_kings;
            else if (get_piece_type(piece) == PieceType::PAWN) {
                if (placement_rank == Rank::RANK_1 ||
                    placement_rank == Rank::RANK_8) {
                    throw std::invalid_argument(
                        "Invalid FEN: Pawn on rank 1 or 8."
                    );
                }
            }

            const auto square =
                get_square_from_file_rank(placement_file, placement_rank);
            position.add_piece(square, piece);
            placement_file = shift(placement_file, 1);
            ++file_count;
        } else {
            throw std::invalid_argument(
                "Invalid FEN: Invalid character in piece placement."
            );
        }
    }

    if (rank_count != 7 || file_count != 8) {
        throw std::invalid_argument(
            "Invalid FEN: Board must have 8 ranks of 8 squares."
        );
    }
    if (white_kings != 1 || black_kings != 1) {
        throw std::invalid_argument(
            "Invalid FEN: Must have exactly one king per side."
        );
    }

    // 2. Active color
    std::string_view active_color = extract_part(' ');
    if (active_color == "w") {
        position.side_to_move_ = Color::WHITE;
    } else if (active_color == "b") {
        position.side_to_move_ = Color::BLACK;
        position.hash_ ^= ZobristHash::get_side_to_move_key();
    } else {
        throw std::invalid_argument("Invalid FEN: Invalid active color.");
    }

    // 3. Castling availability
    std::string_view castling_availability = extract_part(' ');
    if (castling_availability != "-") {
        for (char c : castling_availability) {
            switch (c) {
                case 'K':
                    position.add_castling_rights(
                        CastlingRights::WHITE_KINGSIDE
                    );
                    break;
                case 'Q':
                    position.add_castling_rights(
                        CastlingRights::WHITE_QUEENSIDE
                    );
                    break;
                case 'k':
                    position.add_castling_rights(
                        CastlingRights::BLACK_KINGSIDE
                    );
                    break;
                case 'q':
                    position.add_castling_rights(
                        CastlingRights::BLACK_QUEENSIDE
                    );
                    break;
                default:
                    throw std::invalid_argument(
                        "Invalid FEN: Invalid castling rights character."
                    );
            }
        }
    }

    // Setup castling files & mask
    position.king_file_ = File::FILE_E;
    position.kingside_rook_file_ = File::FILE_H;
    position.queenside_rook_file_ = File::FILE_A;

    position.castling_rights_mask_.fill(CastlingRights::ALL);
    const std::array<std::tuple<File, Rank, CastlingRights>, 6> overrides{{
        {position.get_king_file(), Rank::RANK_1, CastlingRights::WHITE_ALL},
        {position.get_kingside_rook_file(),
         Rank::RANK_1,
         CastlingRights::WHITE_KINGSIDE},
        {position.get_queenside_rook_file(),
         Rank::RANK_1,
         CastlingRights::WHITE_QUEENSIDE},
        {position.get_king_file(), Rank::RANK_8, CastlingRights::BLACK_ALL},
        {position.get_kingside_rook_file(),
         Rank::RANK_8,
         CastlingRights::BLACK_KINGSIDE},
        {position.get_queenside_rook_file(),
         Rank::RANK_8,
         CastlingRights::BLACK_QUEENSIDE},
    }};
    for (const auto& [file, rank, rights_to_mask] : overrides) {
        const auto square = get_square_from_file_rank(file, rank);
        position.set_castling_rights_mask(square, ~rights_to_mask);
    }

    // Sanitize castling rights
    if (position.has_castling_right(CastlingRights::WHITE_KINGSIDE)) {
        Square k =
            get_square_from_file_rank(position.get_king_file(), Rank::RANK_1);
        Square r = get_square_from_file_rank(
            position.get_kingside_rook_file(),
            Rank::RANK_1
        );
        if (position.get_piece_at(k) != Piece::WHITE_KING ||
            position.get_piece_at(r) != Piece::WHITE_ROOK) {
            position.remove_castling_rights(CastlingRights::WHITE_KINGSIDE);
        }
    }
    if (position.has_castling_right(CastlingRights::WHITE_QUEENSIDE)) {
        Square k =
            get_square_from_file_rank(position.get_king_file(), Rank::RANK_1);
        Square r = get_square_from_file_rank(
            position.get_queenside_rook_file(),
            Rank::RANK_1
        );
        if (position.get_piece_at(k) != Piece::WHITE_KING ||
            position.get_piece_at(r) != Piece::WHITE_ROOK) {
            position.remove_castling_rights(CastlingRights::WHITE_QUEENSIDE);
        }
    }
    if (position.has_castling_right(CastlingRights::BLACK_KINGSIDE)) {
        Square k =
            get_square_from_file_rank(position.get_king_file(), Rank::RANK_8);
        Square r = get_square_from_file_rank(
            position.get_kingside_rook_file(),
            Rank::RANK_8
        );
        if (position.get_piece_at(k) != Piece::BLACK_KING ||
            position.get_piece_at(r) != Piece::BLACK_ROOK) {
            position.remove_castling_rights(CastlingRights::BLACK_KINGSIDE);
        }
    }
    if (position.has_castling_right(CastlingRights::BLACK_QUEENSIDE)) {
        Square k =
            get_square_from_file_rank(position.get_king_file(), Rank::RANK_8);
        Square r = get_square_from_file_rank(
            position.get_queenside_rook_file(),
            Rank::RANK_8
        );
        if (position.get_piece_at(k) != Piece::BLACK_KING ||
            position.get_piece_at(r) != Piece::BLACK_ROOK) {
            position.remove_castling_rights(CastlingRights::BLACK_QUEENSIDE);
        }
    }

    position.hash_ ^=
        ZobristHash::get_castling_rights_key(position.castling_rights_);

    // 4. En passant target square
    std::string_view en_passant = extract_part(' ');
    if (en_passant != "-") {
        if (en_passant.size() != 2 || en_passant[0] < 'a' ||
            en_passant[0] > 'h' || en_passant[1] < '1' || en_passant[1] > '8') {
            throw std::invalid_argument(
                "Invalid FEN: Invalid en passant square."
            );
        }
        const auto file = static_cast<File>(en_passant[0] - 'a');
        const auto rank = static_cast<Rank>(en_passant[1] - '1');
        const auto ep_square = get_square_from_file_rank(file, rank);
        position.en_passant_square_ = ep_square;
        position.hash_ ^= ZobristHash::get_en_passant_key(ep_square);
    } else {
        position.en_passant_square_ = Square::NO_SQ;
    }

    // 5. Halfmove clock
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
        halfmove_clock < 0 || halfmove_clock > 150) {
        throw std::invalid_argument("Invalid FEN: Invalid halfmove clock.");
    }
    position.halfmove_clock_ = static_cast<std::uint8_t>(halfmove_clock);

    // 6. Fullmove number
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
        fullmove_number < 1 || fullmove_number > 9999) {
        throw std::invalid_argument("Invalid FEN: Invalid fullmove number.");
    }
    position.initial_fullmove_number_ =
        static_cast<std::uint16_t>(fullmove_number);

    // Check if the side NOT to move is in check (illegal position)
    const Color opponent = invert(position.side_to_move_);
    const Bitboard opponent_king_bb =
        position.get_bitboard(opponent, PieceType::KING);
    const Square opponent_king_sq = opponent_king_bb.lsb_square();

    if (position.is_square_attacked(opponent_king_sq, position.side_to_move_)) {
        throw std::invalid_argument(
            "Illegal FEN: Opponent king is currently in check."
        );
    }

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
                ++empty_squares;
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
    switch (move.get_type()) {
        case MoveType::QUIET:
            make_well_formed_move<MoveType::QUIET>(move);
            break;
        case MoveType::CAPTURE:
            make_well_formed_move<MoveType::CAPTURE>(move);
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            make_well_formed_move<MoveType::DOUBLE_PAWN_PUSH>(move);
            break;
        case MoveType::EN_PASSANT:
            make_well_formed_move<MoveType::EN_PASSANT>(move);
            break;
        case MoveType::PROMOTION:
            make_well_formed_move<MoveType::PROMOTION>(move);
            break;
        case MoveType::PROMOTION_CAPTURE:
            make_well_formed_move<MoveType::PROMOTION_CAPTURE>(move);
            break;
        case MoveType::CASTLE_KINGSIDE:
            make_well_formed_move<MoveType::CASTLE_KINGSIDE>(move);
            break;
        case MoveType::CASTLE_QUEENSIDE:
            make_well_formed_move<MoveType::CASTLE_QUEENSIDE>(move);
            break;
        case MoveType::NULL_MOVE:
            make_well_formed_move<MoveType::NULL_MOVE>(move);
            break;
    }
}

void Position::undo_last_move() noexcept {
    assert(history_.size() > 0);
    const auto& last_move = history_.back().move;
    switch (last_move.get_type()) {
        case MoveType::QUIET:
            undo_last_move<MoveType::QUIET>(last_move);
            break;
        case MoveType::CAPTURE:
            undo_last_move<MoveType::CAPTURE>(last_move);
            break;
        case MoveType::DOUBLE_PAWN_PUSH:
            undo_last_move<MoveType::DOUBLE_PAWN_PUSH>(last_move);
            break;
        case MoveType::EN_PASSANT:
            undo_last_move<MoveType::EN_PASSANT>(last_move);
            break;
        case MoveType::PROMOTION:
            undo_last_move<MoveType::PROMOTION>(last_move);
            break;
        case MoveType::PROMOTION_CAPTURE:
            undo_last_move<MoveType::PROMOTION_CAPTURE>(last_move);
            break;
        case MoveType::CASTLE_KINGSIDE:
            undo_last_move<MoveType::CASTLE_KINGSIDE>(last_move);
            break;
        case MoveType::CASTLE_QUEENSIDE:
            undo_last_move<MoveType::CASTLE_QUEENSIDE>(last_move);
            break;
        case MoveType::NULL_MOVE:
            undo_last_move<MoveType::NULL_MOVE>(last_move);
            break;
    }
    history_.pop_back();
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

template<MoveType Type>
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

    if constexpr (Type == MoveType::QUIET) {
        if (get_piece_type(get_piece_at(move.get_from_square())) ==
            PieceType::PAWN) {
            halfmove_clock_ = 0;
        } else {
            halfmove_clock_ += 1;
        }
        move_piece(move.get_from_square(), move.get_to_square());
        en_passant_square_ = Square::NO_SQ;
    } else if constexpr (Type == MoveType::CAPTURE) {
        remove_piece(move.get_to_square());
        move_piece(move.get_from_square(), move.get_to_square());
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ = 0;
    } else if constexpr (Type == MoveType::DOUBLE_PAWN_PUSH) {
        move_piece(move.get_from_square(), move.get_to_square());
        const auto offset = side_to_move_ == Color::WHITE ? -8 : 8;
        en_passant_square_ = shift(move.get_to_square(), offset);
        halfmove_clock_ = 0;
    } else if constexpr (Type == MoveType::EN_PASSANT) {
        move_piece(move.get_from_square(), move.get_to_square());
        const auto offset = side_to_move_ == Color::WHITE ? -8 : 8;
        remove_piece(shift(move.get_to_square(), offset));
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ = 0;
    } else if constexpr (Type == MoveType::PROMOTION) {
        remove_piece(move.get_from_square());
        add_piece(move.get_to_square(), move.get_promotion_piece());
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ = 0;
    } else if constexpr (Type == MoveType::PROMOTION_CAPTURE) {
        remove_piece(move.get_from_square());
        remove_piece(move.get_to_square());
        add_piece(move.get_to_square(), move.get_promotion_piece());
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ = 0;
    } else if constexpr (Type == MoveType::CASTLE_KINGSIDE) {
        move_piece(move.get_from_square(), move.get_to_square());
        move_piece(
            get_square_from_file_rank(
                get_kingside_rook_file(),
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            ),
            get_square_from_file_rank(
                File::FILE_F,
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            )
        );
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ += 1;
    } else if constexpr (Type == MoveType::CASTLE_QUEENSIDE) {
        move_piece(move.get_from_square(), move.get_to_square());
        move_piece(
            get_square_from_file_rank(
                get_queenside_rook_file(),
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            ),
            get_square_from_file_rank(
                File::FILE_D,
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            )
        );
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ += 1;
    } else if constexpr (Type == MoveType::NULL_MOVE) {
        en_passant_square_ = Square::NO_SQ;
        halfmove_clock_ += 1;
    }

    if constexpr (Type != MoveType::NULL_MOVE) {
        castling_rights_ &= get_castling_rights_mask(move.get_from_square()) &
                            get_castling_rights_mask(move.get_to_square());
    }

    if (en_passant_square_ != Square::NO_SQ) {
        hash_ ^= ZobristHash::get_en_passant_key(en_passant_square_);
    }
    hash_ ^= ZobristHash::get_castling_rights_key(castling_rights_);

    side_to_move_ = invert(side_to_move_);
    hash_ ^= ZobristHash::get_side_to_move_key();
}

template<MoveType Type>
void Position::undo_last_move(const Move& move) noexcept {
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

    if constexpr (Type == MoveType::QUIET) {
        move_piece(move.get_to_square(), move.get_from_square());
    } else if constexpr (Type == MoveType::CAPTURE) {
        move_piece(move.get_to_square(), move.get_from_square());
        add_piece(move.get_to_square(), move.get_captured_piece());
    } else if constexpr (Type == MoveType::DOUBLE_PAWN_PUSH) {
        move_piece(move.get_to_square(), move.get_from_square());
    } else if constexpr (Type == MoveType::EN_PASSANT) {
        const auto offset = side_to_move_ == Color::WHITE ? -8 : 8;
        add_piece(
            shift(move.get_to_square(), offset),
            get_piece_from_color_type(invert(side_to_move_), PieceType::PAWN)
        );
        move_piece(move.get_to_square(), move.get_from_square());
    } else if constexpr (Type == MoveType::PROMOTION) {
        remove_piece(move.get_to_square());
        add_piece(
            move.get_from_square(),
            get_piece_from_color_type(side_to_move_, PieceType::PAWN)
        );
    } else if constexpr (Type == MoveType::PROMOTION_CAPTURE) {
        remove_piece(move.get_to_square());
        add_piece(move.get_to_square(), move.get_captured_piece());
        add_piece(
            move.get_from_square(),
            get_piece_from_color_type(side_to_move_, PieceType::PAWN)
        );
    } else if constexpr (Type == MoveType::CASTLE_KINGSIDE) {
        move_piece(
            get_square_from_file_rank(
                File::FILE_F,
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            ),
            get_square_from_file_rank(
                get_kingside_rook_file(),
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            )
        );
        move_piece(move.get_to_square(), move.get_from_square());
    } else if constexpr (Type == MoveType::CASTLE_QUEENSIDE) {
        move_piece(
            get_square_from_file_rank(
                File::FILE_D,
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            ),
            get_square_from_file_rank(
                get_queenside_rook_file(),
                side_to_move_ == Color::WHITE ? Rank::RANK_1 : Rank::RANK_8
            )
        );
        move_piece(move.get_to_square(), move.get_from_square());
    } else if constexpr (Type == MoveType::NULL_MOVE) {
        // No board changes for null move
    }
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

bool Position::is_square_attacked(Square sq, Color attacker) const noexcept {
    return is_square_attacked(sq, attacker, get_occupancy(), Bitboard(~0ULL));
}

bool Position::is_square_attacked(
    Square sq,
    Color attacker,
    Bitboard occupancy,
    Bitboard attackers_mask
) const noexcept {
    // Pawn attacks
    if (Attacks::get_pawn_attacks(invert(attacker), sq) &
        get_bitboard(attacker, PieceType::PAWN) & attackers_mask) {
        return true;
    }

    // Knight attacks
    if (Attacks::get_knight_attacks(sq) &
        get_bitboard(attacker, PieceType::KNIGHT) & attackers_mask) {
        return true;
    }

    // King attacks
    if (Attacks::get_king_attacks(sq) &
        get_bitboard(attacker, PieceType::KING) & attackers_mask) {
        return true;
    }

    // Bishop / Queen attacks
    Bitboard bishop_queen = (get_bitboard(attacker, PieceType::BISHOP) |
                             get_bitboard(attacker, PieceType::QUEEN)) &
                            attackers_mask;
    if (bishop_queen &&
        (Attacks::get_bishop_attacks(sq, occupancy) & bishop_queen)) {
        return true;
    }

    // Rook / Queen attacks
    Bitboard rook_queen = (get_bitboard(attacker, PieceType::ROOK) |
                           get_bitboard(attacker, PieceType::QUEEN)) &
                          attackers_mask;
    if (rook_queen && (Attacks::get_rook_attacks(sq, occupancy) & rook_queen)) {
        return true;
    }

    return false;
}

}  // namespace wuttang::chess
