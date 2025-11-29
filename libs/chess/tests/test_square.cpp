#include <gtest/gtest.h>

#include "chess/square.hpp"

TEST(Square, GetSquareFromFileRank) {
    EXPECT_EQ(
        chess::get_square_from_file_rank(
            chess::File::FILE_A,
            chess::Rank::RANK_1
        ),
        chess::Square::A1
    );
    EXPECT_EQ(
        chess::get_square_from_file_rank(
            chess::File::FILE_H,
            chess::Rank::RANK_8
        ),
        chess::Square::H8
    );
    EXPECT_EQ(
        chess::get_square_from_file_rank(
            chess::File::FILE_E,
            chess::Rank::RANK_4
        ),
        chess::Square::E4
    );
}

TEST(Square, GetSquareFile) {
    EXPECT_EQ(chess::get_square_file(chess::Square::A1), chess::File::FILE_A);
    EXPECT_EQ(chess::get_square_file(chess::Square::H8), chess::File::FILE_H);
    EXPECT_EQ(chess::get_square_file(chess::Square::E4), chess::File::FILE_E);
}

TEST(Square, GetSquareRank) {
    EXPECT_EQ(chess::get_square_rank(chess::Square::A1), chess::Rank::RANK_1);
    EXPECT_EQ(chess::get_square_rank(chess::Square::H8), chess::Rank::RANK_8);
    EXPECT_EQ(chess::get_square_rank(chess::Square::E4), chess::Rank::RANK_4);
}

TEST(Square, ToString) {
    EXPECT_EQ(chess::to_string(chess::Square::A1), "a1");
    EXPECT_EQ(chess::to_string(chess::Square::H8), "h8");
    EXPECT_EQ(chess::to_string(chess::Square::E4), "e4");
}
