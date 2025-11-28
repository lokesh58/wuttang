#include <gtest/gtest.h>
#include "chess/position.hpp"

using namespace chess;

class MakeUndoTest : public testing::Test {};

TEST_F(MakeUndoTest, QuietMove_Knight) {
    auto pos = Position::standard();
    const auto initial_fen = pos.get_fen();

    // White Knight G1 -> F3
    auto move = Move::quiet(Square::G1, Square::F3);

    // 1. Make Move
    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::G1), Piece::NONE);
    EXPECT_EQ(pos.get_piece_at(Square::F3), Piece::WHITE_KNIGHT);
    EXPECT_EQ(pos.get_side_to_move(), Color::BLACK);
    EXPECT_EQ(pos.get_en_passant_square(), Square::NO_SQ);
    // Castling rights shouldn't change for White yet (unless rook/king moves, here Knight moves)
    EXPECT_TRUE(pos.has_kingside_castling_rights(Color::WHITE));

    // 2. Undo Move
    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_EQ(pos.get_piece_at(Square::G1), Piece::WHITE_KNIGHT);
    EXPECT_EQ(pos.get_piece_at(Square::F3), Piece::NONE);
    EXPECT_EQ(pos.get_side_to_move(), Color::WHITE);
}

TEST_F(MakeUndoTest, DoublePawnPush) {
    auto pos = Position::standard();
    const auto initial_fen = pos.get_fen();

    // White Pawn E2 -> E4
    auto move = Move::double_pawn_push(Square::E2, Square::E4);

    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::E2), Piece::NONE);
    EXPECT_EQ(pos.get_piece_at(Square::E4), Piece::WHITE_PAWN);
    EXPECT_EQ(pos.get_side_to_move(), Color::BLACK);
    // En Passant target should be E3
    EXPECT_EQ(pos.get_en_passant_square(), Square::E3);

    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_EQ(pos.get_en_passant_square(), Square::NO_SQ);
}

TEST_F(MakeUndoTest, CaptureMove) {
    // Setup: Black to move. Black Pawn on D5. White Pawn on E4.
    // FEN: rnbqkbnr/pppppppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1
    // (Modified standard slightly)
    // Let's use a simpler custom FEN
    auto pos = Position::from_fen("8/3p4/8/4P3/8/8/8/8 b - - 0 1"); 
    // Black pawn d7, White pawn e5. d7->e5 is not capture.
    // Wait, pawns capture diagonally.
    // Black pawn at d5, White pawn at e4. d5xe4.
    pos = Position::from_fen("8/8/8/3p4/4P3/8/8/8 b - - 0 1");
    const auto initial_fen = pos.get_fen();

    // Black d5 captures e4
    auto move = Move::capture(Square::D5, Square::E4, Piece::WHITE_PAWN);

    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::D5), Piece::NONE);
    EXPECT_EQ(pos.get_piece_at(Square::E4), Piece::BLACK_PAWN);
    EXPECT_EQ(pos.get_side_to_move(), Color::WHITE);

    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_EQ(pos.get_piece_at(Square::E4), Piece::WHITE_PAWN);
    EXPECT_EQ(pos.get_piece_at(Square::D5), Piece::BLACK_PAWN);
}

TEST_F(MakeUndoTest, EnPassantCapture) {
    // Valid En Passant Scenario:
    // White just moved f2 -> f4.
    // Black pawn at e4.
    // En Passant target at f3.
    auto pos = Position::from_fen("rnbqkbnr/pppp1ppp/8/8/4pP2/8/PPPPP1PP/RNBQKBNR b KQkq f3 0 1");
    const auto initial_fen = pos.get_fen();
    
    // Black captures en passant: e4 -> f3
    auto move = Move::en_passant(Square::E4, Square::F3);

    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::E4), Piece::NONE); // Attacking pawn moved
    EXPECT_EQ(pos.get_piece_at(Square::F3), Piece::BLACK_PAWN); // Landed on target
    EXPECT_EQ(pos.get_piece_at(Square::F4), Piece::NONE); // Captured pawn (was at f4) removed
    EXPECT_EQ(pos.get_side_to_move(), Color::WHITE);

    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_EQ(pos.get_piece_at(Square::F4), Piece::WHITE_PAWN); // Captured pawn back
    EXPECT_EQ(pos.get_piece_at(Square::E4), Piece::BLACK_PAWN); // Attacker back
}

TEST_F(MakeUndoTest, KingsideCastling) {
    auto pos = Position::from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"); 
    const auto initial_fen = pos.get_fen();

    // White O-O: E1 -> G1
    auto move = Move::castle_kingside(Square::E1, Square::G1);

    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::E1), Piece::NONE);
    EXPECT_EQ(pos.get_piece_at(Square::G1), Piece::WHITE_KING);
    EXPECT_EQ(pos.get_piece_at(Square::H1), Piece::NONE); // Rook moved
    EXPECT_EQ(pos.get_piece_at(Square::F1), Piece::WHITE_ROOK); // Rook new pos
    EXPECT_FALSE(pos.has_kingside_castling_rights(Color::WHITE));
    EXPECT_FALSE(pos.has_queenside_castling_rights(Color::WHITE)); // Castling forfeits all rights for that color

    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_TRUE(pos.has_kingside_castling_rights(Color::WHITE));
    EXPECT_EQ(pos.get_piece_at(Square::E1), Piece::WHITE_KING);
    EXPECT_EQ(pos.get_piece_at(Square::H1), Piece::WHITE_ROOK);
}

TEST_F(MakeUndoTest, QueensideCastling) {
    auto pos = Position::from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"); 
    const auto initial_fen = pos.get_fen();

    // White O-O-O: E1 -> C1
    auto move = Move::castle_queenside(Square::E1, Square::C1);

    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::E1), Piece::NONE);
    EXPECT_EQ(pos.get_piece_at(Square::C1), Piece::WHITE_KING);
    EXPECT_EQ(pos.get_piece_at(Square::A1), Piece::NONE); // Rook moved
    EXPECT_EQ(pos.get_piece_at(Square::D1), Piece::WHITE_ROOK); // Rook new pos
    EXPECT_FALSE(pos.has_kingside_castling_rights(Color::WHITE));
    EXPECT_FALSE(pos.has_queenside_castling_rights(Color::WHITE));

    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_TRUE(pos.has_queenside_castling_rights(Color::WHITE));
}

TEST_F(MakeUndoTest, PromotionToQueen) {
    auto pos = Position::from_fen("8/4P3/8/8/8/8/8/8 w - - 0 1"); // pawn on e7
    const auto initial_fen = pos.get_fen();

    // e7 -> e8 = Q
    auto move = Move::promotion(Square::E7, Square::E8, Piece::WHITE_QUEEN);

    pos.make_move(move);

    // Verify Post-Move State
    EXPECT_EQ(pos.get_piece_at(Square::E7), Piece::NONE);
    EXPECT_EQ(pos.get_piece_at(Square::E8), Piece::WHITE_QUEEN);
    EXPECT_EQ(pos.get_side_to_move(), Color::BLACK);

    pos.undo_last_move();

    // Verify Restored State
    EXPECT_EQ(pos.get_fen(), initial_fen);
    EXPECT_EQ(pos.get_piece_at(Square::E7), Piece::WHITE_PAWN);
    EXPECT_EQ(pos.get_piece_at(Square::E8), Piece::NONE);
}
