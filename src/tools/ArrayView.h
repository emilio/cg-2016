#pragma once

template <typename T>
class ArrayView {
  T* m_ptr;
  size_t m_length;

public:
  ArrayView(T* ptr, size_t length) : m_ptr(ptr), m_length(length) {}

  T* begin() {
    return m_ptr;
  }
  T* end() {
    return m_ptr + m_length;
  }

  const T* begin() const {
    return m_ptr;
  }
  const T* end() const {
    return m_ptr + m_length;
  }

  T& operator[](size_t i) {
    assert(i < m_length);
    return m_ptr[m_length];
  }

  const T& operator[](size_t i) const {
    assert(i < m_length);
    return m_ptr[m_length];
  }
};

template <typename T>
ArrayView<T> View(T* ptr, size_t len) {
  return ArrayView<T>(ptr, len);
}

template <typename T>
const ArrayView<T> View(const T* ptr, size_t len) {
  return ArrayView<T>(const_cast<T*>(ptr), len);
}
