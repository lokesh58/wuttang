# Wuttang Project

## Project Overview

Wuttang is a C++ project focused on chess applications. It is structured as a monorepo containing both reusable libraries and executable applications. The project leverages modern C++23 standards and uses CMake as its build system.

## Architecture

The project is organized into `apps` and `libs`:

*   **`libs/`**: Contains the core logic and utilities.
    *   **`chess`**: A library implementing chess rules, board representation (Bitboards), move generation, and Zobrist hashing.
    *   **`utils`**: General utility headers (e.g., strong enums, flags).
*   **`apps/`**: Contains the executable applications that use the libraries.
    *   **`chess_engine`**: A chess engine application (likely implementing UCI or similar protocol).
    *   **`smart_chessboard`**: Another application, possibly for interfacing with hardware or a UI.

## Building and Running

The project uses `CMakePresets.json` to simplify configuration and building.

### Prerequisites

*   **CMake**: Version 3.28 or later.
*   **Compiler**: A C++ compiler supporting C++23 (e.g., GCC, Clang, MSVC).

### Configuration

Configure the project using one of the defined presets (`debug` or `release`):

```bash
cmake --preset debug
# OR
cmake --preset release
```

### Building

Build the project:

```bash
cmake --build --preset debug
# OR
cmake --build --preset release
```

### Running Tests

The project uses GoogleTest. You can run tests using `ctest` with the corresponding preset:

```bash
ctest --preset debug
# OR
ctest --preset release
```

## Development Conventions

*   **Language Standard**: C++23.
*   **Style**:
    *   `snake_case` for variable and function names.
    *   `CamelCase` for types (classes, structs, enums).
    *   `SCREAMING_SNAKE_CASE` for constants.
    *   Use `std::string_view` for string arguments.
    *   Use `constexpr` and `noexcept` where appropriate.
    *   Header files use `#pragma once`.
*   **Directory Structure**:
    *   Public headers are in `libs/<libname>/include/<libname>/`.
    *   Source files are in `libs/<libname>/src/`.
    *   Tests are in `libs/<libname>/tests/`.
*   **Dependencies**: External dependencies (like GoogleTest) are managed via CMake's `FetchContent`.
