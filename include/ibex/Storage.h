#pragma once

#include <iostream>
#include <cstddef>
#include <array>

namespace ibex {

// ---------------------------------------------------------------------------
// Storage
// ---------------------------------------------------------------------------


/// @brief Uninitialised storage for one particular type.
/// @tparam T Type that can be stored inside Storage.
template <typename T>
class Storage {
private:
  alignas(T) std::array<std::byte, sizeof(T)> m_storage;

public:
  template<typename... Args>
  void create(Args&&... args) {
    new (&m_storage) T(std::forward<Args>(args)...);
  }

  void destroy() {
    get().~T();
  }

  T& get() {
    return *std::launder(reinterpret_cast<T*>(&m_storage));
  }

  const T& get() const {
    return *std::launder(reinterpret_cast<T const *>(&m_storage));
  }

  void* raw() {
    return &get();
  }
};


// ---------------------------------------------------------------------------
// PolyStorage
// ---------------------------------------------------------------------------


/// @brief Uninitialised storage for polymorphic types. You can either store a
/// @tparam Base Types of this class, or any derived types, can be stored inside PolyStorage.
/// @tparam Size Maximal size of object that can be sotred.
/// @note Do not forget that every class put in here needs a virtual destructor for destroy() to work properly.
template <typename Base, std::size_t Size>
class PolyStorage {
private:
  std::aligned_storage_t<Size> m_storage;

public:
  template<typename... Args>
  void create(Args&&... args) {
    new (&m_storage) Base(std::forward<Args>(args)...);
  }

  // Create a compatible derived class of T in m_storage.
  // Note: you must ensure that calling ~T() will properly destruct U also.
  template<typename Derived, typename... Args>
  void create(Args&&... args) {
    static_assert(sizeof(m_storage) >= sizeof(Derived), "Class must fit into chosen storage size (Size).");
    static_assert(std::is_base_of<Base, Derived>::value, "Class must inherit from chosen base class (Base).");
    new (&m_storage) Derived(std::forward<Args>(args)...);
  }

  void destroy() {
    get().~Base();
  }

  Base& get() {
    return *std::launder(reinterpret_cast<Base*>(&m_storage));
  }

  const Base& get() const {
    return *std::launder(reinterpret_cast<Base const *>(&m_storage));
  }

  void* raw() {
    return &get();
  }
};

}
