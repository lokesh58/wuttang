#include <gtest/gtest.h>

#include <cstdint>

#include "chess/move_generator.hpp"
#include "chess/position.hpp"

class PerftTest : public ::testing::Test {
protected:
    std::uint64_t perft(chess::Position& pos, int depth) {
        if (depth == 0) {
            return 1;
        }

        chess::MoveList moves;
        chess::MoveGenerator::generate<chess::MoveGenType::LEGAL>(pos, moves);

        if (depth == 1) {
            return moves.size();
        }

        std::uint64_t nodes = 0;
        for (const auto& move : moves) {
            pos.make_move(move);
            nodes += perft(pos, depth - 1);
            pos.undo_last_move();
        }
        return nodes;
    }
};

TEST_F(PerftTest, InitialPosition) {
    auto pos = chess::Position::standard();

    EXPECT_EQ(perft(pos, 0), 1);
    EXPECT_EQ(perft(pos, 1), 20);
    EXPECT_EQ(perft(pos, 2), 400);
    EXPECT_EQ(perft(pos, 3), 8902);
    EXPECT_EQ(perft(pos, 4), 197281);
    EXPECT_EQ(perft(pos, 5), 4865609);
#ifdef NDEBUG
    EXPECT_EQ(perft(pos, 6), 119060324);
#endif
}

TEST_F(PerftTest, Kiwipete) {
    auto pos = chess::Position::from_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
    );

    EXPECT_EQ(perft(pos, 1), 48);
    EXPECT_EQ(perft(pos, 2), 2039);
    EXPECT_EQ(perft(pos, 3), 97862);
    EXPECT_EQ(perft(pos, 4), 4085603);
#ifdef NDEBUG
    EXPECT_EQ(perft(pos, 5), 193690690);
#endif
}

TEST_F(PerftTest, Position3) {
    auto pos =
        chess::Position::from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

    EXPECT_EQ(perft(pos, 1), 14);
    EXPECT_EQ(perft(pos, 2), 191);
    EXPECT_EQ(perft(pos, 3), 2812);
    EXPECT_EQ(perft(pos, 4), 43238);
    EXPECT_EQ(perft(pos, 5), 674624);
    EXPECT_EQ(perft(pos, 6), 11030083);
#ifdef NDEBUG
    EXPECT_EQ(perft(pos, 7), 178633661);
#endif
}

TEST_F(PerftTest, Position4) {
    auto pos = chess::Position::from_fen(
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    );

    EXPECT_EQ(perft(pos, 1), 6);
    EXPECT_EQ(perft(pos, 2), 264);
    EXPECT_EQ(perft(pos, 3), 9467);
    EXPECT_EQ(perft(pos, 4), 422333);
    EXPECT_EQ(perft(pos, 5), 15833292);
#ifdef NDEBUG
    EXPECT_EQ(perft(pos, 6), 706045033);
#endif
}

TEST_F(PerftTest, Position5) {
    auto pos = chess::Position::from_fen(
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    );

    EXPECT_EQ(perft(pos, 1), 44);
    EXPECT_EQ(perft(pos, 2), 1486);
    EXPECT_EQ(perft(pos, 3), 62379);
    EXPECT_EQ(perft(pos, 4), 2103487);
#ifdef NDEBUG
    EXPECT_EQ(perft(pos, 5), 89941194);
#endif
}

TEST_F(PerftTest, Position6) {
    auto pos = chess::Position::from_fen(
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    );

    EXPECT_EQ(perft(pos, 1), 46);
    EXPECT_EQ(perft(pos, 2), 2079);
    EXPECT_EQ(perft(pos, 3), 89890);
    EXPECT_EQ(perft(pos, 4), 3894594);
#ifdef NDEBUG
    EXPECT_EQ(perft(pos, 5), 164075551);
#endif
}
