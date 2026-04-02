cmake_minimum_required(VERSION 3.20)
project(chalk VERSION 0.1.0 LANGUAGES CXX)

add_library(chalk INTERFACE)
add_library(chalk::chalk ALIAS chalk)

target_compile_features(chalk INTERFACE cxx_std_20)

target_include_directories(chalk
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
  target_compile_options(chalk INTERFACE /W4 /permissive-)
else()
  target_compile_options(chalk INTERFACE -Wall -Wextra -Wpedantic)
endif()

option(CHALK_BUILD_TESTS "Build chalk tests" OFF)
option(CHALK_BUILD_EXAMPLES "Build chalk examples" OFF)

include(CTest)

if(CHALK_BUILD_TESTS)
  enable_testing()

  add_executable(chalk_test_basic tests/test_basic.cpp)
  target_link_libraries(chalk_test_basic PRIVATE chalk::chalk)

  add_test(NAME chalk.basic COMMAND chalk_test_basic)
endif()

if(CHALK_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
