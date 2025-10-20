cmake_minimum_required(VERSION 3.14)
project(he_accel_extended VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally enable benchmarks
option(ENABLE_BENCH "Enable benchmark targets" ON)

include(FetchContent)

# Fetch Catch2 for unit tests (only if needed)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(catch2)

# Fetch GoogleBenchmark optionally
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.7.1
)
FetchContent_MakeAvailable(benchmark)

# Main library
add_library(he_core STATIC
  src/mod_arith.cpp
  src/poly.cpp
  src/ntt.cpp
  src/ntt_simd.cpp
)
target_include_directories(he_core PUBLIC include)
target_compile_options(he_core PRIVATE -O3)

# Simple test runner (lightweight) for local quick tests
add_executable(test_runner tests/test_runner.cpp src/ntt.cpp src/mod_arith.cpp src/poly.cpp)
target_include_directories(test_runner PRIVATE include)

# Unit tests with Catch2
add_executable(unit_tests tests/test_ntt.cpp src/ntt.cpp src/mod_arith.cpp src/poly.cpp)
target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

# Simple chrono benchmark (works without GoogleBenchmark)
add_executable(bench_ntt bench/bench_ntt.cpp src/ntt.cpp src/mod_arith.cpp src/poly.cpp)
target_include_directories(bench_ntt PRIVATE include)

# GoogleBenchmark-based target (optional; requires FetchContent success)
if(ENABLE_BENCH)
  add_executable(gbench_ntt bench/bench_gbench.cpp src/ntt.cpp src/mod_arith.cpp src/poly.cpp)
  target_include_directories(gbench_ntt PRIVATE include)
  target_link_libraries(gbench_ntt PRIVATE benchmark::benchmark)
endif()

# Install rules
install(TARGETS he_core DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

message(STATUS "CMake configuration complete")

# Optionally compile with AVX2 if available
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-mavx2" COMPILER_SUPPORTS_MAVX2)
  if(COMPILER_SUPPORTS_MAVX2)
    add_compile_options(-mavx2 -mfma)
  endif()
endif()
