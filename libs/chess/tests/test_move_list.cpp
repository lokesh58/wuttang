#include <gtest/gtest.h>

#include "chess/move.hpp"
#include "chess/move_list.hpp"
#include "chess/piece.hpp"
#include "chess/square.hpp"

TEST(MoveListTest, InitialState) {
    chess::MoveList move_list;
    EXPECT_EQ(move_list.size(), 0);
    EXPECT_TRUE(move_list.empty());
}

TEST(MoveListTest, PushBackAndSize) {
    chess::MoveList move_list;
    move_list.push_back(chess::Move{});
    EXPECT_EQ(move_list.size(), 1);
    EXPECT_FALSE(move_list.empty());
}

TEST(MoveListTest, PopBack) {
    chess::MoveList move_list;
    move_list.push_back(chess::Move{});
    move_list.push_back(chess::Move{});

    move_list.pop_back();
    EXPECT_EQ(move_list.size(), 1);
}

TEST(MoveListTest, Clear) {
    chess::MoveList move_list;
    move_list.push_back(chess::Move{});
    move_list.clear();
    EXPECT_EQ(move_list.size(), 0);
    EXPECT_TRUE(move_list.empty());
}

TEST(MoveListTest, Iterators) {
    chess::MoveList move_list;
    move_list.push_back(chess::Move{});
    move_list.push_back(chess::Move{});

    int count = 0;
    for (const auto& move : move_list) {
        (void) move;
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST(MoveListTest, RandomAccess) {
    chess::MoveList move_list;
    auto move1 = chess::Move::promotion(
        chess::Square::A7,
        chess::Square::A8,
        chess::Piece::WHITE_KNIGHT
    );
    auto move2 = chess::Move::quiet(chess::Square::H2, chess::Square::H1);

    move_list.push_back(move1);
    move_list.push_back(move2);

    EXPECT_EQ(move_list[0], move1);
    EXPECT_EQ(move_list[1], move2);
}
