cmake_minimum_required(VERSION 3.15)

add_executable(Ibex_Test
  Function_Test.cpp
)

target_link_libraries(Ibex_Test
  PRIVATE
    Catch2::Catch2
    Ibex
)

