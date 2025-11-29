#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <stdexcept>

#include "chess/color.hpp"
#include "chess/piece.hpp"
#include "chess/position.hpp"
#include "chess/square.hpp"

class FenParsingTest : public testing::Test {
protected:
    struct ExpectedPosition {
        std::map<chess::Square, chess::Piece> pieces;
        chess::Color side_to_move;
        bool white_kingside_castling;
        bool white_queenside_castling;
        bool black_kingside_castling;
        bool black_queenside_castling;
        chess::Square en_passant_square;
        std::uint8_t halfmove_clock;
        std::uint16_t fullmove_number;
    };

    void verify_position(
        const chess::Position& pos,
        const ExpectedPosition& expected
    ) {
        for (const auto square : chess::SquareRange()) {
            const auto piece_it = expected.pieces.find(square);
            const auto actual_piece = pos.get_piece_at(square);
            if (piece_it != expected.pieces.end()) {
                EXPECT_EQ(actual_piece, piece_it->second);
            } else {
                EXPECT_EQ(actual_piece, chess::Piece::NONE);
            }
        }

        EXPECT_EQ(pos.get_side_to_move(), expected.side_to_move);

        EXPECT_EQ(
            pos.has_kingside_castling_rights(chess::Color::WHITE),
            expected.white_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(chess::Color::WHITE),
            expected.white_queenside_castling
        );
        EXPECT_EQ(
            pos.has_kingside_castling_rights(chess::Color::BLACK),
            expected.black_kingside_castling
        );
        EXPECT_EQ(
            pos.has_queenside_castling_rights(chess::Color::BLACK),
            expected.black_queenside_castling
        );

        EXPECT_EQ(pos.get_en_passant_square(), expected.en_passant_square);
        EXPECT_EQ(pos.get_halfmove_clock(), expected.halfmove_clock);
        EXPECT_EQ(pos.get_fullmove_number(), expected.fullmove_number);
    }
};

TEST_F(FenParsingTest, HandlesStandardPositionFEN) {
    const auto pos = chess::Position::standard();

    ExpectedPosition expected;
    expected.pieces = {
        {chess::Square::A1, chess::Piece::WHITE_ROOK},
        {chess::Square::B1, chess::Piece::WHITE_KNIGHT},
        {chess::Square::C1, chess::Piece::WHITE_BISHOP},
        {chess::Square::D1, chess::Piece::WHITE_QUEEN},
        {chess::Square::E1, chess::Piece::WHITE_KING},
        {chess::Square::F1, chess::Piece::WHITE_BISHOP},
        {chess::Square::G1, chess::Piece::WHITE_KNIGHT},
        {chess::Square::H1, chess::Piece::WHITE_ROOK},
        {chess::Square::A2, chess::Piece::WHITE_PAWN},
        {chess::Square::B2, chess::Piece::WHITE_PAWN},
        {chess::Square::C2, chess::Piece::WHITE_PAWN},
        {chess::Square::D2, chess::Piece::WHITE_PAWN},
        {chess::Square::E2, chess::Piece::WHITE_PAWN},
        {chess::Square::F2, chess::Piece::WHITE_PAWN},
        {chess::Square::G2, chess::Piece::WHITE_PAWN},
        {chess::Square::H2, chess::Piece::WHITE_PAWN},
        {chess::Square::A7, chess::Piece::BLACK_PAWN},
        {chess::Square::B7, chess::Piece::BLACK_PAWN},
        {chess::Square::C7, chess::Piece::BLACK_PAWN},
        {chess::Square::D7, chess::Piece::BLACK_PAWN},
        {chess::Square::E7, chess::Piece::BLACK_PAWN},
        {chess::Square::F7, chess::Piece::BLACK_PAWN},
        {chess::Square::G7, chess::Piece::BLACK_PAWN},
        {chess::Square::H7, chess::Piece::BLACK_PAWN},
        {chess::Square::A8, chess::Piece::BLACK_ROOK},
        {chess::Square::B8, chess::Piece::BLACK_KNIGHT},
        {chess::Square::C8, chess::Piece::BLACK_BISHOP},
        {chess::Square::D8, chess::Piece::BLACK_QUEEN},
        {chess::Square::E8, chess::Piece::BLACK_KING},
        {chess::Square::F8, chess::Piece::BLACK_BISHOP},
        {chess::Square::G8, chess::Piece::BLACK_KNIGHT},
        {chess::Square::H8, chess::Piece::BLACK_ROOK},
    };
    expected.side_to_move = chess::Color::WHITE;
    expected.white_kingside_castling = true;
    expected.white_queenside_castling = true;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = true;
    expected.en_passant_square = chess::Square::NO_SQ;
    expected.halfmove_clock = 0;
    expected.fullmove_number = 1;

    verify_position(pos, expected);
}

TEST_F(FenParsingTest, HandlesCustomMiddleGamePositionFEN) {
    const auto pos = chess::Position::from_fen(
        "3rk2r/pppqbppp/3pbn2/6B1/2BQPP2/2N5/PPP3PP/3R1RK1 b k f3 0 17"
    );

    ExpectedPosition expected;
    expected.pieces = {
        {chess::Square::D1, chess::Piece::WHITE_ROOK},
        {chess::Square::F1, chess::Piece::WHITE_ROOK},
        {chess::Square::G1, chess::Piece::WHITE_KING},
        {chess::Square::A2, chess::Piece::WHITE_PAWN},
        {chess::Square::B2, chess::Piece::WHITE_PAWN},
        {chess::Square::C2, chess::Piece::WHITE_PAWN},
        {chess::Square::G2, chess::Piece::WHITE_PAWN},
        {chess::Square::H2, chess::Piece::WHITE_PAWN},
        {chess::Square::C3, chess::Piece::WHITE_KNIGHT},
        {chess::Square::C4, chess::Piece::WHITE_BISHOP},
        {chess::Square::D4, chess::Piece::WHITE_QUEEN},
        {chess::Square::E4, chess::Piece::WHITE_PAWN},
        {chess::Square::F4, chess::Piece::WHITE_PAWN},
        {chess::Square::G5, chess::Piece::WHITE_BISHOP},
        {chess::Square::D6, chess::Piece::BLACK_PAWN},
        {chess::Square::E6, chess::Piece::BLACK_BISHOP},
        {chess::Square::F6, chess::Piece::BLACK_KNIGHT},
        {chess::Square::A7, chess::Piece::BLACK_PAWN},
        {chess::Square::B7, chess::Piece::BLACK_PAWN},
        {chess::Square::C7, chess::Piece::BLACK_PAWN},
        {chess::Square::D7, chess::Piece::BLACK_QUEEN},
        {chess::Square::E7, chess::Piece::BLACK_BISHOP},
        {chess::Square::F7, chess::Piece::BLACK_PAWN},
        {chess::Square::G7, chess::Piece::BLACK_PAWN},
        {chess::Square::H7, chess::Piece::BLACK_PAWN},
        {chess::Square::D8, chess::Piece::BLACK_ROOK},
        {chess::Square::E8, chess::Piece::BLACK_KING},
        {chess::Square::H8, chess::Piece::BLACK_ROOK},
    };
    expected.side_to_move = chess::Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = true;
    expected.black_queenside_castling = false;
    expected.en_passant_square = chess::Square::F3;
    expected.halfmove_clock = 0;
    expected.fullmove_number = 17;

    verify_position(pos, expected);
}

TEST_F(FenParsingTest, HandlesCustomEndGamePositionFEN) {
    const auto pos =
        chess::Position::from_fen("1k6/8/2K5/1P6/8/8/8/8 b - - 56 83");

    ExpectedPosition expected;
    expected.pieces = {
        {chess::Square::B5, chess::Piece::WHITE_PAWN},
        {chess::Square::C6, chess::Piece::WHITE_KING},
        {chess::Square::B8, chess::Piece::BLACK_KING},
    };
    expected.side_to_move = chess::Color::BLACK;
    expected.white_kingside_castling = false;
    expected.white_queenside_castling = false;
    expected.black_kingside_castling = false;
    expected.black_queenside_castling = false;
    expected.en_passant_square = chess::Square::NO_SQ;
    expected.halfmove_clock = 56;
    expected.fullmove_number = 83;

    verify_position(pos, expected);
}

TEST_F(FenParsingTest, ThrowsOnInvalidFEN) {
    EXPECT_THROW(
        chess::Position::from_fen("invalid fen"),
        std::invalid_argument
    ) << "Invalid FEN format.";

    EXPECT_THROW(
        chess::Position::from_fen("rnbqkr/pppp4/7/7/8/8/4pppp/rnb w - - 0 1"),
        std::invalid_argument
    ) << "Invalid piece placement.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R x KQkq - 0 1"
        ),
        std::invalid_argument
    ) << "Invalid side to move.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w ~*&% - 0 1"
        ),
        std::invalid_argument
    ) << "Invalid castling rights.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq i9 0 1"
        ),
        std::invalid_argument
    ) << "Invalid en passant square.";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - -7 1"
        ),
        std::invalid_argument
    ) << "Invalid halfmove clock (too low).";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 192 1"
        ),
        std::invalid_argument
    ) << "Invalid halfmove clock (too high).";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 0 0"
        ),
        std::invalid_argument
    ) << "Invalid fullmove number (too low).";

    EXPECT_THROW(
        chess::Position::from_fen(
            "r3k2r/p3p2p/8/8/8/8/P3P2P/R3K2R w KQkq - 0 10532"
        ),
        std::invalid_argument
    ) << "Invalid fullmove number (too high).";
}

