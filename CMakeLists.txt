cmake_minimum_required(VERSION 3.28)
project(Wuttang VERSION 1.0.0 LANGUAGES CXX)

# Modern C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings and enable exceptions for MSVC
if(MSVC)
    add_compile_options(/W4 /EHsc /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
if(MSVC)
    # Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)

# Enable testing globally
enable_testing()

# Add subdirectories for libraries
add_subdirectory(libs/chess)
add_subdirectory(libs/utils)

# Add subdirectory for applications
add_subdirectory(apps/smart_chessboard)
add_subdirectory(apps/chess_engine)

# Create symlink for compile_commands.json immediately after configuration
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/build/compile_commands.json
        RESULT_VARIABLE _symlink_result
    )
    if(NOT _symlink_result EQUAL 0)
        message(WARNING "Failed to create symlink for compile_commands.json")
    endif()
endif()
