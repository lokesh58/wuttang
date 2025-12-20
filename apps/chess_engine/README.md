# Chess Engine

A UCI-compatible chess engine built using the Wuttang chess library.

## Overview

This application serves as the main executable for playing chess against the engine or analyzing positions. It is designed to interface with standard Chess GUIs (like Arena, Cute Chess, or Lichess) via the Universal Chess Interface (UCI) protocol.

## Status

**Current Status:** Work In Progress (WIP). Basic infrastructure is being set up.

## Running

After building the project (see root README), the executable can be found in `build/<config>/bin/chess_engine`.

```bash
./build/release/bin/chess_engine
```

## Supported UCI Commands (Planned)

- `uci`
- `isready`
- `ucinewgame`
- `position [fen <fenstring> | startpos] moves <move1> ... <movei>`
- `go [wtime <x>] [btime <y>] ...`
- `quit`
