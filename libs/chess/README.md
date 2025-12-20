# Wuttang Chess Library

A high-performance, modern C++23 chess library providing the core logic for the Wuttang project.

## Features

- **Bitboard Representation**: Efficient 64-bit integer representation of the board for fast move generation and analysis.
- **Move Generation**: Complete pseudo-legal and legal move generation (Quiet, Capture, Castling, En Passant, Promotions).
- **Zobrist Hashing**: Incremental hash updates for efficient position repetition detection and transposition tables.
- **Strong Typing**: Uses strong enums (`Square`, `Piece`, `Color`, `File`, `Rank`) to prevent common logic errors.
- **FEN Support**: Parsing and generation of Forsyth–Edwards Notation strings.

## Key Components

- **`Position`**: The central class representing the state of a chess game (board, turn, castling rights, etc.).
- **`Bitboard`**: A wrapper around `uint64_t` with optimized bit-twiddling operations.
- **`Move`**: Compact representation of a chess move.
- **`MoveGenerator`**: Logic for generating legal moves for a given position.

## Usage

```cpp
#include <wuttang/chess.hpp>

// Create a position from the standard starting FEN
auto pos = wuttang::chess::Position::standard();

// Generate legal moves
wuttang::chess::MoveList moves;
wuttang::chess::MoveGenerator::generate<wuttang::chess::MoveGenType::LEGAL>(pos, moves);

// Make a move (e.g., e2e4)
// Note: In real usage, find the move in the list first
// pos.make_move(some_move);
```
