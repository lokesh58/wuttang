# Wuttang

Wuttang is a modern C++23 chess monorepo containing a high-performance chess
library and associated applications. It utilizes CMake as its build system and
emphasizes strict type safety and modern C++ features.

## Project Structure

The project is organized into libraries (`libs/`) and applications (`apps/`):

### Libraries

- **[`libs/chess`](libs/chess/README.md)**: Core chess logic including move generation, bitboard representation, and Zobrist hashing.
- **[`libs/utils`](libs/utils/README.md)**: General purpose C++ utilities (Enum flags, ranges, etc.).

### Applications

- **[`apps/chess_engine`](apps/chess_engine/README.md)**: A UCI-compatible chess engine (Work In Progress).
- **[`apps/smart_chessboard`](apps/smart_chessboard/README.md)**: Application for interfacing with smart chessboard hardware/UI (Work In Progress).

## Prerequisites

- **Compiler**: A C++ compiler supporting **C++23** (e.g., GCC 13+, Clang 16+, MSVC 19.36+).
- **CMake**: Version **3.28** or later.

## Building and Testing

This project uses `CMakePresets.json` for simplified workflow management.

### Configure, Build, and Test (Workflow)

To run the full development cycle (Configure -> Build -> Test) in one command:

```bash
# Debug Mode
cmake --workflow --preset debug

# Release Mode
cmake --workflow --preset release
```

### Manual Steps

If you prefer running steps individually:

1. **Configure**:

   ```bash
   cmake --preset debug
   ```

2. **Build**:

   ```bash
   cmake --build --preset debug
   ```

3. **Test**:

   ```bash
   ctest --preset debug
   ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file
for details.
