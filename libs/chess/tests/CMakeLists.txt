# Add chess_tests executable
add_executable(chess_tests
    test_fen_parsing.cpp
    test_move_generation.cpp
    test_make_and_undo.cpp
    test_iterators.cpp
    test_piece.cpp
    test_color.cpp
)
target_link_libraries(chess_tests
    PRIVATE
        chess
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(chess_tests)
