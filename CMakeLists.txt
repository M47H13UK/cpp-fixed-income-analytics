cmake_minimum_required(VERSION 3.20)
project(cpp_fixed_income_analytics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----- Engine: header-only interface library (native + WASM share one source) -----
add_library(fi_engine INTERFACE)
target_include_directories(fi_engine
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/engine/include)
target_compile_features(fi_engine INTERFACE cxx_std_20)

# ----- CLI driver (native only) -----
if(NOT EMSCRIPTEN)
    add_executable(fi_cli cli/main.cpp)
    target_link_libraries(fi_cli PRIVATE fi_engine)
endif()

# ----- Tests (native only; pulls Catch2 via FetchContent) -----
option(FI_BUILD_TESTS "Build unit tests" ON)
if(FI_BUILD_TESTS AND NOT EMSCRIPTEN)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()
    add_executable(fi_tests
        tests/daycount_test.cpp
        tests/pricing_test.cpp
        tests/risk_test.cpp
        tests/curve_test.cpp
    )
    target_link_libraries(fi_tests PRIVATE fi_engine Catch2::Catch2WithMain)
    add_test(NAME fi_tests COMMAND fi_tests)
endif()
