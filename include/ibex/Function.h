#pragma once

#include <ibex/Storage.h>

#include <functional>

namespace ibex {


template <typename, size_t>
class Function;

///
/// @brief      This class stores and invokes any callable target.
///             It is similar to std::function but differs in two key aspects:
///             - Size of the target is fixed, no heap allocation will take place.
///             - This is a move-only class. This has the advantage that you can
///
/// @tparam     Size       Maximal target size in bytes
/// @tparam     Result     Target return type
/// @tparam     Arguments  Target argument types
///
template <std::size_t Size, typename Result, typename... Arguments>
class Function<Result(Arguments...), Size> final {
 private:
  // ---------------------------------------------------------------------------
  // Types
  // ---------------------------------------------------------------------------
  struct ErasedFunctorHolder {
    virtual ~ErasedFunctorHolder() {}
    virtual Result operator()(Arguments...) = 0;
    virtual void moveInto(void*) = 0;
  };

  template <typename Functor>
  struct FunctorHolder final : ErasedFunctorHolder {
    // Types
    using functor_t = std::remove_reference_t<Functor>;
    
    // Members
    functor_t f;

    // Functions
    FunctorHolder(functor_t&& func) : f(std::move(func)) {}
    FunctorHolder(const functor_t& func) : f(func) {}

    ~FunctorHolder() override = default;

    Result operator()(Arguments... args) override {
      return f(std::forward<Arguments>(args)...);
    }

    void moveInto(void* destination) override {
      new (destination) FunctorHolder(std::move(f));
    }
  };

  using holder_t = ErasedFunctorHolder;

  // ---------------------------------------------------------------------------
  // Members
  // ---------------------------------------------------------------------------
  mutable PolyStorage<holder_t, Size> m_storage;
  bool m_isValid{false};

  // ---------------------------------------------------------------------------
  // Public Functions
  // ---------------------------------------------------------------------------
 public:
  // Create an empty function
  Function() = default;

  ~Function() {
    if (m_isValid) m_storage.destroy();
  }

  // Construct a Function from a movable or copyable callable
  template <typename Functor>
  Function(Functor&& f) : m_isValid{true} {
    using exactFunctionHolder_t = FunctorHolder<Functor>;
    m_storage.template create<exactFunctionHolder_t>(std::forward<Functor>(f));
  }

  // Move construct from other Function
  Function(Function&& other) { moveFrom(std::move(other)); }

  // Move assignment from other Function
  Function& operator=(Function&& other) {
    if (this != &other) {
      moveFrom(std::move(other));
    }
    return *this;
  }

  // Invoke the contained target. Throws if no valid target has been stored.
  Result operator()(Arguments... args) const {
    if (!m_isValid) throw std::bad_function_call{};
    return m_storage.get()(std::forward<Arguments>(args)...);
  }

  // Check whether a valid function is stored.
  operator bool() const { return m_isValid; }

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

}  // namespace ibex
