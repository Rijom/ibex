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
/// @tparam     R          Target return type
/// @tparam     Args       Target argument types
///
template <std::size_t Size, typename R, typename... Args>
class Function<R(Args...), Size> final {
 private:
  // ---------------------------------------------------------------------------
  // Child Classes: Target Wrappers
  // ---------------------------------------------------------------------------

  // Type erased target functor
  struct ErasedTarget {
    virtual ~ErasedTarget() {}
    virtual R operator()(Args...) = 0;
    virtual void moveInto(void*) = 0;
  };

  // Holds a non-type-erased target
  template <typename Functor>
  struct Target final : ErasedTarget {
    using functor_t = std::remove_reference_t<Functor>;
    functor_t f;

    Target(functor_t&& func) : f(std::move(func)) {}
    Target(const functor_t& func) : f(func) {}
    ~Target() override = default;

    // Call contained target
    R operator()(Args... args) override {
      return f(std::forward<Args>(args)...);
    }

    // Move contained target to a different memory location
    void moveInto(void* destination) override {
      new (destination) Target(std::move(f));
    }
  };

  // ---------------------------------------------------------------------------
  // Members
  // ---------------------------------------------------------------------------
  mutable ErasedStorage<ErasedTarget, Size> m_storage;
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
    m_storage.template create<Target<Functor>>(std::forward<Functor>(f));
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
  R operator()(Args... args) const {
    if (!m_isValid) throw std::bad_function_call{};
    return m_storage.get()(std::forward<Args>(args)...);
  }

  // Check whether a valid function is stored.
  operator bool() const { return m_isValid; }

  // ---------------------------------------------------------------------------
  // Private Functions
  // ---------------------------------------------------------------------------
 private:
  // Steal contents from other function
  void moveFrom(Function&& other) {
    clear();
    if (other.m_isValid) {
      other.m_storage.get().moveInto(m_storage.raw());
      m_isValid = true;
      other.clear();
    }
  }

  // Cleanly destroy contained target
  void clear() {
    if (m_isValid) {
      m_storage.destroy();
      m_isValid = false;
    }
  }
};

}  // namespace ibex
