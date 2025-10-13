cmake_minimum_required(VERSION 3.28)

project(Wuttang VERSION 1.0.0 LANGUAGES CXX)

# Modern C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add subdirectories for libraries
add_subdirectory(libs/chess)

# Add subdirectory for applications
add_subdirectory(apps/smart_chessboard)
add_subdirectory(apps/chess_engine)

# Add subdirectory for tests (optional)
# add_subdirectory(tests)

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
