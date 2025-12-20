# Add smart_chessboard executable
add_executable(smart_chessboard)
target_sources(smart_chessboard
    PRIVATE
        src/main.cpp
)
target_link_libraries(smart_chessboard
    PRIVATE
        chess
)
