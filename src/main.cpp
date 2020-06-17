#include <ibex/Storage.h>
#include <ibex/Function.h>
#include <iostream>


struct A {
  A() { std::cout << "A\n"; }
  virtual ~A() { std::cout << "~A\n"; }
};

struct B : public A {
  B() { std::cout << "B\n"; }
  virtual ~B() { std::cout << "~B\n"; }
};

struct C : public B {
  C() { std::cout << "C\n"; }
  virtual ~C() { std::cout << "~C\n"; }
};

int main() {
  ibex::PolyStorage<A, 8> storage;
  storage.create<C>();
  storage.destroy();
  
}

