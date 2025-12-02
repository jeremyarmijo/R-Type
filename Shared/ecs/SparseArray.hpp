#pragma once
#include <memory>
#include <optional>
#include <utility>
#include <vector>

template <typename Component>
class SparseArray {
 public:
  using value_type = std::optional<Component>;
  using reference_type = value_type&;
  using const_reference_type = const value_type&;
  using container_t = std::vector<value_type>;
  using size_type = typename container_t::size_type;
  using iterator = typename container_t::iterator;
  using const_iterator = typename container_t::const_iterator;

 public:
  SparseArray() = default;
  SparseArray(const SparseArray&) = default;
  SparseArray(SparseArray&&) noexcept = default;
  ~SparseArray() = default;

  SparseArray& operator=(const SparseArray&) = default;
  SparseArray& operator=(SparseArray&&) noexcept = default;

  reference_type operator[](size_t idx) {
    if (idx >= m_data.size()) {
      m_data.resize(idx + 1);
    }
    return m_data[idx];
  }

  const_reference_type operator[](size_t idx) const {
    if (idx >= m_data.size()) {
      static const value_type empty;
      return empty;
    }
    return m_data[idx];
  }

  iterator begin() { return m_data.begin(); }
  const_iterator begin() const { return m_data.begin(); }
  const_iterator cbegin() const { return m_data.cbegin(); }

  iterator end() { return m_data.end(); }
  const_iterator end() const { return m_data.end(); }
  const_iterator cend() const { return m_data.cend(); }

  size_type size() const { return m_data.size(); }

  reference_type insert_at(size_type pos, const Component& c) {
    if (pos >= m_data.size()) {
      m_data.resize(pos + 1);
    }
    m_data[pos] = c;
    return m_data[pos];
  }

  reference_type insert_at(size_type pos, Component&& c) {
    if (pos >= m_data.size()) {
      m_data.resize(pos + 1);
    }
    m_data[pos] = std::move(c);
    return m_data[pos];
  }

  template <class... Params>
  reference_type emplace_at(size_type pos, Params&&... params) {
    if (pos >= m_data.size()) {
      m_data.resize(pos + 1);
    }

    if (m_data[pos].has_value()) {
      m_data[pos].reset();
    }

    m_data[pos].emplace(std::forward<Params>(params)...);
    return m_data[pos];
  }

  void erase(size_type pos) {
    if (pos < m_data.size()) {
      m_data[pos].reset();
    }
  }

  size_type get_index(const value_type& val) const {
    if (!val.has_value()) {
      return static_cast<size_type>(-1);
    }

    for (size_type i = 0; i < m_data.size(); ++i) {
      if (std::addressof(m_data[i]) == std::addressof(val)) {
        return i;
      }
    }
    return static_cast<size_type>(-1);
  }

 private:
  container_t m_data;
};
