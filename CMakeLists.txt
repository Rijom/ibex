cmake_minimum_required(VERSION 3.15)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 REQUIRED)

project(Ibex LANGUAGES CXX)

add_library(Ibex
  include/ibex/Storage.h
  include/ibex/Function.h
  src/main.cpp # test file
)

target_include_directories(Ibex
  PUBLIC
    include
  PRIVATE
    src
)

add_subdirectory(test)

