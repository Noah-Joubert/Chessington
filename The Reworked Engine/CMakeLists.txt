cmake_minimum_required(VERSION 3.14)
project(Chessington)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")

add_executable(app "src/main.cpp")

add_subdirectory(Boost_tests)

set(SOURCE_FILES
        src/main.cpp
        src/perft.cpp
        )

add_library(ChessingtonLib STATIC ${SOURCE_FILES} )