cmake_minimum_required(VERSION 3.14)
project(TSExam LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
)

# Problem 1 library (header-only for now)
add_library(mesh
    src/problem_1/stl_io.cpp
    src/problem_1/triangle_mesh.cpp
    src/problem_1/reorient_triangles.cpp
    src/problem_1/void_detection.cpp
)
target_include_directories(mesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(mesh PRIVATE ${PROJECT_WARNINGS})

# Problem 2 library
add_library(polyline src/problem_2/polyline.cpp)
target_include_directories(polyline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(polyline PRIVATE ${PROJECT_WARNINGS})

# ---------------------------------------------------------------------------
# Google Test via FetchContent
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# Prevent GoogleTest from overriding our compiler/linker options on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# ---------------------------------------------------------------------------
# Test executable — Problem 1
# ---------------------------------------------------------------------------
add_executable(problem1_tests
    tests/problem_1/test_stl_io.cpp
    tests/problem_1/test_geometry.cpp
    tests/problem_1/test_triangle_mesh.cpp
    tests/problem_1/test_reorient_triangles.cpp
    tests/problem_1/test_void_detection.cpp
)
target_include_directories(problem1_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(problem1_tests PRIVATE mesh gtest_main)
target_compile_options(problem1_tests PRIVATE ${PROJECT_WARNINGS})
gtest_discover_tests(problem1_tests)

# ---------------------------------------------------------------------------
# Test executable — Problem 2
# ---------------------------------------------------------------------------
add_executable(problem2_tests tests/problem_2/test_polyline.cpp)
target_link_libraries(problem2_tests PRIVATE polyline gtest_main)
target_compile_options(problem2_tests PRIVATE ${PROJECT_WARNINGS})
gtest_discover_tests(problem2_tests)

add_custom_target(run_tests
  COMMAND $<TARGET_FILE:problem1_tests>
  COMMAND $<TARGET_FILE:problem2_tests>
  DEPENDS problem1_tests problem2_tests
  COMMENT "Running all tests..."
)
