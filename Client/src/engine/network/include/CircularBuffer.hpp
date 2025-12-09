#pragma once
#include <optional>
#include <vector>

template <typename T>
class CircularBuffer {
 private:
  std::vector<T> m_buffer;
  size_t m_maxItem;
  size_t m_lastItemIndex = 0;
  size_t m_firstItemIndex = 0;
  size_t m_nbItem = 0;

 public:
  explicit CircularBuffer(size_t capacity)
      : m_maxItem(capacity), m_buffer(capacity) {}

  size_t size() { return m_nbItem; }

  bool isEmpty() { return m_nbItem == 0; }

  bool isFull() { return m_nbItem == m_maxItem; }

  void push(const T &item) {
    m_buffer[m_lastItemIndex] = item;
    m_lastItemIndex++;
    if (m_lastItemIndex == m_maxItem) {
      m_lastItemIndex = 0;
    }

    if (m_nbItem < m_maxItem) {
      m_nbItem++;
    } else {
      m_firstItemIndex++;
      if (m_firstItemIndex == m_maxItem) {
        m_firstItemIndex = 0;
      }
    }
  }

  std::optional<T> pop() {
    if (isEmpty()) {
      return std::nullopt;
    }

    T item = m_buffer[m_firstItemIndex];
    m_firstItemIndex++;
    if (m_firstItemIndex == m_maxItem) {
      m_firstItemIndex = 0;
    }
    m_nbItem--;
    return item;
  }

  std::optional<T> peek() {
    if (isEmpty()) {
      return std::nullopt;
    }
    return m_buffer[m_firstItemIndex];
  }
};
