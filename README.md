# ibex
An experimental C++17 library providing basic building blocks.

## ibex::Function
A move-only, fixed-size alternative to std::function.
- No heap allocation. Function size is specified as a template parameter. Similar to stdext::inplace_function.
- No copy contructor. This is a move-only class and thus allows for closures containing std::unique_ptrs. Similar to folly::Function.
