#include <gtest/gtest.h>

#include <wuttang/chess.hpp>

using namespace wuttang::chess;

TEST(SquareTest, GetSquareFromFileRank) {
    EXPECT_EQ(
        get_square_from_file_rank(File::FILE_A, Rank::RANK_1),
        Square::A1
    );
    EXPECT_EQ(
        get_square_from_file_rank(File::FILE_H, Rank::RANK_8),
        Square::H8
    );
    EXPECT_EQ(
        get_square_from_file_rank(File::FILE_E, Rank::RANK_4),
        Square::E4
    );
}

TEST(SquareTest, GetSquareFile) {
    EXPECT_EQ(get_square_file(Square::A1), File::FILE_A);
    EXPECT_EQ(get_square_file(Square::H8), File::FILE_H);
    EXPECT_EQ(get_square_file(Square::E4), File::FILE_E);
}

TEST(SquareTest, GetSquareRank) {
    EXPECT_EQ(get_square_rank(Square::A1), Rank::RANK_1);
    EXPECT_EQ(get_square_rank(Square::H8), Rank::RANK_8);
    EXPECT_EQ(get_square_rank(Square::E4), Rank::RANK_4);
}

TEST(SquareTest, ToString) {
    EXPECT_EQ(to_string(Square::A1), "a1");
    EXPECT_EQ(to_string(Square::H8), "h8");
    EXPECT_EQ(to_string(Square::E4), "e4");
}
