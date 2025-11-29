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

add_subdirectory(tests)
