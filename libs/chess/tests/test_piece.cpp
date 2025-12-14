#include <gtest/gtest.h>

#include <wuttang/chess.hpp>

using namespace wuttang::chess;

TEST(PieceTest, GetPieceFromColorType) {
    EXPECT_EQ(
        get_piece_from_color_type(Color::WHITE, PieceType::PAWN),
        Piece::WHITE_PAWN
    );

    EXPECT_EQ(
        get_piece_from_color_type(Color::BLACK, PieceType::KING),
        Piece::BLACK_KING
    );

    EXPECT_EQ(
        get_piece_from_color_type(Color::NONE, PieceType::NONE),
        Piece::NONE
    );
}

TEST(PieceTest, GetPieceColor) {
    EXPECT_EQ(get_piece_color(Piece::WHITE_PAWN), Color::WHITE);

    EXPECT_EQ(get_piece_color(Piece::BLACK_KING), Color::BLACK);

    EXPECT_EQ(get_piece_color(Piece::NONE), Color::NONE);
}

TEST(PieceTest, GetPieceType) {
    EXPECT_EQ(get_piece_type(Piece::WHITE_PAWN), PieceType::PAWN);

    EXPECT_EQ(get_piece_type(Piece::BLACK_KING), PieceType::KING);

    EXPECT_EQ(get_piece_type(Piece::NONE), PieceType::NONE);
}

TEST(PieceTest, GetPieceChar) {
    EXPECT_EQ(get_piece_char(Piece::WHITE_PAWN), 'P');
    EXPECT_EQ(get_piece_char(Piece::WHITE_KNIGHT), 'N');
    EXPECT_EQ(get_piece_char(Piece::WHITE_BISHOP), 'B');
    EXPECT_EQ(get_piece_char(Piece::WHITE_ROOK), 'R');
    EXPECT_EQ(get_piece_char(Piece::WHITE_QUEEN), 'Q');
    EXPECT_EQ(get_piece_char(Piece::WHITE_KING), 'K');

    EXPECT_EQ(get_piece_char(Piece::BLACK_PAWN), 'p');
    EXPECT_EQ(get_piece_char(Piece::BLACK_KNIGHT), 'n');
    EXPECT_EQ(get_piece_char(Piece::BLACK_BISHOP), 'b');
    EXPECT_EQ(get_piece_char(Piece::BLACK_ROOK), 'r');
    EXPECT_EQ(get_piece_char(Piece::BLACK_QUEEN), 'q');
    EXPECT_EQ(get_piece_char(Piece::BLACK_KING), 'k');

    EXPECT_EQ(get_piece_char(Piece::NONE), '.');
}

TEST(PieceTest, GetPieceFromChar) {
    EXPECT_EQ(get_piece_from_char('P'), Piece::WHITE_PAWN);
    EXPECT_EQ(get_piece_from_char('N'), Piece::WHITE_KNIGHT);
    EXPECT_EQ(get_piece_from_char('B'), Piece::WHITE_BISHOP);
    EXPECT_EQ(get_piece_from_char('R'), Piece::WHITE_ROOK);
    EXPECT_EQ(get_piece_from_char('Q'), Piece::WHITE_QUEEN);
    EXPECT_EQ(get_piece_from_char('K'), Piece::WHITE_KING);

    EXPECT_EQ(get_piece_from_char('p'), Piece::BLACK_PAWN);
    EXPECT_EQ(get_piece_from_char('n'), Piece::BLACK_KNIGHT);
    EXPECT_EQ(get_piece_from_char('b'), Piece::BLACK_BISHOP);
    EXPECT_EQ(get_piece_from_char('r'), Piece::BLACK_ROOK);
    EXPECT_EQ(get_piece_from_char('q'), Piece::BLACK_QUEEN);
    EXPECT_EQ(get_piece_from_char('k'), Piece::BLACK_KING);

    EXPECT_EQ(get_piece_from_char('.'), Piece::NONE);
    EXPECT_EQ(get_piece_from_char('a'), Piece::NONE);
    EXPECT_EQ(get_piece_from_char('Z'), Piece::NONE);
    EXPECT_EQ(get_piece_from_char(' '), Piece::NONE);
    EXPECT_EQ(get_piece_from_char('?'), Piece::NONE);
}
