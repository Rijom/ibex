#include <ibex/Function.h>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("Default function is invalid") {
  ibex::Function<int(int), 32> f;
  REQUIRE_FALSE(f);
}

TEST_CASE("New Function is valid") {
  ibex::Function<void(), 32> f([] {});
  REQUIRE(f);
}

TEST_CASE("Invoking a simple function") {
  ibex::Function<int(int), 32> f(([](int i) { return 2 * i; }));
  const int result = f(2);
  REQUIRE(result == 4);
}

TEST_CASE("Move constructor leaves target valid and moved from invalid") {
  ibex::Function<void(), 64> f1([] {});
  ibex::Function<void(), 64> f2 = std::move(f1);

  REQUIRE_FALSE(f1);
  REQUIRE(f2);
}

TEST_CASE("Move assignment leaves target valid and moved from invalid") {
  ibex::Function<void(), 64> f1([] {});
  ibex::Function<void(), 64> f2;
  f2 = std::move(f1);

  REQUIRE_FALSE(f1);
  REQUIRE(f2);
}

TEST_CASE("Moving around a function preserves its state.") {
  auto lambda = [total = 0](int x) mutable {
    total += x;
    return total;
  };
  ibex::Function<int(int), 64> sum(lambda);

  sum(1);
  sum(2);
  auto sum2 = std::move(sum);

  REQUIRE(sum2(5) == 1 + 2 + 5);
}

struct dtor_counter {
  inline static int count{0};
  bool valid{true};

  dtor_counter() = default;
  dtor_counter(dtor_counter&& other) : dtor_counter() {
    valid = other.valid;
    other.valid = false;
  }

  dtor_counter(const dtor_counter& other) = default;

  ~dtor_counter() {
    if (valid) ++count;
  }

  dtor_counter& operator=(dtor_counter&& other) {
    valid = other.valid;
    other.valid = false;
    return *this;
  }

  dtor_counter& operator=(const dtor_counter& other) {
    valid = other.valid;
    return *this;
  }
};

TEST_CASE("Unique_ptr in function calls destructor only once.") {
  std::unique_ptr<dtor_counter> ptr(new dtor_counter);

  {
    auto lambda = [ptr = std::move(ptr)]() {};
    ibex::Function<void(void), 64> f1(std::move(lambda));
    auto f2 = std::move(f1);
  }

  REQUIRE(dtor_counter::count == 1);
}
