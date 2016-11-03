#ifndef Optional_h
#define Optional_h

#include <cassert>
#include <memory>

enum { None };

template <typename T>
class Optional {
  union {
    T m_value;
  };
  bool m_isSome;

public:
  Optional() : Optional(None) {}

  Optional(decltype(None)) : m_isSome(false) {}

  Optional(Optional&& a_other) {
    m_isSome = a_other.isSome();
    if (m_isSome)
      value() = std::move(a_other.value());
    a_other.m_isSome = false;
  }

  Optional& operator=(Optional&& a_other) {
    clear();
    m_isSome = a_other.isSome();
    if (m_isSome)
      value() = std::move(a_other.value());
    a_other.m_isSome = false;
    return *this;
  }

  bool isSome() const {
    return m_isSome;
  }

  bool isNone() const {
    return !isSome();
  }

  explicit operator bool() const {
    return isSome();
  }

  T& value() {
    assert(isSome());
    return m_value;
  }

  const T& value() const {
    return const_cast<Optional*>(this)->value();
  }

  T& operator*() {
    return value();
  }

  const T& operator*() const {
    return value();
  }

  template <typename... Args>
  void set(Args&&... aArgs) {
    m_isSome = true;
    new (&m_value) T(std::forward<Args>(aArgs)...);
  }

  void clear() {
    if (isSome()) {
      m_value.T::~T();
      m_isSome = false;
    }
  }

  ~Optional() {
    clear();
  }
};

#endif
