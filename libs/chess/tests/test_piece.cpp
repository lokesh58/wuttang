#include <gtest/gtest.h>
#include "chess/piece.hpp"

TEST(PieceCharTest, ReturnsCorrectCharForWhitePieces) {
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_PAWN), 'P');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_KNIGHT), 'N');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_BISHOP), 'B');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_ROOK), 'R');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_QUEEN), 'Q');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_KING), 'K');
}

TEST(PieceCharTest, ReturnsCorrectCharForBlackPieces) {
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_PAWN), 'p');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_KNIGHT), 'n');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_BISHOP), 'b');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_ROOK), 'r');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_QUEEN), 'q');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_KING), 'k');
}

TEST(PieceCharTest, ReturnsCorrectCharForNone) {
    EXPECT_EQ(chess::get_piece_char(chess::Piece::NONE), '.');
}

TEST(PieceCharTest, ReturnsQuestionMarkForInvalidIndices) {
    // Testing some invalid indices (gaps in the enum values)
    // 1-16 are unused
    for (std::uint8_t i = 1; i <= 16; ++i) {
        EXPECT_EQ(chess::get_piece_char(static_cast<chess::Piece>(i)), '?');
    }
    // 23-32 are unused
    for (std::uint8_t i = 23; i <= 32; ++i) {
        EXPECT_EQ(chess::get_piece_char(static_cast<chess::Piece>(i)), '?');
    }
    // > 38 are out of bounds of the array
    EXPECT_EQ(chess::get_piece_char(static_cast<chess::Piece>(39)), '?');
    EXPECT_EQ(chess::get_piece_char(static_cast<chess::Piece>(255)), '?');
}
