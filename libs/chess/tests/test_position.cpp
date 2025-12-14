#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
#include <wuttang/chess.hpp>

using namespace wuttang::chess;

class PositionTestBase : public testing::Test {
protected:
    void verify_bitboards(const Position& pos) {
        Bitboard expected_white_bb(0);
        Bitboard expected_black_bb(0);

        Bitboard type_bitboards[7];  // NONE=0 (unused), PAWN=1, etc.
        for (int i = 0; i < 7; ++i)
            type_bitboards[i] = Bitboard(0);

        for (const auto square : SquareRange{}) {
            const auto piece = pos.get_piece_at(square);
            if (piece == Piece::NONE) {
                continue;
            }

            const auto color = get_piece_color(piece);
            const auto type = get_piece_type(piece);

            EXPECT_TRUE(pos.get_occupancy(color).get(square))
                << "Occupancy for color " << (int) color << " missing at "
                << to_string(square);
            EXPECT_TRUE(pos.get_bitboard(type).get(square))
                << "Bitboard for type " << (int) type << " missing at "
                << to_string(square);
            EXPECT_TRUE(pos.get_bitboard(color, type).get(square))
                << "Bitboard for color/type missing at " << to_string(square);

            if (color == Color::WHITE)
                expected_white_bb.set(square);
            else if (color == Color::BLACK)
                expected_black_bb.set(square);

            type_bitboards[static_cast<int>(type)].set(square);
        }

        EXPECT_EQ(pos.get_occupancy(Color::WHITE), expected_white_bb);
        EXPECT_EQ(pos.get_occupancy(Color::BLACK), expected_black_bb);
        EXPECT_EQ(pos.get_occupancy(), expected_white_bb | expected_black_bb);

        for (int i = 1; i <= 6; ++i) {
            PieceType type = static_cast<PieceType>(i);
            EXPECT_EQ(pos.get_bitboard(type), type_bitboards[i])
                << "Mismatch for PieceType " << i;
        }
    }
};

class PositionFenTest : public PositionTestBase {
protected:
    struct ExpectedPosition {
        std::map<Square, Piece> pieces;
        Color side_to_move;
        bool white_kingside_castling;
        bool white_queenside_castling;
        bool black_kingside_castling;
        bool black_queenside_castling;
        Square en_passant_square;
        std::uint8_t halfmove_clock;
        std::uint16_t fullmove_number;
    };

    void verify_position(
        const Position& pos,
        const ExpectedPosition& expected
    ) {
        for (const auto square : SquareRange{}) {
            const auto piece_it = expected.pieces.find(square);
            const auto actual_piece = pos.get_piece_at(square);
            if (piece_it != expected.pieces.end()) {
                EXPECT_EQ(actual_piece, piece_it->second);
            } else {
                EXPECT_EQ(actual_piece, Piece::NONE);
            }
        }

        EXPECT_EQ(pos.get_side_to_move(), expected.side_to_move);

        EXPECT_EQ(
            pos.has_kingside_castling_rights(Color::WHITE),
            expected.white_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(Color::WHITE),
            expected.white_queenside_castling
        );
        EXPECT_EQ(
            pos.has_kingside_castling_rights(Color::BLACK),
            expected.black_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(Color::BLACK),
            expected.black_queenside_castling
        );

        EXPECT_EQ(pos.get_en_passant_square(), expected.en_passant_square);
        EXPECT_EQ(pos.get_halfmove_clock(), expected.halfmove_clock);
        EXPECT_EQ(pos.get_fullmove_number(), expected.fullmove_number);

        verify_bitboards(pos);
    }

    void verify_invalid_fen(
        std::string_view fen,
        std::string_view expected_msg
    ) {
        try {
            Position::from_fen(fen);
            FAIL() << "Expected std::invalid_argument for FEN: " << fen;
        } catch (const std::invalid_argument& e) {
            EXPECT_EQ(std::string(e.what()), expected_msg)
                << "Incorrect error message for FEN: " << fen;
        } catch (...) {
            FAIL()
                << "Expected std::invalid_argument, got unknown exception for FEN: "
                << fen;
        }
    }
};

TEST_F(PositionFenTest, StandardFen) {
    const auto pos = Position::standard();

    ExpectedPosition expected;
    expected.pieces = {
        {Square::A1, Piece::WHITE_ROOK},
        {Square::B1, Piece::WHITE_KNIGHT},
        {Square::C1, Piece::WHITE_BISHOP},
        {Square::D1, Piece::WHITE_QUEEN},
        {Square::E1, Piece::WHITE_KING},
        {Square::F1, Piece::WHITE_BISHOP},
        {Square::G1, Piece::WHITE_KNIGHT},
        {Square::H1, Piece::WHITE_ROOK},
        {Square::A2, Piece::WHITE_PAWN},
        {Square::B2, Piece::WHITE_PAWN},
        {Square::C2, Piece::WHITE_PAWN},
        {Square::D2, Piece::WHITE_PAWN},
        {Square::E2, Piece::WHITE_PAWN},
        {Square::F2, Piece::WHITE_PAWN},
        {Square::G2, Piece::WHITE_PAWN},
        {Square::H2, Piece::WHITE_PAWN},
        {Square::A7, Piece::BLACK_PAWN},
        {Square::B7, Piece::BLACK_PAWN},
        {Square::C7, Piece::BLACK_PAWN},
        {Square::D7, Piece::BLACK_PAWN},
        {Square::E7, Piece::BLACK_PAWN},
        {Square::F7, Piece::BLACK_PAWN},
        {Square::G7, Piece::BLACK_PAWN},
        {Square::H7, Piece::BLACK_PAWN},
        {Square::A8, Piece::BLACK_ROOK},
        {Square::B8, Piece::BLACK_KNIGHT},
        {Square::C8, Piece::BLACK_BISHOP},
        {Square::D8, Piece::BLACK_QUEEN},
        {Square::E8, Piece::BLACK_KING},
        {Square::F8, Piece::BLACK_BISHOP},
        {Square::G8, Piece::BLACK_KNIGHT},
        {Square::H8, Piece::BLACK_ROOK},
    };
    expected.side_to_move = Color::WHITE;
    expected.white_kingside_castling = true;
    expected.white_queenside_castling = true;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = true;
    expected.en_passant_square = Square::NO_SQ;
    expected.halfmove_clock = 0;
    expected.fullmove_number = 1;

    verify_position(pos, expected);

    EXPECT_EQ(pos.get_fen(), Position::STANDARD_STARTING_FEN);
}

TEST_F(PositionFenTest, CustomMiddleGameFen) {
    const auto fen =
        "3rk2r/pppqbppp/3pbn2/6B1/2BQPP2/2N5/PPP3PP/3R1RK1 b k f3 0 17";
    const auto pos = Position::from_fen(fen);

    ExpectedPosition expected;
    expected.pieces = {
        {Square::D1, Piece::WHITE_ROOK},
        {Square::F1, Piece::WHITE_ROOK},
        {Square::G1, Piece::WHITE_KING},
        {Square::A2, Piece::WHITE_PAWN},
        {Square::B2, Piece::WHITE_PAWN},
        {Square::C2, Piece::WHITE_PAWN},
        {Square::G2, Piece::WHITE_PAWN},
        {Square::H2, Piece::WHITE_PAWN},
        {Square::C3, Piece::WHITE_KNIGHT},
        {Square::C4, Piece::WHITE_BISHOP},
        {Square::D4, Piece::WHITE_QUEEN},
        {Square::E4, Piece::WHITE_PAWN},
        {Square::F4, Piece::WHITE_PAWN},
        {Square::G5, Piece::WHITE_BISHOP},
        {Square::D6, Piece::BLACK_PAWN},
        {Square::E6, Piece::BLACK_BISHOP},
        {Square::F6, Piece::BLACK_KNIGHT},
        {Square::A7, Piece::BLACK_PAWN},
        {Square::B7, Piece::BLACK_PAWN},
        {Square::C7, Piece::BLACK_PAWN},
        {Square::D7, Piece::BLACK_QUEEN},
        {Square::E7, Piece::BLACK_BISHOP},
        {Square::F7, Piece::BLACK_PAWN},
        {Square::G7, Piece::BLACK_PAWN},
        {Square::H7, Piece::BLACK_PAWN},
        {Square::D8, Piece::BLACK_ROOK},
        {Square::E8, Piece::BLACK_KING},
        {Square::H8, Piece::BLACK_ROOK},
    };
    expected.side_to_move = Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = false;
    expected.en_passant_square = Square::F3;
    expected.halfmove_clock = 0;
    expected.fullmove_number = 17;

    verify_position(pos, expected);

    EXPECT_EQ(pos.get_fen(), fen);
}

TEST_F(PositionFenTest, ParseCustomEngGameFen) {
    const auto fen = "1k6/8/2K5/1P6/8/8/8/8 b - - 56 83";
    const auto pos = Position::from_fen(fen);

    ExpectedPosition expected;
    expected.pieces = {
        {Square::B5, Piece::WHITE_PAWN},
        {Square::C6, Piece::WHITE_KING},
        {Square::B8, Piece::BLACK_KING},
    };
    expected.side_to_move = Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = false;
    expected.black_queenside_castling = false;
    expected.en_passant_square = Square::NO_SQ;
    expected.halfmove_clock = 56;
    expected.fullmove_number = 83;

    verify_position(pos, expected);

    EXPECT_EQ(pos.get_fen(), fen);
}

TEST_F(PositionFenTest, ThrowsOnInvalidFEN) {
    verify_invalid_fen(
        "invalid fen",
        "Invalid FEN: Invalid character in piece placement."
    );

    verify_invalid_fen(
        "rnbqkr/pppp4/7/7/8/8/4pppp/rnb w - - 0 1",
        "Invalid FEN: Rank does not have 8 squares."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R x KQkq - 0 1",
        "Invalid FEN: Invalid active color."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w ~*&% - 0 1",
        "Invalid FEN: Invalid castling rights character."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq i9 0 1",
        "Invalid FEN: Invalid en passant square."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - -7 1",
        "Invalid FEN: Invalid halfmove clock."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 192 1",
        "Invalid FEN: Invalid halfmove clock."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 0 0",
        "Invalid FEN: Invalid fullmove number."
    );

    verify_invalid_fen(
        "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 0 10532",
        "Invalid FEN: Invalid fullmove number."
    );

    verify_invalid_fen(
        "8/8/8/8/8/8/8/8 w - - 0 1",
        "Invalid FEN: Must have exactly one king per side."
    );

    verify_invalid_fen(
        "4k3/8/8/8/8/8/8/8 w - - 0 1",
        "Invalid FEN: Must have exactly one king per side."
    );

    verify_invalid_fen(
        "4K3/4K3/8/8/8/8/4k3/8 w - - 0 1",
        "Invalid FEN: Must have exactly one king per side."
    );

    verify_invalid_fen(
        "P7/8/8/8/8/8/8/4k2K w - - 0 1",
        "Invalid FEN: Pawn on rank 1 or 8."
    );

    verify_invalid_fen(
        "4k2K/8/8/8/8/8/8/p7 w - - 0 1",
        "Invalid FEN: Pawn on rank 1 or 8."
    );

    verify_invalid_fen(
        "8/8/8/8/8/8/4k3/4K3 w - - 0 1",
        "Illegal FEN: Opponent king is currently in check."
    );

    verify_invalid_fen(
        "4k3/8/8/8/8/8/8/4R1K1 w - - 0 1",
        "Illegal FEN: Opponent king is currently in check."
    );
}

TEST_F(PositionFenTest, SanitizesCastlingRights) {
    // 1. Missing White Kingside Rook
    // FEN: r3k2r/8/8/8/8/8/8/R3K3 w KQkq - 0 1 (Kings present, H1 rook missing)
    {
        const auto pos =
            Position::from_fen("r3k2r/8/8/8/8/8/8/R3K3 w KQkq - 0 1");
        EXPECT_FALSE(pos.has_kingside_castling_rights(Color::WHITE));
        EXPECT_TRUE(pos.has_queenside_castling_rights(Color::WHITE));
    }

    // 2. Missing Black Queenside Rook
    // FEN: 4k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1 (Kings present, A8 rook missing)
    {
        const auto pos =
            Position::from_fen("4k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        EXPECT_TRUE(pos.has_kingside_castling_rights(Color::BLACK));
        EXPECT_FALSE(pos.has_queenside_castling_rights(Color::BLACK));
    }

    // 3. King on wrong square (e.g., d1)
    // FEN: r3k2r/8/8/8/8/8/8/R2K3R w KQkq - 0 1
    {
        const auto pos =
            Position::from_fen("r3k2r/8/8/8/8/8/8/R2K3R w KQkq - 0 1");
        EXPECT_FALSE(pos.has_kingside_castling_rights(Color::WHITE));
        EXPECT_FALSE(pos.has_queenside_castling_rights(Color::WHITE));
    }
}

class PositionMoveTest : public PositionTestBase {
protected:
    void verify_make_undo(
        std::string_view start_fen,
        const Move& move,
        std::string_view expected_fen
    ) {
        auto pos = Position::from_fen(start_fen);
        const auto start_hash = pos.get_hash();

        verify_bitboards(pos);

        pos.make_move(move);
        EXPECT_EQ(pos.get_fen(), expected_fen)
            << "FEN mismatch after make_move for move: " << start_fen << " -> "
            << expected_fen;

        verify_bitboards(pos);

        // Even a null move changes the hash (side to move changes)
        EXPECT_NE(pos.get_hash(), start_hash)
            << "Hash shouldn't be the same after making a move";

        pos.undo_last_move();
        EXPECT_EQ(pos.get_fen(), start_fen)
            << "FEN mismatch after undo_last_move for move: " << start_fen;
        EXPECT_EQ(pos.get_hash(), start_hash)
            << "Hash should be the same after undoing a move";

        verify_bitboards(pos);
    }
};

TEST_F(PositionMoveTest, QuietMove) {
    // c7c6
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        Move::quiet(Square::C7, Square::C6),
        "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"
    );
    // Ng1f3
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        Move::quiet(Square::G1, Square::F3),
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1"
    );
    // e2e3
    verify_make_undo(
        "r1bqkbnr/pppppppp/2n5/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2",
        Move::quiet(Square::E2, Square::E3),
        "r1bqkbnr/pppppppp/2n5/8/8/4PN2/PPPP1PPP/RNBQKB1R b KQkq - 0 2"
    );
}

TEST_F(PositionMoveTest, CaptureMove) {
    // dxc4
    verify_make_undo(
        "r1bqkbnr/ppp1pppp/2n5/3p4/2P5/4PN2/PP1P1PPP/RNBQKB1R b KQkq c3 0 3",
        Move::capture(Square::D5, Square::C4, Piece::WHITE_PAWN),
        "r1bqkbnr/ppp1pppp/2n5/8/2p5/4PN2/PP1P1PPP/RNBQKB1R w KQkq - 0 4"
    );
}

TEST_F(PositionMoveTest, DoublePawnPushMove) {
    // d7d5
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
        Move::double_pawn_push(Square::D7, Square::D5),
        "rnbqkbnr/ppp1pppp/8/3p4/8/5N2/PPPPPPPP/RNBQKB1R w KQkq d6 0 2"
    );
}

TEST_F(PositionMoveTest, EnPassantMove) {
    // exf6
    verify_make_undo(
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        Move::en_passant(Square::E5, Square::F6),
        "rnbqkbnr/ppp1p1pp/5P2/3p4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3"
    );
}

TEST_F(PositionMoveTest, PromotionMove) {
    // a7a8q
    verify_make_undo(
        "8/P7/8/8/8/8/k7/7K w - - 5 23",
        Move::promotion(Square::A7, Square::A8, Piece::WHITE_QUEEN),
        "Q7/8/8/8/8/8/k7/7K b - - 0 23"
    );
}

TEST_F(PositionMoveTest, PromotionCaptureMove) {
    // axb8q
    verify_make_undo(
        "1n6/P7/8/8/8/8/k7/7K w - - 0 1",
        Move::promotion_capture(
            Square::A7,
            Square::B8,
            Piece::BLACK_KNIGHT,
            Piece::WHITE_QUEEN
        ),
        "1Q6/8/8/8/8/8/k7/7K b - - 0 1"
    );
}

TEST_F(PositionMoveTest, CastleKingsideMove) {
    // e1g1
    verify_make_undo(
        "rnbqk2r/pppp1ppp/5n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
        Move::castle_kingside(Square::E1, Square::G1),
        "rnbqk2r/pppp1ppp/5n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 5 4"
    );
}

TEST_F(PositionMoveTest, CastleQueensideMove) {
    // e1c1
    verify_make_undo(
        "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",
        Move::castle_queenside(Square::E1, Square::C1),
        "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/2KR3R b kq - 1 1"
    );
}

TEST_F(PositionMoveTest, NullMove) {
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        Move::null(),
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 1 1"
    );
}

TEST_F(PositionMoveTest, CastlingRightsUpdate) {
    // Rook move a1b1 -> Remove White Queenside
    verify_make_undo(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move::quiet(Square::A1, Square::B1),
        "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 1 1"
    );

    // Rook capture at h8 by White Rook -> Remove Black Kingside
    verify_make_undo(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move::capture(Square::H1, Square::H8, Piece::BLACK_ROOK),
        "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1"
    );

    // King move e1d1 -> Remove White Both
    verify_make_undo(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move::quiet(Square::E1, Square::D1),
        "r3k2r/8/8/8/8/8/8/R2K3R b kq - 1 1"
    );
}

TEST_F(PositionMoveTest, MultipleMovesAndUndos) {
    auto pos = Position::standard();
    std::vector<std::string> fens;
    fens.push_back(pos.get_fen());
    std::vector<std::uint64_t> hashes;
    hashes.push_back(pos.get_hash());

    // 1. e2e4
    auto m1 = Move::double_pawn_push(Square::E2, Square::E4);
    pos.make_move(m1);
    fens.push_back(pos.get_fen());
    hashes.push_back(pos.get_hash());
    EXPECT_EQ(
        fens.back(),
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    );
    EXPECT_NE(hashes.back(), hashes[0]);

    // 2. e7e5
    auto m2 = Move::double_pawn_push(Square::E7, Square::E5);
    pos.make_move(m2);
    fens.push_back(pos.get_fen());
    hashes.push_back(pos.get_hash());
    EXPECT_EQ(
        fens.back(),
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"
    );
    EXPECT_NE(hashes.back(), hashes[1]);

    // 3. g1f3
    auto m3 = Move::quiet(Square::G1, Square::F3);
    pos.make_move(m3);
    fens.push_back(pos.get_fen());
    hashes.push_back(pos.get_hash());
    EXPECT_EQ(
        fens.back(),
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
    );
    EXPECT_NE(hashes.back(), hashes[2]);

    // Check that all hashes are unique
    const std::set<std::uint64_t> unique_hashes(hashes.begin(), hashes.end());
    EXPECT_EQ(unique_hashes.size(), hashes.size())
        << "Hashes for different positions should be unique.";

    // Undo 3
    pos.undo_last_move();
    EXPECT_EQ(pos.get_fen(), fens[2]);
    EXPECT_EQ(pos.get_hash(), hashes[2]);

    // Undo 2
    pos.undo_last_move();
    EXPECT_EQ(pos.get_fen(), fens[1]);
    EXPECT_EQ(pos.get_hash(), hashes[1]);

    // Redo 2 (manual)
    pos.make_move(m2);
    EXPECT_EQ(pos.get_fen(), fens[2]);
    EXPECT_EQ(pos.get_hash(), hashes[2]);

    // Undo 2
    pos.undo_last_move();
    EXPECT_EQ(pos.get_fen(), fens[1]);
    EXPECT_EQ(pos.get_hash(), hashes[1]);

    // Undo 1
    pos.undo_last_move();
    EXPECT_EQ(pos.get_fen(), fens[0]);
    EXPECT_EQ(pos.get_hash(), hashes[0]);
}

TEST_F(PositionMoveTest, SamePositionSameHash) {
    auto pos = Position::standard();
    const auto start_hash = pos.get_hash();

    // 1. Ng1-f3
    pos.make_move(Move::quiet(Square::G1, Square::F3));
    // 2. Ng8-f6
    pos.make_move(Move::quiet(Square::G8, Square::F6));
    // 3. Nf3-g1
    pos.make_move(Move::quiet(Square::F3, Square::G1));
    // 4. Nf6-g8
    pos.make_move(Move::quiet(Square::F6, Square::G8));

    EXPECT_EQ(pos.get_hash(), start_hash)
        << "A sequence of moves returning to the starting position should "
           "result in the same hash.";
}

TEST(PositionTest, IsSquareAttacked) {
    // Setup:
    // White King at E1.
    // Black Rook at A1 (Attacks rank 1).
    // Black Knight at C3 (Attacks E2, A2, B1, D1).
    // White Pawn at E2.
    //
    // FEN: 7k/8/8/8/8/2n5/4P3/r3K3 w - - 0 1
    auto pos = Position::from_fen("7k/8/8/8/8/2n5/4P3/r3K3 w - - 0 1");

    // E1 is attacked by Rook at A1 (Rank 1 is clear between A1 and E1)
    EXPECT_TRUE(pos.is_square_attacked(Square::E1, Color::BLACK));

    // E2 is attacked by Knight at C3
    EXPECT_TRUE(pos.is_square_attacked(Square::E2, Color::BLACK));

    // D1 is attacked by Knight at C3
    EXPECT_TRUE(pos.is_square_attacked(Square::D1, Color::BLACK));

    // D1 is also attacked by Rook at A1
    // (We can't easily distinguish source, but it returns true)

    // F1 is attacked by King at E1
    EXPECT_TRUE(pos.is_square_attacked(Square::F1, Color::WHITE));

    // H1 is NOT attacked by anyone
    EXPECT_FALSE(pos.is_square_attacked(Square::H1, Color::BLACK));
    EXPECT_FALSE(pos.is_square_attacked(Square::H1, Color::WHITE));

    // Blocked ray test
    // Place a blocker at C1.
    // FEN: 7k/8/8/8/8/2n5/4P3/r1B1K3 w - - 0 1
    // White Bishop at C1 blocks A1-E1.
    auto pos_blocked =
        Position::from_fen("7k/8/8/8/8/2n5/4P3/r1B1K3 w - - 0 1");

    // E1 should NOT be attacked by Rook (blocked by C1)
    // But check other attackers... Knight at C3 attacks E2, not E1.
    // So E1 should be safe from BLACK.
    EXPECT_FALSE(pos_blocked.is_square_attacked(Square::E1, Color::BLACK));
}
