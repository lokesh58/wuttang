#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "chess/bitboard.hpp"
#include "chess/color.hpp"
#include "chess/piece.hpp"
#include "chess/position.hpp"
#include "chess/square.hpp"

class PositionTestBase : public testing::Test {
protected:
    void verify_bitboards(const chess::Position& pos) {
        chess::Bitboard expected_white_bb(0);
        chess::Bitboard expected_black_bb(0);

        chess::Bitboard type_bitboards[7];  // NONE=0 (unused), PAWN=1, etc.
        for (int i = 0; i < 7; ++i)
            type_bitboards[i] = chess::Bitboard(0);

        for (const auto square : chess::SquareRange{}) {
            const auto piece = pos.get_piece_at(square);
            if (piece == chess::Piece::NONE) {
                continue;
            }

            const auto color = chess::get_piece_color(piece);
            const auto type = chess::get_piece_type(piece);

            EXPECT_TRUE(pos.get_occupancy(color).get(square))
                << "Occupancy for color " << (int) color << " missing at "
                << to_string(square);
            EXPECT_TRUE(pos.get_bitboard(type).get(square))
                << "Bitboard for type " << (int) type << " missing at "
                << to_string(square);
            EXPECT_TRUE(pos.get_bitboard(color, type).get(square))
                << "Bitboard for color/type missing at " << to_string(square);

            if (color == chess::Color::WHITE)
                expected_white_bb.set(square);
            else if (color == chess::Color::BLACK)
                expected_black_bb.set(square);

            type_bitboards[static_cast<int>(type)].set(square);
        }

        EXPECT_EQ(pos.get_occupancy(chess::Color::WHITE), expected_white_bb);
        EXPECT_EQ(pos.get_occupancy(chess::Color::BLACK), expected_black_bb);
        EXPECT_EQ(pos.get_occupancy(), expected_white_bb | expected_black_bb);

        for (int i = 1; i <= 6; ++i) {
            chess::PieceType type = static_cast<chess::PieceType>(i);
            EXPECT_EQ(pos.get_bitboard(type), type_bitboards[i])
                << "Mismatch for PieceType " << i;
        }
    }
};

class PositionFenTest : public PositionTestBase {
protected:
    struct ExpectedPosition {
        std::map<chess::Square, chess::Piece> pieces;
        chess::Color side_to_move;
        bool white_kingside_castling;
        bool white_queenside_castling;
        bool black_kingside_castling;
        bool black_queenside_castling;
        chess::Square en_passant_square;
        std::uint8_t halfmove_clock;
        std::uint16_t fullmove_number;
    };

    void verify_position(
        const chess::Position& pos,
        const ExpectedPosition& expected
    ) {
        for (const auto square : chess::SquareRange{}) {
            const auto piece_it = expected.pieces.find(square);
            const auto actual_piece = pos.get_piece_at(square);
            if (piece_it != expected.pieces.end()) {
                EXPECT_EQ(actual_piece, piece_it->second);
            } else {
                EXPECT_EQ(actual_piece, chess::Piece::NONE);
            }
        }

        EXPECT_EQ(pos.get_side_to_move(), expected.side_to_move);

        EXPECT_EQ(
            pos.has_kingside_castling_rights(chess::Color::WHITE),
            expected.white_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(chess::Color::WHITE),
            expected.white_queenside_castling
        );
        EXPECT_EQ(
            pos.has_kingside_castling_rights(chess::Color::BLACK),
            expected.black_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(chess::Color::BLACK),
            expected.black_queenside_castling
        );

        EXPECT_EQ(pos.get_en_passant_square(), expected.en_passant_square);
        EXPECT_EQ(pos.get_halfmove_clock(), expected.halfmove_clock);
        EXPECT_EQ(pos.get_fullmove_number(), expected.fullmove_number);

        verify_bitboards(pos);
    }
};

TEST_F(PositionFenTest, StandardFen) {
    const auto pos = chess::Position::standard();

    ExpectedPosition expected;
    expected.pieces = {
        {chess::Square::A1, chess::Piece::WHITE_ROOK},
        {chess::Square::B1, chess::Piece::WHITE_KNIGHT},
        {chess::Square::C1, chess::Piece::WHITE_BISHOP},
        {chess::Square::D1, chess::Piece::WHITE_QUEEN},
        {chess::Square::E1, chess::Piece::WHITE_KING},
        {chess::Square::F1, chess::Piece::WHITE_BISHOP},
        {chess::Square::G1, chess::Piece::WHITE_KNIGHT},
        {chess::Square::H1, chess::Piece::WHITE_ROOK},
        {chess::Square::A2, chess::Piece::WHITE_PAWN},
        {chess::Square::B2, chess::Piece::WHITE_PAWN},
        {chess::Square::C2, chess::Piece::WHITE_PAWN},
        {chess::Square::D2, chess::Piece::WHITE_PAWN},
        {chess::Square::E2, chess::Piece::WHITE_PAWN},
        {chess::Square::F2, chess::Piece::WHITE_PAWN},
        {chess::Square::G2, chess::Piece::WHITE_PAWN},
        {chess::Square::H2, chess::Piece::WHITE_PAWN},
        {chess::Square::A7, chess::Piece::BLACK_PAWN},
        {chess::Square::B7, chess::Piece::BLACK_PAWN},
        {chess::Square::C7, chess::Piece::BLACK_PAWN},
        {chess::Square::D7, chess::Piece::BLACK_PAWN},
        {chess::Square::E7, chess::Piece::BLACK_PAWN},
        {chess::Square::F7, chess::Piece::BLACK_PAWN},
        {chess::Square::G7, chess::Piece::BLACK_PAWN},
        {chess::Square::H7, chess::Piece::BLACK_PAWN},
        {chess::Square::A8, chess::Piece::BLACK_ROOK},
        {chess::Square::B8, chess::Piece::BLACK_KNIGHT},
        {chess::Square::C8, chess::Piece::BLACK_BISHOP},
        {chess::Square::D8, chess::Piece::BLACK_QUEEN},
        {chess::Square::E8, chess::Piece::BLACK_KING},
        {chess::Square::F8, chess::Piece::BLACK_BISHOP},
        {chess::Square::G8, chess::Piece::BLACK_KNIGHT},
        {chess::Square::H8, chess::Piece::BLACK_ROOK},
    };
    expected.side_to_move = chess::Color::WHITE;
    expected.white_kingside_castling = true;
    expected.white_queenside_castling = true;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = true;
    expected.en_passant_square = chess::Square::NO_SQ;
    expected.halfmove_clock = 0;
    expected.fullmove_number = 1;

    verify_position(pos, expected);

    EXPECT_EQ(pos.get_fen(), chess::Position::STANDARD_STARTING_FEN);
}

TEST_F(PositionFenTest, CustomMiddleGameFen) {
    const auto fen =
        "3rk2r/pppqbppp/3pbn2/6B1/2BQPP2/2N5/PPP3PP/3R1RK1 b k f3 0 17";
    const auto pos = chess::Position::from_fen(fen);

    ExpectedPosition expected;
    expected.pieces = {
        {chess::Square::D1, chess::Piece::WHITE_ROOK},
        {chess::Square::F1, chess::Piece::WHITE_ROOK},
        {chess::Square::G1, chess::Piece::WHITE_KING},
        {chess::Square::A2, chess::Piece::WHITE_PAWN},
        {chess::Square::B2, chess::Piece::WHITE_PAWN},
        {chess::Square::C2, chess::Piece::WHITE_PAWN},
        {chess::Square::G2, chess::Piece::WHITE_PAWN},
        {chess::Square::H2, chess::Piece::WHITE_PAWN},
        {chess::Square::C3, chess::Piece::WHITE_KNIGHT},
        {chess::Square::C4, chess::Piece::WHITE_BISHOP},
        {chess::Square::D4, chess::Piece::WHITE_QUEEN},
        {chess::Square::E4, chess::Piece::WHITE_PAWN},
        {chess::Square::F4, chess::Piece::WHITE_PAWN},
        {chess::Square::G5, chess::Piece::WHITE_BISHOP},
        {chess::Square::D6, chess::Piece::BLACK_PAWN},
        {chess::Square::E6, chess::Piece::BLACK_BISHOP},
        {chess::Square::F6, chess::Piece::BLACK_KNIGHT},
        {chess::Square::A7, chess::Piece::BLACK_PAWN},
        {chess::Square::B7, chess::Piece::BLACK_PAWN},
        {chess::Square::C7, chess::Piece::BLACK_PAWN},
        {chess::Square::D7, chess::Piece::BLACK_QUEEN},
        {chess::Square::E7, chess::Piece::BLACK_BISHOP},
        {chess::Square::F7, chess::Piece::BLACK_PAWN},
        {chess::Square::G7, chess::Piece::BLACK_PAWN},
        {chess::Square::H7, chess::Piece::BLACK_PAWN},
        {chess::Square::D8, chess::Piece::BLACK_ROOK},
        {chess::Square::E8, chess::Piece::BLACK_KING},
        {chess::Square::H8, chess::Piece::BLACK_ROOK},
    };
    expected.side_to_move = chess::Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = false;
    expected.en_passant_square = chess::Square::F3;
    expected.halfmove_clock = 0;
    expected.fullmove_number = 17;

    verify_position(pos, expected);

    EXPECT_EQ(pos.get_fen(), fen);
}

TEST_F(PositionFenTest, ParseCustomEngGameFen) {
    const auto fen = "1k6/8/2K5/1P6/8/8/8/8 b - - 56 83";
    const auto pos = chess::Position::from_fen(fen);

    ExpectedPosition expected;
    expected.pieces = {
        {chess::Square::B5, chess::Piece::WHITE_PAWN},
        {chess::Square::C6, chess::Piece::WHITE_KING},
        {chess::Square::B8, chess::Piece::BLACK_KING},
    };
    expected.side_to_move = chess::Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = false;
    expected.black_queenside_castling = false;
    expected.en_passant_square = chess::Square::NO_SQ;
    expected.halfmove_clock = 56;
    expected.fullmove_number = 83;

    verify_position(pos, expected);

    EXPECT_EQ(pos.get_fen(), fen);
}

TEST_F(PositionFenTest, ThrowsOnInvalidFEN) {
    EXPECT_THROW(
        chess::Position::from_fen("invalid fen"),
        std::invalid_argument
    ) << "Invalid FEN format.";

    EXPECT_THROW(
        chess::Position::from_fen("rnbqkr/pppp4/7/7/8/8/4pppp/rnb w - - 0 1"),
        std::invalid_argument
    ) << "Invalid piece placement.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R x KQkq - 0 1"
        ),
        std::invalid_argument
    ) << "Invalid side to move.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w ~*&% - 0 1"
        ),
        std::invalid_argument
    ) << "Invalid castling rights.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq i9 0 1"
        ),
        std::invalid_argument
    ) << "Invalid en passant square.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - -7 1"
        ),
        std::invalid_argument
    ) << "Invalid halfmove clock (too low).";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 192 1"
        ),
        std::invalid_argument
    ) << "Invalid halfmove clock (too high).";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 0 0"
        ),
        std::invalid_argument
    ) << "Invalid fullmove number (too low).";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 0 10532"
        ),
        std::invalid_argument
    ) << "Invalid fullmove number (too high).";
}

class PositionMoveTest : public PositionTestBase {
protected:
    void verify_make_undo(
        std::string_view start_fen,
        const chess::Move& move,
        std::string_view expected_fen
    ) {
        auto pos = chess::Position::from_fen(start_fen);
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
        chess::Move::quiet(chess::Square::C7, chess::Square::C6),
        "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"
    );
    // Ng1f3
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        chess::Move::quiet(chess::Square::G1, chess::Square::F3),
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1"
    );
    // e2e3
    verify_make_undo(
        "r1bqkbnr/pppppppp/2n5/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2",
        chess::Move::quiet(chess::Square::E2, chess::Square::E3),
        "r1bqkbnr/pppppppp/2n5/8/8/4PN2/PPPP1PPP/RNBQKB1R b KQkq - 0 2"
    );
}

TEST_F(PositionMoveTest, CaptureMove) {
    // dxc4
    verify_make_undo(
        "r1bqkbnr/ppp1pppp/2n5/3p4/2P5/4PN2/PP1P1PPP/RNBQKB1R b KQkq c3 0 3",
        chess::Move::capture(
            chess::Square::D5,
            chess::Square::C4,
            chess::Piece::WHITE_PAWN
        ),
        "r1bqkbnr/ppp1pppp/2n5/8/2p5/4PN2/PP1P1PPP/RNBQKB1R w KQkq - 0 4"
    );
}

TEST_F(PositionMoveTest, DoublePawnPushMove) {
    // d7d5
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
        chess::Move::double_pawn_push(chess::Square::D7, chess::Square::D5),
        "rnbqkbnr/ppp1pppp/8/3p4/8/5N2/PPPPPPPP/RNBQKB1R w KQkq d6 0 2"
    );
}

TEST_F(PositionMoveTest, EnPassantMove) {
    // exf6
    verify_make_undo(
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        chess::Move::en_passant(chess::Square::E5, chess::Square::F6),
        "rnbqkbnr/ppp1p1pp/5P2/3p4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3"
    );
}

TEST_F(PositionMoveTest, PromotionMove) {
    // a7a8q
    verify_make_undo(
        "8/P7/8/8/8/8/k7/7K w - - 5 23",
        chess::Move::promotion(
            chess::Square::A7,
            chess::Square::A8,
            chess::Piece::WHITE_QUEEN
        ),
        "Q7/8/8/8/8/8/k7/7K b - - 0 23"
    );
}

TEST_F(PositionMoveTest, PromotionCaptureMove) {
    // axb8q
    verify_make_undo(
        "1n6/P7/8/8/8/8/k7/7K w - - 0 1",
        chess::Move::promotion_capture(
            chess::Square::A7,
            chess::Square::B8,
            chess::Piece::BLACK_KNIGHT,
            chess::Piece::WHITE_QUEEN
        ),
        "1Q6/8/8/8/8/8/k7/7K b - - 0 1"
    );
}

TEST_F(PositionMoveTest, CastleKingsideMove) {
    // e1g1
    verify_make_undo(
        "rnbqk2r/pppp1ppp/5n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
        chess::Move::castle_kingside(chess::Square::E1, chess::Square::G1),
        "rnbqk2r/pppp1ppp/5n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 5 4"
    );
}

TEST_F(PositionMoveTest, CastleQueensideMove) {
    // e1c1
    verify_make_undo(
        "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",
        chess::Move::castle_queenside(chess::Square::E1, chess::Square::C1),
        "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/2KR3R b kq - 1 1"
    );
}

TEST_F(PositionMoveTest, NullMove) {
    verify_make_undo(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        chess::Move::null(),
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 1 1"
    );
}

TEST_F(PositionMoveTest, CastlingRightsUpdate) {
    // Rook move a1b1 -> Remove White Queenside
    verify_make_undo(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        chess::Move::quiet(chess::Square::A1, chess::Square::B1),
        "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 1 1"
    );

    // Rook capture at h8 by White Rook -> Remove Black Kingside
    verify_make_undo(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        chess::Move::capture(
            chess::Square::H1,
            chess::Square::H8,
            chess::Piece::BLACK_ROOK
        ),
        "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1"
    );

    // King move e1d1 -> Remove White Both
    verify_make_undo(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        chess::Move::quiet(chess::Square::E1, chess::Square::D1),
        "r3k2r/8/8/8/8/8/8/R2K3R b kq - 1 1"
    );
}

TEST_F(PositionMoveTest, MultipleMovesAndUndos) {
    auto pos = chess::Position::standard();
    std::vector<std::string> fens;
    fens.push_back(pos.get_fen());
    std::vector<std::uint64_t> hashes;
    hashes.push_back(pos.get_hash());

    // 1. e2e4
    auto m1 =
        chess::Move::double_pawn_push(chess::Square::E2, chess::Square::E4);
    pos.make_move(m1);
    fens.push_back(pos.get_fen());
    hashes.push_back(pos.get_hash());
    EXPECT_EQ(
        fens.back(),
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    );
    EXPECT_NE(hashes.back(), hashes[0]);

    // 2. e7e5
    auto m2 =
        chess::Move::double_pawn_push(chess::Square::E7, chess::Square::E5);
    pos.make_move(m2);
    fens.push_back(pos.get_fen());
    hashes.push_back(pos.get_hash());
    EXPECT_EQ(
        fens.back(),
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"
    );
    EXPECT_NE(hashes.back(), hashes[1]);

    // 3. g1f3
    auto m3 = chess::Move::quiet(chess::Square::G1, chess::Square::F3);
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
    auto pos = chess::Position::standard();
    const auto start_hash = pos.get_hash();

    // 1. Ng1-f3
    pos.make_move(chess::Move::quiet(chess::Square::G1, chess::Square::F3));
    // 2. Ng8-f6
    pos.make_move(chess::Move::quiet(chess::Square::G8, chess::Square::F6));
    // 3. Nf3-g1
    pos.make_move(chess::Move::quiet(chess::Square::F3, chess::Square::G1));
    // 4. Nf6-g8
    pos.make_move(chess::Move::quiet(chess::Square::F6, chess::Square::G8));

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
    // FEN: 8/8/8/8/8/2n5/4P3/r3K3 w - - 0 1
    auto pos = chess::Position::from_fen("8/8/8/8/8/2n5/4P3/r3K3 w - - 0 1");

    // E1 is attacked by Rook at A1 (Rank 1 is clear between A1 and E1)
    EXPECT_TRUE(pos.is_square_attacked(chess::Square::E1, chess::Color::BLACK));

    // E2 is attacked by Knight at C3
    EXPECT_TRUE(pos.is_square_attacked(chess::Square::E2, chess::Color::BLACK));

    // D1 is attacked by Knight at C3
    EXPECT_TRUE(pos.is_square_attacked(chess::Square::D1, chess::Color::BLACK));

    // D1 is also attacked by Rook at A1
    // (We can't easily distinguish source, but it returns true)

    // F1 is attacked by King at E1
    EXPECT_TRUE(pos.is_square_attacked(chess::Square::F1, chess::Color::WHITE));

    // H1 is NOT attacked by anyone
    EXPECT_FALSE(
        pos.is_square_attacked(chess::Square::H1, chess::Color::BLACK)
    );
    EXPECT_FALSE(
        pos.is_square_attacked(chess::Square::H1, chess::Color::WHITE)
    );

    // Blocked ray test
    // Place a blocker at C1.
    // FEN: 8/8/8/8/8/2n5/4P3/r1B1K3 w - - 0 1
    // White Bishop at C1 blocks A1-E1.
    auto pos_blocked =
        chess::Position::from_fen("8/8/8/8/8/2n5/4P3/r1B1K3 w - - 0 1");

    // E1 should NOT be attacked by Rook (blocked by C1)
    // But check other attackers... Knight at C3 attacks E2, not E1.
    // So E1 should be safe from BLACK.
    EXPECT_FALSE(
        pos_blocked.is_square_attacked(chess::Square::E1, chess::Color::BLACK)
    );
}
