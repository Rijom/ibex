#include <ibex/Storage.h>

#include <catch2/catch.hpp>

namespace {
  struct MoveOnly {
    int i;

    MoveOnly() = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&)= default;
    MoveOnly& operator=(const MoveOnly&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;
  };
}

TEST_CASE("Storage Storing a move-only type can be accessed from storage again.") {
  ibex::Storage<MoveOnly> sut;
  const int number = 77;
  MoveOnly obj{number};

  sut.create(std::move(obj));

  REQUIRE(sut.get().i == number);
}

TEST_CASE("Storage Storing a copyable type can be moved out again.") {
  ibex::Storage<int> sut;
  const int number = 77;

  sut.create(number);

  REQUIRE(sut.get() == number);
}