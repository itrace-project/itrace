cmake_minimum_required(VERSION 3.15)

project(itrace VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic")

# Get third party libraries
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

file(GLOB LIB_SOURCES src/libitrace/*.cpp)

message(STATUS "Sources: ${LIB_SOURCES}")

add_library(libitrace
    ${LIB_SOURCES}
)
target_include_directories(libitrace
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(itrace itrace/itrace.cpp)
target_link_libraries(itrace PUBLIC libitrace argparse)

# Optional: Add tests
# enable_testing()
# add_subdirectory(tests)

