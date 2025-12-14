# Add chess_tests executable
add_executable(chess_tests)
target_sources(chess_tests
    PRIVATE
        test_bitboard.cpp
        test_color.cpp
        test_move_list.cpp
        test_move_generator.cpp
        test_piece.cpp
        test_position.cpp
        test_square.cpp
)
target_link_libraries(chess_tests
    PRIVATE
        chess
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(chess_tests)
