# Add chess library
add_library(chess STATIC)
target_include_directories(chess
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_sources(chess
    PRIVATE
        src/move.cpp
        src/position.cpp
)
target_link_libraries(chess
    PRIVATE
        utils
)

# Add chess_tests executable
add_executable(chess_tests)
target_sources(chess_tests
    PRIVATE
        tests/test_fen_parsing.cpp
)
target_link_libraries(chess_tests
    PRIVATE
        chess
        GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(chess_tests)
