#include <gtest/gtest.h>

#include "chess/move_generator.hpp"
#include "chess/position.hpp"
#include "chess/square.hpp"

TEST(MoveGenerator, StandardPositionLegalMoves) {
    auto pos = chess::Position::standard();
    chess::MoveList moves;
    chess::MoveGenerator::generate<chess::MoveGenerationType::LEGAL>(
        pos,
        moves
    );
    EXPECT_EQ(moves.size(), 20);
}

TEST(MoveGenerator, Checkmate) {
    // Fools mate pattern
    auto pos = chess::Position::from_fen(
        "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3"
    );
    chess::MoveList moves;
    chess::MoveGenerator::generate<chess::MoveGenerationType::LEGAL>(
        pos,
        moves
    );
    EXPECT_EQ(moves.size(), 0);
}

TEST(MoveGenerator, Stalemate) {
    // Stalemate position: Black king at h8, White Queen at g6, White King at f7
    auto pos_stalemate =
        chess::Position::from_fen("7k/5K2/6Q1/8/8/8/8/8 b - - 0 1");
    chess::MoveList moves;
    chess::MoveGenerator::generate<chess::MoveGenerationType::LEGAL>(
        pos_stalemate,
        moves
    );
    EXPECT_EQ(moves.size(), 0);
}

TEST(MoveGenerator, CastlingBothSides) {
    // White can castle kingside and queenside
    // R...K..R
    auto pos =
        chess::Position::from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    chess::MoveList moves;
    chess::MoveGenerator::generate<chess::MoveGenerationType::LEGAL>(
        pos,
        moves
    );

    bool has_kingside = false;
    bool has_queenside = false;
    for (const auto& m : moves) {
        if (m.get_type() == chess::MoveType::CASTLE_KINGSIDE)
            has_kingside = true;
        if (m.get_type() == chess::MoveType::CASTLE_QUEENSIDE)
            has_queenside = true;
    }
    EXPECT_TRUE(has_kingside);
    EXPECT_TRUE(has_queenside);
}

TEST(MoveGenerator, CastlingBlocked) {
    // White kingside blocked by bishop
    auto pos =
        chess::Position::from_fen("r3k2r/8/8/8/8/8/8/R3KB1R w KQkq - 0 1");
    chess::MoveList moves;
    chess::MoveGenerator::generate<chess::MoveGenerationType::LEGAL>(
        pos,
        moves
    );

    bool has_kingside = false;
    bool has_queenside = false;
    for (const auto& m : moves) {
        if (m.get_type() == chess::MoveType::CASTLE_KINGSIDE)
            has_kingside = true;
        if (m.get_type() == chess::MoveType::CASTLE_QUEENSIDE)
            has_queenside = true;
    }
    EXPECT_FALSE(has_kingside);
    EXPECT_TRUE(has_queenside);
}

TEST(MoveGenerator, PinnedPiece) {
    // White king at e1, White pawn at d2, Black Bishop a5.
    auto pos_pin = chess::Position::from_fen("8/8/8/b7/8/8/3P4/4K3 w - - 0 1");
    // d2 pawn cannot push (d2-d3) or double push (d2-d4) because it would expose K to B.
    chess::MoveList pinned_moves;
    chess::MoveGenerator::generate<chess::MoveGenerationType::LEGAL>(
        pos_pin,
        pinned_moves
    );

    for (const auto& m : pinned_moves) {
        if (m.get_from_square() == chess::Square::D2) {
            // Should not be able to move
            EXPECT_TRUE(false) << "Pinned pawn at d2 moved to "
                               << chess::to_string(m.get_to_square());
        }
    }
}

TEST(MoveGenerator, IsLegalMove) {
    auto pos = chess::Position::standard();

    // Legal move: Knight b1 to c3 (Quiet) - Safe
    chess::Move nc3 = chess::Move::quiet(chess::Square::B1, chess::Square::C3);
    EXPECT_TRUE(chess::MoveGenerator::is_legal_move(pos, nc3));

    // Unsafe move: Moving king into check
    // Setup: White King e1, Black Rook e8.
    // e1-e2 is valid geometry (King step), but unsafe due to rook
    auto pos_check = chess::Position::from_fen("4r3/8/8/8/8/8/8/4K3 w - - 0 1");
    chess::Move k_e2 = chess::Move::quiet(chess::Square::E1, chess::Square::E2);
    EXPECT_FALSE(chess::MoveGenerator::is_legal_move(pos_check, k_e2));

    // Unsafe move: Moving pinned piece out of line
    // Setup: White King e1, White Bishop e2, Black Rook e8.
    // Bishop e2 is pinned.
    // Be2-d3 (diagonal) is pseudo-legal but exposes King to Rook
    auto pos_pin = chess::Position::from_fen("4r3/8/8/8/8/8/4B3/4K3 w - - 0 1");
    chess::Move b_d3 = chess::Move::quiet(chess::Square::E2, chess::Square::D3);
    EXPECT_FALSE(chess::MoveGenerator::is_legal_move(pos_pin, b_d3));

    // Legal move in check: Blocking or Capturing
    // Setup: White King e1, Black Rook e8. White Bishop c1.
    // e1 is in check.
    // Be3 blocks.
    auto pos_block =
        chess::Position::from_fen("4r3/8/8/8/8/8/8/2B1K3 w - - 0 1");
    chess::Move b_e3 = chess::Move::quiet(chess::Square::C1, chess::Square::E3);
    EXPECT_TRUE(chess::MoveGenerator::is_legal_move(pos_block, b_e3));
}
