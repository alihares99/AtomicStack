# bash .\run_test.sh
cmake_minimum_required(VERSION 3.14.0)
project(AtomicStack VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(
  AtomicStack
  src/main.cpp
  src/atomic_stack.h
  src/stack_i.h
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
enable_testing()
add_executable(
  MyTests
  tests/stack_test.cpp
)
target_link_libraries(
  MyTests
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(MyTests)
