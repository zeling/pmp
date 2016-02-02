cmake_minimum_required(VERSION 2.8)

project(pmp)

add_definitions(-std=c99)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -Wall")

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(proxy ${SOURCES})

enable_testing()
add_executable(testbf test/testbuffer.c src/buffer.c)
add_test(BufferTest "${EXECUTABLE_OUTPUT_PATH}/testbf")
