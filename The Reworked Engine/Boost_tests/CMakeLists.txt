project(BoostTests)
set(CMAKE_CXX_STANDARD 20)

set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

set(SRC
        "moveGenerationTests.cpp"
        )

add_executable (Boost_Tests_run ${SRC})
target_link_libraries (Boost_Tests_run ${Boost_LIBRARIES})
