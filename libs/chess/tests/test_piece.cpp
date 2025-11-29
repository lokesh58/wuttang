#include <gtest/gtest.h>

#include "chess/piece.hpp"

TEST(PieceTest, GetPieceFromColorType) {
    EXPECT_EQ(
        chess::get_piece_from_color_type(
            chess::Color::WHITE,
            chess::PieceType::PAWN
        ),
        chess::Piece::WHITE_PAWN
    );

    EXPECT_EQ(
        chess::get_piece_from_color_type(
            chess::Color::BLACK,
            chess::PieceType::KING
        ),
        chess::Piece::BLACK_KING
    );

    EXPECT_EQ(
        chess::get_piece_from_color_type(
            chess::Color::NONE,
            chess::PieceType::NONE
        ),
        chess::Piece::NONE
    );
}

TEST(PieceTest, GetPieceColor) {
    EXPECT_EQ(
        chess::get_piece_color(chess::Piece::WHITE_PAWN),
        chess::Color::WHITE
    );

    EXPECT_EQ(
        chess::get_piece_color(chess::Piece::BLACK_KING),
        chess::Color::BLACK
    );

    EXPECT_EQ(chess::get_piece_color(chess::Piece::NONE), chess::Color::NONE);
}

TEST(PieceTest, GetPieceType) {
    EXPECT_EQ(
        chess::get_piece_type(chess::Piece::WHITE_PAWN),
        chess::PieceType::PAWN
    );

    EXPECT_EQ(
        chess::get_piece_type(chess::Piece::BLACK_KING),
        chess::PieceType::KING
    );

    EXPECT_EQ(
        chess::get_piece_type(chess::Piece::NONE),
        chess::PieceType::NONE
    );
}

TEST(PieceTest, GetPieceChar) {
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_PAWN), 'P');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_KNIGHT), 'N');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_BISHOP), 'B');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_ROOK), 'R');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_QUEEN), 'Q');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::WHITE_KING), 'K');

    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_PAWN), 'p');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_KNIGHT), 'n');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_BISHOP), 'b');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_ROOK), 'r');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_QUEEN), 'q');
    EXPECT_EQ(chess::get_piece_char(chess::Piece::BLACK_KING), 'k');

    EXPECT_EQ(chess::get_piece_char(chess::Piece::NONE), '.');
}

TEST(PieceTest, GetPieceFromChar) {
    EXPECT_EQ(chess::get_piece_from_char('P'), chess::Piece::WHITE_PAWN);
    EXPECT_EQ(chess::get_piece_from_char('N'), chess::Piece::WHITE_KNIGHT);
    EXPECT_EQ(chess::get_piece_from_char('B'), chess::Piece::WHITE_BISHOP);
    EXPECT_EQ(chess::get_piece_from_char('R'), chess::Piece::WHITE_ROOK);
    EXPECT_EQ(chess::get_piece_from_char('Q'), chess::Piece::WHITE_QUEEN);
    EXPECT_EQ(chess::get_piece_from_char('K'), chess::Piece::WHITE_KING);

    EXPECT_EQ(chess::get_piece_from_char('p'), chess::Piece::BLACK_PAWN);
    EXPECT_EQ(chess::get_piece_from_char('n'), chess::Piece::BLACK_KNIGHT);
    EXPECT_EQ(chess::get_piece_from_char('b'), chess::Piece::BLACK_BISHOP);
    EXPECT_EQ(chess::get_piece_from_char('r'), chess::Piece::BLACK_ROOK);
    EXPECT_EQ(chess::get_piece_from_char('q'), chess::Piece::BLACK_QUEEN);
    EXPECT_EQ(chess::get_piece_from_char('k'), chess::Piece::BLACK_KING);

    EXPECT_EQ(chess::get_piece_from_char('.'), chess::Piece::NONE);
    EXPECT_EQ(chess::get_piece_from_char('a'), chess::Piece::NONE);
    EXPECT_EQ(chess::get_piece_from_char('Z'), chess::Piece::NONE);
    EXPECT_EQ(chess::get_piece_from_char(' '), chess::Piece::NONE);
    EXPECT_EQ(chess::get_piece_from_char('?'), chess::Piece::NONE);
}
