#pragma once

#include <ibex/Storage.h>
#include <functional>

namespace ibex {

template <typename, size_t>
class Function;

template <size_t Size, typename Result, typename... Arguments>
class Function<Result (Arguments...), Size> final {
private:
  // ---------------------------------------------------------------------------
  // Types
  // ---------------------------------------------------------------------------
  template <typename ReturnType, typename... Args>
  struct ErasedFunctorHolder {
    virtual ~ErasedFunctorHolder() {}
    virtual ReturnType operator()(Args&&...) = 0;
    virtual void moveInto (void*) = 0;
  };

  template <typename Functor, typename ReturnType, typename... Args>
  struct FunctorHolder final : ErasedFunctorHolder<Result, Arguments...> {
    // Non-erased Functor
    Functor f;

    FunctorHolder (Functor&& func) : f (std::move(func)) {}
    ~FunctorHolder () override = default;

    ReturnType operator()(Args&&... args) override {
      return f (std::forward<Arguments> (args)...);
    }

    void moveInto (void* destination) override {
      new (destination) FunctorHolder (std::move(f));
    }
  };

  using holder_t = ErasedFunctorHolder<Result, Arguments...>;
  
  
  // ---------------------------------------------------------------------------
  // Members
  // ---------------------------------------------------------------------------
  bool m_isValid{false}; //
  mutable PolyStorage<holder_t, Size> m_storage;
  
  // ---------------------------------------------------------------------------
  // Public Functions
  // ---------------------------------------------------------------------------
public:

  // Create an empty function
  Function() = default;
  
  ~Function() {
    if (m_isValid)
      m_storage.destroy();
  }
   
  // Move CTOR
  Function (Function&& other) {
    moveFrom(std::move(other));
  }
  
  template <typename Functor>
  Function (Functor&& f) {
    using exactFunctionHolder_t = FunctorHolder<Functor, Result, Arguments...>;
    m_storage.template create<exactFunctionHolder_t>(std::move(f));
    m_isValid = true;
  }

  Function& operator= (Function&& other) {
    if (this != &other) {
      moveFrom(std::move(other));
    }
    return *this;
  }
  
  // Invoke the contained target.
  // Note: Undefined behaviour if no valid target is stored .
  Result operator() (Arguments&&... args) const {
    if (!m_isValid) throw std::bad_function_call{};
    return m_storage.get()(std::forward<Arguments> (args)...);
  }
  
  // Check whether a valid function is stored
  operator bool() const {
    return m_isValid;
  }
  
  // ---------------------------------------------------------------------------
  // Private Functions
  // ---------------------------------------------------------------------------
private:
  void moveFrom(Function&& other) {
    clear();
    if (other.m_isValid) {
      other.m_storage.get().moveInto(m_storage.raw());
      m_isValid = true;
      other.clear();
    }
  }
  
  void clear() {
    if (m_isValid) {
      m_storage.destroy();
      m_isValid = false;
    }
  }
  
};

}
