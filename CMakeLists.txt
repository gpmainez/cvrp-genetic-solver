cmake_minimum_required(VERSION 3.20)
project(cvrp_solver VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Core static library
add_library(cvrp_lib STATIC
    src/ProblemInstance.cpp
    src/InstanceLoader.cpp
    src/Individual.cpp
    src/Evaluator.cpp
    src/GeneticAlgorithm.cpp
)

target_include_directories(cvrp_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# CLI Application
add_executable(cvrp_solver src/main.cpp)
target_link_libraries(cvrp_solver PRIVATE cvrp_lib)

# Testing
option(BUILD_TESTS "Build GoogleTest test suite" ON)

if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(cvrp_tests
        tests/test_loader.cpp
        tests/test_individual.cpp
        tests/test_evaluator.cpp
        tests/test_algorithm.cpp
    )

    target_link_libraries(cvrp_tests PRIVATE
        cvrp_lib
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(cvrp_tests)
endif()
