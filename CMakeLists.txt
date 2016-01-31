cmake_minimum_required(VERSION 2.8)

project(pmp)

add_definitions(-std=c99)

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(proxy ${SOURCES})
