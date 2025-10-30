# Add chess_engine executable
add_executable(chess_engine)
target_sources(chess_engine
    PRIVATE
        src/main.cpp
)
target_link_libraries(chess_engine
    PRIVATE
        chess
)
