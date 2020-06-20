#pragma once

#include <array>
#include <cstddef>

namespace ibex {

// ---------------------------------------------------------------------------
// Storage
// ---------------------------------------------------------------------------

///
/// @brief      Uninitialised storage for one particular type.
///             This is mainly useful as a building block for containers.
///
/// @tparam     T     Type that can be stored inside Storage.
///
template <typename T>
class Storage {
 private:
  alignas(T) std::array<std::byte, sizeof(T)> m_storage;

 public:
  template <typename... Args>
  void create(Args&&... args) {
    new (&m_storage) T(std::forward<Args>(args)...);
  }

  ///
  /// @brief      Call destructor on contained type.
  /// 
  void destroy() { get().~T(); }

  /// Returns 
  ///
  /// @return     A reference to the stored value.
  /// 
  /// @note       Calling this function before having actually constructed an
  ///             element by calling 'create()' is undefined behaviour.
  ///
  T& get() { return *std::launder(reinterpret_cast<T*>(&m_storage)); }

  /// Returns 
  ///
  /// @return     Base-class reference to stored value.
  /// 
  /// @note       Calling this function before having actually constructed an
  ///             element by calling 'create()' is undefined behaviour.
  ///
  const T& get() const {
    return *std::launder(reinterpret_cast<T const*>(&m_storage));
  }

  ///
  /// @brief      Access the raw memory of the contained element
  ///
  /// @return     Erased pointer to stored element.
  ///
  void* raw() { return &get(); }
};

// ---------------------------------------------------------------------------
// ErasedStorage
// ---------------------------------------------------------------------------

///
/// @brief      Uninitialised storage for polymorphic types.
///             This is mainly useful type erasure. 
///
/// @tparam     Base  Types of this class, or any derived types, can be stored
///                   inside ErasedStorage.
/// @tparam     Size  Maximal size of object that can be sorted.
/// @note       Do not forget that every class put in here needs a virtual
///             destructor for destroy() to work properly.
///
template <typename Base, std::size_t Size>
class ErasedStorage {
 private:
  std::aligned_storage_t<Size> m_storage; // For certain compilers this will be min. 16 bytes. :(

 public:


  ///
  /// @brief      Constructs an object within its storage.
  ///
  /// @param      args  Arguments passed to the constructor.
  ///
  template <typename... Args>
  void create(Args&&... args) {
    new (&m_storage) Base(std::forward<Args>(args)...);
  }

  /// Create a compatible derived class of T in m_storage. 
  ///
  /// @param      args     Arguments passed to the constructor.
  ///
  /// @tparam     Derived  Type of element to construct.
  ///
  template <typename Derived, typename... Args>
  void create(Args&&... args) {
    static_assert(sizeof(m_storage) >= sizeof(Derived),
                  "Class must fit into chosen storage size (Size).");
    static_assert(std::is_base_of<Base, Derived>::value,
                  "Class must inherit from chosen base class (Base).");

    new (&m_storage) Derived(std::forward<Args>(args)...);
  }

  ///
  /// @brief      Call destructor on contained type.
  /// 
  void destroy() { get().~Base(); }

  /// Returns 
  ///
  /// @return     Base-class reference to stored value.
  /// 
  /// @note       Calling this function before having actually constructed an
  ///             element by calling 'create()' is undefined behaviour.
  ///
  Base& get() { return *std::launder(reinterpret_cast<Base*>(&m_storage)); }

  /// Returns 
  ///
  /// @return     A Base-class reference to the stored value.
  /// 
  /// @note       Calling this function before having actually constructed an
  ///             element by calling 'create()' is undefined behaviour.
  ///
  const Base& get() const {
    return *std::launder(reinterpret_cast<Base const*>(&m_storage));
  }

  ///
  /// @brief      Access the raw memory of the contained element
  ///
  /// @return     Erased pointer to stored element.
  ///
  void* raw() { return &get(); }
};

}  // namespace ibex
