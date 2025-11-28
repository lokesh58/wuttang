# Add chess library
add_library(chess STATIC)
target_include_directories(chess
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_sources(chess
    PRIVATE
        src/position.cpp
)
target_link_libraries(chess
    PUBLIC
        utils
)

# Add chess_tests executable
add_executable(chess_tests)
target_sources(chess_tests
    PRIVATE
        tests/test_fen_parsing.cpp
        tests/test_move_generation.cpp
        tests/test_make_and_undo.cpp
        tests/test_iterators.cpp
        tests/test_piece.cpp
)
target_link_libraries(chess_tests
    PRIVATE
        chess
        GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(chess_tests)
