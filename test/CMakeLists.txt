cmake_minimum_required(VERSION 3.15)

add_executable(Ibex_Test
  Function_Test.cpp
  Storage_test.cpp
)

target_link_libraries(Ibex_Test
  PRIVATE
    Catch2::Catch2
    Ibex
)

