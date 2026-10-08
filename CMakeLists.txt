cmake_minimum_required(VERSION 3.20)

project(flow_ledger
    VERSION 0.1.0
    DESCRIPTION "Flow Ledger"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -------------------------
# Production library
# -------------------------

add_library(flow_ledger_lib
    src/transaction_engine.cpp
)

target_include_directories(flow_ledger_lib
    PUBLIC
        include
)

# -------------------------
# Application
# -------------------------

add_executable(flow_ledger
    src/main.cpp
)

target_link_libraries(flow_ledger
    PRIVATE
        flow_ledger_lib
)

# -------------------------
# Tests
# -------------------------

enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(flow_ledger_tests
    tests/transaction_engine_test.cpp
)

target_link_libraries(flow_ledger_tests
    PRIVATE
        flow_ledger_lib
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(flow_ledger_tests)