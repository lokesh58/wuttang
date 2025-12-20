# Smart Chessboard

The software controller for a custom smart chessboard hardware project.

## Overview

This application serves as the brain for a physical smart chessboard currently under development.
It bridges the gap between the physical hardware (sensors, LEDs) and the Wuttang chess engine logic.

The software is designed to run on a **Raspberry Pi** that is embedded directly into the chessboard.
The board's sensors and peripherals connect directly to the Pi.

## Features (Planned)

- Hardware interface control (GPIO/Serial/Bluetooth) via Raspberry Pi.
- Move detection and validation using `libs/chess`.
- LED feedback for move indication.
- Integration with the `chess_engine` for AI opponents.

## Status

**Current Status:** Active Development.

## Running

After building the project (see root README), the executable can be found in `build/<config>/bin/smart_chessboard`.

```bash
./build/release/bin/smart_chessboard
```
