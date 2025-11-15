cmake_minimum_required(VERSION 3.15)

project(itrace VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic")

# Get third party libraries
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

# FTXUI for main.cpp
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(argparse ftxui)

file(GLOB LIB_SOURCES src/libitrace/*.cpp)
message(STATUS "libitrace sources: ${LIB_SOURCES}")

add_library(libitrace
    ${LIB_SOURCES}
)
target_include_directories(libitrace
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(libitrace PUBLIC argparse::argparse)

file(GLOB ITRACE_SOURCES itrace-cli/*.cpp)
message(STATUS "itrace sources: ${ITRACE_SOURCES}")

add_executable(itrace ${ITRACE_SOURCES})
target_link_libraries(itrace PUBLIC libitrace)

# Optional: Add tests
# enable_testing()
# add_subdirectory(tests)

add_executable(itrace_tui itrace-tui/main.cpp)
target_link_libraries(itrace_tui
    PRIVATE libitrace
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

