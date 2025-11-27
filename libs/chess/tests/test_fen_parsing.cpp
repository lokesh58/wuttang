#include <gtest/gtest.h>

#include <map>
#include <optional>
#include <stdexcept>

#include "chess/color.hpp"
#include "chess/piece.hpp"
#include "chess/position.hpp"
#include "chess/square.hpp"

using namespace chess;

class FenParsingTest: public testing::Test {
protected:
    struct ExpectedPosition {
        std::map<Square, Piece> pieces;
        Color side_to_move;
        bool white_kingside_castling;
        bool white_queenside_castling;
        bool black_kingside_castling;
        bool black_queenside_castling;
        std::optional<Square> en_passant_square;
        std::uint8_t halfmove_clock;
    };

    void verify_position(
        const Position& pos,
        const ExpectedPosition& expected
    ) {
        for (const auto square : SquareRange()) {
            const auto piece_it = expected.pieces.find(square);
            const auto actual_piece = pos.get_piece_at(square);
            if (piece_it != expected.pieces.end()) {
                EXPECT_EQ(actual_piece, piece_it->second);
            } else {
                EXPECT_EQ(actual_piece, std::nullopt);
            }
        }

        EXPECT_EQ(pos.get_side_to_move(), expected.side_to_move);

        EXPECT_EQ(
            pos.has_kingside_castling_rights(Color::WHITE),
            expected.white_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(Color::WHITE),
            expected.white_queenside_castling
        );
        EXPECT_EQ(
            pos.has_kingside_castling_rights(Color::BLACK),
            expected.black_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(Color::BLACK),
            expected.black_queenside_castling
        );

        EXPECT_EQ(pos.get_en_passant_square(), expected.en_passant_square);
        EXPECT_EQ(pos.get_halfmove_clock(), expected.halfmove_clock);
    }
};

TEST_F(FenParsingTest, HandlesStandardPositionFEN) {
    const auto pos = Position::standard();

    ExpectedPosition expected;
    expected.pieces = {
        {Square::A1, Piece::WHITE_ROOK},
        {Square::B1, Piece::WHITE_KNIGHT},
        {Square::C1, Piece::WHITE_BISHOP},
        {Square::D1, Piece::WHITE_QUEEN},
        {Square::E1, Piece::WHITE_KING},
        {Square::F1, Piece::WHITE_BISHOP},
        {Square::G1, Piece::WHITE_KNIGHT},
        {Square::H1, Piece::WHITE_ROOK},
        {Square::A2, Piece::WHITE_PAWN},
        {Square::B2, Piece::WHITE_PAWN},
        {Square::C2, Piece::WHITE_PAWN},
        {Square::D2, Piece::WHITE_PAWN},
        {Square::E2, Piece::WHITE_PAWN},
        {Square::F2, Piece::WHITE_PAWN},
        {Square::G2, Piece::WHITE_PAWN},
        {Square::H2, Piece::WHITE_PAWN},
        {Square::A7, Piece::BLACK_PAWN},
        {Square::B7, Piece::BLACK_PAWN},
        {Square::C7, Piece::BLACK_PAWN},
        {Square::D7, Piece::BLACK_PAWN},
        {Square::E7, Piece::BLACK_PAWN},
        {Square::F7, Piece::BLACK_PAWN},
        {Square::G7, Piece::BLACK_PAWN},
        {Square::H7, Piece::BLACK_PAWN},
        {Square::A8, Piece::BLACK_ROOK},
        {Square::B8, Piece::BLACK_KNIGHT},
        {Square::C8, Piece::BLACK_BISHOP},
        {Square::D8, Piece::BLACK_QUEEN},
        {Square::E8, Piece::BLACK_KING},
        {Square::F8, Piece::BLACK_BISHOP},
        {Square::G8, Piece::BLACK_KNIGHT},
        {Square::H8, Piece::BLACK_ROOK},
    };
    expected.side_to_move = Color::WHITE;
    expected.white_kingside_castling = true;
    expected.white_queenside_castling = true;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = true;
    expected.en_passant_square = std::nullopt;
    expected.halfmove_clock = 0;

    verify_position(pos, expected);
}

TEST_F(FenParsingTest, HandlesCustomMiddleGamePositionFEN) {
    const auto pos = Position::from_fen(
        "3rk2r/pppqbppp/3pbn2/6B1/2BQPP2/2N5/PPP3PP/3R1RK1 b k f3 0 1"
    );

    ExpectedPosition expected;
    expected.pieces = {
        {Square::D1, Piece::WHITE_ROOK},
        {Square::F1, Piece::WHITE_ROOK},
        {Square::G1, Piece::WHITE_KING},
        {Square::A2, Piece::WHITE_PAWN},
        {Square::B2, Piece::WHITE_PAWN},
        {Square::C2, Piece::WHITE_PAWN},
        {Square::G2, Piece::WHITE_PAWN},
        {Square::H2, Piece::WHITE_PAWN},
        {Square::C3, Piece::WHITE_KNIGHT},
        {Square::C4, Piece::WHITE_BISHOP},
        {Square::D4, Piece::WHITE_QUEEN},
        {Square::E4, Piece::WHITE_PAWN},
        {Square::F4, Piece::WHITE_PAWN},
        {Square::G5, Piece::WHITE_BISHOP},
        {Square::D6, Piece::BLACK_PAWN},
        {Square::E6, Piece::BLACK_BISHOP},
        {Square::F6, Piece::BLACK_KNIGHT},
        {Square::A7, Piece::BLACK_PAWN},
        {Square::B7, Piece::BLACK_PAWN},
        {Square::C7, Piece::BLACK_PAWN},
        {Square::D7, Piece::BLACK_QUEEN},
        {Square::E7, Piece::BLACK_BISHOP},
        {Square::F7, Piece::BLACK_PAWN},
        {Square::G7, Piece::BLACK_PAWN},
        {Square::H7, Piece::BLACK_PAWN},
        {Square::D8, Piece::BLACK_ROOK},
        {Square::E8, Piece::BLACK_KING},
        {Square::H8, Piece::BLACK_ROOK},
    };
    expected.side_to_move = Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = false;
    expected.en_passant_square = Square::F3;
    expected.halfmove_clock = 0;

    verify_position(pos, expected);
}

TEST_F(FenParsingTest, HandlesCustomEndGamePositionFEN) {
    const auto pos = Position::from_fen("1k6/8/2K5/1P6/8/8/8/8 b - - 56 1");

    ExpectedPosition expected;
    expected.pieces = {
        {Square::B5, Piece::WHITE_PAWN},
        {Square::C6, Piece::WHITE_KING},
        {Square::B8, Piece::BLACK_KING},
    };
    expected.side_to_move = Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = false;
    expected.black_queenside_castling = false;
    expected.en_passant_square = std::nullopt;
    expected.halfmove_clock = 56;

    verify_position(pos, expected);
}

TEST_F(FenParsingTest, ThrowsOnInvalidFEN) {
    EXPECT_THROW(Position::from_fen("invalid fen"), std::invalid_argument)
        << "Invalid FEN format.";

    EXPECT_THROW(
        Position::from_fen("rnbqkr/pppp4/7/7/8/8/4pppp/rnb w - - 0 1"),
        std::invalid_argument
    ) << "Invalid piece placement.";

    EXPECT_THROW(
        Position::from_fen("r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R x KQkq - 0 1"),
        std::invalid_argument
    ) << "Invalid side to move.";

    EXPECT_THROW(
        Position::from_fen("r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w ~*&% - 0 1"),
        std::invalid_argument
    ) << "Invalid castling rights.";

    EXPECT_THROW(
        Position::from_fen("r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq i9 0 1"),
        std::invalid_argument
    ) << "Invalid en passant square.";

    EXPECT_THROW(
        Position::from_fen("r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - -7 1"),
        std::invalid_argument
    ) << "Invalid halfmove clock (too low).";

    EXPECT_THROW(
        Position::from_fen("r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 122 1"),
        std::invalid_argument
    ) << "Invalid halfmove clock (too high).";
}

TEST_F(FenParsingTest, LegalMovesInStandardPosition) {
    const auto pos = Position::standard();
    const auto moves = pos.generate_legal_moves();
    EXPECT_EQ(moves.size(), 20);
}

