#include <gtest/gtest.h>

#include "chess/position.hpp"

using namespace chess;

class MoveGenerationTest : public testing::Test {
protected:
    std::uint64_t perft(int depth, Position& pos) {
        if (depth == 0) {
            return 1;
        }

        std::uint64_t nodes = 0;
        const auto moves = pos.generate_legal_moves();
        for (const auto& move : moves) {
            pos.make_move(move);
            nodes += perft(depth - 1, pos);
            pos.undo_last_move();
        }

        return nodes;
    }
};

TEST_F(MoveGenerationTest, Perft) {
    auto pos = Position::standard();
    EXPECT_EQ(perft(0, pos), 1);
    EXPECT_EQ(perft(1, pos), 20);
    EXPECT_EQ(perft(2, pos), 400);
    EXPECT_EQ(perft(3, pos), 8902);
}